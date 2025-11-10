#include "main.h"

int main(int cphzArgs, char* aphzArgs[])
{
    Startup();

    while (!glfwWindowShouldClose(g_gl.window) && fQuitGame != true)
    {
        if (g_transition.m_fPending != 0)
            g_transition.Execute(file);

        // Render scene to offscreen framebuffer ===
        glBindFramebuffer(GL_FRAMEBUFFER, g_gl.fbo);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        g_joy.Update(g_gl.window);

        RenderMenuGui(g_psw);

        if (g_psw != nullptr)
        {
            UpdateUi(&g_ui);

            SetupCm(g_pcm);
            MarkClockTick(&g_clock);
            UpdateSw(g_psw, g_clock.dt);
            UpdateCpman(g_gl.window, &g_pcm->cpman, nullptr, g_clock.dt);

            if (g_fRenderModels) {
                RenderSw(g_psw, g_pcm);
                //RenderSwAloAll(g_psw, g_pcm);
                DrawSw(g_psw, g_pcm);
            }

            if (g_fRenderCollision)
                DrawSwCollisionAll(g_pcm);

            DrawUi(&g_ui);
        }

        // Render framebuffer to screen with black bars ===
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glScreenShader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_gl.fbc);

        glBindVertexArray(g_gl.sao);
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Render ImGui directly to screen (after scaled scene is drawn) ===
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // === Swap ===
        glfwSwapBuffers(g_gl.window);
        glfwPollEvents();
    }

    if (g_psw != nullptr)
        DeleteWorld(g_psw);

    g_gl.TerminateGL();
    return 0;
}

void Startup()
{
    g_gl.InitGL();

    std::cout << "Sly Cooper 2002 Sony Computer Entertainment America & Sucker Punch Productions\n";
    SetPhase(PHASE_Startup);

    StartupClock();
    StartupBrx();
    StartupScreen();
    StartupUi();
    glfwSwapInterval(1);
}

bool fQuitGame;