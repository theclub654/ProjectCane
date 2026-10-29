#include "gl.h"
#include "render.h"
#include "sqtr.h"
#include "shadow.h"
#include "tv.h"
#include "game.h"
#include "debug.h"
#include "runtime_error.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#ifdef __APPLE__
#include <string>
#include "../platform/platform.h"

// AppKit through the Objective-C runtime's C API. objc's own headers can't be
// included here: their BOOL clashes with the game's (spaprops.h).
struct objc_object;
struct objc_selector;
extern "C" objc_object* objc_getClass(const char* name);
extern "C" objc_selector* sel_registerName(const char* name);
extern "C" void objc_msgSend(void);
#endif

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

#ifdef _WIN32
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace
{
int s_windowedX = 100;
int s_windowedY = 100;
int s_windowedWidth = 800;
int s_windowedHeight = 800;
bool s_lastFramebufferResizeSucceeded = true;
double s_autoIconifySuppressedUntil = 0.0;
#ifdef __APPLE__

// Hides the menu bar and the Dock while the game is the active app
// (NSApp.presentationOptions = NSApplicationPresentationHideDock |
// NSApplicationPresentationHideMenuBar); macOS shows them again by itself
// whenever another app is active.
void SetMenuBarAndDockHidden(bool hidden)
{
	objc_object* app = reinterpret_cast<objc_object* (*)(objc_object*, objc_selector*)>(objc_msgSend)(
		objc_getClass("NSApplication"), sel_registerName("sharedApplication"));
	if (app == nullptr)
		return;
	const unsigned long options = hidden ? (1ul << 1) | (1ul << 3) : 0ul;
	reinterpret_cast<void (*)(objc_object*, objc_selector*, unsigned long)>(objc_msgSend)(
		app, sel_registerName("setPresentationOptions:"), options);
}
#endif

GLFWmonitor* MonitorByIndex(int index)
{
	int count = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	if (monitors == nullptr || count == 0)
		return glfwGetPrimaryMonitor();
	return monitors[(std::clamp)(index, 0, count - 1)];
}

void GlfwErrorCallback(int error, const char* description)
{
	ReportRuntimeError("Graphics", "GLFW error %d: %s", error,
		description != nullptr ? description : "No description was provided");
}

void WindowFocusCallback(GLFWwindow* window, int focused)
{
	if (focused == GLFW_FALSE && g_fAutoIconify &&
		glfwGetTime() >= s_autoIconifySuppressedUntil &&
		glfwGetWindowAttrib(window, GLFW_ICONIFIED) == GLFW_FALSE)
	{
		// GLFW_AUTO_ICONIFY only covers exclusive fullscreen windows. Apply the
		// user-facing setting explicitly so windowed and borderless modes behave
		// the same way when the game loses focus.
		glfwIconifyWindow(window);
	}
	else if (focused == GLFW_TRUE)
	{
		// A minimized GLFW window can report a zero-sized drawable. Windows does
		// not consistently follow focus restoration with another framebuffer-size
		// callback for borderless windows, so explicitly restore the render targets
		// from the current drawable dimensions.
		int framebufferWidth = 0;
		int framebufferHeight = 0;
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
		if (framebufferWidth > 0 && framebufferHeight > 0)
			FrameBufferSizeCallBack(window, framebufferWidth, framebufferHeight);
	}
}

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
	glfwSetErrorCallback(GlfwErrorCallback);
	if (glfwInit() != GLFW_TRUE)
		throw std::runtime_error("GLFW could not initialize. See the diagnostic console for details.");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#ifdef CANE_GL41
	// macOS caps OpenGL at 4.1 and only grants core contexts that are forward compatible.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_AUTO_ICONIFY, g_fAutoIconify ? GLFW_TRUE : GLFW_FALSE);

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
		const char* description = nullptr;
		const int error = glfwGetError(&description);
#ifdef CANE_GL41
		ReportRuntimeError("Graphics", "Could not create the OpenGL 4.1 window (GLFW error %d: %s)",
			error, description != nullptr ? description : "No description was provided");
		glfwTerminate();
		throw std::runtime_error(
			"The game window could not be created. Update the graphics driver and verify OpenGL 4.1 support.");
#else
		ReportRuntimeError("Graphics", "Could not create the OpenGL 4.3 window (GLFW error %d: %s)",
			error, description != nullptr ? description : "No description was provided");
		glfwTerminate();
		throw std::runtime_error(
			"The game window could not be created. Update the graphics driver and verify OpenGL 4.3 support.");
#endif
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallBack);
	glfwSetWindowFocusCallback(window, WindowFocusCallback);
	glfwSetCursorPosCallback(window, MOUSE::CursorPosCallback);
	glfwSetMouseButtonCallback(window, MOUSE::MouseButtonCallback);
	glfwSetScrollCallback(window, MOUSE::MouseWheelCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
#ifdef CANE_GL41
		ReportRuntimeError("Graphics",
			"GLAD could not load the required OpenGL functions. Update the graphics driver and verify OpenGL 4.1 support.");
#else
		ReportRuntimeError("Graphics",
			"GLAD could not load the required OpenGL functions. Update the graphics driver and verify OpenGL 4.3 support.");
#endif
		throw std::runtime_error("OpenGL function loading failed.");
	}

#ifdef CANE_GL41
	// glad only loads glTexStorage2D/3D for GL 4.2+; the 4.1 driver still exports them.
	if (!LoadTexStorageFallback((GLADloadproc)glfwGetProcAddress))
	{
		ReportRuntimeError("Graphics",
			"glTexStorage2D/glTexStorage3D are unavailable on this OpenGL 4.1 driver (GL_ARB_texture_storage is required).");
		throw std::runtime_error("OpenGL function loading failed.");
	}

#endif
	// GLFW window dimensions are logical screen coordinates and can differ
	// from the drawable pixel dimensions on a DPI-scaled display. Initialize
	// every framebuffer-related dimension from the actual drawable size so the
	// first frame does not depend on receiving a later resize callback.
	int framebufferWidth = 0;
	int framebufferHeight = 0;
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	framebufferWidth = std::max(1, framebufferWidth);
	framebufferHeight = std::max(1, framebufferHeight);
	width = static_cast<float>(framebufferWidth);
	height = static_cast<float>(framebufferHeight);
	renderWidth = framebufferWidth;
	renderHeight = framebufferHeight;
	outputWidth = framebufferWidth;
	outputHeight = framebufferHeight;
	presentX = 0;
	presentY = 0;
	presentWidth = framebufferWidth;
	presentHeight = framebufferHeight;
	aspectRatio = width / height;

	const char* glVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
	const char* glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
	const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
	std::cout << "OpenGL vendor: " << (glVendor != nullptr ? glVendor : "unknown") << '\n'
		<< "OpenGL renderer: " << (glRenderer != nullptr ? glRenderer : "unknown") << '\n'
		<< "OpenGL version: " << (glVersion != nullptr ? glVersion : "unknown") << std::endl;

#ifdef CANE_GL41
	// GL 4.1 has no SSBOs, so the SSBO limit queries below would be
	// GL_INVALID_ENUM and read 0; the 4.1 renderer reads buffer textures instead.
#else
	// The main renderer intentionally uses vertex-stage SSBOs for lights,
	// shadows, skinning, poses, warps, and cached lighting. OpenGL 4.3 exposes
	// implementation-dependent per-stage limits, so reject an incapable driver
	// explicitly instead of letting glob.vert fail later with a vendor-specific
	// linker message or silently missing effects.
	GLint maxVertexSsboBlocks = 0;
	GLint maxCombinedSsboBlocks = 0;
	GLint maxSsboBindings = 0;
	GLint maxVertexOutputComponents = 0;
	glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &maxVertexSsboBlocks);
	glGetIntegerv(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS, &maxCombinedSsboBlocks);
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxSsboBindings);
	glGetIntegerv(GL_MAX_VERTEX_OUTPUT_COMPONENTS, &maxVertexOutputComponents);

	constexpr GLint kRequiredVertexSsboBlocks = 10;
	constexpr GLint kRequiredCombinedSsboBlocks = 11;
	constexpr GLint kRequiredSsboBindings = 14;
	constexpr GLint kRequiredVertexOutputComponents = 48;
	if (maxVertexSsboBlocks < kRequiredVertexSsboBlocks ||
		maxCombinedSsboBlocks < kRequiredCombinedSsboBlocks ||
		maxSsboBindings < kRequiredSsboBindings ||
		maxVertexOutputComponents < kRequiredVertexOutputComponents)
	{
		ReportRuntimeError("Graphics",
			"GPU shader limits are below ProjectCane's renderer requirements. "
			"Vertex SSBOs %d/%d, combined SSBOs %d/%d, SSBO bindings %d/%d, "
			"vertex outputs %d/%d. Update the graphics driver or select a more capable GPU.",
			maxVertexSsboBlocks, kRequiredVertexSsboBlocks,
			maxCombinedSsboBlocks, kRequiredCombinedSsboBlocks,
			maxSsboBindings, kRequiredSsboBindings,
			maxVertexOutputComponents, kRequiredVertexOutputComponents);
		throw std::runtime_error("GPU shader resource limits are insufficient.");
	}
#endif

	// ========== ImGui Setup ==========
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
#ifdef __APPLE__
	// ImGui saves imgui.ini relative to the working directory, which for an
	// app started from Finder is / and could also be inside ProjectCane.app,
	// breaking its signature. Keep it with the saves and settings instead
	// (ConfigureRuntime has created that folder). ImGui keeps the pointer,
	// so the string lives for the rest of the program.
	static const std::string s_imguiIniPath = cane::Utf8(cane::UserDirectory() / "imgui.ini");
	io.IniFilename = s_imguiIniPath.c_str();
#endif
	ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef CANE_GL41
	ImGui_ImplOpenGL3_Init("#version 410");
#else
	ImGui_ImplOpenGL3_Init("#version 430");
#endif
	ImGui::StyleColorsDark();

	const float imguiOffset = g_fDebugGuiOpen ? ImGui::GetFrameHeight() : 0.0f;
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

	// ========== Batched font geometry ==========
	fontVboCapacity = sizeof(BLOTLINEVERTEX) * 6 * 256;
	glGenVertexArrays(1, &fontVao);
	glGenBuffers(1, &fontVbo);
	glBindVertexArray(fontVao);
	glBindBuffer(GL_ARRAY_BUFFER, fontVbo);
	glBufferData(GL_ARRAY_BUFFER, fontVboCapacity, nullptr, GL_STREAM_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(BLOTLINEVERTEX),
		reinterpret_cast<void*>(offsetof(BLOTLINEVERTEX, position)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(BLOTLINEVERTEX),
		reinterpret_cast<void*>(offsetof(BLOTLINEVERTEX, uv)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(BLOTLINEVERTEX),
		reinterpret_cast<void*>(offsetof(BLOTLINEVERTEX, color)));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

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
	glUniform1f(glGetUniformLocation(glScreenShader.ID, "outputGamma"), g_outputGamma);

	glDyshadow.Init("dysh.vert", NULL, "dysh.frag");
	glDyshadow.Use();
	
	glslDyshMatWorldClip = glGetUniformLocation(glDyshadow.ID, "matWorldToClip");
	glslDyshModel        = glGetUniformLocation(glDyshadow.ID, "model");
	glslDyshfSkin        = glGetUniformLocation(glDyshadow.ID, "fSkin");

#ifdef CANE_GL41
	// The material cache bake (render.cpp's DrawGlob) captures these.
	glGlobShader.Init("glob.vert", NULL, "glob.frag", kGlobFeedbackVaryings, kGlobFeedbackVaryingCount);
#else
	glGlobShader.Init("glob.vert", NULL, "glob.frag");
#endif
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
	glslProjectedVolumeColor = glGetUniformLocation(glGlobShader.ID, "projectedVolumeColor");
	glslProjectedVolumeFinalPass = glGetUniformLocation(glGlobShader.ID, "projectedVolumeFinalPass");
	glslfAnimateUv       = glGetUniformLocation(glGlobShader.ID, "fAnimateUv");
	glsluvOffsets		 = glGetUniformLocation(glGlobShader.ID, "uvOffsets");
	glslUnSelfIllum		 = glGetUniformLocation(glGlobShader.ID, "unSelfIllum");
	glslSubGlobPosCenter = glGetUniformLocation(glGlobShader.ID, "subGlobPosCenter");
	glslSubGlobRadius    = glGetUniformLocation(glGlobShader.ID, "subGlobRadius");
#ifdef CANE_GL41
	glslfCacheBake       = glGetUniformLocation(glGlobShader.ID, "fCacheBake");
	glUniform1i(glslfCacheBake, 0);
#endif

	glslAmbientMap  = glGetUniformLocation(glGlobShader.ID, "ambientMap");
	glslDiffuseMap  = glGetUniformLocation(glGlobShader.ID, "diffuseMap");
	glslSaturateMap = glGetUniformLocation(glGlobShader.ID, "saturateMap");
	glUniform1i(glslAmbientMap, 0);
	glUniform1i(glslDiffuseMap, 1);
	glUniform1i(glslSaturateMap, 2);

	glUniform1i(glGetUniformLocation(glGlobShader.ID, "shadowTextureArray"), SHADOW_TEXTURE_UNIT);

#ifdef CANE_GL41
	// GL 4.1 core only allows 1 px lines; the geometry stage draws the
	// outlines at their full width instead (see SetCelBorderLineWidth).
	glCelBorderShader.Init("celborder.vert", kCelBorderGeometryShader, "celborder.frag");
	glslCelLineWidthPx = glGetUniformLocation(glCelBorderShader.ID, kCelLineWidthUniform);
	glslCelViewportPx  = glGetUniformLocation(glCelBorderShader.ID, kCelViewportUniform);
	glslCelMultisampled = glGetUniformLocation(glCelBorderShader.ID, kCelMultisampledUniform);
#else
	glCelBorderShader.Init("celborder.vert", NULL, "celborder.frag");
#endif
	rcbStream.bindIndex = 1;
	align = 256;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);

	rcbStream.stride = (sizeof(ROCEL) + align - 1) & ~(align - 1);
	glGeomShader.Init("geom.vert", NULL, "geom.frag");
	glslGeomModelToClip = glGetUniformLocation(glGeomShader.ID, "modelToClip");
	glslGeomDebugColor = glGetUniformLocation(glGeomShader.ID, "debugColor");

	glBlotShader.Init("blot.vert", NULL, "blot.frag");
	glBlotShader.Use();

	u_projectionLoc = glGetUniformLocation(glBlotShader.ID, "u_projection");
	u_modelLoc      = glGetUniformLocation(glBlotShader.ID, "u_model");
	uvRectLoc       = glGetUniformLocation(glBlotShader.ID, "u_uvRect");
	u_useVertexColorLoc = glGetUniformLocation(glBlotShader.ID, "u_useVertexColor");
	glUniform1i(u_useVertexColorLoc, 0);
	blotColorLoc    = glGetUniformLocation(glBlotShader.ID, "blotColor");
	u_fontTexLoc    = glGetUniformLocation(glBlotShader.ID, "u_fontTex");
	glUniform1i(u_fontTexLoc, 0);

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

	glBindTexture(GL_TEXTURE_2D, 0);

	glslDyshfSkin = glGetUniformLocation(glDyshadow.ID, "fSkin");
	glslfSkin     = glGetUniformLocation(glGlobShader.ID, "fSkin");
	glslCelSkin   = glGetUniformLocation(glCelBorderShader.ID, "fSkin");

	glslfPose       = glGetUniformLocation(glGlobShader.ID, "fPose");
	glslPoseCount   = glGetUniformLocation(glGlobShader.ID, "poseCount");

	glslCelPose        = glGetUniformLocation(glCelBorderShader.ID, "fPose");
	glslCelPoseCount   = glGetUniformLocation(glCelBorderShader.ID, "poseCount");
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

	if (dyshFbo) glDeleteFramebuffers(1, &dyshFbo);
	if (dyshFbc) glDeleteTextures(1, &dyshFbc);
	if (dyshRbo) glDeleteRenderbuffers(1, &dyshRbo);

	fbo = fbc = rbo = 0;
	fboMSAA = rboColorMSAA = rboDepthStencilMSAA = 0;
	dyshFbo = dyshFbc = dyshRbo = 0;

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
		ReportRuntimeError("Graphics", "The initial scene framebuffer could not be created.");
		throw std::runtime_error("Initial scene framebuffer creation failed.");
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
			ReportRuntimeError("Graphics",
				"The initial %dx MSAA framebuffer could not be created; MSAA has been disabled.",
				g_msaaSamples);
			glDeleteFramebuffers(1, &fboMSAA);
			glDeleteRenderbuffers(1, &rboColorMSAA);
			glDeleteRenderbuffers(1, &rboDepthStencilMSAA);
			fboMSAA = rboColorMSAA = rboDepthStencilMSAA = 0;
			g_fMsaa = false;
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

	// No color layer is attached until DrawDysh selects one from the shared
	// texture array. Keep the idle framebuffer complete as a depth-only target.
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		ReportRuntimeError("Graphics", "The dynamic-shadow depth framebuffer is incomplete.");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

bool GL::ResizeFramebuffers(int w, int h)
{
	const int fbW = std::max(1, w);
	const int fbH = std::max(1, h);
	GLint maxTextureSize = 0;
	GLint maxRenderbufferSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
	const GLint maxFramebufferSize = std::min(maxTextureSize, maxRenderbufferSize);
	if (fbW > maxFramebufferSize || fbH > maxFramebufferSize)
	{
		ReportRuntimeError("Graphics",
			"Internal resolution %dx%d exceeds this GPU's maximum framebuffer size of %d.",
			fbW, fbH, maxFramebufferSize);
		return false;
	}

	GLuint newFbo = 0;
	GLuint newColor = 0;
	GLuint newDepth = 0;
	GLuint newMsaaFbo = 0;
	GLuint newMsaaColor = 0;
	GLuint newMsaaDepth = 0;
	for (GLenum staleError = glGetError(); staleError != GL_NO_ERROR;
		staleError = glGetError())
	{
		ReportRuntimeError("Graphics",
			"OpenGL error 0x%X was pending before framebuffer resize began.",
			staleError);
	}

	glGenFramebuffers(1, &newFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, newFbo);
	glGenTextures(1, &newColor);
	glBindTexture(GL_TEXTURE_2D, newColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fbW, fbH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newColor, 0);
	glGenRenderbuffers(1, &newDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, newDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbW, fbH);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newDepth);

	GLenum error = glGetError();
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	bool complete = error == GL_NO_ERROR && status == GL_FRAMEBUFFER_COMPLETE;
	if (complete && g_fMsaa)
	{
		GLint maxSamples = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
		g_msaaSamples = std::max(1, std::min(g_msaaSamples, maxSamples));
		glGenFramebuffers(1, &newMsaaFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, newMsaaFbo);
		glGenRenderbuffers(1, &newMsaaColor);
		glBindRenderbuffer(GL_RENDERBUFFER, newMsaaColor);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_RGBA8, fbW, fbH);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, newMsaaColor);
		glGenRenderbuffers(1, &newMsaaDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, newMsaaDepth);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_DEPTH24_STENCIL8, fbW, fbH);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newMsaaDepth);
		error = glGetError();
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		complete = error == GL_NO_ERROR && status == GL_FRAMEBUFFER_COMPLETE;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (!complete)
	{
		if (newFbo) glDeleteFramebuffers(1, &newFbo);
		if (newColor) glDeleteTextures(1, &newColor);
		if (newDepth) glDeleteRenderbuffers(1, &newDepth);
		if (newMsaaFbo) glDeleteFramebuffers(1, &newMsaaFbo);
		if (newMsaaColor) glDeleteRenderbuffers(1, &newMsaaColor);
		if (newMsaaDepth) glDeleteRenderbuffers(1, &newMsaaDepth);
		ReportRuntimeError("Graphics",
			"Could not allocate the %dx%d%s framebuffer (OpenGL error 0x%X, status 0x%X). The previous setting was retained.",
			fbW, fbH, g_fMsaa ? " MSAA" : "", error, status);
		return false;
	}

	if (fbo) glDeleteFramebuffers(1, &fbo);
	if (fbc) glDeleteTextures(1, &fbc);
	if (rbo) glDeleteRenderbuffers(1, &rbo);
	if (fboMSAA) glDeleteFramebuffers(1, &fboMSAA);
	if (rboColorMSAA) glDeleteRenderbuffers(1, &rboColorMSAA);
	if (rboDepthStencilMSAA) glDeleteRenderbuffers(1, &rboDepthStencilMSAA);
	fbo = newFbo;
	fbc = newColor;
	rbo = newDepth;
	fboMSAA = newMsaaFbo;
	rboColorMSAA = newMsaaColor;
	rboDepthStencilMSAA = newMsaaDepth;
	renderWidth = fbW;
	renderHeight = fbH;
	return true;
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
	if (whiteTex)
	{
		glDeleteTextures(1, &whiteTex);
		whiteTex = 0;
	}
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &fbc);
	glDeleteRenderbuffers(1, &rbo);
	
	glDeleteFramebuffers(1, &fboMSAA);
	glDeleteRenderbuffers(1, &rboColorMSAA);
	glDeleteRenderbuffers(1, &rboDepthStencilMSAA);

	glDeleteFramebuffers(1, &dyshFbo);
	glDeleteTextures(1, &dyshFbc);
	glDeleteRenderbuffers(1, &dyshRbo);

	glDeleteVertexArrays(1, &sao);
	glDeleteBuffers(1, &sbo);
	glDeleteVertexArrays(1, &gao);
	glDeleteBuffers(1, &gbo);
	glDeleteBuffers(1, &geo);
	glDeleteVertexArrays(1, &fontVao);
	glDeleteBuffers(1, &fontVbo);

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
#ifdef CANE_GL41
	// The placeholders bound on empty buffer-sampler and shadow units (glshaders.h).
	DeletePlaceholderBufferTextures();
#endif

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
#ifdef __APPLE__
	SetMenuBarAndDockHidden(false);
#endif
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
	pstream->mappedPtr = nullptr;
	pstream->staging.resize(static_cast<size_t>(pstream->frameSize));
}

GLsizeiptr StageFrameStream(STREAM* pstream, const void* ptr, int copySize)
{
	if (copySize < 0 || copySize > pstream->stride ||
		pstream->cursor + pstream->stride > pstream->frameSize)
	{
		__debugbreak();
		return -1;
	}

	const GLsizeiptr offset = pstream->frameBase + pstream->cursor;
	uint8_t* destination = pstream->staging.data() + pstream->cursor;
	std::memcpy(destination, ptr, static_cast<size_t>(copySize));
	if (copySize < pstream->stride)
		std::memset(destination + copySize, 0,
			static_cast<size_t>(pstream->stride - copySize));

	pstream->cursor += pstream->stride;
	return offset;
}

void UploadFrameStream(STREAM* pstream)
{
	if (pstream->cursor == 0)
		return;

	// BeginFrameStream retired this slot's previous fence, so the whole range
	// is CPU-owned until EndFrameStream fences the draws that consume it.
	glBindBuffer(GL_UNIFORM_BUFFER, pstream->bufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER, pstream->frameBase, pstream->cursor, pstream->staging.data());
}

void BindFrameStreamRange(STREAM* pstream, GLsizeiptr offset, int size)
{
	if (offset >= 0)
		glBindBufferRange(GL_UNIFORM_BUFFER, pstream->bindIndex,
			pstream->bufferObject, offset, size);
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

	// A non-persistently mapped buffer cannot legally be consumed by draw calls
	// until it is unmapped. The old path kept this UBO mapped for the complete
	// render pass; NVIDIA tolerated that, while AMD could observe stale or
	// partially written object transforms. Upload each aligned range normally.
	glBindBuffer(GL_UNIFORM_BUFFER, pstream->bufferObject);
	glBufferSubData(GL_UNIFORM_BUFFER, off, copySize, ptr);
	glBindBufferRange(GL_UNIFORM_BUFFER, pstream->bindIndex, pstream->bufferObject, off, size);
}

void EndFrameStream(STREAM* pstream)
{
	if (pstream->cursor == 0)
		return;

	int slot = g_cframe % g_frames;

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
	pstream->staging.clear();
	pstream->staging.shrink_to_fit();
}

bool ApplyMsaaSettings()
{
	if (!g_fMsaa)
	{
		if (g_gl.fboMSAA) glDeleteFramebuffers(1, &g_gl.fboMSAA);
		if (g_gl.rboColorMSAA) glDeleteRenderbuffers(1, &g_gl.rboColorMSAA);
		if (g_gl.rboDepthStencilMSAA) glDeleteRenderbuffers(1, &g_gl.rboDepthStencilMSAA);
		g_gl.fboMSAA = g_gl.rboColorMSAA = g_gl.rboDepthStencilMSAA = 0;
		// Video options are changed from UpdateUi after the scene target has
		// already been bound for this frame.  Continue that frame on the resolved
		// scene target instead of accidentally drawing it to the window.
		g_sceneFbo = g_gl.fbo;
		glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);
		glViewport(0, 0, g_gl.renderWidth, g_gl.renderHeight);
		return true;
	}

	// MSAA storage must match the off-screen scene target, not the UI canvas.
	int w = g_gl.renderWidth;
	int h = g_gl.renderHeight;
	w = std::max(w, 1);
	h = std::max(h, 1);

	GLint maxSamples = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
	g_msaaSamples = std::max(1, std::min(g_msaaSamples, maxSamples));

	GLuint newFbo = 0;
	GLuint newColor = 0;
	GLuint newDepth = 0;
	for (GLenum staleError = glGetError(); staleError != GL_NO_ERROR;
		staleError = glGetError())
	{
		ReportRuntimeError("Graphics",
			"OpenGL error 0x%X was pending before MSAA framebuffer creation began.",
			staleError);
	}
	glGenFramebuffers(1, &newFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, newFbo);

	glGenRenderbuffers(1, &newColor);
	glBindRenderbuffer(GL_RENDERBUFFER, newColor);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_RGBA8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, newColor);

	glGenRenderbuffers(1, &newDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, newDepth);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, g_msaaSamples, GL_DEPTH24_STENCIL8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, newDepth);

	const GLenum error = glGetError();
	const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (error != GL_NO_ERROR || status != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (newFbo) glDeleteFramebuffers(1, &newFbo);
		if (newColor) glDeleteRenderbuffers(1, &newColor);
		if (newDepth) glDeleteRenderbuffers(1, &newDepth);
		if (g_gl.fboMSAA) glDeleteFramebuffers(1, &g_gl.fboMSAA);
		if (g_gl.rboColorMSAA) glDeleteRenderbuffers(1, &g_gl.rboColorMSAA);
		if (g_gl.rboDepthStencilMSAA) glDeleteRenderbuffers(1, &g_gl.rboDepthStencilMSAA);
		g_gl.fboMSAA = 0;
		g_gl.rboColorMSAA = 0;
		g_gl.rboDepthStencilMSAA = 0;
		g_fMsaa = false;
		g_sceneFbo = g_gl.fbo;
		glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);
		glViewport(0, 0, g_gl.renderWidth, g_gl.renderHeight);
		ReportRuntimeError("Graphics",
			"Could not allocate %dx MSAA at %dx%d (OpenGL error 0x%X, status 0x%X). MSAA has been disabled.",
			g_msaaSamples, w, h, error, status);
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (g_gl.fboMSAA) glDeleteFramebuffers(1, &g_gl.fboMSAA);
	if (g_gl.rboColorMSAA) glDeleteRenderbuffers(1, &g_gl.rboColorMSAA);
	if (g_gl.rboDepthStencilMSAA) glDeleteRenderbuffers(1, &g_gl.rboDepthStencilMSAA);
	g_gl.fboMSAA = newFbo;
	g_gl.rboColorMSAA = newColor;
	g_gl.rboDepthStencilMSAA = newDepth;
	// ApplyMsaaSettings can run in the middle of a frame.  Bind the replacement
	// immediately so world/UI drawing and the resolve at frame end agree on the
	// same framebuffer.
	g_sceneFbo = g_gl.fboMSAA;
	glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);
	glViewport(0, 0, g_gl.renderWidth, g_gl.renderHeight);
	return true;
}

bool ApplyInternalResolutionSettings()
{
	int width = 1;
	int height = 1;
	glfwGetFramebufferSize(g_gl.window, &width, &height);
	s_lastFramebufferResizeSucceeded = true;
	FrameBufferSizeCallBack(g_gl.window, width, height);
	return s_lastFramebufferResizeSucceeded;
}

bool ApplyWindowModeSettings(WindowMode mode)
{
	if (g_gl.window == nullptr)
		return false;

	GLFWmonitor* monitor = MonitorByIndex(g_monitorIndex);
	if (monitor == nullptr)
		return false;

	if (g_windowMode == WindowMode_Windowed)
	{
		glfwGetWindowPos(g_gl.window, &s_windowedX, &s_windowedY);
		glfwGetWindowSize(g_gl.window, &s_windowedWidth, &s_windowedHeight);
	}

	// Some window systems deliver the focus event from a monitor/mode change
	// later, during event polling. Cover that delayed event as well as callbacks
	// delivered synchronously by GLFW.
	s_autoIconifySuppressedUntil = glfwGetTime() + 0.5;
	glfwGetError(nullptr);
	if (mode == WindowMode_Windowed)
	{
		glfwSetWindowAttrib(g_gl.window, GLFW_DECORATED, GLFW_TRUE);
		glfwSetWindowMonitor(g_gl.window, nullptr, s_windowedX, s_windowedY,
			(std::max)(s_windowedWidth, 320), (std::max)(s_windowedHeight, 240), GLFW_DONT_CARE);
	}
#ifdef __APPLE__
	// GLFW's exclusive fullscreen on macOS switches the display to the nearest
	// mode in its own list, which on the built-in display can be another
	// resolution than the current one. Fullscreen is therefore the borderless
	// window over the whole display at its current mode, with the menu bar and
	// Dock hidden while the game is active (below).
	else if (mode == WindowMode_Borderless || mode == WindowMode_Fullscreen)
#else
	else if (mode == WindowMode_Borderless)
#endif
	{
		int x = 0;
		int y = 0;
		glfwGetMonitorPos(monitor, &x, &y);
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
		if (videoMode == nullptr)
			return false;

		// Borderless is a fullscreen-sized window, not a work-area-sized window.
		// The work area excludes the taskbar and left a visible strip along the
		// bottom of the selected monitor.
		glfwSetWindowAttrib(g_gl.window, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowMonitor(g_gl.window, nullptr, x, y,
			videoMode->width, videoMode->height, GLFW_DONT_CARE);
	}
	else
	{
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);
		if (videoMode == nullptr)
			return false;
		glfwSetWindowAttrib(g_gl.window, GLFW_DECORATED, GLFW_TRUE);
		glfwSetWindowMonitor(g_gl.window, monitor, 0, 0,
			videoMode->width, videoMode->height, videoMode->refreshRate);
	}
	if (glfwGetError(nullptr) != GLFW_NO_ERROR)
		return false;

	g_windowMode = mode;
#ifdef __APPLE__
	SetMenuBarAndDockHidden(mode == WindowMode_Fullscreen);
#endif
	glfwSwapInterval(g_fVsync ? 1 : 0);
	return true;
}

int VideoMonitorCount()
{
	int count = 0;
	glfwGetMonitors(&count);
	return (std::max)(count, 1);
}

const char* VideoMonitorName(int monitorIndex)
{
	GLFWmonitor* monitor = MonitorByIndex(monitorIndex);
	const char* name = monitor != nullptr ? glfwGetMonitorName(monitor) : nullptr;
	return name != nullptr ? name : "Primary Monitor";
}

int VideoMonitorIndexByName(const char* name)
{
	if (name == nullptr || *name == '\0')
		return -1;
	int count = 0;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	for (int i = 0; monitors != nullptr && i < count; ++i)
	{
		const char* candidate = glfwGetMonitorName(monitors[i]);
		if (candidate != nullptr && std::strcmp(candidate, name) == 0)
			return i;
	}
	return -1;
}

bool ApplyMonitorSettings(int monitorIndex)
{
	const int previousMonitor = g_monitorIndex;
	g_monitorIndex = (std::clamp)(monitorIndex, 0, VideoMonitorCount() - 1);
	if (!ApplyWindowModeSettings(g_windowMode))
	{
		g_monitorIndex = previousMonitor;
		return false;
	}
	if (g_windowMode == WindowMode_Windowed && g_gl.window != nullptr)
	{
		GLFWmonitor* monitor = MonitorByIndex(g_monitorIndex);
		if (monitor == nullptr)
			return false;
		int x = 0, y = 0, width = 0, height = 0;
		int windowWidth = 0, windowHeight = 0;
		glfwGetMonitorWorkarea(monitor, &x, &y, &width, &height);
		glfwGetWindowSize(g_gl.window, &windowWidth, &windowHeight);
		s_windowedX = x + (width - windowWidth) / 2;
		s_windowedY = y + (height - windowHeight) / 2;
		s_autoIconifySuppressedUntil = glfwGetTime() + 0.5;
		glfwGetError(nullptr);
		glfwSetWindowPos(g_gl.window, s_windowedX, s_windowedY);
		if (glfwGetError(nullptr) != GLFW_NO_ERROR)
		{
			g_monitorIndex = previousMonitor;
			return false;
		}
	}
	return true;
}

void ApplyOutputGammaSettings(float gamma)
{
	g_outputGamma = std::clamp(gamma, 0.50f, 1.50f);
	if (glScreenShader.ID != 0)
	{
		glScreenShader.Use();
		glUniform1f(glGetUniformLocation(glScreenShader.ID, "outputGamma"), g_outputGamma);
	}
}

void ApplyAutoIconifySettings(bool enabled)
{
	g_fAutoIconify = enabled;
	if (g_gl.window != nullptr)
		glfwSetWindowAttrib(g_gl.window, GLFW_AUTO_ICONIFY, enabled ? GLFW_TRUE : GLFW_FALSE);
}

bool ApplyAspectRatioSettings(AspectMode mode)
{
	const AspectMode previousMode = g_gl.aspectMode;
	const float previousAspect = g_gl.aspectRatio;
	g_gl.aspectMode = mode;
	switch (mode)
	{
		case Fixed_4_3: g_gl.aspectRatio = 4.0f / 3.0f; break;
		case Fixed_16_10: g_gl.aspectRatio = 16.0f / 10.0f; break;
		case Fixed_16_9: g_gl.aspectRatio = 16.0f / 9.0f; break;
		case PS2_4_3: g_gl.aspectRatio = 4.0f / 3.0f; break;
		case PS2_16_9: g_gl.aspectRatio = 16.0f / 9.0f; break;
		case FitToScreen:
		default: break;
	}
	if (ApplyInternalResolutionSettings())
		return true;
	g_gl.aspectMode = previousMode;
	g_gl.aspectRatio = previousAspect;
	ApplyInternalResolutionSettings();
	return false;
}

void FrameBufferSizeCallBack(GLFWwindow* window, int width, int height)
{
	// GLFW reports a 0x0 framebuffer while a window is minimized. Rebuilding
	// every render target as 1x1 here can leave borderless mode black after an
	// Alt+Tab when the platform omits a matching restore-size notification.
	if (width <= 0 || height <= 0)
		return;

	constexpr float kVirtualUiWidth = 640.0f;
	constexpr float kVirtualUiHeight = 492.80002f;

	float imguiOffset = 0.0f;
	if (g_fDebugGuiOpen && ImGui::GetCurrentContext() != nullptr)
	{
		float contentScaleX = 1.0f;
		float contentScaleY = 1.0f;
		glfwGetWindowContentScale(window, &contentScaleX, &contentScaleY);
		imguiOffset = ImGui::GetFrameHeight() * contentScaleY;
	}

	const int outputW = std::max(1, width);
	const int outputH = std::max(1, int(std::ceil(float(height) - imguiOffset)));
	float targetAspect = static_cast<float>(outputW) / static_cast<float>(outputH);
	if (g_gl.aspectMode == Fixed_4_3 || g_gl.aspectMode == PS2_4_3)
		targetAspect = 4.0f / 3.0f;
	else if (g_gl.aspectMode == Fixed_16_9 || g_gl.aspectMode == PS2_16_9)
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
		// The original PS2 raster is explicitly 640x448. Other internal
		// resolutions preserve the selected presentation aspect ratio.
		if (sceneH == 448)
			sceneW = 640;
		else
			sceneW = std::max(1, static_cast<int>(std::lround(
				static_cast<double>(sceneH) * static_cast<double>(presentW) /
				static_cast<double>(presentH))));
	}
	else if (g_resolutionScalePercent != 100)
	{
		const float scale = static_cast<float>(g_resolutionScalePercent) * 0.01f;
		sceneW = (std::max)(1, static_cast<int>(std::lround(presentW * scale)));
		sceneH = (std::max)(1, static_cast<int>(std::lround(presentH * scale)));
	}

	// UI and HUD layout always use the actual output size. Only the scene
	// framebuffer changes with the internal-resolution option.
	g_gl.width  = static_cast<float>(presentW);
	g_gl.height = static_cast<float>(presentH);
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

	const int previousSceneW = g_gl.renderWidth;
	const int previousSceneH = g_gl.renderHeight;
	s_lastFramebufferResizeSucceeded = g_gl.ResizeFramebuffers(sceneW, sceneH);
	if (!s_lastFramebufferResizeSucceeded)
	{
		sceneW = previousSceneW;
		sceneH = previousSceneH;
	}

	// Aspect/internal-resolution settings can be changed from UpdateUi after
	// this frame's scene target has already been bound. ResizeFramebuffers
	// replaces and deletes that target, so update the cached binding now rather
	// than allowing the rest of the frame to reuse a deleted framebuffer name.
	g_sceneFbo = g_fMsaa ? g_gl.fboMSAA : g_gl.fbo;
	glBindFramebuffer(GL_FRAMEBUFFER, g_sceneFbo);
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
GLuint glslProjectedVolumeColor = 0;
GLuint glslProjectedVolumeFinalPass = 0;
GLuint glslfAnimateUv = 0;
GLuint glsluvOffsets = 0;
GLuint glslUnSelfIllum = 0;
GLuint glslSubGlobPosCenter = 0;
GLuint glslSubGlobRadius = 0;
#ifdef CANE_GL41
GLuint glslfCacheBake = 0;
#endif
GLuint glslDyshMatWorldClip = 0;
GLuint glslDyshModel = 0;
GLuint glslDyshfSkin = 0;
GLuint glslAmbientMap = 0;
GLuint glslDiffuseMap = 0;
GLuint glslSaturateMap = 0;
GLuint glslGeomModelToClip = 0;
GLuint glslGeomDebugColor = 0;
GLuint u_fontTexLoc = 0;
GLuint glslfCull = 0;
uint64_t screenTextureHandle = 0;
GLuint g_sceneFbo = 0;
int g_msaaSamples = 4;
bool g_fMsaa = false;
int g_internalResolutionHeight = 0;
int g_resolutionScalePercent = 100;
int g_monitorIndex = 0;
int g_textureFiltering = 1;
float g_outputGamma = 1.0f;
float g_guiScale = 1.0f;
GuiStyle g_guiStyle = GuiStyle_PS2;
WindowMode g_windowMode = WindowMode_Windowed;
bool g_fAutoIconify = false;
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
GLuint glslCelPose = 0;
GLuint glslCelPoseCount = 0;
#ifdef CANE_GL41
GLint glslCelLineWidthPx = -1;
GLint glslCelViewportPx = -1;
GLint glslCelMultisampled = -1;
#endif
