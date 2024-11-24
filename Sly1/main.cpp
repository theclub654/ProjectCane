#include "main.h"

// Temporary flag
bool loadEmitMesh = 0;
GL g_gl;
GLSHADER glScreenShader;
GLSHADER glGlobShader;
GLSHADER glCollisionShader;
std::string file;
CTransition g_transition;
FREECAMERA  g_freecamera(glm::vec3{0.0});
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
		
		double currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;
		
		// Using custom frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, g_gl.fbo);
		// Making custom frame buffer all black
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		// Clear framebuffer color buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Activate Depth testing
		glEnable(GL_DEPTH_TEST);

		// Draws everything into the framebuffer
		RenderMenuGui(g_psw);

		ProcessInput(g_gl.window, deltaTime);
		
		if (g_psw != nullptr)
		{
			if (fRenderModels != 0)
			{
				//UpdateCpman(g_gl.window, &g_pcm->cpman, nullptr, deltaTime);
				g_freecamera.UpdateViewProjMatrix(g_gl.height, g_gl.width, glGlobShader);
				RenderSwAll(g_psw, g_pcm);
				DrawSwAll(g_psw, g_gl.window);
			}

			if (fRenderCollision != 0)
			{
				g_freecamera.UpdateViewProjMatrix(g_gl.height, g_gl.width, glCollisionShader);
				DrawSwCollisionAll();
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Switches back to default frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Making the default frame buffer black
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		// Clearing the color buffer of the default frame buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Using screen shader
		glScreenShader.Use();
		glDisable(GL_DEPTH_TEST);
		// Binding the screen canvas
		glBindVertexArray(g_gl.sbo);
		// Binding scene texture
		glBindTexture(GL_TEXTURE_2D, g_gl.fbc);
		// Drawing the scene texture on the screen
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(g_gl.window);
		glfwSwapInterval(1);
		glfwPollEvents();
	}

	DeleteSw(g_psw);
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
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "shadowTexture"),   0);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "diffuseTexture"),  1);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "saturateTexture"), 2);

	std::cout << "Sly Cooper 2002 Sony Computer Entertainment America & Sucker Punch Productions\n";
	SetPhase(PHASE_Startup);
	StartupBrx();
}

void ProcessInput(GLFWwindow* window, double dt)
{
	float velocity = (float)dt * 10000.0;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		g_freecamera.cameraPos += g_freecamera.cameraDirection * velocity;

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		g_freecamera.cameraPos -= g_freecamera.cameraDirection * velocity;

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		g_freecamera.cameraPos += g_freecamera.cameraRight * velocity;

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		g_freecamera.cameraPos -= g_freecamera.cameraRight * velocity;

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		g_freecamera.cameraPos += g_freecamera.cameraUp * velocity;

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		g_freecamera.cameraPos -= g_freecamera.cameraUp * velocity;

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		g_freecamera.cameraPos = glm::vec3{ 0.0 };


	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		g_freecamera.UpdateCameraDirection(MOUSE::GetDX(), MOUSE::GetDY());
	}

	double scrollDy = MOUSE::GetScrollDY();

	if (scrollDy != 0)
		g_freecamera.UpdateCameraFov(scrollDy);
}
