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

    while (!glfwWindowShouldClose(g_gl.window) && fQuitGame != true)
    {
        // Retail leaves the movie queued while the outgoing wipe is visible.
        // Once the screen reaches black, play the movie before executing the
        // transition that replaces the current world.
        if (FCutscenePending() && g_wipe.wipes != WIPES_WipingOut)
            ExecutePendingCutscenes();

        if (g_transition.m_fPending != 0)
            g_transition.Execute(file);

        if (FCutscenePending() && g_wipe.wipes != WIPES_WipingOut)
            ExecutePendingCutscenes();

        // 1) Render scene to MSAA (or resolved if MSAA off)
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
            UpdateSw(g_psw, g_clock.dt);
            //UpdateCpman(g_gl.window, &g_pcm->cpman, nullptr, g_clock.dt);

            if (g_fRenderModels == true)
            {
                RenderSw(g_psw, g_pcm);
                RenderUi(&g_ui);
                //RenderSwGlobset(g_psw, g_pcm);
                DrawSw(g_psw, g_pcm);
            }

            if (g_fRenderCollision == true)
                DrawSwCollisionAll(g_pcm);

            DrawUi(&g_ui);
        }

        // 2) Resolve MSAA -> resolved texture FBO (ONLY if MSAA)
        if (g_fMsaa)
        {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, g_gl.fboMSAA);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_gl.fbo);
            glBlitFramebuffer(0, 0, g_gl.renderWidth, g_gl.renderHeight,
                0, 0, g_gl.renderWidth, g_gl.renderHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }

        // 3) Present: draw fullscreen quad sampling resolved texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(g_gl.presentX, g_gl.presentY, g_gl.presentWidth, g_gl.presentHeight);

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        // g_gl.fbc is the completed scene, not another translucent layer.
        // Present it by replacement. In particular, world-map textures write
        // their source alpha into the scene target; blending this fullscreen
        // quad again would turn their transparent rectangular bounds black.
        glDisable(GL_BLEND);

        glScreenShader.Use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_gl.fbc);

        glBindVertexArray(g_gl.sao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(g_gl.window);
        glfwPollEvents();

        // This remains authoritative with VSync on or off. VSync controls
        // presentation; it no longer changes the game's update frequency.
        framePacer.Wait();

        g_cframe++;
    }

    if (g_psw != nullptr)
        DeleteWorld(g_psw);

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
}

bool fQuitGame;
