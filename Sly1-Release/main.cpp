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
#include <atomic>
#include <cwchar>
#include <filesystem>
#include <iostream>
#include <thread>
#include <stb/stb_image_write.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

bool g_fDeveloperDebugMode = false;
void PrintSoundDiagnostics();
void TestSaveCompatibility(const std::filesystem::path& fixtures);

namespace
{
void LoadDebugModeSetting()
{
    g_fDeveloperDebugMode = cane::debugMode();
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
        {
#ifdef _WIN32
            YieldProcessor();
#else
            __builtin_ia32_pause();
#endif
        }
    }

private:
#ifdef _WIN32
    HANDLE m_timer = nullptr;
#endif
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
constexpr UINT kRunTrackedWindowFrame = WM_APP + 0x504;
std::atomic_bool s_windowTracking = false;
std::atomic_bool s_windowFrameThreadRunning = false;
HWND s_gameWindow = nullptr;
std::thread s_windowFrameThread;

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
	// Keep Windows' native caption-button behavior. Its private tracking loop
	// blocks GLFW's outer loop, so the helper thread sends this window a frame
	// request which Windows dispatches back on the original OpenGL thread.
	if (message == WM_NCLBUTTONDOWN &&
		(wParam == HTMINBUTTON || wParam == HTMAXBUTTON || wParam == HTCLOSE))
	{
		s_windowTracking.store(true, std::memory_order_release);
		const LRESULT result = CallWindowProcW(s_glfwWindowProc, hwnd, message, wParam, lParam);
		s_windowTracking.store(false, std::memory_order_release);
		return result;
	}

    switch (message)
    {
        case WM_ENTERSIZEMOVE:
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

#if defined(NDEBUG) && defined(_WIN32) && !defined(CANE_PORTABLE_SHADERS)
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
#else
int main(int cphzArgs, char* aphzArgs[])
#endif
{
    try {
    double smokeSeconds=0;std::string smokeLevel;std::filesystem::path screenshotPath;
#if defined(NDEBUG) && defined(_WIN32) && !defined(CANE_PORTABLE_SHADERS)
    auto args=cane::arguments(0, nullptr);
#else
    auto args=cane::arguments(cphzArgs, aphzArgs);
#endif
    if(args.size()==2 && args[0]=="--test-save-compatibility"){TestSaveCompatibility(cane::path(args[1]));std::cout<<"Save compatibility fixtures passed"<<std::endl;return 0;}
    std::vector<std::string> runtimeArgs;
    for(size_t i=0;i<args.size();++i) {
        if(args[i]=="--smoke-test-seconds"||args[i]=="--smoke-level"||args[i]=="--screenshot") {
            auto option=args[i];if(++i==args.size())throw std::runtime_error("Missing value for "+option);
            if(option=="--smoke-test-seconds") {smokeSeconds=std::stod(args[i]);if(!std::isfinite(smokeSeconds)||smokeSeconds<=0||smokeSeconds>600)throw std::runtime_error("Smoke duration must be 1..600 seconds");}
            else if(option=="--smoke-level")smokeLevel=args[i];else screenshotPath=cane::path(args[i]);
        }else runtimeArgs.push_back(args[i]);
    }
    cane::configureRuntime(runtimeArgs);
    Startup();
    if(!smokeLevel.empty()) {
        if(!PlevelinfoFromLevelName(smokeLevel))throw std::runtime_error("Unknown smoke-test level: "+smokeLevel);

    }
    auto smokeStart=std::chrono::steady_clock::now();size_t smokeFrames=0;bool screenshotWritten=false,controllerSeen=false,smokeWarpRequested=false;size_t buttonPresses=0;std::vector<double> frameTimes;auto frameStart=smokeStart;
#ifdef _WIN32
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);
#endif
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
        ++smokeFrames;
        if(!smokeLevel.empty() && !smokeWarpRequested && smokeFrames>=120 && !g_transition.m_fPending && g_wipe.wipes==WIPES_Idle) {WipeToWorldWarp(smokeLevel,(OID)-1,WIPEK_Fade);smokeWarpRequested=true;}
        auto now=std::chrono::steady_clock::now();
        if(smokeSeconds>0){controllerSeen=controllerSeen||g_joy.gamepadConnected;for(int b=0;b<BTN_MAX;++b)if(g_joy.current[static_cast<JOY_BUTTON>(b)]&&!g_joy.previous[static_cast<JOY_BUTTON>(b)])++buttonPresses;frameTimes.push_back(std::chrono::duration<double,std::milli>(now-frameStart).count());}frameStart=now;
        if(!screenshotPath.empty() && !screenshotWritten && smokeFrames>=120 && (smokeSeconds<=0 || std::chrono::duration<double>(now-smokeStart).count()>=smokeSeconds-1)) {
            int w=g_gl.renderWidth,h=g_gl.renderHeight;
            std::vector<unsigned char> pixels(static_cast<size_t>(w)*h*4);
            glBindFramebuffer(GL_READ_FRAMEBUFFER,g_gl.fbo);glReadBuffer(GL_COLOR_ATTACHMENT0);glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());glBindFramebuffer(GL_READ_FRAMEBUFFER,0);glReadBuffer(GL_BACK);
            stbi_flip_vertically_on_write(true);
            if(!stbi_write_png(cane::utf8(screenshotPath).c_str(),w,h,4,pixels.data(),w*4))throw std::runtime_error("Cannot write screenshot");
            screenshotWritten=true;
        }
        if(smokeSeconds>0 && smokeFrames>=120 && std::chrono::duration<double>(std::chrono::steady_clock::now()-smokeStart).count()>=smokeSeconds) {
            std::cout<<"SMOKE: frames="<<smokeFrames<<" world="<<g_transition.m_worldCurrent<<" seconds="<<std::chrono::duration<double>(std::chrono::steady_clock::now()-smokeStart).count()<<std::endl;
            if(g_psw==nullptr || smokeFrames<2)throw std::runtime_error("Smoke test did not load a playable world");
            if(!smokeLevel.empty() && g_transition.m_plevelCurrent!=PlevelinfoFromLevelName(smokeLevel))throw std::runtime_error("Smoke test did not reach requested world");
            PrintSoundDiagnostics();
            std::sort(frameTimes.begin(),frameTimes.end());std::cout<<"INPUT: controller="<<controllerSeen<<" presses="<<buttonPresses<<" FRAME: median_ms="<<frameTimes[frameTimes.size()/2]<<" p95_ms="<<frameTimes[frameTimes.size()*95/100]<<std::endl;
            fQuitGame=true;
        }

    }

#ifdef _WIN32
	ShutdownLiveResizeWindowProc();
#endif

    if (g_psw != nullptr)
        DeleteWorld(g_psw);

    ShutdownSound();
    FreeTvGL();
    g_gl.TerminateGL();
    return 0;
    } catch (const std::exception& e) { std::cerr << "ProjectCane: " << e.what() << std::endl; return 1; }
}

void Startup()
{
	// Debug mode is intentionally sampled once. Editing the INI while running
	// takes effect only after restarting the program.
	LoadDebugModeSetting();
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
