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

	glslLsmShadow  = glGetUniformLocation(glGlobShader.ID, "swp.uShadow");
	glslLsmDiffuse = glGetUniformLocation(glGlobShader.ID, "swp.uMidtone");
	glslFogType    = glGetUniformLocation(glGlobShader.ID, "swp.fogType");
	glslFogNear    = glGetUniformLocation(glGlobShader.ID, "swp.fogNear");
	glslFogFar     = glGetUniformLocation(glGlobShader.ID, "swp.fogFar");
	glslFogMax     = glGetUniformLocation(glGlobShader.ID, "swp.fogMax");
	glslFogColor   = glGetUniformLocation(glGlobShader.ID, "swp.fogColor");

	glslfAlphaTest    = glGetUniformLocation(glGlobShader.ID, "fAlphaTest");

	glUniform1i(glGetUniformLocation(glGlobShader.ID, "shadowMap"),   0);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "diffuseMap"),  1);
	glUniform1i(glGetUniformLocation(glGlobShader.ID, "saturateMap"), 2);

	glCelBorderShader.Init("celborder.vert", NULL, "celborder.frag");
	glGeomShader.Init("geom.vert", NULL, "geom.frag");

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
	glDeleteBuffers(1, &ropUBO);
	glDeleteBuffers(1, &rcbUBO);
	glDeleteBuffers(1, &geomUBO);
	glDeleteBuffers(1, &cmSSBO);
	glDeleteBuffers(1, &activeLightsSbo);

	glScreenShader.Delete();
	glGlobShader.Delete();
	glCelBorderShader.Delete();
	glGeomShader.Delete();
	glBlotShader.Delete();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void InitCameraSbo()
{
	// Camera SBO
	glGenBuffers(1, &cmSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, cmSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CMGL), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cmSSBO);
}

void InitRopUbo()
{
	// Render Object UBO
	glGenBuffers(1, &ropUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, ropUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ROTHREEWAY), nullptr, GL_DYNAMIC_DRAW);

	GLuint RoblockIndex = glGetUniformBlockIndex(glGlobShader.ID, "RO");
	glUniformBlockBinding(glGlobShader.ID, RoblockIndex, 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, ropUBO);
}

void InitRcbUbo()
{
	// Render Cel Object UBO
	glGenBuffers(1, &rcbUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, rcbUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ROCEL), nullptr, GL_DYNAMIC_DRAW);

	GLuint RCBblockIndex = glGetUniformBlockIndex(glCelBorderShader.ID, "ROCEL");
	glUniformBlockBinding(glCelBorderShader.ID, RCBblockIndex, 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, rcbUBO);
}

void InitGeomUbo()
{
	// Render GEOM object UBO
	glGeomShader.Use();
	glGenBuffers(1, &geomUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, geomUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ROGEOM), nullptr, GL_DYNAMIC_DRAW);

	GLuint ROGeomblockIndex = glGetUniformBlockIndex(glGeomShader.ID, "ROGEOM");
	glUniformBlockBinding(glGeomShader.ID, ROGeomblockIndex, 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, geomUBO);
}

void FrameBufferSizeCallBack(GLFWwindow *window, int width, int height)
{
	float imguiOffset = ImGui::GetFrameHeight();

	g_gl.width  = width;
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
GLuint cmSSBO = 0;
GLuint ropUBO = 0;
GLuint rcbUBO = 0;
GLuint geomUBO = 0; 
GLuint activeLightsSbo = 0;
GLuint screenQuadMatrixLoc = 0;
GLuint glslLsmShadow = 0;
GLuint glslLsmDiffuse = 0;
GLuint glslFogType = 0;
GLuint glslFogNear = 0;
GLuint glslFogFar = 0;
GLuint glslFogMax = 0;
GLuint glslFogColor = 0;
GLuint glslfAlphaTest = 0;
GLuint glslfCull = 0;