#include "main.h"
#include "hide.h"
#include "update.h"
#include "jump.h"
#include "pipe.h"
#include "target.h"
#include "sensor.h"
#include "save.h"
#include "fmv.h"
#include "sound.h"
#include "tv.h"
#include "mods.h"
#include "perf.h"
#include "qol.h"
#include "menustyle.h"
#include "mouseinput.h"
#include "prompt.h"
#include "ui.h"
#include "resource1.h"
#include "runtime_error.h"
#include "emitter.h"
#include "blip.h"
#include "../platform/platform.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <exception>
#include <filesystem>
#include <iostream>
#include <thread>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

bool g_fDeveloperDebugMode = false;

namespace
{
void LoadDebugModeSetting()
{
	g_fDeveloperDebugMode = cane::DebugMode();
}

// Retail Timing paces at the PS2's 59.94 Hz. With VSync on a 60 Hz display,
// pacing at 59.94 would drift against the vblanks and periodically hold a frame
// for two refreshes (a double-length game step), so pace at the display's 60 Hz
// there instead; every step is still exactly one PS2 field (clock.cpp).
double PacerRateHz()
{
    if (g_fRetailTiming && g_fVsync)
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = monitor != nullptr ? glfwGetVideoMode(monitor) : nullptr;
        if (mode != nullptr && mode->refreshRate >= 59 && mode->refreshRate <= 61)
            return 60.0;
    }
    return EffectiveFrameRateHz();
}

class FramePacer
{
public:
    using Clock = std::chrono::steady_clock;

    FramePacer()
        : m_deadline(Clock::now())
    {
#ifdef _WIN32
        // CREATE_WAITABLE_TIMER_HIGH_RESOLUTION is 0x2. Use the value directly
        // so this still builds with Windows SDKs that predate the named flag.
        m_timer = CreateWaitableTimerExW(
            nullptr, nullptr, 0x2, TIMER_MODIFY_STATE | SYNCHRONIZE);

        if (m_timer == nullptr)
            m_timer = CreateWaitableTimerExW(
                nullptr, nullptr, 0, TIMER_MODIFY_STATE | SYNCHRONIZE);
#endif
    }

    ~FramePacer()
    {
#ifdef _WIN32
        if (m_timer != nullptr)
            CloseHandle(m_timer);
#endif
    }

    void Wait()
    {
        constexpr auto kSpinMargin = std::chrono::microseconds(500);

        // Retail Timing paces at the PS2's 59.94 Hz. "Uncapped" (0) still stops
        // at 1000 FPS: the minimum update step is 1/1000 s, and going faster
        // would speed the game up.
        const double target = PacerRateHz();
        if (m_frameRate != target)
        {
            m_frameRate = target;
            m_deadline = Clock::now();
        }

        const auto frameDuration = std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(1.0 / m_frameRate));

        m_deadline += frameDuration;
        auto now = Clock::now();

        if (now >= m_deadline)
        {
            // Drop missed presentation deadlines. The next call schedules one
            // complete interval from this presentation rather than catching up.
            m_deadline = now;
            return;
        }

        const auto coarseDeadline = m_deadline - kSpinMargin;
        if (now < coarseDeadline)
        {
#ifdef _WIN32
            if (m_timer != nullptr)
            {
                const auto wait100ns = std::chrono::duration_cast<
                    std::chrono::duration<LONGLONG, std::ratio<1, 10000000>>>(
                        coarseDeadline - now).count();

                LARGE_INTEGER dueTime{};
                dueTime.QuadPart = -(std::max<LONGLONG>)(1, wait100ns);

                if (SetWaitableTimer(m_timer, &dueTime, 0, nullptr, nullptr, FALSE))
                    WaitForSingleObject(m_timer, INFINITE);
                else
                    std::this_thread::sleep_until(coarseDeadline);
            }
            else
            {
                std::this_thread::sleep_until(coarseDeadline);
            }
#else
			std::this_thread::sleep_until(coarseDeadline);
#endif
        }

        // YieldProcessor is a CPU pause hint, not a scheduler yield. It keeps
        // the final sub-millisecond boundary precise without giving up a full
        // Windows scheduling quantum.
        while (Clock::now() < m_deadline)
#ifdef _WIN32
            YieldProcessor();
#else
			std::this_thread::yield();
#endif
    }

    void Resync()
    {
        m_frameRate = PacerRateHz();
        m_deadline = Clock::now();
    }

private:
#ifdef _WIN32
    HANDLE m_timer = nullptr;
#endif
    Clock::time_point m_deadline;
    double m_frameRate = 60.0;
};

FramePacer* s_framePacer = nullptr;
bool s_runningFrame = false;

void FlushTransitionFrame()
{
    // The previous game frame has already been presented. Drain outstanding
    // GPU work and leave that front-buffer image visible while the synchronous
    // world transition runs. This is the original lightweight desktop path:
    // no extra framebuffer allocation, copy, redraw, or swap.
    glFinish();
}

void RunGameFrame()
{
    if (s_runningFrame)
        return;

    s_runningFrame = true;
    PerfFrameStart();

    // Retail leaves the movie queued while the outgoing wipe is visible.
    // Once the screen reaches black, play the movie before executing the
    // transition that replaces the current world.
    if (FCutscenePending() && g_wipe.wipes != WIPES_WipingOut)
    {
        ExecutePendingCutscenes();
        ResetDlssHistory();
        ResetFsrHistory();
    }

    if (g_transition.m_fPending != 0)
    {
        FlushTransitionFrame();
        g_transition.Execute(file);
        ResetDlssHistory();
        ResetFsrHistory();
        OnModsWorldLoaded();
        // Retail does not resync the game clock after a load: the first frame
        // of the new world simply takes the maximum (two-frame) step. Only the
        // presentation pacer is resynchronized.
        if (s_framePacer != nullptr)
            s_framePacer->Resync();
    }

    if (FCutscenePending() && g_wipe.wipes != WIPES_WipingOut)
    {
        ExecutePendingCutscenes();
        ResetDlssHistory();
        ResetFsrHistory();
    }

    static bool s_fF3WasDown = false;
	const bool fF3Down = g_fDeveloperDebugMode && g_gl.window != nullptr &&
        glfwGetKey(g_gl.window, GLFW_KEY_F3) == GLFW_PRESS;
    if (fF3Down && !s_fF3WasDown)
    {
        g_fDebugGuiOpen = !g_fDebugGuiOpen;

        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(g_gl.window, &framebufferWidth, &framebufferHeight);
        FrameBufferSizeCallBack(g_gl.window, framebufferWidth, framebufferHeight);
    }
    s_fF3WasDown = fF3Down;

    // DLSS replaces MSAA (it is the anti-aliaser) and needs the resolved
    // scene color/depth textures, so it always renders into the plain FBO.
    // Switches "Fullscreen" between exclusive and borderless when DLSS Frame
    // Generation turns on/off (no-op otherwise).
    ApplyWindowModeSettings(g_windowMode);

    const bool fDlss = FDlssActive();
    // Temporal FSR (FSR 4 / 3.1) is also the anti-aliaser and needs the plain
    // FBO's depth texture, like DLSS. FSR 1 keeps MSAA.
    const bool fMsaaThisFrame = g_fMsaa && !fDlss && !FFsrTemporal() && g_gl.fboMSAA != 0;
    g_sceneFbo = fMsaaThisFrame ? g_gl.fboMSAA : g_gl.fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);

    // Pick this frame's sub-pixel jitter before any camera is uploaded.
    BeginDlssFrame(fDlss && g_psw != nullptr, g_gl.renderWidth, g_gl.renderHeight,
        g_gl.presentWidth, g_gl.presentHeight);
    bool fUpscaled = false;
    bool fFsrUpscaled = false;
    const bool fFsr = FFsrActive();
    // Temporal FSR picks its own sub-pixel jitter (after BeginDlssFrame).
    BeginFsrFrame(fFsr && g_psw != nullptr, g_gl.renderWidth, g_gl.renderHeight,
        g_gl.presentWidth, g_gl.presentHeight);

    glViewport(0, 0, g_gl.renderWidth, g_gl.renderHeight);
    glClearColor(rgbaSky.r, rgbaSky.g, rgbaSky.b, rgbaSky.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    UpdateMouseInput(g_gl.window);
    g_joy.Update(g_gl.window);
    // Mods: hotkeys, controller combos (may hide combo buttons from the game).
    UpdateModsInput();
    UpdateQolInput();
    UpdateQolFocus();
    UpdateModsTimer();
    RenderMenuGui(g_psw);

    if (g_psw != nullptr)
    {
        // Fast forward runs several normal-length game updates per rendered
        // frame (large single steps would break collision/physics).
        const int cUpdates = CModsGameUpdates();
        for (int iUpdate = 0; iUpdate < cUpdates; ++iUpdate)
        {
            if (iUpdate > 0)
            {
                // Extra steps must not see button presses a second time.
                if (g_transition.m_fPending != 0)
                    break;
                g_joy.previous = g_joy.current;
            }

            UpdateUi(&g_ui);
            UpdateGameState(g_clock.dt);

            SetupCm(g_pcm);
            if (iUpdate == 0)
                OpenFrame();
            MarkClockTick(&g_clock);
            UpdateCodes();
            UpdateSw(g_psw, g_clock.dt);
        }

        DlssMarkRenderStart();
        PerfZone(PERFZ_Render);

        if (g_fRenderModels == true)
        {
            g_worldMapDrawnCount = 0;
            RenderSw(g_psw, g_pcm);
            RenderUi(&g_ui);
            DrawSw(g_psw, g_pcm);
        }

        if (g_fRenderCollision == true)
		{
            DrawSwCollisionAll(g_pcm);
		}

		if (g_fRenderDialogTriggers == true)
			DrawSwDialogTriggers(g_pcm);

        PerfGpuSceneEnd();
        PerfZone(PERFZ_Post);

        if (fDlss && g_pcm != nullptr)
        {
            DLSSCAMERA cam;
            cam.matWorldToClip = g_pcm->matWorldToClip;
            cam.matProj = g_pcm->matProj;
            cam.pos = g_pcm->pos;
            cam.radFov = g_pcm->radFOV;
            cam.sNearClip = g_pcm->sRenderNearClip;
            cam.sFarClip = g_pcm->sFarClip;

            // On success the upscaled output (present resolution) is now
            // bound, so the HUD below is drawn at full resolution on top of it.
            fUpscaled = FEvaluateDlss(cam, g_gl.fbc, g_gl.fbd);
            if (fUpscaled)
            {
                g_uiTargetWidth = g_gl.presentWidth;
                g_uiTargetHeight = g_gl.presentHeight;
            }
        }

        // AMD FSR 1: needs the resolved scene, then leaves its full-resolution
        // output bound so the HUD below is drawn on top of it.
        if (fFsr && !fUpscaled && g_pcm != nullptr)
        {
            if (fMsaaThisFrame)
            {
                // Color for FSR, depth/stencil for the HUD drawn on its output.
                glBindFramebuffer(GL_READ_FRAMEBUFFER, g_gl.fboMSAA);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_gl.fbo);
                const GLboolean fScissor = glIsEnabled(GL_SCISSOR_TEST);
                const GLboolean fDepthWrite = g_glDepthMask;
                glDisable(GL_SCISSOR_TEST);
                GlDepthMask(GL_TRUE);
                glStencilMask(0xFF);
                glBlitFramebuffer(0, 0, g_gl.renderWidth, g_gl.renderHeight,
                    0, 0, g_gl.renderWidth, g_gl.renderHeight,
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
                GlDepthMask(fDepthWrite);
                if (fScissor)
                    glEnable(GL_SCISSOR_TEST);
            }
            DLSSCAMERA fsrCam;
            fsrCam.matWorldToClip = g_pcm->matWorldToClip;
            fsrCam.matProj = g_pcm->matProj;
            fsrCam.pos = g_pcm->pos;
            fsrCam.radFov = g_pcm->radFOV;
            fsrCam.sNearClip = g_pcm->sRenderNearClip;
            fsrCam.sFarClip = g_pcm->sFarClip;
            fFsrUpscaled = FEvaluateFsr(fsrCam, g_gl.fbc, g_gl.fbd, g_gl.renderWidth, g_gl.renderHeight,
                g_gl.presentWidth, g_gl.presentHeight);
            if (fFsrUpscaled)
            {
                g_uiTargetWidth = g_gl.presentWidth;
                g_uiTargetHeight = g_gl.presentHeight;
            }
            else
            {
                glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);
                glViewport(0, 0, g_gl.renderWidth, g_gl.renderHeight);
            }
        }

        // Shift+F12: screenshot of the 3D image without the HUD.
        QolCaptureBeforeHud((fUpscaled || fFsrUpscaled) ? g_gl.presentWidth : g_gl.renderWidth,
            (fUpscaled || fFsrUpscaled) ? g_gl.presentHeight : g_gl.renderHeight);
        DrawUi(&g_ui);

        g_uiTargetWidth = 0;
        g_uiTargetHeight = 0;
    }

    if (fMsaaThisFrame && !fFsrUpscaled)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, g_gl.fboMSAA);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_gl.fbo);
        glBlitFramebuffer(0, 0, g_gl.renderWidth, g_gl.renderHeight,
            0, 0, g_gl.renderWidth, g_gl.renderHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    // With DLSS Frame Generation the final frame goes to an off-screen target
    // that is presented through DXGI; otherwise it is the window itself.
    int windowFbWidth = 0;
    int windowFbHeight = 0;
    glfwGetFramebufferSize(g_gl.window, &windowFbWidth, &windowFbHeight);
    const GLuint presentFbo = DlssBeginPresent(windowFbWidth, windowFbHeight,
        g_gl.presentX, g_gl.presentY, g_gl.presentWidth, g_gl.presentHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, presentFbo);
    glViewport(g_gl.presentX, g_gl.presentY, g_gl.presentWidth, g_gl.presentHeight);

	// Presentation is a hard OpenGL state boundary. World, shadow, particle,
	// and UI passes are allowed to use these states internally, but none of
	// them may influence the default-framebuffer clear or fullscreen composite.
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	GlDepthMask(GL_TRUE);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glScreenShader.Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fUpscaled ? DlssOutputTexture() :
        (fFsrUpscaled ? FsrOutputTexture() : g_gl.fbc));
    glBindVertexArray(g_gl.sao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // F12: screenshot of the final image (with HUD, before our overlays).
    QolCaptureFinal(presentFbo, g_gl.presentX, g_gl.presentY, g_gl.presentWidth, g_gl.presentHeight);

    DrawModsOverlay();
    DrawQolOverlay();
    DrawDlssOverlay();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Frame generation pauses itself in menus and when no level is loaded.
    const bool fGameplay = g_psw != nullptr && g_ui.uis == UIS_Playing &&
        g_prompt.blots == BLOTS_Hidden;
    PerfZone(PERFZ_Present);
    PerfGpuFrameEnd();
    if (!FDlssPresentFrame(fGameplay))
        glfwSwapBuffers(g_gl.window);
    PerfZone(PERFZ_Wait);
    if (s_framePacer != nullptr)
        s_framePacer->Wait();

    ++g_cframe;
    s_runningFrame = false;
}

#ifdef _WIN32
WNDPROC s_glfwWindowProc = nullptr;
constexpr UINT kRunTrackedWindowFrame = WM_APP + 0x504;
std::atomic_bool s_windowTracking = false;
std::atomic_bool s_windowFrameThreadRunning = false;
HWND s_gameWindow = nullptr;
std::thread s_windowFrameThread;
int s_cutsceneNcDrag = HTNOWHERE;
POINT s_cutsceneNcDragStart{};
RECT s_cutsceneNcWindowStart{};

bool BeginCutsceneNonClientDrag(HWND hwnd, int hitTest)
{
	switch (hitTest)
	{
	case HTCAPTION:
	case HTLEFT:
	case HTRIGHT:
	case HTTOP:
	case HTTOPLEFT:
	case HTTOPRIGHT:
	case HTBOTTOM:
	case HTBOTTOMLEFT:
	case HTBOTTOMRIGHT:
		break;
	default:
		return false;
	}

	s_cutsceneNcDrag = hitTest;
	GetCursorPos(&s_cutsceneNcDragStart);
	GetWindowRect(hwnd, &s_cutsceneNcWindowStart);
	SetCapture(hwnd);
	return true;
}

void UpdateCutsceneNonClientDrag(HWND hwnd)
{
	if (s_cutsceneNcDrag == HTNOWHERE)
		return;

	POINT cursor{};
	GetCursorPos(&cursor);
	const int dx = cursor.x - s_cutsceneNcDragStart.x;
	const int dy = cursor.y - s_cutsceneNcDragStart.y;
	RECT rect = s_cutsceneNcWindowStart;

	if (s_cutsceneNcDrag == HTCAPTION)
	{
		OffsetRect(&rect, dx, dy);
	}
	else
	{
		if (s_cutsceneNcDrag == HTLEFT || s_cutsceneNcDrag == HTTOPLEFT ||
			s_cutsceneNcDrag == HTBOTTOMLEFT)
			rect.left += dx;
		if (s_cutsceneNcDrag == HTRIGHT || s_cutsceneNcDrag == HTTOPRIGHT ||
			s_cutsceneNcDrag == HTBOTTOMRIGHT)
			rect.right += dx;
		if (s_cutsceneNcDrag == HTTOP || s_cutsceneNcDrag == HTTOPLEFT ||
			s_cutsceneNcDrag == HTTOPRIGHT)
			rect.top += dy;
		if (s_cutsceneNcDrag == HTBOTTOM || s_cutsceneNcDrag == HTBOTTOMLEFT ||
			s_cutsceneNcDrag == HTBOTTOMRIGHT)
			rect.bottom += dy;

		constexpr int minimumWidth = 320;
		constexpr int minimumHeight = 240;
		if (rect.right - rect.left < minimumWidth)
		{
			if (s_cutsceneNcDrag == HTLEFT || s_cutsceneNcDrag == HTTOPLEFT ||
				s_cutsceneNcDrag == HTBOTTOMLEFT)
				rect.left = rect.right - minimumWidth;
			else
				rect.right = rect.left + minimumWidth;
		}
		if (rect.bottom - rect.top < minimumHeight)
		{
			if (s_cutsceneNcDrag == HTTOP || s_cutsceneNcDrag == HTTOPLEFT ||
				s_cutsceneNcDrag == HTTOPRIGHT)
				rect.top = rect.bottom - minimumHeight;
			else
				rect.bottom = rect.top + minimumHeight;
		}
	}

	SetWindowPos(hwnd, nullptr, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_NOACTIVATE | SWP_NOZORDER);
}

void EndCutsceneNonClientDrag(HWND hwnd)
{
	if (s_cutsceneNcDrag == HTNOWHERE)
		return;
	s_cutsceneNcDrag = HTNOWHERE;
	if (GetCapture() == hwnd)
		ReleaseCapture();
}

void WindowFrameThreadMain()
{
	while (s_windowFrameThreadRunning.load(std::memory_order_acquire))
	{
		if (s_windowTracking.load(std::memory_order_acquire) && s_gameWindow != nullptr)
		{
			DWORD_PTR result = 0;
			SendMessageTimeoutW(s_gameWindow, kRunTrackedWindowFrame, 0, 0,
				SMTO_ABORTIFHUNG | SMTO_BLOCK, 50, &result);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}

LRESULT CALLBACK LiveResizeWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// DefWindowProc enters a modal tracking loop for caption dragging, resizing,
	// caption buttons, and the system menu. A movie is driven by the main thread,
	// so that loop freezes its video while XAudio continues independently. During
	// an FMV, handle these interactions without entering the modal loop.
	if (FCutscenePlaying())
	{
		if (message == WM_NCLBUTTONDOWN)
		{
			const int hitTest = static_cast<int>(wParam);
			if (BeginCutsceneNonClientDrag(hwnd, hitTest))
				return 0;
			if (hitTest == HTMINBUTTON)
			{
				ShowWindow(hwnd, SW_MINIMIZE);
				return 0;
			}
			if (hitTest == HTMAXBUTTON)
			{
				ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
				return 0;
			}
			if (hitTest == HTCLOSE)
			{
				PostMessageW(hwnd, WM_CLOSE, 0, 0);
				return 0;
			}
			// Do not pass any remaining caption presses (for example the window
			// icon) to DefWindowProc while an FMV is active; they can also start a
			// modal system-menu loop.
			return 0;
		}
		else if (message == WM_NCLBUTTONDBLCLK && wParam == HTCAPTION)
		{
			ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
			return 0;
		}
		else if (message == WM_NCRBUTTONDOWN || message == WM_NCRBUTTONUP ||
			message == WM_NCMBUTTONDOWN || message == WM_NCMBUTTONUP ||
			message == WM_NCXBUTTONDOWN || message == WM_NCXBUTTONUP ||
			message == WM_CONTEXTMENU)
		{
			// The native system menu is modal too. Suppress it only while a movie
			// is active so right-clicking or holding the caption cannot stop video.
			return 0;
		}
		else if (message == WM_MOUSEMOVE && s_cutsceneNcDrag != HTNOWHERE)
		{
			UpdateCutsceneNonClientDrag(hwnd);
			return 0;
		}
		else if (message == WM_LBUTTONUP && s_cutsceneNcDrag != HTNOWHERE)
		{
			EndCutsceneNonClientDrag(hwnd);
			return 0;
		}
		else if (message == WM_CAPTURECHANGED && s_cutsceneNcDrag != HTNOWHERE)
		{
			s_cutsceneNcDrag = HTNOWHERE;
		}
		else if (message == WM_CANCELMODE && s_cutsceneNcDrag != HTNOWHERE)
		{
			EndCutsceneNonClientDrag(hwnd);
			return 0;
		}
	}
	else if (s_cutsceneNcDrag != HTNOWHERE)
	{
		EndCutsceneNonClientDrag(hwnd);
	}

	// Any mouse hold in the non-client area can enter a private Windows tracking
	// loop before GLFW receives another event. Start pumping immediately instead
	// of waiting for WM_ENTERSIZEMOVE or WM_ENTERMENULOOP. This covers caption
	// buttons, title-bar dragging, and holding right-click before the system menu
	// is shown, while still letting GLFW's original procedure own the behavior.
	if (message == WM_NCLBUTTONDOWN ||
		message == WM_NCRBUTTONDOWN ||
		message == WM_NCMBUTTONDOWN)
	{
		s_windowTracking.store(true, std::memory_order_release);
		const LRESULT result = CallWindowProcW(s_glfwWindowProc, hwnd, message, wParam, lParam);
		s_windowTracking.store(false, std::memory_order_release);
		return result;
	}

    switch (message)
    {
        case WM_ENTERSIZEMOVE:
		case WM_ENTERMENULOOP:
		s_windowTracking.store(true, std::memory_order_release);
		break;

        case kRunTrackedWindowFrame:
        if (s_windowTracking.load(std::memory_order_acquire))
        {
            RunGameFrame();
            return 0;
        }
        break;

        case WM_EXITSIZEMOVE:
		case WM_EXITMENULOOP:
		s_windowTracking.store(false, std::memory_order_release);
		break;

        case WM_DESTROY:
		s_windowTracking.store(false, std::memory_order_release);
        break;
    }

    return CallWindowProcW(s_glfwWindowProc, hwnd, message, wParam, lParam);
}

void InstallLiveResizeWindowProc()
{
    HWND hwnd = glfwGetWin32Window(g_gl.window);
    if (hwnd == nullptr)
        return;

	HINSTANCE instance = GetModuleHandleW(nullptr);
	HICON largeIcon = static_cast<HICON>(LoadImageW(instance,
		MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
		GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR));
	HICON smallIcon = static_cast<HICON>(LoadImageW(instance,
		MAKEINTRESOURCEW(IDI_APP_ICON), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));
	if (largeIcon != nullptr)
		SendMessageW(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(largeIcon));
	if (smallIcon != nullptr)
		SendMessageW(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(smallIcon));

    s_glfwWindowProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(LiveResizeWindowProc));
	s_gameWindow = hwnd;
	s_windowFrameThreadRunning.store(true, std::memory_order_release);
	s_windowFrameThread = std::thread(WindowFrameThreadMain);
}

void ShutdownLiveResizeWindowProc()
{
	s_windowTracking.store(false, std::memory_order_release);
	s_windowFrameThreadRunning.store(false, std::memory_order_release);
	if (s_windowFrameThread.joinable())
		s_windowFrameThread.join();
}
#endif
}

int RunApplication(const std::vector<std::string>& arguments)
{
    InstallRuntimeErrorHandlers();
    try
    {
		cane::ConfigureRuntime(arguments);
        Startup();
		#ifdef _WIN32
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
		#endif
		FramePacer framePacer;
		s_framePacer = &framePacer;
#ifdef _WIN32
		InstallLiveResizeWindowProc();
#endif

		while (!glfwWindowShouldClose(g_gl.window) && fQuitGame != true)
		{
			RunGameFrame();
			glfwPollEvents();

		}
		s_framePacer = nullptr;

#ifdef _WIN32
		ShutdownLiveResizeWindowProc();
#endif

		if (g_psw != nullptr)
			DeleteWorld(g_psw);

		ShutdownSound();
		FreeTvGL();
		ShutdownFsr();
		ShutdownDlss();
		g_gl.TerminateGL();
		return EXIT_SUCCESS;
	}
	catch (const std::exception& exception)
	{
		ReportFatalRuntimeError(exception.what());
		return EXIT_FAILURE;
	}
	catch (...)
	{
		ReportFatalRuntimeError("Unknown C++ exception");
		return EXIT_FAILURE;
	}
}

#if defined(_WIN32) && defined(NDEBUG)
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
	return RunApplication(cane::Arguments(0, nullptr));
}
#else
int main(int argc, char* argv[])
{
	return RunApplication(cane::Arguments(argc, argv));
}
#endif

void Startup()
{
	// Must run before the OpenGL context exists (see dlss.h).
	PreloadDlssModules();

	// Debug mode is intentionally sampled once. Editing the INI while running
	// takes effect only after restarting the program.
	LoadDebugModeSetting();
    g_gl.InitGL();
    glfwSwapInterval(g_fVsync ? 1 : 0);
    // NVIDIA DLSS through Streamline. Safe on any GPU: it reports "not
    // available" and leaves the normal OpenGL path untouched when it can't run.
    StartupDlss(glfwGetWin32Window(g_gl.window));
    // AMD FidelityFX Super Resolution 1 (any GPU, pure OpenGL).
    StartupFsr();
    // Save states, fast forward and controller combos (Options > Mods).
    StartupMods();
    // Anisotropic filtering, FOV, camera sensitivity, screenshots (QOL.ini).
    StartupQol();
    StartupMenuStyle();
    StartupMouseInput();

    std::cout << "Sly Cooper 2002 Sony Computer Entertainment America & Sucker Punch Productions\n";
    SetPhase(PHASE_Startup);

    StartupClock();
    StartupBrx();
    StartupScreen();
    StartupUi();
    StartupFrame();
    StartupTarget();
    StartupJmt();
    StartupPipe();
    StartupHide();
    StartupSaveData(&g_saveData);
    StartupGame();
    StartupSound();
    StartupCodes();
}

bool fQuitGame;
