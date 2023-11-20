#include "main.h"

// Temporary flag
bool loadEmitMesh = 0;
GL g_gl;
GLSHADER glShader;
std::string file;
CTransition g_transition;

int main(int cphzArgs, char* aphzArgs[])
{
	// Initializing all things needed for game to be started
	Startup();

	while (!glfwWindowShouldClose(g_gl.window))
	{
		// If level pending flag is set to other than zero load up level
		if (g_transition.m_fPending != 0)
		{
			glFlush();
			// Loads level
			g_transition.Execute(file);
		}
		
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (g_psw != nullptr)
		{
			//glShader.Use();
			//DrawSwAll(g_psw, g_pcm);
		}

		RenderOpenFileGui();
		RenderCloseWorldGui(g_psw);

		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_gl.window);
		glfwSwapInterval(1);
		glfwPollEvents();
	}
	
	if (g_psw != nullptr)
		DeleteWorld(g_psw);

	glShader.Delete();
	g_gl.GLFWTerminate();
	return 0;
}

void Startup()
{
	std::cout << "Sly Cooper 2002 Sony Computer Entertainment America & Sucker Punch Productions\n";
	SetPhase(PHASE_Startup);
	StartupBrx();
	g_gl.InitGL();
	glShader.Init("glob.vert", "glob.frag");
}