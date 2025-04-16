#include "main.h"

// Temporary flag
bool loadEmitMesh = 0;
GL g_gl;
GLSHADER glScreenShader;
GLSHADER glGlobShader;
GLSHADER glCollisionShader;
std::string file;
std::string levelName;
CTransition g_transition;
bool firstClick = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main(int cphzArgs, char* aphzArgs[])
{
	// Initializing all things needed for game to be started
	Startup();
	
	while (!glfwWindowShouldClose(g_gl.window))
	{
		// If level pending flag is set to other than zero load up level
		if (g_transition.m_fPending != 0)
		{
			// Loads level
			g_transition.Execute(file);
		}
		
		// Using custom frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, g_gl.fbo);
		// Making custom frame buffer all black
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		// Clear framebuffer color buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		// Activate Depth testing
		glEnable(GL_DEPTH_TEST);
		
		
		RenderMenuGui(g_psw);
		
		if (g_psw != nullptr)
		{
			//SetupCm(g_pcm);
			double currentTime = glfwGetTime();
			deltaTime = currentTime - lastFrame;
			lastFrame = currentTime;

			UpdateCpman(g_gl.window, &g_pcm->cpman, nullptr, deltaTime);

			UpdateSw(g_psw, deltaTime);

			if (g_fRenderModels != 0)
			{
				RenderSwAloAll(g_psw, g_pcm);
				DrawSw(g_psw, g_pcm);
			}

			if (g_fRenderCollision != 0)
				DrawSwCollisionAll();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Switches back to default frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Making the default frame buffer black
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		////// Clearing the color buffer of the default frame buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		// Using screen shader
		glScreenShader.Use();
		// Binding the screen canvas
		glBindVertexArray(g_gl.sao);
		glDisable(GL_DEPTH_TEST);
		// Binding scene texture
		glBindTexture(GL_TEXTURE_2D, g_gl.fbc);
		// Drawing the scene texture on the screen
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(g_gl.window);
		glfwSwapInterval(1);
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
	glGlobShader.Init("glob.vert", NULL, "glob.frag");
	glCollisionShader.Init("collision.vert", NULL, "collision.frag");

	// Initialize texture samplers for glob shader
	glGlobShader.Use();
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "shadowMap"),   0);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "diffuseMap"),  1);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "saturateMap"), 2);

	std::cout << "Sly Cooper 2002 Sony Computer Entertainment America & Sucker Punch Productions\n";
	SetPhase(PHASE_Startup);
	StartupBrx();
}