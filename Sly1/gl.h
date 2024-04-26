#pragma once
#include "glshaders.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/string_cast.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialog.h"
#include "mouse.h"

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
		// Screen Object
		GLuint sbo;

		int width  = 800;
		int height = 800;

		void InitGL();
		void TerminateGL();
};

void FrameBufferSizeCallBack(GLFWwindow* window, int width, int height);

extern GL g_gl;