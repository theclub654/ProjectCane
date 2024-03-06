#include "main.h"

// Temporary flag
bool loadEmitMesh = 0;
GL g_gl;
GLSHADER glShader;
GLSHADER glShaderCollision;
std::string file;
CTransition g_transition;
FREECAMERA g_freecamera(glm::vec3{0.0});
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
			glFlush();
			// Loads level
			g_transition.Execute(file);
		}
		
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		RenderMenuGui(g_psw);

		if (g_psw != nullptr)
		{
			double currentTime = glfwGetTime();
			deltaTime = currentTime - lastFrame;
			lastFrame = currentTime;

			ProcessInput(g_gl.window, deltaTime);

			g_freecamera.UpdateViewProjMatrix(g_gl.height, g_gl.width, glShader);
			g_freecamera.UpdateViewProjMatrix(g_gl.height, g_gl.width, glShaderCollision);

			if (fRenderWireFrame != 0)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			if (fRenderModels != 0)
				DrawSwAll();

			if (fRenderCollision != 0)
				DrawSwCollisionAll();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_gl.window);
		glfwSwapInterval(1);
		glfwPollEvents();
	}

	DeleteSw(g_psw);
	glShader.Delete();
	glShaderCollision.Delete();
	g_gl.GLFWTerminate();
	return 0;
}

void Startup()
{
	std::cout << "Sly Cooper 2002 Sony Computer Entertainment America & Sucker Punch Productions\n\n";
	SetPhase(PHASE_Startup);
	StartupBrx();
	g_gl.InitGL();

	glShader.Init("glob.vert", NULL, "glob.frag");
	glShaderCollision.Init("collision.vert", NULL, "collision.frag");
}

void ProcessInput(GLFWwindow* window, double dt)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		g_freecamera.UpdateCameraPos(CAMERADIRECTION::FORWARD, dt);

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		g_freecamera.UpdateCameraPos(CAMERADIRECTION::BACKWARD, dt);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		g_freecamera.UpdateCameraPos(CAMERADIRECTION::RIGHT, dt);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		g_freecamera.UpdateCameraPos(CAMERADIRECTION::LEFT, dt);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		g_freecamera.UpdateCameraPos(CAMERADIRECTION::UP, dt);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		g_freecamera.UpdateCameraPos(CAMERADIRECTION::DOWN, dt);

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
