#include "freecamera.h"

void FREECAMERA::Transformations(int height, int width)
{
	glm::mat4 proj{ 1.0 };
	glm::mat4 view{ 1.0 };

	proj = glm::perspective(fov, (float)height / (float)width, nearRenderDistance, farRenderDistance);
	view = glm::lookAt(cameraPos, cameraPos + cameraOrientation, cameraUp);

	UpdateMatrix(view, proj);
}

void FREECAMERA::UpdateMatrix(glm::mat4 view, glm::mat4 proj)
{
	glShader.Use();

	int viewUniformLocation = viewUniformLocation = glGetUniformLocation(glShader.ID, "view");
	glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, glm::value_ptr(view));

	int projUniformLocation = projUniformLocation = glGetUniformLocation(glShader.ID, "proj");
	glUniformMatrix4fv(projUniformLocation, 1, GL_FALSE, glm::value_ptr(proj));
}

void FREECAMERA::ProcessInputs(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += speed * cameraOrientation;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= speed * cameraOrientation;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraOrientation, cameraUp)) * speed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraOrientation, cameraUp)) * speed;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		speed = 100.0f;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
		speed = 10.0f;
}