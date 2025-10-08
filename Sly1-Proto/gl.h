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

// Initializes glsl uniform locations
void InitGlslUniforms();
void FrameBufferSizeCallBack(GLFWwindow* window, int width, int height);
void RescaleLineWidth();

extern GL g_gl;
extern GLuint screenQuadMatrixLoc;
extern GLuint glslNumLights;
extern GLuint glslLightIndices;
extern GLuint glslmatWorldToClip;
extern GLuint glslCameraPos;
extern GLuint glslFogType;
extern GLuint glslFogNear;
extern GLuint glslFogFar;
extern GLuint glslFogMax;
extern GLuint glslFogColor;
extern GLuint glslLsmShadow;
extern GLuint glslLsmDiffuse;
extern GLuint glslRgbaCel;
extern GLuint glslModel;
extern GLuint glslUFog;
extern GLuint glslUAlpha;
extern GLuint glsluAlphaCelBorder;
extern GLuint glslRDarken;
extern GLuint glslRko;
extern GLuint glslusSelfIllum;
extern GLuint glslFDynamic;
extern GLuint glslPosCenter;
extern GLuint glslfAlphaTest;
extern GLuint glslAlphaThresHold;
extern GLuint glslfCull;
extern GLuint glslCollisionRgba;