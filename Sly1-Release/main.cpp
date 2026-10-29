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
#include "resource1.h"
#include "runtime_error.h"
#include "emitter.h"
#include "blip.h"
#include "content.h"
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
#ifdef __APPLE__
#include <SDL_messagebox.h>
#include <string>
#endif

bool g_fDeveloperDebugMode = false;

namespace
{
void LoadDebugModeSetting()
{
	g_fDeveloperDebugMode = cane::DebugMode();
}
#ifdef __APPLE__

// Started from ProjectCane.app (Finder, Dock) there is no terminal to print
// to, and without data the game would sit on an empty window. Say where the
// data goes instead. SDL's message box needs no SDL_Init or window: on macOS
// it runs a modal NSAlert and returns when the user clicks OK.
void ShowMissingGameDataAlert(const cane::fs::path& bundle)
{
	const std::string message =
		"ProjectCane could not find the game data: the Worlds and Cutscenes folders and "
		"extraction_manifest.json.\n\n"
		"Put them in one of these folders:\n\n" +
		cane::Utf8(cane::DefaultUserDirectory()) + "\n\n" +
		cane::Utf8(bundle.parent_path()) + " (the folder that contains " +
		cane::Utf8(bundle.filename()) + ")\n\n"
		"The ProjectCane File Extractor app creates them from your Sly Cooper disc image "
		"and saves them in the first folder by default.";
	ReportRuntimeError("Data", "No game data found. %s", message.c_str());
	if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "ProjectCane - Game Data Not Found",
		message.c_str(), nullptr) != 0)
		ReportRuntimeError("Data", "Could not show the message box: %s", SDL_GetError());
}
#endif

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
        if (m_frameRate != g_targetFrameRate)
        {
            m_frameRate = g_targetFrameRate;
            m_deadline = Clock::now();
        }

        const auto frameDuration = std::chrono::duration_cast<Clock::duration>(
            std::chrono::duration<double>(1.0 / static_cast<double>(m_frameRate)));

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
        m_frameRate = g_targetFrameRate;
        m_deadline = Clock::now();
    }

private:
#ifdef _WIN32
    HANDLE m_timer = nullptr;
#endif
    Clock::time_point m_deadline;
    int m_frameRate = 60;
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

    // Retail leaves the movie queued while the outgoing wipe is visible.
    // Once the screen reaches black, play the movie before executing the
    // transition that replaces the current world.
    if (FCutscenePending() && g_wipe.wipes != WIPES_WipingOut)
    {
        ExecutePendingCutscenes();
    }

    if (g_transition.m_fPending != 0)
    {
		FlushTransitionFrame();
		g_transition.Execute(file);
		if (s_framePacer != nullptr)
			s_framePacer->Resync();
    }

    if (FCutscenePending() && g_wipe.wipes != WIPES_WipingOut)
    {
        ExecutePendingCutscenes();
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

    g_sceneFbo = g_fMsaa ? g_gl.fboMSAA : g_gl.fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);

    glViewport(0, 0, g_gl.renderWidth, g_gl.renderHeight);
    glClearColor(rgbaSky.r, rgbaSky.g, rgbaSky.b, rgbaSky.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    g_joy.Update(g_gl.window);
    RenderMenuGui(g_psw);

    if (g_psw != nullptr)
    {
        UpdateUi(&g_ui);
        UpdateGameState(g_clock.dt);

        SetupCm(g_pcm);
        OpenFrame();
        MarkClockTick(&g_clock);
        UpdateCodes();
        UpdateSw(g_psw, g_clock.dt);

        if (g_fRenderModels == true)
        {
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

        DrawUi(&g_ui);
    }

    if (g_fMsaa)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, g_gl.fboMSAA);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_gl.fbo);
        glBlitFramebuffer(0, 0, g_gl.renderWidth, g_gl.renderHeight,
            0, 0, g_gl.renderWidth, g_gl.renderHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(g_gl.presentX, g_gl.presentY, g_gl.presentWidth, g_gl.presentHeight);

	// Presentation is a hard OpenGL state boundary. World, shadow, particle,
	// and UI passes are allowed to use these states internally, but none of
	// them may influence the default-framebuffer clear or fullscreen composite.
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    {
        glScreenShader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_gl.fbc);
        glBindVertexArray(g_gl.sao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    glfwSwapBuffers(g_gl.window);
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
#ifdef __APPLE__
		// Only for runs from an .app bundle; development builds and tests
		// keep failing later as before. Checked before any window exists.
		if (const cane::fs::path bundle = cane::AppBundleDirectory(cane::ExecutableDirectory());
			!bundle.empty() && !cane::HasGameData(cane::DataDirectory()))
		{
			ShowMissingGameDataAlert(bundle);
			return EXIT_FAILURE;
		}
#endif
        RefreshContentPackage(cane::DataDirectory());
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
		ShutdownControllerInput();
		FreeTvGL();
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
	// Debug mode is intentionally sampled once. Editing the INI while running
	// takes effect only after restarting the program.
	LoadDebugModeSetting();
    g_gl.InitGL();
    StartupControllerInput();
    glfwSwapInterval(g_fVsync ? 1 : 0);

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
