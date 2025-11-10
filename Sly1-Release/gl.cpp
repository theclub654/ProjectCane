#include "gl.h"

#ifdef _WIN32
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int NvidiaPowerXpressRequestHighPerformance = 1;
}
#endif

void GL::InitGL()
{
	// Create GLFW context and window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	width  = 800;
	height = 800;

	aspectRatio = width / height;
	aspectMode  = FitToScreen;

	window = glfwCreateWindow(width, height, "Sly 1", nullptr, nullptr);

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

	float imguiOffset = ImGui::GetFrameHeight(); // or use a cached value after rendering ImGui menu

	// ========== Framebuffer setup ==========
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &fbc);
	glBindTexture(GL_TEXTURE_2D, fbc);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height - imguiOffset, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbc, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height - imguiOffset);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER::Framebuffer is not complete!" << std::endl;
		while (true);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
	screenQuadMatrixLoc = glGetUniformLocation(glScreenShader.ID, "u_model");
	glUniformMatrix4fv(screenQuadMatrixLoc, 1, GL_FALSE, glm::value_ptr(g_gl.screenProjection));
	glUniform1i(glGetUniformLocation(glScreenShader.ID, "screenTexture"), 0);

	glGlobShader.Init("glob.vert", NULL, "glob.frag");
	glGlobShader.Use();

	GLuint blockIndex;

	// Camera UBO
	glGenBuffers(1, &cmUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(CMGL), nullptr, GL_DYNAMIC_DRAW);

	blockIndex = glGetUniformBlockIndex(glGlobShader.ID, "CMGL");
	glUniformBlockBinding(glGlobShader.ID, blockIndex, 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, cmUBO);

	// Render Object UBO
	glGenBuffers(1, &ropUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, ropUBO);
	glBufferData(GL_UNIFORM_BUFFER, roSize, nullptr, GL_DYNAMIC_DRAW);

	blockIndex = glGetUniformBlockIndex(glGlobShader.ID, "RO");
	glUniformBlockBinding(glGlobShader.ID, blockIndex, 2);
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, ropUBO);
	
	// Active Lights
	GLuint idxActive = glGetUniformBlockIndex(glGlobShader.ID, "ACTIVELIGHTS");
	glUniformBlockBinding(glGlobShader.ID, idxActive, 3);

	glGenBuffers(1, &alUbo);
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, alUbo);

	glslLsmShadow  = glGetUniformLocation(glGlobShader.ID, "swp.uShadow");
	glslLsmDiffuse = glGetUniformLocation(glGlobShader.ID, "swp.uMidtone");
	glslFogType    = glGetUniformLocation(glGlobShader.ID, "swp.fogType");
	glslFogNear    = glGetUniformLocation(glGlobShader.ID, "swp.fogNear");
	glslFogFar     = glGetUniformLocation(glGlobShader.ID, "swp.fogFar");
	glslFogMax     = glGetUniformLocation(glGlobShader.ID, "swp.fogMax");
	glslFogColor   = glGetUniformLocation(glGlobShader.ID, "swp.fogColor");

	glslRgbaCel		  = glGetUniformLocation(glGlobShader.ID, "rgbaCel");
	glslfAlphaTest    = glGetUniformLocation(glGlobShader.ID, "fAlphaTest");
	glslCollisionRgba = glGetUniformLocation(glGlobShader.ID, "collisionRgba");
	glUniform4fv(glslCollisionRgba, 1, glm::value_ptr(glm::vec4(0.76, 0.76, 0.76, 1.0)));

	glUniform1i(glGetUniformLocation(glGlobShader.ID, "shadowMap"),   0);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "diffuseMap"),  1);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "saturateMap"), 2);

	glGenVertexArrays(1, &gEmptyVAO);

	glBlotShader.Init("blot.vert", NULL, "blot.frag");
	glBlotShader.Use();
	glUniform1i(glGetUniformLocation(glBlotShader.ID, "u_fontTex"), 0);
	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

	u_projectionLoc = glGetUniformLocation(glBlotShader.ID, "u_projection");
	u_modelLoc      = glGetUniformLocation(glBlotShader.ID, "u_model");
	uvRectLoc       = glGetUniformLocation(glBlotShader.ID, "u_uvRect");
	blotColorLoc    = glGetUniformLocation(glBlotShader.ID, "blotColor");

	glDepthMask(GL_TRUE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}

void GL::UpdateGLProjections()
{
	float windowAspect = width / height;

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

	// Set screen quad model matrix for aspect ratio scaling
	glUniformMatrix4fv(screenQuadMatrixLoc, 1, GL_FALSE, glm::value_ptr(g_gl.screenProjection));
}

void GL::TerminateGL()
{
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &fbc);
	glDeleteRenderbuffers(1, &rbo);
	glDeleteVertexArrays(1, &sao);
	glDeleteBuffers(1, &sbo);
	glDeleteVertexArrays(1, &gao);
	glDeleteBuffers(1, &gbo);
	glDeleteVertexArrays(1, &gEmptyVAO);
	glDeleteBuffers(1, &ropUBO);
	glDeleteBuffers(1, &cmUBO);
	glDeleteBuffers(1, &alUbo);

	glScreenShader.Delete();
	glGlobShader.Delete();
	glBlotShader.Delete();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void FrameBufferSizeCallBack(GLFWwindow *window, int width, int height)
{
	float imguiOffset = ImGui::GetFrameHeight();

	g_gl.width = width;
	g_gl.height = height - imguiOffset;

	// Resize framebuffer attachments
	glBindFramebuffer(GL_FRAMEBUFFER, g_gl.fbo);

	glBindTexture(GL_TEXTURE_2D, g_gl.fbc);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height - imguiOffset, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_gl.fbc, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, g_gl.rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height - imguiOffset);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, g_gl.rbo);

	glViewport(0, 0, width, height - imguiOffset);

	g_gl.UpdateGLProjections();

	g_gl.blotProjection = glm::ortho(0.0f, float(width), float(height - imguiOffset), 0.0f, -1.0f, 1.0f);

	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

	if (g_pcm != nullptr)
		RecalcCm(g_pcm);

	BuildBinocBackGround(&g_binoc);
	BuildBinocOutline(&g_binoc);

	RepositionAllBlots();
}

GL g_gl;
GLuint cmUBO = 0;
GLuint ropUBO = 0;
GLuint alUbo = 0;
GLuint roSize = sizeof(RO) - 16;
GLuint roCelSize = sizeof(RO) - 56;
GLuint roCollisionSize = sizeof(RO) - 60;
GLuint screenQuadMatrixLoc = 0;
GLuint glslLsmShadow = 0;
GLuint glslLsmDiffuse = 0;
GLuint glslFogType = 0;
GLuint glslFogNear = 0;
GLuint glslFogFar = 0;
GLuint glslFogMax = 0;
GLuint glslFogColor = 0;
GLuint glslRgbaCel = 0;
GLuint glslfAlphaTest = 0;
GLuint glslfCull = 0;
GLuint glslCollisionRgba = 0;
GLuint gEmptyVAO = 0;