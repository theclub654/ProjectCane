#pragma once
#include "gl.h"

class FREECAMERA
{
	public:
		glm::vec3 cameraPos{ 0.0 };
		glm::vec3 cameraOrientation = glm::vec3(-0.010699, 1.584016, -0.700555);
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

		bool firstClick = true;
		float speed = 10.0;
		float sensitivity = 100.0f;
		float yaw = -90.0f;
		float pitch = 0.0f;
		float fov = 45.0f;
		float nearRenderDistance = 100.0f;
		float farRenderDistance = 1000000.0f;

		void Transformations(int height, int width);
		void UpdateMatrix(glm::mat4 view, glm::mat4 proj);
		void ProcessInputs(GLFWwindow* window);
};

// Global free camera class
extern FREECAMERA g_freecamera;