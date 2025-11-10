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

struct CMGL
{
	glm::mat4 matWorldToClip;
	glm::vec4 cameraPos;
};

struct alignas(16) RO
{
	glm::mat4 model;           // 0..63
	int       rko;             // 64
	float     uAlpha;          // 68
	float     uFog;            // 72
	float     darken;          // 76
	int       fDynamic;        // 80
	float     unSelfIllum;	   // 84
	float	  sRadius;		   // 88
	int       trlk;            // 92
	glm::vec4 posCenter;	   // 96..111
	float     uAlphaCelBorder; // 112
	uint32_t  _pad2;		   // 116
	uint32_t  _pad3;           // 120
	uint32_t  _pad4;           // 124  -> struct size 128 (rounded to 16)
};

class GL
{
	public:

	// Window Object
	GLFWwindow *window;
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
extern GLuint cmUBO;
extern GLuint ropUBO;
extern GLuint alUbo;
extern GLuint roSize;
extern GLuint roCelSize;
extern GLuint roCollisionSize;
extern GLuint screenQuadMatrixLoc;
extern GLuint glslLsmShadow;
extern GLuint glslLsmDiffuse;
extern GLuint glslFogType;
extern GLuint glslFogNear;
extern GLuint glslFogFar;
extern GLuint glslFogMax;
extern GLuint glslFogColor;
extern GLuint glslRgbaCel;
extern GLuint glslfAlphaTest;
extern GLuint glslfCull;
extern GLuint glslCollisionRgba;
extern GLuint gEmptyVAO;