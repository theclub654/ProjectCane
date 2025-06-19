#pragma once
#include "glshaders.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/ImGuiFileDialog.h"
#include "mouse.h"

enum AspectMode
{
	FitToScreen,
	Fixed_4_3,
	Fixed_16_9,
	Fixed_16_10,
};

class GL
{
public:

	// Window Object
	GLFWwindow* window;
	// Frame Buffer Object
	GLuint fbo;
	// Frame Buffer Color
	GLuint fbc;

	// Render Buffer Object
	GLuint rbo;

	// Screen Array Object
	GLuint sao;
	// Screen Buffer Object
	GLuint sbo;
	glm::mat4 screenProjection;

	// Text Array Object
	GLuint gao;
	// Text Buffer Object
	GLuint gbo;
	GLuint geo;
	glm::mat4 blotProjection;

	// Window width
	float width;
	// Window height
	float height;

	float aspectRatio;
	AspectMode aspectMode;

	void InitGL();
	void UpdateGLProjections();
	void TerminateGL();
};

void FrameBufferSizeCallBack(GLFWwindow* window, int width, int height);

extern GL g_gl;
extern GLuint screenQuadMatrixLoc;