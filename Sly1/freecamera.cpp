#include "freecamera.h"

FREECAMERA::FREECAMERA(glm::vec3 position)
{
	cameraPos = position;
	worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
	yaw = -90;
	pitch = 0;
	speed = 10000.0;
	fov = 45.0f;
	cameraDirection = glm::vec3{0.0};
	cameraRight = glm::vec3(0.0f, 0.0f, 0.0f);
	UpdateCameraVectors();
}

void FREECAMERA::UpdateCameraDirection(double dx, double dy)
{
	yaw += dx;
	pitch += dy;

	if (pitch > 89.0f)
		pitch = 89.0f;

	else if (pitch < -89.0f)
		pitch = -89.0f;

	UpdateCameraVectors();
}

void FREECAMERA::UpdateCameraPos(CAMERADIRECTION direction, double dt)
{
	float velocity = (float)dt * speed;

	switch (direction)
	{
		case CAMERADIRECTION::FORWARD:
			cameraPos += cameraDirection * velocity;
		break;

		case CAMERADIRECTION::BACKWARD:
			cameraPos -= cameraDirection * velocity;
		break;

		case CAMERADIRECTION::RIGHT:
			cameraPos += cameraRight * velocity;
		break;

		case CAMERADIRECTION::LEFT:
			cameraPos -= cameraRight * velocity;
		break;

		case CAMERADIRECTION::UP:
			cameraPos += cameraUp * velocity;
		break;

		case CAMERADIRECTION::DOWN:
			cameraPos -= cameraUp * velocity;
		break;
	}
}

void FREECAMERA::UpdateCameraFov(double dy)
{
	if (fov > 1.0f && fov <= 45.0f)
		fov -= dy;

	else if (fov < 1.0f)
		fov = 1.0f;

	else
		fov = 45.0f;
}

void FREECAMERA::UpdateViewProjMatrix(int height, int width, GLSHADER shader)
{
	// Creates a large frustum
	proj = glm::perspective(fov, (float)height / (float)width, 100.0f, 1000000.0f);

	// Transform coordinates from world space to camera space
	view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);
	
	SendViewProjShader(proj, view, shader);
}

void FREECAMERA::SendViewProjShader(glm::mat4 proj, glm::mat4 view, GLSHADER shader)
{
	shader.Use();

	// Sends view matrix to GPU shader
	int viewUniformLocation = glGetUniformLocation(shader.ID, "view");
	glUniformMatrix4fv(viewUniformLocation, 1, GL_FALSE, glm::value_ptr(view));

	// Sends projection matrix to GPU shader
	int projUniformLocation = glGetUniformLocation(shader.ID, "proj");
	glUniformMatrix4fv(projUniformLocation, 1, GL_FALSE, glm::value_ptr(proj));
}

void FREECAMERA::UpdateCameraVectors()
{
	glm::vec3 direction;
	direction.x = cos(glm::radians(-yaw)) * cos(glm::radians(-pitch));
	direction.z = sin(glm::radians(pitch));
	direction.y = sin(glm::radians(-yaw)) * cos(glm::radians(-pitch));

	cameraDirection = glm::normalize(direction);

	cameraRight = glm::normalize(glm::cross(cameraDirection, worldUp));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));
}