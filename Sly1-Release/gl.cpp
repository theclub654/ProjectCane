#include "gl.h"
#include "render.h"
#include "sqtr.h"
#include "tv.h"
#include "game.h"
#include <cmath>

#ifdef _WIN32
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int NvidiaPowerXpressRequestHighPerformance = 1;
}
#endif

namespace
{
int s_windowedX = 100;
int s_windowedY = 100;
int s_windowedWidth = 800;
int s_windowedHeight = 800;

GLFWmonitor* MonitorForWindow(GLFWwindow* window)
{
	if (GLFWmonitor* monitor = glfwGetWindowMonitor(window))
		return monitor;

	int windowX = 0;
	int windowY = 0;
	int windowWidth = 0;
	int windowHeight = 0;
	glfwGetWindowPos(window, &windowX, &windowY);
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();
	int bestOverlap = -1;
	int monitorCount = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
	for (int i = 0; i < monitorCount; ++i)
	{
		int monitorX = 0;
		int monitorY = 0;
		int monitorWidth = 0;
		int monitorHeight = 0;
		glfwGetMonitorWorkarea(monitors[i], &monitorX, &monitorY, &monitorWidth, &monitorHeight);
		const int overlapWidth = (std::max)(0,
			(std::min)(windowX + windowWidth, monitorX + monitorWidth) - (std::max)(windowX, monitorX));
		const int overlapHeight = (std::max)(0,
			(std::min)(windowY + windowHeight, monitorY + monitorHeight) - (std::max)(windowY, monitorY));
		const int overlap = overlapWidth * overlapHeight;
		if (overlap > bestOverlap)
		{
			bestOverlap = overlap;
			bestMonitor = monitors[i];
		}
	}

	return bestMonitor;
}
}

void GL::InitGL()
{
	// Create GLFW context and window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	width  = 1280;
	height = 720;
	renderWidth = static_cast<int>(width);
	renderHeight = static_cast<int>(height);
	outputWidth = static_cast<int>(width);
	outputHeight = static_cast<int>(height);
	presentX = 0;
	presentY = 0;
	presentWidth = outputWidth;
	presentHeight = outputHeight;

	aspectRatio = float(width) / float(height);
	aspectMode = FitToScreen;

	window = glfwCreateWindow(width, height, "Sly Cooper and the Thievius Raccoonus", nullptr, nullptr);

	if (!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		while (true);
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallBack);
	glfwSetCursorPosCallback(window, MOUSE::CursorPosCallback);
	glfwSetMouseButtonCallback(window, MOUSE::MouseButtonCallback);
	glfwSetScrollCallback(window, MOUSE::MouseWheelCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		while (true);
	}

	// ========== ImGui Setup ==========
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
	ImGui::StyleColorsDark();

	const float imguiOffset = g_fDebugMode != 0 ? ImGui::GetFrameHeight() : 0.0f;
	const float initialSceneHeight = std::max(1.0f, height - imguiOffset);
	uiScale = std::min(width / 640.0f, initialSceneHeight / 492.80002f);
	uiOrigin.x = (width - 640.0f * uiScale) * 0.5f;
	uiOrigin.y = (initialSceneHeight - 492.80002f * uiScale) * 0.5f;

	// ========== Framebuffer setup ==========
	CreateFramebuffers(width, height);

	// ========== Screen Quad ==========
	float screen[] = {
		// pos      // tex
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,

		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};

	glGenVertexArrays(1, &sao);
	glGenBuffers(1, &sbo);

	glBindVertexArray(sao);
	glBindBuffer(GL_ARRAY_BUFFER, sbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen), screen, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);

	// ========== Instanced BLIP quad ==========
	const float blipCorners[] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f
	};
	const uint16_t blipIndices[] = { 0, 1, 2, 0, 2, 3 };

	glGenVertexArrays(1, &blipVao);
	glGenBuffers(1, &blipVbo);
	glGenBuffers(1, &blipEbo);
	glGenBuffers(1, &blipInstanceVbo);
	glBindVertexArray(blipVao);

	glBindBuffer(GL_ARRAY_BUFFER, blipVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(blipCorners), blipCorners, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blipEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(blipIndices), blipIndices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, blipInstanceVbo);
	blipInstanceCapacity = 64 * 384;
	glBufferData(GL_ARRAY_BUFFER, blipInstanceCapacity, nullptr, GL_STREAM_DRAW);
	for (GLuint attribute = 1; attribute <= 4; ++attribute)
	{
		glEnableVertexAttribArray(attribute);
		glVertexAttribPointer(attribute, 4, GL_FLOAT, GL_FALSE, 64,
			reinterpret_cast<void*>(static_cast<uintptr_t>((attribute - 1) * 16)));
		glVertexAttribDivisor(attribute, 1);
	}
	glBindVertexArray(0);

	// ========== SQTR ribbon trail ==========
	glGenVertexArrays(1, &sqtrVao);
	glGenBuffers(1, &sqtrVbo);
	glBindVertexArray(sqtrVao);
	glBindBuffer(GL_ARRAY_BUFFER, sqtrVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SQTRGPU) * 64 * 2, nullptr, GL_STREAM_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SQTRGPU),
		reinterpret_cast<void*>(offsetof(SQTRGPU, pos)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(SQTRGPU),
		reinterpret_cast<void*>(offsetof(SQTRGPU, rgba)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(SQTRGPU),
		reinterpret_cast<void*>(offsetof(SQTRGPU, uv)));
	glBindVertexArray(0);

	glViewport(0, 0, width, height - imguiOffset);

	UpdateGLProjections();

	float quadVertices[] = {
		// x, y,        u, v
		 0.0f, 0.0f,	0.0f, 0.0f, // top-left
		 1.0f, 0.0f,	1.0f, 0.0f, // top-right
		 1.0f, 1.0f,	1.0f, 1.0f, // bottom-right
		 0.0f, 1.0f,	0.0f, 1.0f  // bottom-left
	};

	uint16_t indices[] = {
		0,1,2, 0,2,3
	};

	glGenVertexArrays(1, &gao);
	glGenBuffers(1, &gbo);

	glBindVertexArray(gao);
	glBindBuffer(GL_ARRAY_BUFFER, gbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &geo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);

	blotProjection = glm::ortho(0.0f, float(width), float(height - imguiOffset), 0.0f, -1.0f, 1.0f);

	glScreenShader.Init("screen.vert", NULL, "screen.frag");
	glScreenShader.Use();
	glUniform1i(glGetUniformLocation(glScreenShader.ID, "screenTexture"), 0);
	//glslScreenSampler = glGetUniformLocation(glScreenShader.ID, "screenTexture");
	//glUniformHandleui64ARB(glslScreenSampler, fbc);

	glDyshadow.Init("dysh.vert", NULL, "dysh.frag");
	glDyshadow.Use();
	
	glslDyshMatWorldClip = glGetUniformLocation(glDyshadow.ID, "matWorldToClip");
	glslDyshModel        = glGetUniformLocation(glDyshadow.ID, "model");
	glslDyshfSkin        = glGetUniformLocation(glDyshadow.ID, "fSkin");

	glGlobShader.Init("glob.vert", NULL, "glob.frag");
	ropStream.bindIndex = 1;
	GLint align = 256;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);

	ropStream.stride = (sizeof(ROGL) + align - 1) & ~(align - 1);
	glGlobShader.Use();

	glslLsmShadow  = glGetUniformLocation(glGlobShader.ID, "swp.uShadow");
	glslLsmDiffuse = glGetUniformLocation(glGlobShader.ID, "swp.uMidtone");
	glslFogType    = glGetUniformLocation(glGlobShader.ID, "swp.fogType");
	glslFogNear    = glGetUniformLocation(glGlobShader.ID, "swp.fogNear");
	glslFogFar     = glGetUniformLocation(glGlobShader.ID, "swp.fogFar");
	glslFogMax     = glGetUniformLocation(glGlobShader.ID, "swp.fogMax");
	glslFogColor   = glGetUniformLocation(glGlobShader.ID, "swp.fogColor");

	glslfAlphaTest  = glGetUniformLocation(glGlobShader.ID, "fAlphaTest");
	glslAlphaCutOff = glGetUniformLocation(glGlobShader.ID, "alphaCutOff");

	glslRko              = glGetUniformLocation(glGlobShader.ID, "rko");
	glslfAnimateUv       = glGetUniformLocation(glGlobShader.ID, "fAnimateUv");
	glsluvOffsets		 = glGetUniformLocation(glGlobShader.ID, "uvOffsets");
	glslUnSelfIllum		 = glGetUniformLocation(glGlobShader.ID, "unSelfIllum");
	glslSubGlobPosCenter = glGetUniformLocation(glGlobShader.ID, "subGlobPosCenter");
	glslSubGlobRadius    = glGetUniformLocation(glGlobShader.ID, "subGlobRadius");

	glslAmbientMap  = glGetUniformLocation(glGlobShader.ID, "ambientMap");
	glslDiffuseMap  = glGetUniformLocation(glGlobShader.ID, "diffuseMap");
	glslSaturateMap = glGetUniformLocation(glGlobShader.ID, "saturateMap");

	glCelBorderShader.Init("celborder.vert", NULL, "celborder.frag");
	rcbStream.bindIndex = 1;
	align = 256;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);

	rcbStream.stride = (sizeof(ROCEL) + align - 1) & ~(align - 1);
	glGeomShader.Init("geom.vert", NULL, "geom.frag");
	glslGeomModelToClip = glGetUniformLocation(glGeomShader.ID, "modelToClip");

	glBlotShader.Init("blot.vert", NULL, "blot.frag");
	glBlotShader.Use();

	u_projectionLoc = glGetUniformLocation(glBlotShader.ID, "u_projection");
	u_modelLoc      = glGetUniformLocation(glBlotShader.ID, "u_model");
	uvRectLoc       = glGetUniformLocation(glBlotShader.ID, "u_uvRect");
	u_useVertexColorLoc = glGetUniformLocation(glBlotShader.ID, "u_useVertexColor");
	glUniform1i(u_useVertexColorLoc, 0);
	blotColorLoc    = glGetUniformLocation(glBlotShader.ID, "blotColor");
	u_fontTexLoc    = glGetUniformLocation(glBlotShader.ID, "u_fontTex");

	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

	glBlipShader.Init("blip.vert", NULL, "blip.frag");
	glBlipShader.Use();
	glUniform1i(glGetUniformLocation(glBlipShader.ID, "diffuseMap"), 0);
	glslBlipCurrentTime = glGetUniformLocation(glBlipShader.ID, "currentTime");
	glslBlipDtFrame = glGetUniformLocation(glBlipShader.ID, "dtFrame");
	glslBlipCameraMat = glGetUniformLocation(glBlipShader.ID, "cameraMat");
	glslBlipAlphaPass = glGetUniformLocation(glBlipShader.ID, "alphaPass");

	blipStream.bindIndex = 1;
	align = 256;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);
	blipStream.stride = (sizeof(BLIPGROUPGPU) + align - 1) & ~(align - 1);
	const GLuint blipGroupBlock = glGetUniformBlockIndex(glBlipShader.ID, "BlipGroup");
	if (blipGroupBlock != GL_INVALID_INDEX)
		glUniformBlockBinding(glBlipShader.ID, blipGroupBlock, blipStream.bindIndex);

	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	////
	glGenVertexArrays(1, &lineVao);
	glGenBuffers(1, &lineVbo);

	glBindVertexArray(lineVao);
	glBindBuffer(GL_ARRAY_BUFFER, lineVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(BLOTLINEVERTEX) * 2, nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BLOTLINEVERTEX), reinterpret_cast<void*>(offsetof(BLOTLINEVERTEX, position)));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BLOTLINEVERTEX), reinterpret_cast<void*>(offsetof(BLOTLINEVERTEX, uv)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BLOTLINEVERTEX), reinterpret_cast<void*>(offsetof(BLOTLINEVERTEX, color)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	////
	glGenTextures(1, &whiteTex);
	glBindTexture(GL_TEXTURE_2D, whiteTex);

	uint32_t white = 0xFFFFFFFFu;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	whiteHandle = glGetTextureHandleARB(whiteTex);
	glMakeTextureHandleResidentARB(whiteHandle);

	glBindTexture(GL_TEXTURE_2D, 0);

	glslDyshfSkin = glGetUniformLocation(glDyshadow.ID, "fSkin");
	glslfSkin     = glGetUniformLocation(glGlobShader.ID, "fSkin");
	glslCelSkin   = glGetUniformLocation(glCelBorderShader.ID, "fSkin");

	glslfPose       = glGetUniformLocation(glGlobShader.ID, "fPose");
	glslPoseCount   = glGetUniformLocation(glGlobShader.ID, "poseCount");
	glslPoseWeights = glGetUniformLocation(glGlobShader.ID, "poseWeights[0]");

	glslCelPose        = glGetUniformLocation(glCelBorderShader.ID, "fPose");
	glslCelPoseCount   = glGetUniformLocation(glCelBorderShader.ID, "poseCount");
	glslCelPoseWeights = glGetUniformLocation(glCelBorderShader.ID, "poseWeights[0]");
}

void GL::CreateFramebuffers(int w, int h)
{
	// delete old
	if (fbo)  glDeleteFramebuffers(1, &fbo);
	if (fbc)  glDeleteTextures(1, &fbc);
	if (rbo)  glDeleteRenderbuffers(1, &rbo);

	if (fboMSAA) glDeleteFramebuffers(1, &fboMSAA);
	if (rboColorMSAA) glDeleteRenderbuffers(1, &rboColorMSAA);
	if (rboDepthStencilMSAA) glDeleteRenderbuffers(1, &rboDepthStencilMSAA);

	fbo = fbc = rbo = 0;
	fboMSAA = rboColorMSAA = rboDepthStencilMSAA = 0;

	dyshFbo = 0;
	dyshFbc = 0;
	dyshRbo = 0;

	// resolved FBO (texture)
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &fbc);
	glBindTexture(GL_TEXTURE_2D, fbc);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbc, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Resolved FBO incomplete\n";
		while (true);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// MSAA FBO (renderbuffers)
	if (g_fMsaa)
	{
		GLint maxSamples = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
		g_msaaSamples = std::max(1, std::min(g_msaaSamples, maxSamples));

		glGenFramebuffers(1, &fboMSAA);
		glBindFramebuffer(GL_FRAMEBUFFER, fboMSAA);

		glGenRenderbuffers(1, &rboColorMSAA);
		glBindRenderbuffer(GL_RENDERBUFFER, rboColorMSAA);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_RGBA8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboColorMSAA);

		glGenRenderbuffers(1, &rboDepthStencilMSAA);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencilMSAA);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_DEPTH24_STENCIL8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencilMSAA);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "MSAA FBO incomplete\n";
			while (true);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	dyshWidth  = 1024;
	dyshHeight = 1024;

	glGenFramebuffers(1, &dyshFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, dyshFbo);

	glGenRenderbuffers(1, &dyshRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, dyshRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, dyshWidth, dyshHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, dyshRbo);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void GL::ResizeFramebuffers(int w, int h)
{
	int fbW = std::max(1, w);
	int fbH = std::max(1, h);

	// Resizing the off-screen scene target must not change the logical UI
	// canvas. width/height track the output framebuffer for HUD/blot layout.
	renderWidth = fbW;
	renderHeight = fbH;

	if (fbc)
	{
		glBindTexture(GL_TEXTURE_2D, fbc);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fbW, fbH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	}

	if (rbo)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbW, fbH);
	}

	if (fbo)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Resolved FBO incomplete after resize: 0x" << std::hex << status << std::dec << "\n";
			while (true) {}
		}
	}

	if (g_fMsaa && fboMSAA)
	{
		GLint maxSamples = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
		g_msaaSamples = std::max(1, std::min(g_msaaSamples, maxSamples));

		if (rboColorMSAA)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, rboColorMSAA);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_RGBA8, fbW, fbH);
		}

		if (rboDepthStencilMSAA)
		{
			glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencilMSAA);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_DEPTH24_STENCIL8, fbW, fbH);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fboMSAA);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "MSAA FBO incomplete after resize: 0x" << std::hex << status << std::dec << "\n";
			while (true) {}
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void GL::UpdateGLProjections()
{
	const float windowAspect = static_cast<float>(width) / static_cast<float>(height);

	float scaleX = 1.0f;
	float scaleY = 1.0f;

	if (aspectMode == FitToScreen)
	{
		screenProjection = glm::mat4(1.0f);
	}

	else if (windowAspect > aspectRatio) {
		scaleX = aspectRatio / windowAspect;
		screenProjection = glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, 1.0f, 1.0f));
	}
	else {
		scaleY = windowAspect / aspectRatio;
		screenProjection = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, scaleY, 1.0f));
	}
}

void GL::TerminateGL()
{
	if (whiteHandle)
	{
		glMakeTextureHandleNonResidentARB(whiteHandle);
		whiteHandle = 0;
	}

	if (whiteTex)
	{
		glDeleteTextures(1, &whiteTex);
		whiteTex = 0;
	}
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &fbc);
	glDeleteRenderbuffers(1, &rbo);
	
	glDeleteFramebuffers(1, &fboMSAA);
	glDeleteTextures(1, &rboColorMSAA);
	glDeleteRenderbuffers(1, &rboDepthStencilMSAA);

	glDeleteFramebuffers(1, &dyshFbo);
	glDeleteTextures(1, &dyshFbc);
	glDeleteRenderbuffers(1, &dyshRbo);

	glDeleteVertexArrays(1, &sao);
	glDeleteBuffers(1, &sbo);
	glDeleteVertexArrays(1, &gao);
	glDeleteBuffers(1, &gbo);
	glDeleteBuffers(1, &geo);

	glDeleteVertexArrays(1, &blipVao);
	glDeleteBuffers(1, &blipVbo);
	glDeleteBuffers(1, &blipEbo);
	glDeleteBuffers(1, &blipInstanceVbo);
	glDeleteVertexArrays(1, &sqtrVao);
	glDeleteBuffers(1, &sqtrVbo);

	glDeleteVertexArrays(1, &lineVao);
	glDeleteBuffers(1, &lineVbo);

	DeleteFrameStream(&ropStream);
	DeleteFrameStream(&rcbStream);
	DeleteFrameStream(&blipStream);
	glDeleteBuffers(1, &geomUBO);
	glDeleteBuffers(1, &cmUBO);

	glScreenShader.Delete();
	glGlobShader.Delete();
	glCelBorderShader.Delete();
	glGeomShader.Delete();
	glBlotShader.Delete();
	glBlipShader.Delete();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void InitCameraUbo()
{
	// Camera UBO
	glGenBuffers(1, &cmUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CMGL), nullptr, GL_DYNAMIC_DRAW);

	GLuint CMGLblockIndex = glGetUniformBlockIndex(glGlobShader.ID, "CMGL");
	glUniformBlockBinding(glGlobShader.ID, CMGLblockIndex, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cmUBO);
}

void InitFrameStream(STREAM* pstream, int streamSize)
{
	if (!pstream->fences)
		pstream->fences = (GLsync*)calloc(g_frames, sizeof(GLsync));

	if (pstream->bufferObject == 0)
		glGenBuffers(1, &pstream->bufferObject);

	glBindBuffer(GL_UNIFORM_BUFFER, pstream->bufferObject);

	GLsizeiptr perFrameBytes = (GLsizeiptr)streamSize * pstream->stride;
	GLsizeiptr totalSize = (GLsizeiptr)g_frames * perFrameBytes;

	glBufferData(GL_UNIFORM_BUFFER, totalSize, nullptr, GL_STREAM_DRAW);

	pstream->frameBase = 0;
	pstream->cursor    = 0;
	pstream->frameSize = perFrameBytes;
}

void BeginFrameStream(STREAM* pstream)
{
	int slot = g_cframe % g_frames;

	GLsync s = pstream->fences[slot];
	if (s)
	{
		// poll
		GLenum r = glClientWaitSync(s, 0, 0);
		if (r == GL_TIMEOUT_EXPIRED)
		{
			// only stall if we have to
			glClientWaitSync(s, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
		}
		glDeleteSync(s);
		pstream->fences[slot] = nullptr;
	}

	pstream->frameBase = (GLsizeiptr)slot * pstream->frameSize;
	pstream->cursor = 0;

	glBindBuffer(GL_UNIFORM_BUFFER, pstream->bufferObject);
	pstream->mappedPtr = (uint8_t*)glMapBufferRange(GL_UNIFORM_BUFFER, pstream->frameBase, pstream->frameSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
}

void AppendStream(STREAM* pstream, void* ptr, int size, int copySize)
{
	if (pstream->cursor + pstream->stride > pstream->frameSize)
	{
		__debugbreak(); // overflow protection
		return;
	}

	GLsizeiptr off = pstream->frameBase + pstream->cursor;
	pstream->cursor += pstream->stride;

	memcpy(pstream->mappedPtr + (off - pstream->frameBase), ptr, copySize);

	glBindBufferRange(GL_UNIFORM_BUFFER, pstream->bindIndex, pstream->bufferObject, off, size);
}

void EndFrameStream(STREAM* pstream)
{
	if (!pstream->mappedPtr)
		return;

	int slot = g_cframe % g_frames;

	glBindBuffer(GL_UNIFORM_BUFFER, pstream->bufferObject);
	glUnmapBuffer(GL_UNIFORM_BUFFER);

	pstream->mappedPtr = nullptr;

	pstream->fences[slot] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void DeleteFrameStream(STREAM* pstream)
{
	if (pstream->fences)
	{
		for (int i = 0; i < g_frames; i++)
		{
			if (pstream->fences[i])
				glDeleteSync(pstream->fences[i]);
		}

		free(pstream->fences);
		pstream->fences = nullptr;
	}

	if (pstream->bufferObject)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, pstream->bufferObject);

		if (pstream->mappedPtr)
		{
			glUnmapBuffer(GL_UNIFORM_BUFFER);
			pstream->mappedPtr = nullptr;
		}

		glDeleteBuffers(1, &pstream->bufferObject);
		pstream->bufferObject = 0;
	}

	pstream->frameBase = 0;
	pstream->cursor    = 0;
	pstream->frameSize = 0;
}

void ApplyMsaaSettings()
{
	if (g_gl.fboMSAA)
		glDeleteFramebuffers(1, &g_gl.fboMSAA);
	if (g_gl.rboColorMSAA)
		glDeleteRenderbuffers(1, &g_gl.rboColorMSAA);
	if (g_gl.rboDepthStencilMSAA)
		glDeleteRenderbuffers(1, &g_gl.rboDepthStencilMSAA);

	g_gl.fboMSAA = 0;
	g_gl.rboColorMSAA = 0;
	g_gl.rboDepthStencilMSAA = 0;

	if (!g_fMsaa)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	// MSAA storage must match the off-screen scene target, not the UI canvas.
	int w = g_gl.renderWidth;
	int h = g_gl.renderHeight;
	w = std::max(w, 1);
	h = std::max(h, 1);

	GLint maxSamples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	g_msaaSamples = std::max(1, std::min(g_msaaSamples, maxSamples));

	glGenFramebuffers(1, &g_gl.fboMSAA);
	glBindFramebuffer(GL_FRAMEBUFFER, g_gl.fboMSAA);

	glGenRenderbuffers(1, &g_gl.rboColorMSAA);
	glBindRenderbuffer(GL_RENDERBUFFER, g_gl.rboColorMSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_RGBA8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, g_gl.rboColorMSAA);

	glGenRenderbuffers(1, &g_gl.rboDepthStencilMSAA);
	glBindRenderbuffer(GL_RENDERBUFFER, g_gl.rboDepthStencilMSAA);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_DEPTH24_STENCIL8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, g_gl.rboDepthStencilMSAA);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "MSAA FBO incomplete\n";
		while (true);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ApplyInternalResolutionSettings()
{
	int width = 1;
	int height = 1;
	glfwGetFramebufferSize(g_gl.window, &width, &height);
	FrameBufferSizeCallBack(g_gl.window, width, height);
}

void ApplyWindowModeSettings(WindowMode mode)
{
	if (g_gl.window == nullptr || mode == g_windowMode)
		return;

	GLFWmonitor* monitor = MonitorForWindow(g_gl.window);
	if (monitor == nullptr)
		return;

	if (g_windowMode == WindowMode_Windowed)
	{
		glfwGetWindowPos(g_gl.window, &s_windowedX, &s_windowedY);
		glfwGetWindowSize(g_gl.window, &s_windowedWidth, &s_windowedHeight);
	}

	if (mode == WindowMode_Windowed)
	{
		glfwSetWindowAttrib(g_gl.window, GLFW_DECORATED, GLFW_TRUE);
		glfwSetWindowMonitor(g_gl.window, nullptr, s_windowedX, s_windowedY,
			(std::max)(s_windowedWidth, 320), (std::max)(s_windowedHeight, 240), GLFW_DONT_CARE);
	}
	else if (mode == WindowMode_Borderless)
	{
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
		glfwSetWindowAttrib(g_gl.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowMonitor(g_gl.window, nullptr, x, y, width, height, GLFW_DONT_CARE);
	}
	else
	{
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
		if (videoMode == nullptr)
			return;
		glfwSetWindowAttrib(g_gl.window, GLFW_DECORATED, GLFW_TRUE);
		glfwSetWindowMonitor(g_gl.window, monitor, 0, 0,
			videoMode->width, videoMode->height, videoMode->refreshRate);
	}

	g_windowMode = mode;
	glfwSwapInterval(g_fVsync ? 1 : 0);
}

void ApplyAspectRatioSettings(AspectMode mode)
{
	g_gl.aspectMode = mode;
	switch (mode)
	{
		case Fixed_4_3: g_gl.aspectRatio = 4.0f / 3.0f; break;
		case Fixed_16_10: g_gl.aspectRatio = 16.0f / 10.0f; break;
		case Fixed_16_9: g_gl.aspectRatio = 16.0f / 9.0f; break;
		case FitToScreen:
		default: break;
	}
	ApplyInternalResolutionSettings();
}

void FrameBufferSizeCallBack(GLFWwindow* window, int width, int height)
{
	constexpr float kVirtualUiWidth = 640.0f;
	constexpr float kVirtualUiHeight = 492.80002f;

	float imguiOffset = 0.0f;
	if (g_fDebugMode != 0 && ImGui::GetCurrentContext() != nullptr)
	{
		float contentScaleX = 1.0f;
		float contentScaleY = 1.0f;
		glfwGetWindowContentScale(window, &contentScaleX, &contentScaleY);
		imguiOffset = ImGui::GetFrameHeight() * contentScaleY;
	}

	const int outputW = std::max(1, width);
	const int outputH = std::max(1, int(std::ceil(float(height) - imguiOffset)));
	float targetAspect = static_cast<float>(outputW) / static_cast<float>(outputH);
	if (g_gl.aspectMode == Fixed_4_3)
		targetAspect = 4.0f / 3.0f;
	else if (g_gl.aspectMode == Fixed_16_9)
		targetAspect = 16.0f / 9.0f;
	else if (g_gl.aspectMode == Fixed_16_10)
		targetAspect = 16.0f / 10.0f;
	g_gl.aspectRatio = targetAspect;
	int presentW = outputW;
	int presentH = std::max(1, static_cast<int>(std::lround(outputW / targetAspect)));
	if (presentH > outputH)
	{
		presentH = outputH;
		presentW = std::max(1, static_cast<int>(std::lround(outputH * targetAspect)));
	}

	int sceneW = presentW;
	int sceneH = presentH;

	if (g_internalResolutionHeight > 0)
	{
		sceneH = g_internalResolutionHeight;
		sceneW = std::max(1, static_cast<int>(std::lround(
			static_cast<double>(sceneH) * static_cast<double>(presentW) /
			static_cast<double>(presentH))));
	}

	// UI and HUD layout always use the actual output size. Only the scene
	// framebuffer changes with the internal-resolution option.
	g_gl.width  = static_cast<float>(presentW);
	g_gl.height = static_cast<float>(presentH);
	g_gl.renderWidth = sceneW;
	g_gl.renderHeight = sceneH;
	g_gl.outputWidth = outputW;
	g_gl.outputHeight = outputH;
	g_gl.presentX = (outputW - presentW) / 2;
	g_gl.presentY = (outputH - presentH) / 2;
	g_gl.presentWidth = presentW;
	g_gl.presentHeight = presentH;

	// Preserve the PS2 UI aspect ratio and center the virtual canvas.  UI
	// renderers can now share one resize result instead of independently
	// stretching X and Y or mixing virtual and framebuffer coordinates.
	g_gl.uiScale = std::min(
		static_cast<float>(presentW) / kVirtualUiWidth,
		static_cast<float>(presentH) / kVirtualUiHeight);
	g_gl.uiOrigin.x = (static_cast<float>(presentW) - kVirtualUiWidth * g_gl.uiScale) * 0.5f;
	g_gl.uiOrigin.y = (static_cast<float>(presentH) - kVirtualUiHeight * g_gl.uiScale) * 0.5f;

	g_gl.ResizeFramebuffers(sceneW, sceneH);

	glViewport(0, 0, sceneW, sceneH);

	g_gl.UpdateGLProjections();

	g_gl.blotProjection = glm::ortho(0.0f, float(presentW), float(presentH), 0.0f, -1.0f, 1.0f);

	glBlotShader.Use();
	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

	if (g_pcm != nullptr)
		RecalcCm(g_pcm);

	ResizeTvUiForFramebuffer();
	RepositionAllBlots();
}

GL g_gl;
GLuint cmUBO = 0;
STREAM ropStream;
STREAM rcbStream;
STREAM blipStream;
GLuint geomUBO = 0;
GLint glslBlipCurrentTime = -1;
GLint glslBlipDtFrame = -1;
GLint glslBlipCameraMat = -1;
GLint glslBlipAlphaPass = -1;
GLuint glslLsmShadow = 0;
GLuint glslLsmDiffuse = 0;
GLuint glslFogType = 0;
GLuint glslFogNear = 0;
GLuint glslFogFar = 0;
GLuint glslFogMax = 0;
GLuint glslFogColor = 0;
GLuint glslfAlphaTest = 0;
GLuint glslAlphaCutOff = 0;
GLuint glslRko = 0;
GLuint glslfAnimateUv = 0;
GLuint glsluvOffsets = 0;
GLuint glslUnSelfIllum = 0;
GLuint glslSubGlobPosCenter = 0;
GLuint glslSubGlobRadius = 0;
GLuint glslDyshMatWorldClip = 0;
GLuint glslDyshModel = 0;
GLuint glslDyshfSkin = 0;
GLuint glslAmbientMap = 0;
GLuint glslDiffuseMap = 0;
GLuint glslSaturateMap = 0;
GLuint glslGeomModelToClip = 0;
GLuint u_fontTexLoc = 0;
GLuint glslfCull = 0;
uint64_t screenTextureHandle = 0;
GLuint g_sceneFbo = 0;
int g_msaaSamples = 4;
bool g_fMsaa = false;
int g_internalResolutionHeight = 0;
WindowMode g_windowMode = WindowMode_Windowed;
float g_drawDistanceMultiplier = 1.0f;
int g_frames = 3;
float s_dxDisplay;
float s_dyDisplay;
float s_dxDisplayOriginal;
float s_dyDisplayOriginal;
GLuint lineVao = 0;
GLuint lineVbo = 0;
GLuint glslfSkin = 0;
GLint glslCelSkin = 0;
GLuint glslfPose = 0;
GLuint glslPoseCount = 0;
GLuint glslPoseWeights = 0;
GLuint glslCelPose = 0;
GLuint glslCelPoseCount = 0;
GLuint glslCelPoseWeights = 0;
