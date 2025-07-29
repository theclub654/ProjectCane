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

	glScreenShader.Init("screen.vert", NULL, "screen.frag");
	glScreenShader.Use();
	screenQuadMatrixLoc = glGetUniformLocation(glScreenShader.ID, "u_model");
	glUniformMatrix4fv(screenQuadMatrixLoc, 1, GL_FALSE, glm::value_ptr(g_gl.screenProjection));
	glUniform1i(glGetUniformLocation(glScreenShader.ID, "screenTexture"), 0);

	glGlobShader.Init("glob.vert", NULL, "glob.frag");
	glGlobShader.Use();
	InitGlslUniforms();

    glBlotShader.Init("blot.vert", NULL, "blot.frag");
    glBlotShader.Use();
	glUniform1i(glGetUniformLocation(glBlotShader.ID, "u_fontTex"), 0);

    u_projectionLoc = glGetUniformLocation(glBlotShader.ID, "u_projection");
    u_modelLoc   = glGetUniformLocation(glBlotShader.ID, "u_model");
    uvRectLoc    = glGetUniformLocation(glBlotShader.ID, "u_uvRect");
    blotColorLoc = glGetUniformLocation(glBlotShader.ID, "blotColor");
    
	std::cout << "Sly Cooper 2002 Sony Computer Entertainment America & Sucker Punch Productions\n";
	SetPhase(PHASE_Startup);

	StartupClock();
	StartupBrx();
	StartupScreen();
	StartupUi();
	glfwSwapInterval(1);
}

float prevTime = 0.0f;
float accumulator = 0.0f;
unsigned int counter = 0;
bool fQuitGame;