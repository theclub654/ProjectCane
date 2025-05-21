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

		// Glyff Array Object
		GLuint gao;
		// Glyff Buffer Object
		GLuint gbo;
		GLuint geo;
		glm::mat4 textProjection;
		// Window width
		float width  = 800;
		// Window height
		float height = 800;

		void InitGL();
		void TerminateGL();
};

// Retrieves the uniform location with string
GLint GetUniformLocation(GLuint program​, const std::string& name);
void  FrameBufferSizeCallBack(GLFWwindow* window, int width, int height);

extern GL g_gl;