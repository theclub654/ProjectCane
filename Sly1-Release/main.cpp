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
#include <iostream>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace
{
class FramePacer
{
public:
    using Clock = std::chrono::steady_clock;

    FramePacer()
        : m_deadline(Clock::now())
    {
        // CREATE_WAITABLE_TIMER_HIGH_RESOLUTION is 0x2. Use the value directly
        // so this still builds with Windows SDKs that predate the named flag.
        m_timer = CreateWaitableTimerExW(
            nullptr, nullptr, 0x2, TIMER_MODIFY_STATE | SYNCHRONIZE);

        if (m_timer == nullptr)
            m_timer = CreateWaitableTimerExW(
                nullptr, nullptr, 0, TIMER_MODIFY_STATE | SYNCHRONIZE);
    }

    ~FramePacer()
    {
        if (m_timer != nullptr)
            CloseHandle(m_timer);
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
            // Do not issue rapid catch-up frames after a stall or breakpoint.
            m_deadline = now;
            return;
        }

        const auto coarseDeadline = m_deadline - kSpinMargin;
        if (now < coarseDeadline)
        {
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
        }

        // YieldProcessor is a CPU pause hint, not a scheduler yield. It keeps
        // the final sub-millisecond boundary precise without giving up a full
        // Windows scheduling quantum.
        while (Clock::now() < m_deadline)
            YieldProcessor();
    }

private:
    HANDLE m_timer = nullptr;
    Clock::time_point m_deadline;
    int m_frameRate = 60;
};

bool s_runningFrame = false;

void RunGameFrame()
{
    if (s_runningFrame)
        return;

    s_runningFrame = true;

    // Retail leaves the movie queued while the outgoing wipe is visible.
    // Once the screen reaches black, play the movie before executing the
    // transition that replaces the current world.
    if (FCutscenePending() && g_wipe.wipes != WIPES_WipingOut)
        ExecutePendingCutscenes();

    if (g_transition.m_fPending != 0)
        g_transition.Execute(file);

    if (FCutscenePending() && g_wipe.wipes != WIPES_WipingOut)
        ExecutePendingCutscenes();

    static bool s_fF3WasDown = false;
    const bool fF3Down = g_gl.window != nullptr &&
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
            DrawSwCollisionAll(g_pcm);

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

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glScreenShader.Use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_gl.fbc);
    glBindVertexArray(g_gl.sao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(g_gl.window);

    ++g_cframe;
    s_runningFrame = false;
}

#ifdef _WIN32
WNDPROC s_glfwWindowProc = nullptr;
constexpr UINT_PTR kLiveResizeTimer = 0x5043;

LRESULT CALLBACK LiveResizeWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_ENTERSIZEMOVE:
        {
            const UINT intervalMs = static_cast<UINT>((std::max)(1, 1000 / (std::max)(1, g_targetFrameRate)));
            SetTimer(hwnd, kLiveResizeTimer, intervalMs, nullptr);
            break;
        }

        case WM_TIMER:
        if (wParam == kLiveResizeTimer)
        {
            RunGameFrame();
            return 0;
        }
        break;

        case WM_EXITSIZEMOVE:
        case WM_DESTROY:
        KillTimer(hwnd, kLiveResizeTimer);
        break;
    }

    return CallWindowProcW(s_glfwWindowProc, hwnd, message, wParam, lParam);
}

void InstallLiveResizeWindowProc()
{
    HWND hwnd = glfwGetWin32Window(g_gl.window);
    if (hwnd == nullptr)
        return;

    s_glfwWindowProc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(LiveResizeWindowProc));
}
#endif
}

#ifdef NDEBUG
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#else
int main(int cphzArgs, char* aphzArgs[])
#endif
{
    Startup();
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
    FramePacer framePacer;
#ifdef _WIN32
	InstallLiveResizeWindowProc();
#endif

    while (!glfwWindowShouldClose(g_gl.window) && fQuitGame != true)
    {
		RunGameFrame();
		glfwPollEvents();

        // This remains authoritative with VSync on or off. VSync controls
        // presentation; it no longer changes the game's update frequency.
        framePacer.Wait();

    }

    if (g_psw != nullptr)
        DeleteWorld(g_psw);

    ShutdownSound();
    FreeTvGL();
    g_gl.TerminateGL();
    return 0;
}

void Startup()
{
    g_gl.InitGL();
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
