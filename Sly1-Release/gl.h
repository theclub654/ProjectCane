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

struct alignas(16) ROONEWAY
{
	glm::mat4 model;       //  0 -  63
	int       rko;         //  64
	float     uAlpha;      //  68
	float     uFog;        //  72
	float     darken;      //  76
	glm::vec2 uvOffsets;   //  80 -  87
	int       _pad0;       //  88
	int       _pad1;       //  92
	int       warpType;    //  96
	int       warpCmat;    // 100
	int       warpCvtx;    // 104
	int       _padWarp0;   // 108
	int       _padWarp1;   // 112
	int       _padWarp2;   // 116
	int       _padWarp3;   // 120
	int       _padWarp4;   // 124
	glm::mat4 amatDpos[4]; // 128 - 383
	glm::mat4 amatDuv[4];  // 384 - 639
};

struct alignas(16) ROTHREEWAY
{
	glm::mat4 model;       //  0 -  63
	int       rko;         //  64
	float     uAlpha;      //  68
	float     uFog;        //  72
	float     darken;      //  76
	glm::vec2 uvOffsets;   //  80 -  87
	int       _pad0;       //  88
	int       _pad1;       //  92
	int       warpType;    //  96
	int       warpCmat;    // 100
	int       warpCvtx;    // 104
	int       _padWarp0;   // 108
	int       _padWarp1;   // 112
	int       _padWarp2;   // 116
	int       _padWarp3;   // 120
	int       _padWarp4;   // 124
	glm::mat4 amatDpos[4]; // 128 - 383
	glm::mat4 amatDuv[4];  // 384 - 639
	int       fDynamic;    // 640
	float     unSelfIllum; // 644
	float     sRadius;     // 648
	int       _pad2;       // 652
	glm::vec4 posCenter;   // 656 - 671
};

struct alignas(16) ROCEL
{
	glm::mat4 model;           
	glm::vec4 celRgba;         
	float     uAlphaCelBorder;
};

struct ROGEOM
{
	glm::mat4 model;
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

void InitCameraSbo();
void InitRopUbo();
void InitRcbUbo();
void InitGeomUbo();
void FrameBufferSizeCallBack(GLFWwindow* window, int width, int height);

extern GL g_gl;
extern GLuint cmSSBO;
extern GLuint ropUBO;
extern GLuint rcbUBO;
extern GLuint geomUBO;
extern GLuint activeLightsSbo;
extern GLuint screenQuadMatrixLoc;
extern GLuint glslLsmShadow;
extern GLuint glslLsmDiffuse;
extern GLuint glslFogType;
extern GLuint glslFogNear;
extern GLuint glslFogFar;
extern GLuint glslFogMax;
extern GLuint glslFogColor;
extern GLuint glslfAlphaTest;
extern GLuint glslfCull;