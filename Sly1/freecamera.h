#pragma once
#include "glshaders.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

// Sly 1 uses a Z up axis

enum class CAMERADIRECTION
{
	NONE = 0,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class FREECAMERA
{
	public:

		glm::vec3 cameraPos;
		glm::vec3 cameraDirection;
		glm::vec3 cameraUp;
		glm::vec3 cameraRight;
		glm::vec3 worldUp;

		float yaw;
		float pitch;
		float speed;
		float fov;

		FREECAMERA(glm::vec3 position);

		void UpdateCameraDirection(double dx, double dy);
		void UpdateCameraPos(CAMERADIRECTION direction, double dt);
		void UpdateCameraFov(double dy);
		void UpdateViewProjMatrix(int height, int width, GLSHADER shader);
		void SendViewProjShader(glm::mat4 proj, glm::mat4 view, GLSHADER shader);
		void UpdateCameraVectors();
};

// Global free camera class
extern FREECAMERA g_freecamera;