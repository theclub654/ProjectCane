// NVIDIA DLSS Super Resolution / DLAA for ProjectCane through NVIDIA Streamline.
// See dlss.h for the overall design.

#include "dlss.h"
#include "fsr.h"
#include "perf.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#include <sl.h>
#include <sl_consts.h>
#include <sl_dlss.h>
#include <sl_dlss_g.h>
#include <sl_reflex.h>
#include <sl_pcl.h>
#include <sl_security.h>

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

// Provided by the rest of the engine (gl.h / gl.cpp). Declared here so this
// module does not need the full game header tree.
extern GLuint cmUBO;
extern bool g_fVsync;
bool ApplyInternalResolutionSettings();

int g_uiTargetWidth = 0;
int g_uiTargetHeight = 0;

// =============================================================================
// GL_EXT_memory_object(_win32) / GL_EXT_semaphore(_win32)
// glad in this project was generated without these, so load them by hand.
// =============================================================================
namespace
{
constexpr GLenum kGL_DEDICATED_MEMORY_OBJECT_EXT = 0x9581;
constexpr GLenum kGL_HANDLE_TYPE_OPAQUE_WIN32_EXT = 0x9587;
constexpr GLenum kGL_LAYOUT_GENERAL_EXT = 0x958D;
constexpr GLenum kGL_DEVICE_UUID_EXT = 0x9597;
constexpr GLenum kGL_DEVICE_LUID_EXT = 0x9599;
constexpr GLenum kGL_HANDLE_TYPE_D3D12_RESOURCE_EXT = 0x958A;
constexpr GLenum kGL_HANDLE_TYPE_D3D12_FENCE_EXT = 0x9594;
constexpr GLenum kGL_D3D12_FENCE_VALUE_EXT = 0x9595;

typedef void (APIENTRY* PFN_glGetUnsignedBytei_vEXT)(GLenum target, GLuint index, GLubyte* data);
typedef void (APIENTRY* PFN_glGetUnsignedBytevEXT)(GLenum pname, GLubyte* data);
typedef void (APIENTRY* PFN_glSemaphoreParameterui64vEXT)(GLuint semaphore, GLenum pname, const GLuint64* params);
typedef void (APIENTRY* PFN_glCreateMemoryObjectsEXT)(GLsizei n, GLuint* memoryObjects);
typedef void (APIENTRY* PFN_glDeleteMemoryObjectsEXT)(GLsizei n, const GLuint* memoryObjects);
typedef void (APIENTRY* PFN_glMemoryObjectParameterivEXT)(GLuint memoryObject, GLenum pname, const GLint* params);
typedef void (APIENTRY* PFN_glTexStorageMem2DEXT)(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLuint memory, GLuint64 offset);
typedef void (APIENTRY* PFN_glImportMemoryWin32HandleEXT)(GLuint memory, GLuint64 size, GLenum handleType, void* handle);
typedef void (APIENTRY* PFN_glGenSemaphoresEXT)(GLsizei n, GLuint* semaphores);
typedef void (APIENTRY* PFN_glDeleteSemaphoresEXT)(GLsizei n, const GLuint* semaphores);
typedef void (APIENTRY* PFN_glImportSemaphoreWin32HandleEXT)(GLuint semaphore, GLenum handleType, void* handle);
typedef void (APIENTRY* PFN_glSignalSemaphoreEXT)(GLuint semaphore, GLuint numBufferBarriers, const GLuint* buffers, GLuint numTextureBarriers, const GLuint* textures, const GLenum* dstLayouts);
typedef void (APIENTRY* PFN_glWaitSemaphoreEXT)(GLuint semaphore, GLuint numBufferBarriers, const GLuint* buffers, GLuint numTextureBarriers, const GLuint* textures, const GLenum* srcLayouts);

struct GLINTEROP
{
	PFN_glGetUnsignedBytei_vEXT GetUnsignedBytei_v = nullptr;
	PFN_glGetUnsignedBytevEXT GetUnsignedBytev = nullptr;
	PFN_glSemaphoreParameterui64vEXT SemaphoreParameterui64v = nullptr;
	PFN_glCreateMemoryObjectsEXT CreateMemoryObjects = nullptr;
	PFN_glDeleteMemoryObjectsEXT DeleteMemoryObjects = nullptr;
	PFN_glMemoryObjectParameterivEXT MemoryObjectParameteriv = nullptr;
	PFN_glTexStorageMem2DEXT TexStorageMem2D = nullptr;
	PFN_glImportMemoryWin32HandleEXT ImportMemoryWin32Handle = nullptr;
	PFN_glGenSemaphoresEXT GenSemaphores = nullptr;
	PFN_glDeleteSemaphoresEXT DeleteSemaphores = nullptr;
	PFN_glImportSemaphoreWin32HandleEXT ImportSemaphoreWin32Handle = nullptr;
	PFN_glSignalSemaphoreEXT SignalSemaphore = nullptr;
	PFN_glWaitSemaphoreEXT WaitSemaphore = nullptr;
};

GLINTEROP s_gl;

// =============================================================================
// Vulkan function table (all entry points come from sl.interposer.dll so the
// Streamline hooks see device creation and presents).
// =============================================================================
#define DLSS_VK_INSTANCE_FUNCS(X) \
	X(vkDestroyInstance) \
	X(vkEnumeratePhysicalDevices) \
	X(vkGetPhysicalDeviceProperties) \
	X(vkGetPhysicalDeviceProperties2) \
	X(vkGetPhysicalDeviceQueueFamilyProperties) \
	X(vkGetPhysicalDeviceMemoryProperties) \
	X(vkEnumerateDeviceExtensionProperties) \
	X(vkCreateDevice) \
	X(vkCreateWin32SurfaceKHR) \
	X(vkDestroySurfaceKHR) \
	X(vkGetPhysicalDeviceSurfaceSupportKHR) \
	X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) \
	X(vkGetPhysicalDeviceSurfaceFormatsKHR) \
	X(vkGetPhysicalDeviceSurfacePresentModesKHR)

#define DLSS_VK_DEVICE_FUNCS(X) \
	X(vkDestroyDevice) \
	X(vkGetDeviceQueue) \
	X(vkDeviceWaitIdle) \
	X(vkCreateCommandPool) \
	X(vkDestroyCommandPool) \
	X(vkAllocateCommandBuffers) \
	X(vkResetCommandBuffer) \
	X(vkBeginCommandBuffer) \
	X(vkEndCommandBuffer) \
	X(vkCmdPipelineBarrier) \
	X(vkQueueSubmit) \
	X(vkQueueWaitIdle) \
	X(vkCreateFence) \
	X(vkDestroyFence) \
	X(vkWaitForFences) \
	X(vkResetFences) \
	X(vkCreateSemaphore) \
	X(vkDestroySemaphore) \
	X(vkCreateImage) \
	X(vkDestroyImage) \
	X(vkGetImageMemoryRequirements) \
	X(vkAllocateMemory) \
	X(vkFreeMemory) \
	X(vkBindImageMemory) \
	X(vkCreateImageView) \
	X(vkDestroyImageView) \
	X(vkGetMemoryWin32HandleKHR) \
	X(vkGetSemaphoreWin32HandleKHR) \
	X(vkCreateSwapchainKHR) \
	X(vkDestroySwapchainKHR) \
	X(vkGetSwapchainImagesKHR) \
	X(vkAcquireNextImageKHR) \
	X(vkQueuePresentKHR)

struct VKFUNCS
{
	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
	PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr;
	PFN_vkCreateInstance vkCreateInstance = nullptr;
#define DLSS_VK_DECL(name) PFN_##name name = nullptr;
	DLSS_VK_INSTANCE_FUNCS(DLSS_VK_DECL)
	DLSS_VK_DEVICE_FUNCS(DLSS_VK_DECL)
#undef DLSS_VK_DECL
};

VKFUNCS s_vk;

// =============================================================================
// Streamline entry points (loaded dynamically: the game must keep running
// when the DLLs are absent, so nothing links against sl.interposer.lib).
// =============================================================================
struct SLFUNCS
{
	PFun_slInit* Init = nullptr;
	PFun_slShutdown* Shutdown = nullptr;
	PFun_slIsFeatureSupported* IsFeatureSupported = nullptr;
	PFun_slIsFeatureLoaded* IsFeatureLoaded = nullptr;
	PFun_slSetTagForFrame* SetTagForFrame = nullptr;
	PFun_slSetConstants* SetConstants = nullptr;
	PFun_slEvaluateFeature* EvaluateFeature = nullptr;
	PFun_slFreeResources* FreeResources = nullptr;
	PFun_slGetNewFrameToken* GetNewFrameToken = nullptr;
	PFun_slGetFeatureFunction* GetFeatureFunction = nullptr;
	PFun_slGetFeatureVersion* GetFeatureVersion = nullptr;

	PFun_slSetD3DDevice* SetD3DDevice = nullptr;
	PFun_slUpgradeInterface* UpgradeInterface = nullptr;
	PFun_slGetNativeInterface* GetNativeInterface = nullptr;
	PFun_slSetFeatureLoaded* SetFeatureLoaded = nullptr;

	PFun_slDLSSGetOptimalSettings* DLSSGetOptimalSettings = nullptr;
	PFun_slDLSSSetOptions* DLSSSetOptions = nullptr;
	PFun_slDLSSGetState* DLSSGetState = nullptr;

	// D3D12 backend only: frame generation + Reflex/PC latency markers.
	PFun_slDLSSGGetState* DLSSGGetState = nullptr;
	PFun_slDLSSGSetOptions* DLSSGSetOptions = nullptr;
	PFun_slReflexSetOptions* ReflexSetOptions = nullptr;
	PFun_slReflexSleep* ReflexSleep = nullptr;
	PFun_slPCLSetMarker* PCLSetMarker = nullptr;
};

SLFUNCS s_sl;
HMODULE s_hmodInterposer = nullptr;
bool s_fSlInitialized = false;

// =============================================================================
// Shared GL <-> Vulkan image
// =============================================================================
struct SHAREDIMAGE
{
	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkImageUsageFlags usage = 0;
	HANDLE handle = nullptr;
	GLuint glMemory = 0;
	GLuint glTexture = 0;
	int width = 0;
	int height = 0;
	// D3D12 backend
	ID3D12Resource* d3dResource = nullptr;
	DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
};

enum BACKEND
{
	BACKEND_None = 0,
	BACKEND_Vulkan,
	BACKEND_D3D12
};

constexpr int kDxFrames = 3;

struct DXFRAME
{
	ID3D12CommandAllocator* allocEval = nullptr;
	ID3D12GraphicsCommandList* listEval = nullptr;
	ID3D12CommandAllocator* allocPresent = nullptr;
	ID3D12GraphicsCommandList* listPresent = nullptr;
	uint64_t fenceValue = 0;   // fence value that marks this slot's GPU work done
};

constexpr int kFramesInFlight = 2;
constexpr int kMaxStaticRects = 4;

struct FRAMESLOT
{
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	VkFence fence = VK_NULL_HANDLE;
};

struct DLSSSTATE
{
	// Settings (persisted in DLSS.ini)
	DLSSQ dlssq = DLSSQ_Off;
	int preset = 0;              // 0 = NVIDIA default, otherwise sl::DLSSPreset value
	bool fShowConsole = false;
	bool fVerboseLog = false;
	float jitterSignX = 1.0f;
	float jitterSignY = 1.0f;
	bool fJitterEnabled = true;
	bool fCameraCutDetection = true;

	// Status
	bool fSupported = false;
	std::string status = "Not initialized";
	std::wstring exeDirectory;

	// Vulkan objects
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	uint32_t queueFamily = 0;
	VkQueue queue = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;
	FRAMESLOT frames[kFramesInFlight];
	VkPhysicalDeviceMemoryProperties memoryProperties{};

	// GL <-> VK semaphores
	VkSemaphore semGlDone = VK_NULL_HANDLE;   // signalled by GL, waited by VK
	VkSemaphore semVkDone = VK_NULL_HANDLE;   // signalled by VK, waited by GL
	HANDLE hSemGlDone = nullptr;
	HANDLE hSemVkDone = nullptr;
	GLuint glSemGlDone = 0;
	GLuint glSemVkDone = 0;

	// Shared images
	SHAREDIMAGE color;     // render res, RGBA8
	SHAREDIMAGE depth;     // render res, R32F (hardware depth 0..1)
	SHAREDIMAGE motion;    // render res, RG16F (UV-space, current -> previous)
	SHAREDIMAGE output;    // present res, RGBA8

	// GL helpers
	GLuint prepFbo = 0;
	GLuint prepProgram = 0;
	GLuint prepVao = 0;
	GLint uClipToPrevClip = -1;
	GLint uJitterNdc = -1;
	GLint uRenderSize = -1;
	GLint uStaticRectCount = -1;
	GLint uStaticRects = -1;
	GLuint outputFbo = 0;
	GLuint outputDepthStencil = 0;

	// Present hook: Streamline performs its per-frame bookkeeping and
	// garbage collection inside vkQueuePresentKHR. Our final image is
	// presented by OpenGL, so a never-shown 1x1-ish window with a tiny
	// swapchain is presented once per frame to keep Streamline healthy.
	HWND hwndHidden = nullptr;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkFence acquireFence = VK_NULL_HANDLE;
	bool fPresentHook = false;
	int presentFailures = 0;

	// Frame state
	uint32_t frameIndex = 0;
	glm::vec2 jitterPixels = glm::vec2(0.0f);
	glm::vec2 jitterNdc = glm::vec2(0.0f);
	bool fJitterThisFrame = false;
	bool fResetHistory = true;
	bool fHavePrevCamera = false;
	glm::mat4 matPrevWorldToClip = glm::mat4(1.0f);
	glm::vec3 posPrevCamera = glm::vec3(0.0f);
	glm::vec3 dirPrevForward = glm::vec3(0.0f, 0.0f, -1.0f);
	int renderWidth = 0;
	int renderHeight = 0;
	int outputWidth = 0;
	int outputHeight = 0;
	DLSSQ frameQuality = DLSSQ_Off; // mode in effect when BeginDlssFrame ran
	int frameRenderWidth = 0;    // sizes passed to BeginDlssFrame
	int frameRenderHeight = 0;
	int frameOutputWidth = 0;
	int frameOutputHeight = 0;
	int staticRectCount = 0;
	glm::ivec4 staticRects[kMaxStaticRects]{};

	// Backend selection
	BACKEND backend = BACKEND_None;
	bool fPreferD3D12 = true;          // DLSS.ini Backend=D3D12|Vulkan
	sl::FrameToken* token = nullptr;   // this frame's Streamline token

	// D3D12 backend (frame generation capable)
	HMODULE hmodD3D12 = nullptr;
	HMODULE hmodDxgi = nullptr;
	ID3D12Device* d3dDevice = nullptr;        // native
	ID3D12Device* d3dProxyDevice = nullptr;   // Streamline proxy (CreateCommandQueue hook)
	ID3D12CommandQueue* d3dQueue = nullptr;
	IDXGIFactory4* dxgiFactory = nullptr;     // native
	IDXGIFactory2* dxgiProxyFactory = nullptr;// Streamline proxy (swapchain hooks)
	ID3D12Fence* d3dFence = nullptr;          // shared with OpenGL
	HANDLE hD3dFence = nullptr;
	GLuint glD3dFence = 0;
	uint64_t fenceValue = 0;
	HANDLE hFenceEvent = nullptr;
	DXFRAME dxFrames[kDxFrames];
	bool fTearing = false;
	HWND hwndGame = nullptr;
	HWND hwndPresent = nullptr;               // child window the frame-gen swapchain presents to
	bool fFgChildWindow = true;               // DLSS.ini FgPresentWindow=Child|Game
	int glKickFrames = 0;                     // OpenGL presents still to issue after FG starts
	int glKickSetting = 0;                    // DLSS.ini FgGlKickFrames
	int fgFramesLogged = 0;
	bool fFgAllowTearing = false;             // DLSS.ini FgAllowTearing
	int unstickMode = 1;                      // DLSS.ini FgUnstick=Off|Resize|Minimize
	int unstickFrames = 0;
	RECT unstickRect{};
	IDXGISwapChain3* hiddenSwap = nullptr;    // keeps Streamline's present hook alive
	IDXGISwapChain3* mainSwap = nullptr;      // presents the game while frame gen is on
	int swapWidth = 0;
	int swapHeight = 0;
	SHAREDIMAGE presentShared;                // window-size final frame (top-down)
	GLuint presentGlTex = 0;                  // window-size final frame (GL)
	GLuint presentFbo = 0;
	int presentW = 0;
	int presentH = 0;
	GLuint composeTex = 0;                    // GL-orientation upscaled image for the HUD
	GLuint blitReadFbo = 0;
	GLuint blitDrawFbo = 0;
	GLint uFlipY = -1;

	// Frame generation
	bool fFgSupported = false;
	int fgMaxGenerated = 1;                   // numFramesToGenerateMax
	int frameGen = 1;                         // 1 = off, 2..4 = 2x..4x (DLSS.ini)
	bool fUpscaledThisFrame = false;
	bool fReflexOn = false;
	int fgPresentedFactor = 1;
	int fgStatus = 0;

	// FPS counter
	int fpsMode = 0;   // 0 off, 1 FPS, 2 FPS + frame breakdown
	double tFpsWindow = 0.0;
	int cFpsFrames = 0;
	float fpsShown = 0.0f;
	bool fFpsShownIsFg = false;

	// Stats
	uint64_t vramBytes = 0;
	std::string featureVersion;
	int lastResult = 0;
};

DLSSSTATE s_dlss;

// =============================================================================
// Helpers
// =============================================================================
void DlssLog(const char* fmt, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, fmt);
	std::vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);
	std::cout << "[DLSS] " << buffer << std::endl;

	// Also keep a DLSS.log next to the exe (recreated every launch).
	static FILE* s_pfLog = nullptr;
	static bool s_fTried = false;
	if (!s_fTried)
	{
		s_fTried = true;
		wchar_t path[MAX_PATH] = {};
		GetModuleFileNameW(nullptr, path, MAX_PATH);
		std::filesystem::path logPath = std::filesystem::path(path).parent_path() / L"DLSS.log";
		if (_wfopen_s(&s_pfLog, logPath.c_str(), L"w") != 0)
			s_pfLog = nullptr;
	}
	if (s_pfLog != nullptr)
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		std::fprintf(s_pfLog, "[%02d:%02d:%02d.%03d] %s\n", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, buffer);
		std::fflush(s_pfLog);
	}
}

// Logs any DLSS/Streamline call that blocks the game thread for too long.
struct DLSSTIMER
{
	const char* what;
	LARGE_INTEGER start;
	explicit DLSSTIMER(const char* name) : what(name) { QueryPerformanceCounter(&start); }
	~DLSSTIMER()
	{
		LARGE_INTEGER end, freq;
		QueryPerformanceCounter(&end);
		QueryPerformanceFrequency(&freq);
		const double ms = 1000.0 * static_cast<double>(end.QuadPart - start.QuadPart) / static_cast<double>(freq.QuadPart);
		if (ms > 30.0)
			DlssLog("SLOW: %s took %.1f ms", what, ms);
	}
};

bool FailDlss(const char* reason)
{
	s_dlss.fSupported = false;
	s_dlss.status = reason;
	DlssLog("Disabled: %s", reason);
	return false;
}

void SlLogCallback(sl::LogType type, const char* msg)
{
	if (type == sl::LogType::eError)
		std::cout << "[Streamline ERROR] " << msg;
	else if (type == sl::LogType::eWarn)
		std::cout << "[Streamline WARN] " << msg;
	else if (s_dlss.fVerboseLog)
		std::cout << "[Streamline] " << msg;
}

std::wstring ExeDirectory()
{
	wchar_t path[MAX_PATH]{};
	const DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
	if (len == 0)
		return std::filesystem::current_path().wstring();
	return std::filesystem::path(path).parent_path().wstring();
}

std::wstring IniPath()
{
	return (std::filesystem::path(s_dlss.exeDirectory) / L"DLSS.ini").wstring();
}

const char* const s_apchzQuality[DLSSQ_Max] =
{
	"Off", "DLAA", "Quality", "Balanced", "Performance", "Ultra Performance"
};

const wchar_t* const s_apwchzQualityKey[DLSSQ_Max] =
{
	L"Off", L"DLAA", L"Quality", L"Balanced", L"Performance", L"UltraPerformance"
};

sl::DLSSMode SlModeFromQuality(DLSSQ dlssq)
{
	switch (dlssq)
	{
		case DLSSQ_DLAA: return sl::DLSSMode::eDLAA;
		case DLSSQ_Quality: return sl::DLSSMode::eMaxQuality;
		case DLSSQ_Balanced: return sl::DLSSMode::eBalanced;
		case DLSSQ_Performance: return sl::DLSSMode::eMaxPerformance;
		case DLSSQ_UltraPerformance: return sl::DLSSMode::eUltraPerformance;
		default: return sl::DLSSMode::eOff;
	}
}

void LoadDlssSettings()
{
	const std::wstring ini = IniPath();
	wchar_t value[64]{};

	GetPrivateProfileStringW(L"DLSS", L"Mode", L"Off", value, 64, ini.c_str());
	s_dlss.dlssq = DLSSQ_Off;
	for (int i = 0; i < DLSSQ_Max; ++i)
	{
		if (_wcsicmp(value, s_apwchzQualityKey[i]) == 0)
			s_dlss.dlssq = static_cast<DLSSQ>(i);
	}

	s_dlss.preset = GetPrivateProfileIntW(L"DLSS", L"Preset", 0, ini.c_str());
	s_dlss.fShowConsole = GetPrivateProfileIntW(L"DLSS", L"ShowStreamlineConsole", 0, ini.c_str()) != 0;
	s_dlss.fVerboseLog = GetPrivateProfileIntW(L"DLSS", L"VerboseLog", 0, ini.c_str()) != 0;
	s_dlss.jitterSignX = GetPrivateProfileIntW(L"DLSS", L"FlipJitterX", 0, ini.c_str()) != 0 ? -1.0f : 1.0f;
	s_dlss.jitterSignY = GetPrivateProfileIntW(L"DLSS", L"FlipJitterY", 0, ini.c_str()) != 0 ? -1.0f : 1.0f;

	GetPrivateProfileStringW(L"DLSS", L"Backend", L"D3D12", value, 64, ini.c_str());
	s_dlss.fPreferD3D12 = _wcsicmp(value, L"Vulkan") != 0;
	s_dlss.frameGen = std::clamp(static_cast<int>(GetPrivateProfileIntW(L"DLSS", L"FrameGeneration", 1, ini.c_str())), 1, 4);
	s_dlss.fpsMode = std::clamp(static_cast<int>(GetPrivateProfileIntW(L"DLSS", L"ShowFps", 0, ini.c_str())), 0, 2);
	GetPrivateProfileStringW(L"DLSS", L"FgPresentWindow", L"Child", value, 64, ini.c_str());
	s_dlss.fFgChildWindow = _wcsicmp(value, L"Game") != 0;
	s_dlss.fFgAllowTearing = GetPrivateProfileIntW(L"DLSS", L"FgAllowTearing", 0, ini.c_str()) != 0;
	GetPrivateProfileStringW(L"DLSS", L"FgUnstick", L"Resize", value, 64, ini.c_str());
	s_dlss.unstickMode = _wcsicmp(value, L"Off") == 0 ? 0 : (_wcsicmp(value, L"Minimize") == 0 ? 2 : 1);
	s_dlss.glKickSetting = std::clamp(static_cast<int>(GetPrivateProfileIntW(L"DLSS", L"FgGlKickFrames", 0, ini.c_str())), 0, 600);
}

void SaveDlssSettings()
{
	const std::wstring ini = IniPath();
	WritePrivateProfileStringW(L"DLSS", L"Mode", s_apwchzQualityKey[s_dlss.dlssq], ini.c_str());
	WritePrivateProfileStringW(L"DLSS", L"Preset", std::to_wstring(s_dlss.preset).c_str(), ini.c_str());
	WritePrivateProfileStringW(L"DLSS", L"ShowStreamlineConsole", s_dlss.fShowConsole ? L"1" : L"0", ini.c_str());
	WritePrivateProfileStringW(L"DLSS", L"VerboseLog", s_dlss.fVerboseLog ? L"1" : L"0", ini.c_str());
	WritePrivateProfileStringW(L"DLSS", L"FlipJitterX", s_dlss.jitterSignX < 0.0f ? L"1" : L"0", ini.c_str());
	WritePrivateProfileStringW(L"DLSS", L"FlipJitterY", s_dlss.jitterSignY < 0.0f ? L"1" : L"0", ini.c_str());
	WritePrivateProfileStringW(L"DLSS", L"Backend", s_dlss.fPreferD3D12 ? L"D3D12" : L"Vulkan", ini.c_str());
	WritePrivateProfileStringW(L"DLSS", L"FrameGeneration", std::to_wstring(s_dlss.frameGen).c_str(), ini.c_str());
	WritePrivateProfileStringW(L"DLSS", L"ShowFps", std::to_wstring(s_dlss.fpsMode).c_str(), ini.c_str());
}

bool FHasGlExtension(const char* name)
{
	GLint count = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &count);
	for (GLint i = 0; i < count; ++i)
	{
		const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
		if (ext != nullptr && std::strcmp(ext, name) == 0)
			return true;
	}
	return false;
}

template <typename T>
bool LoadGlProc(T& fn, const char* name)
{
	// glad was initialised with glfwGetProcAddress; wglGetProcAddress is what
	// GLFW uses underneath on Windows for extension entry points.
	fn = reinterpret_cast<T>(wglGetProcAddress(name));
	return fn != nullptr;
}

bool LoadGlInterop()
{
	const char* required[] =
	{
		"GL_EXT_memory_object", "GL_EXT_memory_object_win32",
		"GL_EXT_semaphore", "GL_EXT_semaphore_win32"
	};
	for (const char* ext : required)
	{
		if (!FHasGlExtension(ext))
		{
			static std::string reason;
			reason = std::string("OpenGL driver lacks ") + ext;
			return FailDlss(reason.c_str());
		}
	}

	bool ok = true;
	ok &= LoadGlProc(s_gl.GetUnsignedBytei_v, "glGetUnsignedBytei_vEXT");
	ok &= LoadGlProc(s_gl.CreateMemoryObjects, "glCreateMemoryObjectsEXT");
	ok &= LoadGlProc(s_gl.DeleteMemoryObjects, "glDeleteMemoryObjectsEXT");
	ok &= LoadGlProc(s_gl.MemoryObjectParameteriv, "glMemoryObjectParameterivEXT");
	ok &= LoadGlProc(s_gl.TexStorageMem2D, "glTexStorageMem2DEXT");
	ok &= LoadGlProc(s_gl.ImportMemoryWin32Handle, "glImportMemoryWin32HandleEXT");
	ok &= LoadGlProc(s_gl.GenSemaphores, "glGenSemaphoresEXT");
	ok &= LoadGlProc(s_gl.DeleteSemaphores, "glDeleteSemaphoresEXT");
	ok &= LoadGlProc(s_gl.ImportSemaphoreWin32Handle, "glImportSemaphoreWin32HandleEXT");
	ok &= LoadGlProc(s_gl.SignalSemaphore, "glSignalSemaphoreEXT");
	ok &= LoadGlProc(s_gl.WaitSemaphore, "glWaitSemaphoreEXT");
	if (!ok)
		return FailDlss("OpenGL interop entry points missing");
	// Only needed by the D3D12 backend (LUID + D3D12 fence values).
	LoadGlProc(s_gl.GetUnsignedBytev, "glGetUnsignedBytevEXT");
	LoadGlProc(s_gl.SemaphoreParameterui64v, "glSemaphoreParameterui64vEXT");
	return true;
}

// =============================================================================
// Streamline loading
// =============================================================================
template <typename T>
bool LoadSlProc(T*& fn, const char* name)
{
	fn = reinterpret_cast<T*>(GetProcAddress(s_hmodInterposer, name));
	return fn != nullptr;
}

bool LoadStreamline(bool fD3D12)
{
	const std::wstring interposerPath =
		(std::filesystem::path(s_dlss.exeDirectory) / L"sl.interposer.dll").wstring();

	if (!std::filesystem::exists(interposerPath))
		return FailDlss("sl.interposer.dll not found next to the executable");

	// NVIDIA requires Streamline modules to be signature checked before load.
	if (!sl::security::verifyEmbeddedSignature(interposerPath.c_str()))
		return FailDlss("sl.interposer.dll is not signed by NVIDIA");

	s_hmodInterposer = LoadLibraryW(interposerPath.c_str());
	if (s_hmodInterposer == nullptr)
		return FailDlss("Failed to load sl.interposer.dll");

	bool ok = true;
	ok &= LoadSlProc(s_sl.Init, "slInit");
	ok &= LoadSlProc(s_sl.Shutdown, "slShutdown");
	ok &= LoadSlProc(s_sl.IsFeatureSupported, "slIsFeatureSupported");
	ok &= LoadSlProc(s_sl.IsFeatureLoaded, "slIsFeatureLoaded");
	ok &= LoadSlProc(s_sl.SetTagForFrame, "slSetTagForFrame");
	ok &= LoadSlProc(s_sl.SetConstants, "slSetConstants");
	ok &= LoadSlProc(s_sl.EvaluateFeature, "slEvaluateFeature");
	ok &= LoadSlProc(s_sl.FreeResources, "slFreeResources");
	ok &= LoadSlProc(s_sl.GetNewFrameToken, "slGetNewFrameToken");
	ok &= LoadSlProc(s_sl.GetFeatureFunction, "slGetFeatureFunction");
	ok &= LoadSlProc(s_sl.GetFeatureVersion, "slGetFeatureVersion");
	ok &= LoadSlProc(s_sl.SetD3DDevice, "slSetD3DDevice");
	ok &= LoadSlProc(s_sl.UpgradeInterface, "slUpgradeInterface");
	ok &= LoadSlProc(s_sl.GetNativeInterface, "slGetNativeInterface");
	ok &= LoadSlProc(s_sl.SetFeatureLoaded, "slSetFeatureLoaded");

	s_vk.vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
		GetProcAddress(s_hmodInterposer, "vkGetInstanceProcAddr"));
	s_vk.vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
		GetProcAddress(s_hmodInterposer, "vkGetDeviceProcAddr"));
	ok &= s_vk.vkGetInstanceProcAddr != nullptr && s_vk.vkGetDeviceProcAddr != nullptr;

	if (!ok)
		return FailDlss("sl.interposer.dll is missing expected exports (version mismatch?)");

	static std::wstring pluginPath;
	pluginPath = s_dlss.exeDirectory;
	static const wchar_t* pluginPaths[] = { nullptr };
	pluginPaths[0] = pluginPath.c_str();

	// D3D12: DLSS + Frame Generation (+ Reflex/PCL, which DLSS-G requires).
	static const sl::Feature featuresVk[] = { sl::kFeatureDLSS };
	static const sl::Feature featuresDx[] =
	{
		sl::kFeatureDLSS, sl::kFeatureDLSS_G, sl::kFeatureReflex, sl::kFeaturePCL
	};

	sl::Preferences pref{};
	pref.showConsole = s_dlss.fShowConsole;
	pref.logLevel = s_dlss.fVerboseLog ? sl::LogLevel::eVerbose : sl::LogLevel::eDefault;
	pref.pathsToPlugins = pluginPaths;
	pref.numPathsToPlugins = 1;
	pref.pathToLogsAndData = pluginPath.c_str();
	pref.logMessageCallback = SlLogCallback;
	pref.featuresToLoad = fD3D12 ? featuresDx : featuresVk;
	pref.numFeaturesToLoad = fD3D12 ? static_cast<uint32_t>(std::size(featuresDx)) : 1u;
	pref.engine = sl::EngineType::eCustom;
	pref.engineVersion = "ProjectCane";
	// Development project id. Shipping builds should use an application id
	// issued by NVIDIA (pref.applicationId).
	pref.projectId = "6c1f6e1e-2b0d-4c6a-9d8e-5c0a3a1d0cae";
	pref.renderAPI = fD3D12 ? sl::RenderAPI::eD3D12 : sl::RenderAPI::eVulkan;
	pref.flags |= sl::PreferenceFlags::eUseFrameBasedResourceTagging;
	// D3D12 is integrated with manual hooking: native device/factory, with
	// Streamline proxies only for the hooked calls (queue/swapchain creation,
	// Present, GetBuffer, ResizeBuffers, GetCurrentBackBufferIndex).
	if (fD3D12)
		pref.flags |= sl::PreferenceFlags::eUseManualHooking;

	const sl::Result res = s_sl.Init(pref, sl::kSDKVersion);
	if (res != sl::Result::eOk)
	{
		static std::string reason;
		reason = "slInit failed (" + std::to_string(static_cast<int>(res)) + ")";
		if (res == sl::Result::eErrorDriverOutOfDate)
			reason = "NVIDIA driver is out of date";
		return FailDlss(reason.c_str());
	}

	s_fSlInitialized = true;
	return true;
}

// =============================================================================
// Vulkan setup
// =============================================================================
template <typename T>
void LoadInstanceProc(T& fn, const char* name)
{
	fn = reinterpret_cast<T>(s_vk.vkGetInstanceProcAddr(s_dlss.instance, name));
}

template <typename T>
void LoadDeviceProc(T& fn, const char* name)
{
	fn = reinterpret_cast<T>(s_vk.vkGetDeviceProcAddr(s_dlss.device, name));
}

bool FDeviceHasExtensions(VkPhysicalDevice pd, const std::vector<const char*>& names)
{
	uint32_t count = 0;
	s_vk.vkEnumerateDeviceExtensionProperties(pd, nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> props(count);
	s_vk.vkEnumerateDeviceExtensionProperties(pd, nullptr, &count, props.data());
	for (const char* name : names)
	{
		bool found = false;
		for (const VkExtensionProperties& p : props)
			found |= std::strcmp(p.extensionName, name) == 0;
		if (!found)
			return false;
	}
	return true;
}

bool CreateVulkanDevice()
{
	s_vk.vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
		s_vk.vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance"));
	if (s_vk.vkCreateInstance == nullptr)
		return FailDlss("Vulkan runtime not available");

	VkApplicationInfo app{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
	app.pApplicationName = "ProjectCane";
	app.applicationVersion = 1;
	app.pEngineName = "ProjectCane";
	app.engineVersion = 1;
	app.apiVersion = VK_API_VERSION_1_3;

	const char* instanceExtensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	};

	VkInstanceCreateInfo ici{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	ici.pApplicationInfo = &app;
	ici.enabledExtensionCount = static_cast<uint32_t>(std::size(instanceExtensions));
	ici.ppEnabledExtensionNames = instanceExtensions;

	if (s_vk.vkCreateInstance(&ici, nullptr, &s_dlss.instance) != VK_SUCCESS)
		return FailDlss("vkCreateInstance failed");

#define DLSS_VK_LOAD_INSTANCE(name) LoadInstanceProc(s_vk.name, #name);
	DLSS_VK_INSTANCE_FUNCS(DLSS_VK_LOAD_INSTANCE)
#undef DLSS_VK_LOAD_INSTANCE

	// Pick the Vulkan physical device that is the same GPU as the GL context.
	GLubyte glUuid[16]{};
	s_gl.GetUnsignedBytei_v(kGL_DEVICE_UUID_EXT, 0, glUuid);

	uint32_t pdCount = 0;
	s_vk.vkEnumeratePhysicalDevices(s_dlss.instance, &pdCount, nullptr);
	std::vector<VkPhysicalDevice> pds(pdCount);
	s_vk.vkEnumeratePhysicalDevices(s_dlss.instance, &pdCount, pds.data());

	for (VkPhysicalDevice pd : pds)
	{
		VkPhysicalDeviceIDProperties idProps{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES };
		VkPhysicalDeviceProperties2 props2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		props2.pNext = &idProps;
		s_vk.vkGetPhysicalDeviceProperties2(pd, &props2);

		if (std::memcmp(idProps.deviceUUID, glUuid, VK_UUID_SIZE) == 0)
		{
			s_dlss.physicalDevice = pd;
			DlssLog("Vulkan device: %s (matches OpenGL context)", props2.properties.deviceName);
			break;
		}
	}

	if (s_dlss.physicalDevice == VK_NULL_HANDLE)
		return FailDlss("Could not find the OpenGL GPU in Vulkan (hybrid graphics? run the game on the NVIDIA GPU)");

	VkPhysicalDeviceProperties props{};
	s_vk.vkGetPhysicalDeviceProperties(s_dlss.physicalDevice, &props);
	if (props.vendorID != 0x10DE)
		return FailDlss("DLSS requires an NVIDIA RTX GPU");

	std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME,
		VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
		VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME,
		VK_KHR_EXTERNAL_SEMAPHORE_WIN32_EXTENSION_NAME,
		VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
		VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
	};
	if (!FDeviceHasExtensions(s_dlss.physicalDevice, deviceExtensions))
		return FailDlss("Vulkan driver lacks external memory/semaphore support");

	uint32_t qfCount = 0;
	s_vk.vkGetPhysicalDeviceQueueFamilyProperties(s_dlss.physicalDevice, &qfCount, nullptr);
	std::vector<VkQueueFamilyProperties> qfs(qfCount);
	s_vk.vkGetPhysicalDeviceQueueFamilyProperties(s_dlss.physicalDevice, &qfCount, qfs.data());
	bool foundQueue = false;
	for (uint32_t i = 0; i < qfCount; ++i)
	{
		const VkQueueFlags need = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
		if ((qfs[i].queueFlags & need) == need)
		{
			s_dlss.queueFamily = i;
			foundQueue = true;
			break;
		}
	}
	if (!foundQueue)
		return FailDlss("No Vulkan graphics queue");

	const float priority = 1.0f;
	VkDeviceQueueCreateInfo qci{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	qci.queueFamilyIndex = s_dlss.queueFamily;
	qci.queueCount = 1;
	qci.pQueuePriorities = &priority;

	VkDeviceCreateInfo dci{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	dci.queueCreateInfoCount = 1;
	dci.pQueueCreateInfos = &qci;
	dci.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	dci.ppEnabledExtensionNames = deviceExtensions.data();

	// This is Streamline's vkCreateDevice proxy: it appends the extensions,
	// features and queues DLSS needs and registers the device with Streamline.
	if (s_vk.vkCreateDevice(s_dlss.physicalDevice, &dci, nullptr, &s_dlss.device) != VK_SUCCESS)
		return FailDlss("vkCreateDevice failed");

#define DLSS_VK_LOAD_DEVICE(name) LoadDeviceProc(s_vk.name, #name);
	DLSS_VK_DEVICE_FUNCS(DLSS_VK_LOAD_DEVICE)
#undef DLSS_VK_LOAD_DEVICE

	if (s_vk.vkGetMemoryWin32HandleKHR == nullptr || s_vk.vkGetSemaphoreWin32HandleKHR == nullptr)
		return FailDlss("Vulkan Win32 handle export unavailable");

	s_vk.vkGetDeviceQueue(s_dlss.device, s_dlss.queueFamily, 0, &s_dlss.queue);
	s_vk.vkGetPhysicalDeviceMemoryProperties(s_dlss.physicalDevice, &s_dlss.memoryProperties);

	VkCommandPoolCreateInfo cpci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cpci.queueFamilyIndex = s_dlss.queueFamily;
	if (s_vk.vkCreateCommandPool(s_dlss.device, &cpci, nullptr, &s_dlss.commandPool) != VK_SUCCESS)
		return FailDlss("vkCreateCommandPool failed");

	for (FRAMESLOT& slot : s_dlss.frames)
	{
		VkCommandBufferAllocateInfo cbai{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		cbai.commandPool = s_dlss.commandPool;
		cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cbai.commandBufferCount = 1;
		s_vk.vkAllocateCommandBuffers(s_dlss.device, &cbai, &slot.cmd);

		VkFenceCreateInfo fci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		s_vk.vkCreateFence(s_dlss.device, &fci, nullptr, &slot.fence);
	}

	return true;
}

bool CreateSharedSemaphore(VkSemaphore* psem, HANDLE* phandle, GLuint* pglsem)
{
	VkExportSemaphoreCreateInfo esci{ VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO };
	esci.handleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
	VkSemaphoreCreateInfo sci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	sci.pNext = &esci;
	if (s_vk.vkCreateSemaphore(s_dlss.device, &sci, nullptr, psem) != VK_SUCCESS)
		return false;

	VkSemaphoreGetWin32HandleInfoKHR gi{ VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR };
	gi.semaphore = *psem;
	gi.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
	if (s_vk.vkGetSemaphoreWin32HandleKHR(s_dlss.device, &gi, phandle) != VK_SUCCESS)
		return false;

	s_gl.GenSemaphores(1, pglsem);
	s_gl.ImportSemaphoreWin32Handle(*pglsem, kGL_HANDLE_TYPE_OPAQUE_WIN32_EXT, *phandle);
	return glGetError() == GL_NO_ERROR;
}

uint32_t FindMemoryType(uint32_t typeBits, VkMemoryPropertyFlags flags)
{
	for (uint32_t i = 0; i < s_dlss.memoryProperties.memoryTypeCount; ++i)
	{
		if ((typeBits & (1u << i)) != 0 &&
			(s_dlss.memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
			return i;
	}
	return UINT32_MAX;
}

void DestroySharedImageVk(SHAREDIMAGE& img)
{
	if (img.glTexture != 0)
		glDeleteTextures(1, &img.glTexture);
	if (img.glMemory != 0)
		s_gl.DeleteMemoryObjects(1, &img.glMemory);
	if (img.view != VK_NULL_HANDLE)
		s_vk.vkDestroyImageView(s_dlss.device, img.view, nullptr);
	if (img.image != VK_NULL_HANDLE)
		s_vk.vkDestroyImage(s_dlss.device, img.image, nullptr);
	if (img.memory != VK_NULL_HANDLE)
		s_vk.vkFreeMemory(s_dlss.device, img.memory, nullptr);
	if (img.handle != nullptr)
		CloseHandle(img.handle);
	img = SHAREDIMAGE{};
}

bool CreateSharedImageVk(SHAREDIMAGE& img, int width, int height, VkFormat format, GLenum glFormat)
{
	img.width = width;
	img.height = height;
	img.format = format;
	img.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkExternalMemoryImageCreateInfo emici{ VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO };
	emici.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

	VkImageCreateInfo ici{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	ici.pNext = &emici;
	ici.imageType = VK_IMAGE_TYPE_2D;
	ici.format = format;
	ici.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
	ici.mipLevels = 1;
	ici.arrayLayers = 1;
	ici.samples = VK_SAMPLE_COUNT_1_BIT;
	ici.tiling = VK_IMAGE_TILING_OPTIMAL;
	ici.usage = img.usage;
	ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (s_vk.vkCreateImage(s_dlss.device, &ici, nullptr, &img.image) != VK_SUCCESS)
		return false;

	VkMemoryRequirements req{};
	s_vk.vkGetImageMemoryRequirements(s_dlss.device, img.image, &req);

	VkMemoryDedicatedAllocateInfo dedicated{ VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO };
	dedicated.image = img.image;
	VkExportMemoryAllocateInfo export_{ VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO };
	export_.pNext = &dedicated;
	export_.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

	VkMemoryAllocateInfo mai{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	mai.pNext = &export_;
	mai.allocationSize = req.size;
	mai.memoryTypeIndex = FindMemoryType(req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (mai.memoryTypeIndex == UINT32_MAX)
		return false;
	if (s_vk.vkAllocateMemory(s_dlss.device, &mai, nullptr, &img.memory) != VK_SUCCESS)
		return false;
	if (s_vk.vkBindImageMemory(s_dlss.device, img.image, img.memory, 0) != VK_SUCCESS)
		return false;

	VkImageViewCreateInfo ivci{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	ivci.image = img.image;
	ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
	ivci.format = format;
	ivci.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	if (s_vk.vkCreateImageView(s_dlss.device, &ivci, nullptr, &img.view) != VK_SUCCESS)
		return false;

	VkMemoryGetWin32HandleInfoKHR ghi{ VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR };
	ghi.memory = img.memory;
	ghi.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
	if (s_vk.vkGetMemoryWin32HandleKHR(s_dlss.device, &ghi, &img.handle) != VK_SUCCESS)
		return false;

	// Import into OpenGL.
	while (glGetError() != GL_NO_ERROR) {}
	s_gl.CreateMemoryObjects(1, &img.glMemory);
	const GLint dedicatedFlag = GL_TRUE;
	s_gl.MemoryObjectParameteriv(img.glMemory, kGL_DEDICATED_MEMORY_OBJECT_EXT, &dedicatedFlag);
	s_gl.ImportMemoryWin32Handle(img.glMemory, req.size, kGL_HANDLE_TYPE_OPAQUE_WIN32_EXT, img.handle);

	glGenTextures(1, &img.glTexture);
	glBindTexture(GL_TEXTURE_2D, img.glTexture);
	s_gl.TexStorageMem2D(GL_TEXTURE_2D, 1, glFormat, width, height, img.glMemory, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	const GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		DlssLog("GL import of shared image failed: 0x%x", err);
		return false;
	}
	return true;
}

// One-shot Vulkan command buffer helper (setup only).
template <typename Fn>
bool SubmitImmediate(Fn&& record)
{
	FRAMESLOT& slot = s_dlss.frames[0];
	s_vk.vkWaitForFences(s_dlss.device, 1, &slot.fence, VK_TRUE, UINT64_MAX);
	s_vk.vkResetCommandBuffer(slot.cmd, 0);

	VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	s_vk.vkBeginCommandBuffer(slot.cmd, &bi);
	record(slot.cmd);
	s_vk.vkEndCommandBuffer(slot.cmd);

	VkSubmitInfo si{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	si.commandBufferCount = 1;
	si.pCommandBuffers = &slot.cmd;
	const VkResult res = s_vk.vkQueueSubmit(s_dlss.queue, 1, &si, VK_NULL_HANDLE);
	s_vk.vkQueueWaitIdle(s_dlss.queue);
	return res == VK_SUCCESS;
}

void TransitionToGeneral(VkCommandBuffer cmd, const std::vector<VkImage>& images)
{
	std::vector<VkImageMemoryBarrier> barriers;
	for (VkImage image : images)
	{
		VkImageMemoryBarrier b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		b.srcAccessMask = 0;
		b.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		b.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		b.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		b.image = image;
		b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		barriers.push_back(b);
	}
	s_vk.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barriers.size()), barriers.data());
}

// =============================================================================
// Hidden swapchain so Streamline's per-frame present hook runs.
// =============================================================================
LRESULT CALLBACK HiddenWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	return DefWindowProcW(hwnd, msg, wp, lp);
}

void DestroyPresentHook()
{
	if (s_dlss.swapchain != VK_NULL_HANDLE)
		s_vk.vkDestroySwapchainKHR(s_dlss.device, s_dlss.swapchain, nullptr);
	if (s_dlss.acquireFence != VK_NULL_HANDLE)
		s_vk.vkDestroyFence(s_dlss.device, s_dlss.acquireFence, nullptr);
	if (s_dlss.surface != VK_NULL_HANDLE)
		s_vk.vkDestroySurfaceKHR(s_dlss.instance, s_dlss.surface, nullptr);
	if (s_dlss.hwndHidden != nullptr)
		DestroyWindow(s_dlss.hwndHidden);
	s_dlss.swapchain = VK_NULL_HANDLE;
	s_dlss.acquireFence = VK_NULL_HANDLE;
	s_dlss.surface = VK_NULL_HANDLE;
	s_dlss.hwndHidden = nullptr;
	s_dlss.fPresentHook = false;
}

bool CreatePresentHook()
{
	HINSTANCE hinst = GetModuleHandleW(nullptr);
	WNDCLASSW wc{};
	wc.lpfnWndProc = HiddenWndProc;
	wc.hInstance = hinst;
	wc.lpszClassName = L"ProjectCaneStreamlinePresent";
	RegisterClassW(&wc);

	s_dlss.hwndHidden = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, wc.lpszClassName,
		L"ProjectCane Streamline", WS_POPUP, 0, 0, 16, 16, nullptr, nullptr, hinst, nullptr);
	if (s_dlss.hwndHidden == nullptr)
		return false;

	VkWin32SurfaceCreateInfoKHR sci{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	sci.hinstance = hinst;
	sci.hwnd = s_dlss.hwndHidden;
	if (s_vk.vkCreateWin32SurfaceKHR(s_dlss.instance, &sci, nullptr, &s_dlss.surface) != VK_SUCCESS)
		return false;

	VkBool32 supported = VK_FALSE;
	s_vk.vkGetPhysicalDeviceSurfaceSupportKHR(s_dlss.physicalDevice, s_dlss.queueFamily, s_dlss.surface, &supported);
	if (!supported)
		return false;

	VkSurfaceCapabilitiesKHR caps{};
	s_vk.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_dlss.physicalDevice, s_dlss.surface, &caps);

	uint32_t formatCount = 0;
	s_vk.vkGetPhysicalDeviceSurfaceFormatsKHR(s_dlss.physicalDevice, s_dlss.surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	s_vk.vkGetPhysicalDeviceSurfaceFormatsKHR(s_dlss.physicalDevice, s_dlss.surface, &formatCount, formats.data());
	if (formats.empty())
		return false;

	uint32_t modeCount = 0;
	s_vk.vkGetPhysicalDeviceSurfacePresentModesKHR(s_dlss.physicalDevice, s_dlss.surface, &modeCount, nullptr);
	std::vector<VkPresentModeKHR> modes(modeCount);
	s_vk.vkGetPhysicalDeviceSurfacePresentModesKHR(s_dlss.physicalDevice, s_dlss.surface, &modeCount, modes.data());
	// Never block the game on this dummy swapchain.
	VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
	for (VkPresentModeKHR m : modes)
	{
		if (m == VK_PRESENT_MODE_IMMEDIATE_KHR) { mode = m; break; }
		if (m == VK_PRESENT_MODE_MAILBOX_KHR) mode = m;
	}

	VkExtent2D extent = caps.currentExtent;
	if (extent.width == UINT32_MAX || extent.width == 0 || extent.height == 0)
		extent = { 16, 16 };

	VkSwapchainCreateInfoKHR sc{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	sc.surface = s_dlss.surface;
	sc.minImageCount = (std::max)(caps.minImageCount, 2u);
	if (caps.maxImageCount != 0)
		sc.minImageCount = (std::min)(sc.minImageCount + 1, caps.maxImageCount);
	sc.imageFormat = formats[0].format;
	sc.imageColorSpace = formats[0].colorSpace;
	sc.imageExtent = extent;
	sc.imageArrayLayers = 1;
	sc.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	sc.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	sc.preTransform = caps.currentTransform;
	sc.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	sc.presentMode = mode;
	sc.clipped = VK_TRUE;
	if (s_vk.vkCreateSwapchainKHR(s_dlss.device, &sc, nullptr, &s_dlss.swapchain) != VK_SUCCESS)
		return false;

	uint32_t imageCount = 0;
	s_vk.vkGetSwapchainImagesKHR(s_dlss.device, s_dlss.swapchain, &imageCount, nullptr);
	std::vector<VkImage> images(imageCount);
	s_vk.vkGetSwapchainImagesKHR(s_dlss.device, s_dlss.swapchain, &imageCount, images.data());

	// Put every swapchain image into PRESENT_SRC once; nothing is ever drawn.
	SubmitImmediate([&](VkCommandBuffer cmd)
	{
		std::vector<VkImageMemoryBarrier> barriers;
		for (VkImage image : images)
		{
			VkImageMemoryBarrier b{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			b.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			b.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			b.image = image;
			b.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			barriers.push_back(b);
		}
		s_vk.vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, static_cast<uint32_t>(barriers.size()), barriers.data());
	});

	VkFenceCreateInfo fci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	s_vk.vkCreateFence(s_dlss.device, &fci, nullptr, &s_dlss.acquireFence);

	s_dlss.fPresentHook = true;
	return true;
}

void RunPresentHook()
{
	if (!s_dlss.fPresentHook)
		return;

	uint32_t index = 0;
	VkResult res = s_vk.vkAcquireNextImageKHR(s_dlss.device, s_dlss.swapchain, 0,
		VK_NULL_HANDLE, s_dlss.acquireFence, &index);
	if (res == VK_NOT_READY || res == VK_TIMEOUT)
		return;

	if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
	{
		if (++s_dlss.presentFailures > 8)
		{
			DlssLog("Hidden present hook failed repeatedly (%d); disabling it", static_cast<int>(res));
			DestroyPresentHook();
		}
		return;
	}

	s_vk.vkWaitForFences(s_dlss.device, 1, &s_dlss.acquireFence, VK_TRUE, UINT64_MAX);
	s_vk.vkResetFences(s_dlss.device, 1, &s_dlss.acquireFence);

	VkPresentInfoKHR pi{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	pi.swapchainCount = 1;
	pi.pSwapchains = &s_dlss.swapchain;
	pi.pImageIndices = &index;
	// Streamline's vkQueuePresentKHR proxy runs its common present hook here.
	res = s_vk.vkQueuePresentKHR(s_dlss.queue, &pi);
	if (res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR)
		s_dlss.presentFailures = 0;
	else if (++s_dlss.presentFailures > 8)
	{
		DlssLog("Hidden present failed repeatedly (%d); disabling it", static_cast<int>(res));
		DestroyPresentHook();
	}
}

// =============================================================================
// D3D12 backend (enables DLSS Frame Generation)
//
// Everything D3D12 sees is top-down (row 0 = top), the opposite of OpenGL's
// textures. GL therefore writes the shared inputs vertically flipped and flips
// the upscaled result back for the HUD; the final frame handed to DXGI is
// flipped once more so it lands upright in the swapchain.
// =============================================================================
template <typename T>
void SafeRelease(T*& p)
{
	if (p != nullptr)
	{
		p->Release();
		p = nullptr;
	}
}

uint64_t SignalDx()
{
	const uint64_t value = ++s_dlss.fenceValue;
	s_dlss.d3dQueue->Signal(s_dlss.d3dFence, value);
	return value;
}

void WaitDxCpu(uint64_t value)
{
	if (s_dlss.d3dFence == nullptr || s_dlss.d3dFence->GetCompletedValue() >= value)
		return;
	s_dlss.d3dFence->SetEventOnCompletion(value, s_dlss.hFenceEvent);
	if (WaitForSingleObject(s_dlss.hFenceEvent, 1000) == WAIT_TIMEOUT)
	{
		DlssLog("TIMEOUT: CPU wait for D3D12 fence value %llu (completed %llu, last issued %llu, frame %u, FG %s)",
			static_cast<unsigned long long>(value),
			static_cast<unsigned long long>(s_dlss.d3dFence->GetCompletedValue()),
			static_cast<unsigned long long>(s_dlss.fenceValue), s_dlss.frameIndex,
			s_dlss.mainSwap != nullptr ? "on" : "off");
	}
}

void WaitDxIdle()
{
	if (s_dlss.d3dQueue != nullptr && s_dlss.d3dFence != nullptr)
		WaitDxCpu(SignalDx());
}

// GL -> D3D12: GL writes to the listed textures are visible to the next
// D3D12 work on the queue.
void GlSignalDx(const GLuint* textures, int count)
{
	GLenum layouts[8];
	for (int i = 0; i < count && i < 8; ++i)
		layouts[i] = kGL_LAYOUT_GENERAL_EXT;
	const GLuint64 value = ++s_dlss.fenceValue;
	s_gl.SemaphoreParameterui64v(s_dlss.glD3dFence, kGL_D3D12_FENCE_VALUE_EXT, &value);
	s_gl.SignalSemaphore(s_dlss.glD3dFence, 0, nullptr, static_cast<GLuint>(count), textures, layouts);
	glFlush();
	s_dlss.d3dQueue->Wait(s_dlss.d3dFence, value);
}

// D3D12 -> GL: later GL commands wait for the D3D12 work submitted so far.
void DxSignalGl(const GLuint* textures, int count)
{
	GLenum layouts[8];
	for (int i = 0; i < count && i < 8; ++i)
		layouts[i] = kGL_LAYOUT_GENERAL_EXT;
	const GLuint64 value = SignalDx();
	s_gl.SemaphoreParameterui64v(s_dlss.glD3dFence, kGL_D3D12_FENCE_VALUE_EXT, &value);
	s_gl.WaitSemaphore(s_dlss.glD3dFence, 0, nullptr, static_cast<GLuint>(count), textures, layouts);
}

bool CreateD3D12Device()
{
	if (s_gl.GetUnsignedBytev == nullptr || s_gl.SemaphoreParameterui64v == nullptr)
		return FailDlss("OpenGL driver lacks D3D12 interop");

	s_dlss.hmodD3D12 = LoadLibraryW(L"d3d12.dll");
	s_dlss.hmodDxgi = LoadLibraryW(L"dxgi.dll");
	if (s_dlss.hmodD3D12 == nullptr || s_dlss.hmodDxgi == nullptr)
		return FailDlss("D3D12 runtime not available");

	auto pfnCreateDevice = reinterpret_cast<PFN_D3D12_CREATE_DEVICE>(
		GetProcAddress(s_dlss.hmodD3D12, "D3D12CreateDevice"));
	using PFN_CreateDXGIFactory2 = HRESULT(WINAPI*)(UINT, REFIID, void**);
	auto pfnCreateFactory = reinterpret_cast<PFN_CreateDXGIFactory2>(
		GetProcAddress(s_dlss.hmodDxgi, "CreateDXGIFactory2"));
	if (pfnCreateDevice == nullptr || pfnCreateFactory == nullptr)
		return FailDlss("D3D12 entry points missing");

	if (FAILED(pfnCreateFactory(0, __uuidof(IDXGIFactory4), reinterpret_cast<void**>(&s_dlss.dxgiFactory))))
		return FailDlss("CreateDXGIFactory2 failed");

	// Use the same GPU as the OpenGL context.
	GLubyte luidBytes[8]{};
	s_gl.GetUnsignedBytev(kGL_DEVICE_LUID_EXT, luidBytes);
	LUID luid{};
	std::memcpy(&luid, luidBytes, sizeof(luid));

	IDXGIAdapter1* adapter = nullptr;
	if (FAILED(s_dlss.dxgiFactory->EnumAdapterByLuid(luid, __uuidof(IDXGIAdapter1), reinterpret_cast<void**>(&adapter))))
		return FailDlss("Could not find the OpenGL GPU in DXGI (hybrid graphics? run the game on the NVIDIA GPU)");

	DXGI_ADAPTER_DESC1 desc{};
	adapter->GetDesc1(&desc);
	if (desc.VendorId != 0x10DE)
	{
		adapter->Release();
		return FailDlss("DLSS requires an NVIDIA RTX GPU");
	}

	const HRESULT hr = pfnCreateDevice(adapter, D3D_FEATURE_LEVEL_12_0,
		__uuidof(ID3D12Device), reinterpret_cast<void**>(&s_dlss.d3dDevice));
	adapter->Release();
	if (FAILED(hr))
		return FailDlss("D3D12CreateDevice failed");

	char name[128]{};
	WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, name, sizeof(name), nullptr, nullptr);
	DlssLog("D3D12 device: %s (matches OpenGL context)", name);

	if (s_sl.SetD3DDevice(s_dlss.d3dDevice) != sl::Result::eOk)
		return FailDlss("slSetD3DDevice failed");

	// Streamline proxies for the hooked device/factory calls. The proxy takes
	// over the reference it is given.
	s_dlss.d3dProxyDevice = s_dlss.d3dDevice;
	s_dlss.d3dProxyDevice->AddRef();
	if (s_sl.UpgradeInterface(reinterpret_cast<void**>(&s_dlss.d3dProxyDevice)) != sl::Result::eOk)
		return FailDlss("slUpgradeInterface(device) failed");

	IDXGIFactory2* factory2 = nullptr;
	if (FAILED(s_dlss.dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&factory2))))
		return FailDlss("IDXGIFactory2 unavailable");
	s_dlss.dxgiProxyFactory = factory2;
	if (s_sl.UpgradeInterface(reinterpret_cast<void**>(&s_dlss.dxgiProxyFactory)) != sl::Result::eOk)
		return FailDlss("slUpgradeInterface(factory) failed");

	IDXGIFactory5* factory5 = nullptr;
	if (SUCCEEDED(s_dlss.dxgiFactory->QueryInterface(__uuidof(IDXGIFactory5), reinterpret_cast<void**>(&factory5))))
	{
		BOOL allowTearing = FALSE;
		if (SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
			s_dlss.fTearing = allowTearing != FALSE;
		factory5->Release();
	}

	D3D12_COMMAND_QUEUE_DESC qd{};
	qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (FAILED(s_dlss.d3dProxyDevice->CreateCommandQueue(&qd, __uuidof(ID3D12CommandQueue), reinterpret_cast<void**>(&s_dlss.d3dQueue))))
		return FailDlss("CreateCommandQueue failed");

	for (DXFRAME& frame : s_dlss.dxFrames)
	{
		ID3D12Device* dev = s_dlss.d3dDevice;
		if (FAILED(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&frame.allocEval))) ||
			FAILED(dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), reinterpret_cast<void**>(&frame.allocPresent))) ||
			FAILED(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.allocEval, nullptr, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&frame.listEval))) ||
			FAILED(dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, frame.allocPresent, nullptr, __uuidof(ID3D12GraphicsCommandList), reinterpret_cast<void**>(&frame.listPresent))))
			return FailDlss("D3D12 command list creation failed");
		frame.listEval->Close();
		frame.listPresent->Close();
	}

	// One fence shared with OpenGL orders all GL <-> D3D12 work.
	if (FAILED(s_dlss.d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_SHARED, __uuidof(ID3D12Fence), reinterpret_cast<void**>(&s_dlss.d3dFence))) ||
		FAILED(s_dlss.d3dDevice->CreateSharedHandle(s_dlss.d3dFence, nullptr, GENERIC_ALL, nullptr, &s_dlss.hD3dFence)))
		return FailDlss("Shared D3D12 fence creation failed");
	s_dlss.hFenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

	while (glGetError() != GL_NO_ERROR) {}
	s_gl.GenSemaphores(1, &s_dlss.glD3dFence);
	s_gl.ImportSemaphoreWin32Handle(s_dlss.glD3dFence, kGL_HANDLE_TYPE_D3D12_FENCE_EXT, s_dlss.hD3dFence);
	if (glGetError() != GL_NO_ERROR)
		return FailDlss("OpenGL could not import the D3D12 fence");

	return true;
}

void DestroySharedImageDx(SHAREDIMAGE& img)
{
	if (img.glTexture != 0)
		glDeleteTextures(1, &img.glTexture);
	if (img.glMemory != 0)
		s_gl.DeleteMemoryObjects(1, &img.glMemory);
	SafeRelease(img.d3dResource);
	if (img.handle != nullptr)
		CloseHandle(img.handle);
	img = SHAREDIMAGE{};
}

bool CreateSharedImageDx(SHAREDIMAGE& img, int width, int height, DXGI_FORMAT format, GLenum glFormat)
{
	img.width = width;
	img.height = height;
	img.dxgiFormat = format;

	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC rd{};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rd.Width = static_cast<UINT64>(width);
	rd.Height = static_cast<UINT>(height);
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.Format = format;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	if (FAILED(s_dlss.d3dDevice->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_SHARED, &rd,
		D3D12_RESOURCE_STATE_COMMON, nullptr, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&img.d3dResource))))
		return false;
	if (FAILED(s_dlss.d3dDevice->CreateSharedHandle(img.d3dResource, nullptr, GENERIC_ALL, nullptr, &img.handle)))
		return false;

	const D3D12_RESOURCE_ALLOCATION_INFO info = s_dlss.d3dDevice->GetResourceAllocationInfo(0, 1, &rd);

	while (glGetError() != GL_NO_ERROR) {}
	s_gl.CreateMemoryObjects(1, &img.glMemory);
	const GLint dedicatedFlag = GL_TRUE;
	s_gl.MemoryObjectParameteriv(img.glMemory, kGL_DEDICATED_MEMORY_OBJECT_EXT, &dedicatedFlag);
	s_gl.ImportMemoryWin32Handle(img.glMemory, info.SizeInBytes, kGL_HANDLE_TYPE_D3D12_RESOURCE_EXT, img.handle);

	glGenTextures(1, &img.glTexture);
	glBindTexture(GL_TEXTURE_2D, img.glTexture);
	s_gl.TexStorageMem2D(GL_TEXTURE_2D, 1, glFormat, width, height, img.glMemory, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);

	const GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		DlssLog("GL import of D3D12 texture failed: 0x%x", err);
		return false;
	}
	return true;
}

// Blit with a vertical flip between two GL textures (no shader needed).
void BlitFlip(GLuint srcTex, int srcW, int srcH, GLuint dstTex, int dstW, int dstH)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, s_dlss.blitReadFbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTex, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, s_dlss.blitDrawFbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTex, 0);
	glDisable(GL_SCISSOR_TEST);
	glBlitFramebuffer(0, 0, srcW, srcH, 0, dstH, dstW, 0, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

DXFRAME& CurrentDxFrame()
{
	return s_dlss.dxFrames[s_dlss.frameIndex % kDxFrames];
}

ID3D12GraphicsCommandList* BeginDxList(bool fPresentList)
{
	DXFRAME& frame = CurrentDxFrame();
	ID3D12CommandAllocator* alloc = fPresentList ? frame.allocPresent : frame.allocEval;
	ID3D12GraphicsCommandList* list = fPresentList ? frame.listPresent : frame.listEval;
	// The allocator was last used kDxFrames frames ago.
	WaitDxCpu(frame.fenceValue);
	alloc->Reset();
	list->Reset(alloc, nullptr);
	return list;
}

void ExecuteDxList(ID3D12GraphicsCommandList* list)
{
	list->Close();
	ID3D12CommandList* lists[] = { list };
	s_dlss.d3dQueue->ExecuteCommandLists(1, lists);
}

// ---- Swapchains --------------------------------------------------------------
IDXGISwapChain3* CreateSwapchainDx(HWND hwnd, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC1 sd{};
	sd.Width = static_cast<UINT>(width);
	sd.Height = static_cast<UINT>(height);
	sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 3;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	sd.Flags = s_dlss.fTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	IDXGISwapChain1* swap1 = nullptr;
	const HRESULT hr = s_dlss.dxgiProxyFactory->CreateSwapChainForHwnd(s_dlss.d3dQueue, hwnd, &sd, nullptr, nullptr, &swap1);
	if (FAILED(hr) || swap1 == nullptr)
	{
		DlssLog("CreateSwapChainForHwnd failed (0x%08X)", static_cast<unsigned>(hr));
		return nullptr;
	}
	s_dlss.dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

	IDXGISwapChain3* swap3 = nullptr;
	swap1->QueryInterface(__uuidof(IDXGISwapChain3), reinterpret_cast<void**>(&swap3));
	swap1->Release();
	return swap3;
}

void DestroyHiddenSwapDx()
{
	if (s_dlss.hiddenSwap != nullptr)
	{
		WaitDxIdle();
		SafeRelease(s_dlss.hiddenSwap);
	}
}

bool CreateHiddenSwapDx()
{
	if (s_dlss.hwndHidden == nullptr)
	{
		HINSTANCE hinst = GetModuleHandleW(nullptr);
		WNDCLASSW wc{};
		wc.lpfnWndProc = DefWindowProcW;
		wc.hInstance = hinst;
		wc.lpszClassName = L"ProjectCaneStreamlinePresentDx";
		RegisterClassW(&wc);
		s_dlss.hwndHidden = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, wc.lpszClassName,
			L"ProjectCane Streamline", WS_POPUP, 0, 0, 16, 16, nullptr, nullptr, hinst, nullptr);
	}
	if (s_dlss.hwndHidden == nullptr)
		return false;
	s_dlss.hiddenSwap = CreateSwapchainDx(s_dlss.hwndHidden, 16, 16);
	return s_dlss.hiddenSwap != nullptr;
}

// Frame generation presents through DXGI. Presenting to the OpenGL window
// itself fights with the GL driver's own presentation of that window: DWM
// keeps showing the last OpenGL frame until something (e.g. Alt+Tab) makes it
// re-evaluate the window. A child window covering the client area has its own
// surface, so the DXGI image shows immediately. It is disabled and
// hit-test transparent, so mouse and keyboard input still reach the game.
LRESULT CALLBACK PresentChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_NCHITTEST:   return HTTRANSPARENT;
	case WM_ERASEBKGND:  return 1;
	case WM_PAINT:       ValidateRect(hwnd, nullptr); return 0;
	default:             return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
}

HWND CreatePresentChildWindow(int width, int height)
{
	if (s_dlss.hwndPresent != nullptr)
		return s_dlss.hwndPresent;
	HINSTANCE hinst = GetModuleHandleW(nullptr);
	static bool s_fRegistered = false;
	if (!s_fRegistered)
	{
		WNDCLASSW wc{};
		wc.lpfnWndProc = PresentChildWndProc;
		wc.hInstance = hinst;
		wc.hCursor = nullptr;
		wc.lpszClassName = L"ProjectCaneFrameGenPresent";
		RegisterClassW(&wc);
		s_fRegistered = true;
	}
	s_dlss.hwndPresent = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"ProjectCaneFrameGenPresent", L"",
		WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_CLIPSIBLINGS,
		0, 0, (std::max)(width, 1), (std::max)(height, 1), s_dlss.hwndGame, nullptr, hinst, nullptr);
	if (s_dlss.hwndPresent == nullptr)
		DlssLog("Frame Generation: could not create the present child window (%lu)", GetLastError());
	return s_dlss.hwndPresent;
}

void DestroyPresentChildWindow()
{
	if (s_dlss.hwndPresent != nullptr)
	{
		DestroyWindow(s_dlss.hwndPresent);
		s_dlss.hwndPresent = nullptr;
		// Make the OpenGL window repaint now that it is uncovered.
		if (s_dlss.hwndGame != nullptr)
			RedrawWindow(s_dlss.hwndGame, nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
	}
}

void ReleaseMainSwapDx()
{
	if (s_dlss.mainSwap == nullptr)
	{
		DestroyPresentChildWindow();
		return;
	}
	WaitDxIdle();
	SafeRelease(s_dlss.mainSwap);
	s_dlss.swapWidth = s_dlss.swapHeight = 0;
	DestroyPresentChildWindow();
}

// =============================================================================
// GL resources: motion-vector/depth prep pass and DLSS output framebuffer
// =============================================================================
const char* const s_szPrepVert = R"(#version 430 core
void main()
{
	// Fullscreen triangle.
	vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)";

const char* const s_szPrepFrag = R"(#version 430 core
layout(binding = 0) uniform sampler2D uSceneDepth;
uniform mat4 uClipToPrevClip;   // unjittered: prevWorldToClip * inverse(worldToClip)
uniform vec2 uJitterNdc;        // jitter added to this frame's projection
uniform vec2 uRenderSize;
uniform int  uStaticRectCount;
uniform ivec4 uStaticRects[4];  // x, y, width, height in render pixels
uniform int  uFlipY;            // 1 = write top-down (D3D12 backend)

layout(location = 0) out float outDepth;
layout(location = 1) out vec2 outMotion;

void main()
{
	// With uFlipY the output row y holds GL source row (height - 1 - y).
	ivec2 pixel = ivec2(gl_FragCoord.xy);
	if (uFlipY != 0)
		pixel.y = int(uRenderSize.y) - 1 - pixel.y;
	float depth = texelFetch(uSceneDepth, pixel, 0).r;
	outDepth = depth;

	for (int i = 0; i < uStaticRectCount; ++i)
	{
		ivec4 r = uStaticRects[i];
		if (pixel.x >= r.x && pixel.y >= r.y && pixel.x < r.x + r.z && pixel.y < r.y + r.w)
		{
			outMotion = vec2(0.0);
			return;
		}
	}

	// Reconstruct this pixel's unjittered clip position and reproject it with
	// last frame's camera. Result is in UV units (current -> previous), which
	// Streamline expects with mvecScale = {1, 1}.
	vec2 uv = (vec2(pixel) + 0.5) / uRenderSize;
	vec2 ndc = uv * 2.0 - 1.0 - uJitterNdc;
	vec4 clip = vec4(ndc, depth * 2.0 - 1.0, 1.0);
	vec4 prevClip = uClipToPrevClip * clip;
	vec2 prevNdc = prevClip.xy / max(abs(prevClip.w), 1e-6) * sign(prevClip.w);
	outMotion = (prevNdc - ndc) * 0.5;
	if (uFlipY != 0)
		outMotion.y = -outMotion.y;   // top-down images: +y points down
}
)";

GLuint CompileShader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	GLint ok = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		char log[2048]{};
		glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
		DlssLog("Prep shader compile error: %s", log);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

bool CreateGlResources()
{
	GLuint vs = CompileShader(GL_VERTEX_SHADER, s_szPrepVert);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, s_szPrepFrag);
	if (vs == 0 || fs == 0)
		return FailDlss("DLSS prep shader failed to compile");

	s_dlss.prepProgram = glCreateProgram();
	glAttachShader(s_dlss.prepProgram, vs);
	glAttachShader(s_dlss.prepProgram, fs);
	glLinkProgram(s_dlss.prepProgram);
	glDeleteShader(vs);
	glDeleteShader(fs);

	GLint linked = 0;
	glGetProgramiv(s_dlss.prepProgram, GL_LINK_STATUS, &linked);
	if (!linked)
		return FailDlss("DLSS prep shader failed to link");

	s_dlss.uClipToPrevClip = glGetUniformLocation(s_dlss.prepProgram, "uClipToPrevClip");
	s_dlss.uJitterNdc = glGetUniformLocation(s_dlss.prepProgram, "uJitterNdc");
	s_dlss.uRenderSize = glGetUniformLocation(s_dlss.prepProgram, "uRenderSize");
	s_dlss.uStaticRectCount = glGetUniformLocation(s_dlss.prepProgram, "uStaticRectCount");
	s_dlss.uStaticRects = glGetUniformLocation(s_dlss.prepProgram, "uStaticRects");
	s_dlss.uFlipY = glGetUniformLocation(s_dlss.prepProgram, "uFlipY");

	glGenVertexArrays(1, &s_dlss.prepVao);
	glGenFramebuffers(1, &s_dlss.prepFbo);
	glGenFramebuffers(1, &s_dlss.outputFbo);
	glGenRenderbuffers(1, &s_dlss.outputDepthStencil);
	glGenFramebuffers(1, &s_dlss.blitReadFbo);
	glGenFramebuffers(1, &s_dlss.blitDrawFbo);
	return true;
}

void DestroySharedImage(SHAREDIMAGE& img)
{
	if (s_dlss.backend == BACKEND_D3D12)
		DestroySharedImageDx(img);
	else
		DestroySharedImageVk(img);
}

bool CreateSharedImage(SHAREDIMAGE& img, int width, int height, VkFormat vkFormat, DXGI_FORMAT dxgiFormat, GLenum glFormat)
{
	return s_dlss.backend == BACKEND_D3D12
		? CreateSharedImageDx(img, width, height, dxgiFormat, glFormat)
		: CreateSharedImageVk(img, width, height, vkFormat, glFormat);
}

void WaitBackendIdle()
{
	glFinish();
	if (s_dlss.backend == BACKEND_Vulkan && s_dlss.device != VK_NULL_HANDLE)
		s_vk.vkDeviceWaitIdle(s_dlss.device);
	else if (s_dlss.backend == BACKEND_D3D12)
		WaitDxIdle();
}

void DestroySizedResources()
{
	if (s_dlss.backend == BACKEND_None)
		return;

	WaitBackendIdle();

	if (s_fSlInitialized && s_sl.FreeResources != nullptr)
		s_sl.FreeResources(sl::kFeatureDLSS, sl::ViewportHandle(0));

	DestroySharedImage(s_dlss.color);
	DestroySharedImage(s_dlss.depth);
	DestroySharedImage(s_dlss.motion);
	DestroySharedImage(s_dlss.output);
	if (s_dlss.composeTex != 0)
	{
		glDeleteTextures(1, &s_dlss.composeTex);
		s_dlss.composeTex = 0;
	}
	s_dlss.renderWidth = s_dlss.renderHeight = 0;
	s_dlss.outputWidth = s_dlss.outputHeight = 0;
}

bool EnsureSizedResources(int renderWidth, int renderHeight, int outputWidth, int outputHeight)
{
	if (renderWidth == s_dlss.renderWidth && renderHeight == s_dlss.renderHeight &&
		outputWidth == s_dlss.outputWidth && outputHeight == s_dlss.outputHeight)
		return true;

	DestroySizedResources();

	bool ok = true;
	ok &= CreateSharedImage(s_dlss.color, renderWidth, renderHeight, VK_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, GL_RGBA8);
	ok &= CreateSharedImage(s_dlss.depth, renderWidth, renderHeight, VK_FORMAT_R32_SFLOAT, DXGI_FORMAT_R32_FLOAT, GL_R32F);
	ok &= CreateSharedImage(s_dlss.motion, renderWidth, renderHeight, VK_FORMAT_R16G16_SFLOAT, DXGI_FORMAT_R16G16_FLOAT, GL_RG16F);
	ok &= CreateSharedImage(s_dlss.output, outputWidth, outputHeight, VK_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, GL_RGBA8);
	if (!ok)
	{
		DestroySizedResources();
		return FailDlss("Failed to create shared GL/D3D12/Vulkan images");
	}

	GLuint hudTarget = s_dlss.output.glTexture;
	if (s_dlss.backend == BACKEND_Vulkan)
	{
		SubmitImmediate([](VkCommandBuffer cmd)
		{
			TransitionToGeneral(cmd, { s_dlss.color.image, s_dlss.depth.image, s_dlss.motion.image, s_dlss.output.image });
		});
	}
	else
	{
		// D3D12 images are top-down; the HUD is drawn on a GL-orientation copy.
		glGenTextures(1, &s_dlss.composeTex);
		glBindTexture(GL_TEXTURE_2D, s_dlss.composeTex);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, outputWidth, outputHeight);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		hudTarget = s_dlss.composeTex;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, s_dlss.prepFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_dlss.depth.glTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, s_dlss.motion.glTexture, 0);
	const GLenum prepBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, prepBuffers);
	ok &= glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

	glBindRenderbuffer(GL_RENDERBUFFER, s_dlss.outputDepthStencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, outputWidth, outputHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, s_dlss.outputFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hudTarget, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, s_dlss.outputDepthStencil);
	ok &= glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (!ok)
	{
		DestroySizedResources();
		return FailDlss("DLSS framebuffers incomplete");
	}

	s_dlss.renderWidth = renderWidth;
	s_dlss.renderHeight = renderHeight;
	s_dlss.outputWidth = outputWidth;
	s_dlss.outputHeight = outputHeight;
	s_dlss.fResetHistory = true;
	DlssLog("Render %dx%d -> output %dx%d (%s)", renderWidth, renderHeight,
		outputWidth, outputHeight, s_apchzQuality[s_dlss.dlssq]);
	return true;
}

// =============================================================================
// Misc math
// =============================================================================
float Halton(uint32_t index, uint32_t base)
{
	float f = 1.0f;
	float r = 0.0f;
	while (index > 0)
	{
		f /= static_cast<float>(base);
		r += f * static_cast<float>(index % base);
		index /= base;
	}
	return r;
}

// Streamline matrices are "row-major" in the D3D row-vector sense. A GLM
// (column-major, column-vector) matrix has exactly that memory layout.
sl::float4x4 ToSl(const glm::mat4& m)
{
	sl::float4x4 out;
	std::memcpy(&out, &m, sizeof(out));
	return out;
}

sl::Resource MakeSlResource(const SHAREDIMAGE& img)
{
	sl::Resource r(sl::ResourceType::eTex2d, reinterpret_cast<void*>(img.image),
		reinterpret_cast<void*>(img.memory), reinterpret_cast<void*>(img.view),
		static_cast<uint32_t>(VK_IMAGE_LAYOUT_GENERAL));
	r.width = static_cast<uint32_t>(img.width);
	r.height = static_cast<uint32_t>(img.height);
	r.nativeFormat = static_cast<uint32_t>(img.format);
	r.mipLevels = 1;
	r.arrayLayers = 1;
	r.flags = 0;
	r.usage = static_cast<uint32_t>(img.usage);
	return r;
}

void ShutdownDx12Objects()
{
	if (s_dlss.d3dQueue != nullptr)
		WaitDxIdle();
	ReleaseMainSwapDx();
	DestroyHiddenSwapDx();
	DestroySharedImageDx(s_dlss.presentShared);
	for (DXFRAME& frame : s_dlss.dxFrames)
	{
		SafeRelease(frame.listEval);
		SafeRelease(frame.listPresent);
		SafeRelease(frame.allocEval);
		SafeRelease(frame.allocPresent);
		frame.fenceValue = 0;
	}
	if (s_dlss.glD3dFence != 0 && s_gl.DeleteSemaphores != nullptr)
		s_gl.DeleteSemaphores(1, &s_dlss.glD3dFence);
	s_dlss.glD3dFence = 0;
	SafeRelease(s_dlss.d3dFence);
	if (s_dlss.hD3dFence != nullptr)
		CloseHandle(s_dlss.hD3dFence);
	s_dlss.hD3dFence = nullptr;
	if (s_dlss.hFenceEvent != nullptr)
		CloseHandle(s_dlss.hFenceEvent);
	s_dlss.hFenceEvent = nullptr;
	SafeRelease(s_dlss.d3dQueue);
	SafeRelease(s_dlss.dxgiProxyFactory);
	SafeRelease(s_dlss.dxgiFactory);
	SafeRelease(s_dlss.d3dProxyDevice);
	SafeRelease(s_dlss.d3dDevice);
	s_dlss.fenceValue = 0;
}

void ShutdownBackend()
{
	if (s_dlss.backend != BACKEND_None)
		DestroySizedResources();

	// Streamline must shut down before the devices are destroyed.
	if (s_fSlInitialized)
	{
		if (s_dlss.backend == BACKEND_D3D12)
		{
			// Swapchains are hooked by Streamline: release them first.
			ReleaseMainSwapDx();
			DestroyHiddenSwapDx();
		}
		s_sl.Shutdown();
		s_fSlInitialized = false;
	}

	if (s_dlss.backend == BACKEND_D3D12)
		ShutdownDx12Objects();

	if (s_dlss.device != VK_NULL_HANDLE)
	{
		s_vk.vkDeviceWaitIdle(s_dlss.device);
		DestroyPresentHook();
		if (s_dlss.glSemGlDone != 0) s_gl.DeleteSemaphores(1, &s_dlss.glSemGlDone);
		if (s_dlss.glSemVkDone != 0) s_gl.DeleteSemaphores(1, &s_dlss.glSemVkDone);
		if (s_dlss.semGlDone != VK_NULL_HANDLE) s_vk.vkDestroySemaphore(s_dlss.device, s_dlss.semGlDone, nullptr);
		if (s_dlss.semVkDone != VK_NULL_HANDLE) s_vk.vkDestroySemaphore(s_dlss.device, s_dlss.semVkDone, nullptr);
		if (s_dlss.hSemGlDone != nullptr) CloseHandle(s_dlss.hSemGlDone);
		if (s_dlss.hSemVkDone != nullptr) CloseHandle(s_dlss.hSemVkDone);
		for (FRAMESLOT& slot : s_dlss.frames)
		{
			if (slot.fence != VK_NULL_HANDLE)
				s_vk.vkDestroyFence(s_dlss.device, slot.fence, nullptr);
			slot = FRAMESLOT{};
		}
		if (s_dlss.commandPool != VK_NULL_HANDLE)
			s_vk.vkDestroyCommandPool(s_dlss.device, s_dlss.commandPool, nullptr);
		s_vk.vkDestroyDevice(s_dlss.device, nullptr);
	}
	if (s_dlss.instance != VK_NULL_HANDLE && s_vk.vkDestroyInstance != nullptr)
		s_vk.vkDestroyInstance(s_dlss.instance, nullptr);

	s_dlss.device = VK_NULL_HANDLE;
	s_dlss.instance = VK_NULL_HANDLE;
	s_dlss.physicalDevice = VK_NULL_HANDLE;
	s_dlss.commandPool = VK_NULL_HANDLE;
	s_dlss.semGlDone = s_dlss.semVkDone = VK_NULL_HANDLE;
	s_dlss.hSemGlDone = s_dlss.hSemVkDone = nullptr;
	s_dlss.glSemGlDone = s_dlss.glSemVkDone = 0;

	if (s_dlss.hwndHidden != nullptr)
	{
		DestroyWindow(s_dlss.hwndHidden);
		s_dlss.hwndHidden = nullptr;
	}

	if (s_dlss.prepProgram != 0) glDeleteProgram(s_dlss.prepProgram);
	if (s_dlss.prepVao != 0) glDeleteVertexArrays(1, &s_dlss.prepVao);
	if (s_dlss.prepFbo != 0) glDeleteFramebuffers(1, &s_dlss.prepFbo);
	if (s_dlss.outputFbo != 0) glDeleteFramebuffers(1, &s_dlss.outputFbo);
	if (s_dlss.outputDepthStencil != 0) glDeleteRenderbuffers(1, &s_dlss.outputDepthStencil);
	if (s_dlss.blitReadFbo != 0) glDeleteFramebuffers(1, &s_dlss.blitReadFbo);
	if (s_dlss.blitDrawFbo != 0) glDeleteFramebuffers(1, &s_dlss.blitDrawFbo);
	if (s_dlss.presentFbo != 0) glDeleteFramebuffers(1, &s_dlss.presentFbo);
	if (s_dlss.presentGlTex != 0) glDeleteTextures(1, &s_dlss.presentGlTex);
	s_dlss.prepProgram = s_dlss.prepVao = s_dlss.prepFbo = s_dlss.outputFbo = s_dlss.outputDepthStencil = 0;
	s_dlss.blitReadFbo = s_dlss.blitDrawFbo = s_dlss.presentFbo = s_dlss.presentGlTex = 0;
	s_dlss.presentW = s_dlss.presentH = 0;

	if (s_hmodInterposer != nullptr)
	{
		FreeLibrary(s_hmodInterposer);
		s_hmodInterposer = nullptr;
	}
	s_sl = SLFUNCS{};
	s_dlss.backend = BACKEND_None;
	s_dlss.fFgSupported = false;
	s_dlss.fReflexOn = false;
	s_dlss.token = nullptr;
}

const char* PchzSupportError(sl::Result result)
{
	static std::string reason;
	switch (result)
	{
		case sl::Result::eErrorDriverOutOfDate: reason = "NVIDIA driver is out of date"; break;
		case sl::Result::eErrorOSOutOfDate: reason = "Windows is out of date"; break;
		case sl::Result::eErrorNoSupportedAdapterFound: reason = "No DLSS capable (RTX) GPU found"; break;
		case sl::Result::eErrorFeatureMissing: reason = "sl.dlss.dll / nvngx_dlss.dll missing"; break;
		default: reason = "DLSS not supported (" + std::to_string(static_cast<int>(result)) + ")"; break;
	}
	return reason.c_str();
}

template <typename T>
bool GetSlFeatureFunction(sl::Feature feature, const char* name, T*& fn)
{
	return s_sl.GetFeatureFunction(feature, name, reinterpret_cast<void*&>(fn)) == sl::Result::eOk && fn != nullptr;
}

bool InitializeDlss(BACKEND backend)
{
	s_dlss.backend = backend;
	const bool fD3D12 = backend == BACKEND_D3D12;

	if (!LoadGlInterop())
		return false;
	if (!LoadStreamline(fD3D12))
		return false;
	if (fD3D12 ? !CreateD3D12Device() : !CreateVulkanDevice())
		return false;

	// DLSS availability on this exact GPU.
	sl::AdapterInfo adapter{};
	LUID luid{};
	if (fD3D12)
	{
		luid = s_dlss.d3dDevice->GetAdapterLuid();
		adapter.deviceLUID = reinterpret_cast<uint8_t*>(&luid);
		adapter.deviceLUIDSizeInBytes = sizeof(LUID);
	}
	else
	{
		adapter.vkPhysicalDevice = s_dlss.physicalDevice;
	}
	const sl::Result supported = s_sl.IsFeatureSupported(sl::kFeatureDLSS, adapter);
	if (supported != sl::Result::eOk)
		return FailDlss(PchzSupportError(supported));

	bool ok = true;
	ok &= GetSlFeatureFunction(sl::kFeatureDLSS, "slDLSSGetOptimalSettings", s_sl.DLSSGetOptimalSettings);
	ok &= GetSlFeatureFunction(sl::kFeatureDLSS, "slDLSSSetOptions", s_sl.DLSSSetOptions);
	ok &= GetSlFeatureFunction(sl::kFeatureDLSS, "slDLSSGetState", s_sl.DLSSGetState);
	if (!ok)
		return FailDlss("DLSS plugin functions unavailable");

	sl::FeatureVersion version{};
	if (s_sl.GetFeatureVersion(sl::kFeatureDLSS, version) == sl::Result::eOk)
	{
		char buffer[128];
		std::snprintf(buffer, sizeof(buffer), "%s - SL %u.%u.%u / NGX %u.%u.%u",
			fD3D12 ? "D3D12" : "Vulkan",
			version.versionSL.major, version.versionSL.minor, version.versionSL.build,
			version.versionNGX.major, version.versionNGX.minor, version.versionNGX.build);
		s_dlss.featureVersion = buffer;
		DlssLog("DLSS loaded: %s", buffer);
	}

	if (fD3D12)
	{
		// Reflex / PC Latency markers (required by Frame Generation).
		GetSlFeatureFunction(sl::kFeatureReflex, "slReflexSetOptions", s_sl.ReflexSetOptions);
		GetSlFeatureFunction(sl::kFeatureReflex, "slReflexSleep", s_sl.ReflexSleep);
		GetSlFeatureFunction(sl::kFeaturePCL, "slPCLSetMarker", s_sl.PCLSetMarker);

		// Frame Generation. On RTX 20/30 NVIDIA's runtime reports it as
		// unsupported unless a compatibility layer is installed next to the exe.
		const sl::Result fgSupported = s_sl.IsFeatureSupported(sl::kFeatureDLSS_G, adapter);
		if (fgSupported == sl::Result::eOk &&
			GetSlFeatureFunction(sl::kFeatureDLSS_G, "slDLSSGGetState", s_sl.DLSSGGetState) &&
			GetSlFeatureFunction(sl::kFeatureDLSS_G, "slDLSSGSetOptions", s_sl.DLSSGSetOptions) &&
			s_sl.ReflexSetOptions != nullptr && s_sl.PCLSetMarker != nullptr)
		{
			s_dlss.fFgSupported = true;
			sl::DLSSGState fgState{};
			if (s_sl.DLSSGGetState(sl::ViewportHandle(0), fgState, nullptr) == sl::Result::eOk)
				s_dlss.fgMaxGenerated = static_cast<int>((std::max)(1u, fgState.numFramesToGenerateMax));
			DlssLog("Frame Generation available (up to %dx)", s_dlss.fgMaxGenerated + 1);
		}
		else
		{
			DlssLog("Frame Generation unavailable on this GPU/driver (%d)", static_cast<int>(fgSupported));
		}

		// DLSS-G stays unloaded until the player turns it on: it must only attach
		// to the game window's swapchain, never to the hidden helper swapchain.
		if (s_sl.SetFeatureLoaded != nullptr)
			s_sl.SetFeatureLoaded(sl::kFeatureDLSS_G, false);
	}
	else
	{
		if (!CreateSharedSemaphore(&s_dlss.semGlDone, &s_dlss.hSemGlDone, &s_dlss.glSemGlDone) ||
			!CreateSharedSemaphore(&s_dlss.semVkDone, &s_dlss.hSemVkDone, &s_dlss.glSemVkDone))
			return FailDlss("Failed to share semaphores between OpenGL and Vulkan");
	}

	if (!CreateGlResources())
		return false;

	const bool fPresentHook = fD3D12 ? CreateHiddenSwapDx() : CreatePresentHook();
	if (!fPresentHook)
	{
		DlssLog("Hidden present hook unavailable; Streamline garbage collection will be limited");
		if (fD3D12)
			DestroyHiddenSwapDx();
		else
			DestroyPresentHook();
	}

	s_dlss.fSupported = true;
	s_dlss.status = fD3D12 ? "Available (D3D12)" : "Available (Vulkan)";
	return true;
}
} // namespace

// =============================================================================
// Public API
// =============================================================================
void PreloadDlssModules()
{
	const std::filesystem::path exeDir = ExeDirectory();
	const std::wstring ini = (exeDir / L"DLSS.ini").wstring();

	// Default: a frame-generation compatibility layer for RTX 20/30 ships as a
	// version.dll proxy + dlssg_sm86.ini. Games normally import version.dll from
	// their exe; ProjectCane does not, and by the time Streamline would pull it
	// in, the graphics driver has already loaded the system copy - so load the
	// proxy explicitly, before the OpenGL context is created.
	std::wstring defaults;
	std::error_code error;
	if (std::filesystem::exists(exeDir / L"dlssg_sm86.ini", error))
		defaults = L"version.dll";

	wchar_t value[512]{};
	GetPrivateProfileStringW(L"DLSS", L"PreloadModules", defaults.c_str(), value, 512, ini.c_str());

	std::wstring list = value;
	size_t start = 0;
	while (start <= list.size())
	{
		size_t end = list.find(L',', start);
		if (end == std::wstring::npos)
			end = list.size();
		std::wstring name = list.substr(start, end - start);
		start = end + 1;

		const size_t first = name.find_first_not_of(L" \t");
		const size_t last = name.find_last_not_of(L" \t");
		if (first == std::wstring::npos)
			continue;
		name = name.substr(first, last - first + 1);

		const std::filesystem::path path = exeDir / name;
		if (!std::filesystem::exists(path, error))
		{
			std::wcout << L"[DLSS] PreloadModules: " << name << L" not found next to the exe" << std::endl;
			continue;
		}

		HMODULE existing = GetModuleHandleW(name.c_str());
		wchar_t existingPath[MAX_PATH]{};
		if (existing != nullptr)
			GetModuleFileNameW(existing, existingPath, MAX_PATH);

		HMODULE loaded = LoadLibraryW(path.wstring().c_str());
		if (loaded == nullptr)
		{
			std::wcout << L"[DLSS] PreloadModules: failed to load " << path.wstring()
				<< L" (error " << GetLastError() << L")" << std::endl;
			continue;
		}
		std::wcout << L"[DLSS] PreloadModules: loaded " << path.wstring() << std::endl;
		if (existing != nullptr && existing != loaded)
			std::wcout << L"[DLSS]   note: " << name << L" was already loaded from " << existingPath << std::endl;
	}
}

void StartupDlss(void* hwndGame)
{
	s_dlss.exeDirectory = ExeDirectory();
	s_dlss.hwndGame = static_cast<HWND>(hwndGame);
	LoadDlssSettings();

	// D3D12 first (needed for Frame Generation), Vulkan as the fallback.
	bool ok = false;
	if (s_dlss.fPreferD3D12)
	{
		ok = InitializeDlss(BACKEND_D3D12);
		if (!ok)
		{
			DlssLog("D3D12 backend failed (%s); falling back to Vulkan", s_dlss.status.c_str());
			ShutdownBackend();
		}
	}
	if (!ok)
	{
		ok = InitializeDlss(BACKEND_Vulkan);
		if (!ok)
		{
			const std::string reason = s_dlss.status;
			ShutdownBackend();
			s_dlss.status = reason;
		}
	}
	s_dlss.fSupported = ok;

	// Pick up the render size for the saved mode.
	if (FDlssActive())
		ApplyInternalResolutionSettings();
}

void ShutdownDlss()
{
	ShutdownBackend();
	s_dlss.fSupported = false;
}

bool FDlssSupported()
{
	return s_dlss.fSupported;
}

bool FDlssActive()
{
	return s_dlss.fSupported && s_dlss.dlssq != DLSSQ_Off;
}

DLSSQ DlssQuality()
{
	return s_dlss.dlssq;
}

void SetDlssQuality(DLSSQ dlssq)
{
	if (dlssq < DLSSQ_Off || dlssq >= DLSSQ_Max)
		dlssq = DLSSQ_Off;
	if (!s_dlss.fSupported)
		dlssq = DLSSQ_Off;

	s_dlss.dlssq = dlssq;
	s_dlss.fResetHistory = true;
	SaveDlssSettings();
	ApplyInternalResolutionSettings();
}

void SelectNextDlssQuality()
{
	if (!s_dlss.fSupported)
		return;
	SetDlssQuality(static_cast<DLSSQ>((s_dlss.dlssq + 1) % DLSSQ_Max));
}

const char* PchzDlssQuality(DLSSQ dlssq)
{
	if (dlssq < DLSSQ_Off || dlssq >= DLSSQ_Max)
		return "Off";
	return s_apchzQuality[dlssq];
}

const char* PchzDlssStatus()
{
	return s_dlss.status.c_str();
}

namespace
{
struct PRESETINFO { int value; const char* name; };
// sl::DLSSPreset values. J/K = transformer, L/M = second-gen transformer.
const PRESETINFO s_apresetinfo[] =
{
	{ 0, "Default" }, { 10, "J" }, { 11, "K" }, { 12, "L" }, { 13, "M" }
};
}

void SelectNextDlssPreset()
{
	int index = 0;
	for (int i = 0; i < static_cast<int>(std::size(s_apresetinfo)); ++i)
	{
		if (s_apresetinfo[i].value == s_dlss.preset)
			index = i;
	}
	index = (index + 1) % static_cast<int>(std::size(s_apresetinfo));
	s_dlss.preset = s_apresetinfo[index].value;
	s_dlss.fResetHistory = true;
	SaveDlssSettings();
}

const char* PchzDlssPreset()
{
	for (const PRESETINFO& p : s_apresetinfo)
	{
		if (p.value == s_dlss.preset)
			return p.name;
	}
	return "Default";
}

bool FDlssRenderSize(int outputWidth, int outputHeight, int* pRenderWidth, int* pRenderHeight)
{
	if (!FDlssActive() || s_sl.DLSSGetOptimalSettings == nullptr)
		return false;

	sl::DLSSOptions options{};
	options.mode = SlModeFromQuality(s_dlss.dlssq);
	options.outputWidth = static_cast<uint32_t>((std::max)(outputWidth, 1));
	options.outputHeight = static_cast<uint32_t>((std::max)(outputHeight, 1));

	sl::DLSSOptimalSettings settings{};
	if (s_sl.DLSSGetOptimalSettings(options, settings) != sl::Result::eOk ||
		settings.optimalRenderWidth == 0 || settings.optimalRenderHeight == 0)
	{
		// Fall back to the published DLSS scale factors.
		static const float s_scale[DLSSQ_Max] = { 1.0f, 1.0f, 0.667f, 0.58f, 0.5f, 0.333f };
		const float scale = s_scale[s_dlss.dlssq];
		*pRenderWidth = (std::max)(1, static_cast<int>(std::lround(outputWidth * scale)));
		*pRenderHeight = (std::max)(1, static_cast<int>(std::lround(outputHeight * scale)));
		return true;
	}

	*pRenderWidth = static_cast<int>(settings.optimalRenderWidth);
	*pRenderHeight = static_cast<int>(settings.optimalRenderHeight);
	return true;
}

void BeginDlssFrame(bool fUseDlss, int renderWidth, int renderHeight, int outputWidth, int outputHeight)
{
	// One Streamline frame token per rendered frame, shared by constants,
	// tags, evaluate, Reflex markers and Present.
	++s_dlss.frameIndex;
	s_dlss.token = nullptr;
	s_dlss.fUpscaledThisFrame = false;
	if (s_dlss.fSupported && s_sl.GetNewFrameToken != nullptr)
	{
		const uint32_t frame = s_dlss.frameIndex;
		s_sl.GetNewFrameToken(s_dlss.token, &frame);
		if (s_dlss.token != nullptr && s_dlss.backend == BACKEND_D3D12)
		{
			if (s_dlss.fReflexOn && s_sl.ReflexSleep != nullptr)
			{
				DLSSTIMER timer("slReflexSleep");
				s_sl.ReflexSleep(*s_dlss.token);
			}
			if (s_sl.PCLSetMarker != nullptr)
				s_sl.PCLSetMarker(sl::PCLMarker::eSimulationStart, *s_dlss.token);
		}
	}

	s_dlss.staticRectCount = 0;
	s_dlss.frameQuality = s_dlss.dlssq;
	s_dlss.frameRenderWidth = renderWidth;
	s_dlss.frameRenderHeight = renderHeight;
	s_dlss.frameOutputWidth = outputWidth;
	s_dlss.frameOutputHeight = outputHeight;
	s_dlss.fJitterThisFrame = fUseDlss && s_dlss.fJitterEnabled;
	if (!s_dlss.fJitterThisFrame)
	{
		s_dlss.jitterPixels = glm::vec2(0.0f);
		s_dlss.jitterNdc = glm::vec2(0.0f);
		return;
	}

	// NVIDIA recommends a Halton(2,3) sequence with at least 8 phases scaled
	// by the square of the upscale ratio.
	const float ratio = static_cast<float>((std::max)(outputHeight, 1)) / static_cast<float>((std::max)(renderHeight, 1));
	const uint32_t phases = (std::max)(8u, static_cast<uint32_t>(std::ceil(8.0f * ratio * ratio)));
	const uint32_t index = (s_dlss.frameIndex % phases) + 1;

	s_dlss.jitterPixels = glm::vec2(Halton(index, 2) - 0.5f, Halton(index, 3) - 0.5f);
	s_dlss.jitterNdc = glm::vec2(
		2.0f * s_dlss.jitterPixels.x / static_cast<float>((std::max)(renderWidth, 1)),
		2.0f * s_dlss.jitterPixels.y / static_cast<float>((std::max)(renderHeight, 1)));
}

void UploadCmUbo(const void* pcmgl, unsigned int cb)
{
	// cb is sizeof(CMGL): mat4 matWorldToClip followed by vec4 cameraPos.
	alignas(16) unsigned char data[256];
	cb = (std::min)(cb, static_cast<unsigned int>(sizeof(data)));
	std::memcpy(data, pcmgl, cb);

	if (s_dlss.fJitterThisFrame && cb >= sizeof(glm::mat4))
	{
		glm::mat4 matWorldToClip;
		std::memcpy(&matWorldToClip, data, sizeof(glm::mat4));
		glm::mat4 jitter(1.0f);
		jitter[3][0] = s_dlss.jitterNdc.x;
		jitter[3][1] = s_dlss.jitterNdc.y;
		matWorldToClip = jitter * matWorldToClip;
		std::memcpy(data, &matWorldToClip, sizeof(glm::mat4));
	}

	glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, cb, data);
}

void BlitDlssStaticRects(GLuint sceneColorTex, int renderWidth, int renderHeight,
	GLuint dstFbo, int outputWidth, int outputHeight)
{
	if (s_dlss.staticRectCount <= 0 || sceneColorTex == 0 || dstFbo == 0 ||
		renderWidth <= 0 || renderHeight <= 0)
		return;

	static GLuint s_readFbo = 0;
	if (s_readFbo == 0)
		glGenFramebuffers(1, &s_readFbo);

	const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
	glDisable(GL_SCISSOR_TEST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, s_readFbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneColorTex, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);

	const double sx = static_cast<double>(outputWidth) / static_cast<double>(renderWidth);
	const double sy = static_cast<double>(outputHeight) / static_cast<double>(renderHeight);
	for (int i = 0; i < s_dlss.staticRectCount; ++i)
	{
		const glm::ivec4 r = s_dlss.staticRects[i];
		const GLint x0 = std::clamp(r.x, 0, renderWidth);
		const GLint y0 = std::clamp(r.y, 0, renderHeight);
		const GLint x1 = std::clamp(r.x + r.z, 0, renderWidth);
		const GLint y1 = std::clamp(r.y + r.w, 0, renderHeight);
		if (x1 <= x0 || y1 <= y0)
			continue;
		glBlitFramebuffer(x0, y0, x1, y1,
			static_cast<GLint>(std::lround(x0 * sx)), static_cast<GLint>(std::lround(y0 * sy)),
			static_cast<GLint>(std::lround(x1 * sx)), static_cast<GLint>(std::lround(y1 * sy)),
			GL_COLOR_BUFFER_BIT, (renderWidth == outputWidth && renderHeight == outputHeight) ? GL_NEAREST : GL_LINEAR);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, dstFbo);
	if (scissor)
		glEnable(GL_SCISSOR_TEST);
}

void BlitSceneDepthStencil(GLuint sceneDepthTex, int renderWidth, int renderHeight,
	GLuint dstFbo, int outputWidth, int outputHeight)
{
	if (sceneDepthTex == 0 || dstFbo == 0 || renderWidth <= 0 || renderHeight <= 0)
		return;

	static GLuint s_readFbo = 0;
	if (s_readFbo == 0)
		glGenFramebuffers(1, &s_readFbo);

	const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
	GLboolean depthMask = GL_TRUE;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
	glDisable(GL_SCISSOR_TEST);
	glDepthMask(GL_TRUE);
	glStencilMask(0xFF);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, s_readFbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, sceneDepthTex, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFbo);
	if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		glBlitFramebuffer(0, 0, renderWidth, renderHeight, 0, 0, outputWidth, outputHeight,
			GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	}
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, dstFbo);

	glDepthMask(depthMask);
	if (scissor)
		glEnable(GL_SCISSOR_TEST);
}

void SetDlssExternalJitter(float jitterNdcX, float jitterNdcY)
{
	s_dlss.fJitterThisFrame = true;
	s_dlss.jitterNdc = glm::vec2(jitterNdcX, jitterNdcY);
	s_dlss.jitterPixels = glm::vec2(0.0f);   // DLSS does not run this frame
}

void AddDlssStaticRect(int x, int y, int width, int height)
{
	if (!s_dlss.fJitterThisFrame || s_dlss.staticRectCount >= kMaxStaticRects)
		return;
	s_dlss.staticRects[s_dlss.staticRectCount++] = glm::ivec4(x, y, width, height);
}

void ResetDlssHistory()
{
	s_dlss.fResetHistory = true;
}

GLuint DlssOutputTexture()
{
	return s_dlss.backend == BACKEND_D3D12 ? s_dlss.composeTex : s_dlss.output.glTexture;
}

namespace
{
struct GLSTATESAVE
{
	GLint program = 0;
	GLint vao = 0;
	GLint activeTexture = 0;
	GLint texture0 = 0;
	GLboolean depthTest = GL_FALSE;
	GLboolean blend = GL_FALSE;
	GLboolean cull = GL_FALSE;
	GLboolean stencil = GL_FALSE;
	GLboolean scissor = GL_FALSE;
	GLboolean depthMask = GL_TRUE;

	void Save()
	{
		glGetIntegerv(GL_CURRENT_PROGRAM, &program);
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vao);
		glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
		glActiveTexture(GL_TEXTURE0);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture0);
		depthTest = glIsEnabled(GL_DEPTH_TEST);
		blend = glIsEnabled(GL_BLEND);
		cull = glIsEnabled(GL_CULL_FACE);
		stencil = glIsEnabled(GL_STENCIL_TEST);
		scissor = glIsEnabled(GL_SCISSOR_TEST);
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
	}

	static void Set(GLenum cap, GLboolean on)
	{
		if (on) glEnable(cap); else glDisable(cap);
	}

	void Restore() const
	{
		glUseProgram(program);
		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture0);
		glActiveTexture(activeTexture);
		Set(GL_DEPTH_TEST, depthTest);
		Set(GL_BLEND, blend);
		Set(GL_CULL_FACE, cull);
		Set(GL_STENCIL_TEST, stencil);
		Set(GL_SCISSOR_TEST, scissor);
		glDepthMask(depthMask);
	}
};

bool FDetectCameraCut(const DLSSCAMERA& cam)
{
	if (!s_dlss.fHavePrevCamera)
		return true;
	if (!s_dlss.fCameraCutDetection)
		return false;

	// Forward direction from the inverse view (GL convention: -Z forward).
	const glm::mat4 view = glm::inverse(cam.matProj) * cam.matWorldToClip;
	const glm::mat4 eye = glm::inverse(view);
	const glm::vec3 forward = -glm::normalize(glm::vec3(eye[2]));

	// More than ~40 degrees of rotation in one frame is a cut, not a pan.
	if (glm::dot(forward, s_dlss.dirPrevForward) < 0.766f)
		return true;

	// Reproject the screen centre at mid depth; if it lands far off screen
	// the camera teleported.
	const glm::mat4 clipToPrev = s_dlss.matPrevWorldToClip * glm::inverse(cam.matWorldToClip);
	const glm::vec4 prev = clipToPrev * glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);
	if (prev.w <= 0.0f)
		return true;
	const glm::vec2 prevNdc = glm::vec2(prev) / prev.w;
	return glm::length(prevNdc) > 1.5f;
}
} // namespace

bool FEvaluateDlss(const DLSSCAMERA& cam, GLuint sceneColorTex, GLuint sceneDepthTex)
{
	s_dlss.fUpscaledThisFrame = false;
	if (!FDlssActive() || s_sl.DLSSSetOptions == nullptr || s_dlss.token == nullptr)
		return false;

	const int rw = s_dlss.frameRenderWidth;
	const int rh = s_dlss.frameRenderHeight;
	const int ow = s_dlss.frameOutputWidth;
	const int oh = s_dlss.frameOutputHeight;
	if (rw <= 0 || rh <= 0 || ow <= 0 || oh <= 0)
		return false;
	// Minimized window (GLFW reports a tiny framebuffer): nothing to upscale.
	if (ow < 64 || oh < 64)
		return false;

	// The mode was changed from a menu during this frame: the scene was
	// rendered at the old mode's resolution, which the new mode rejects
	// (NGX 0xbad00005). Show this frame un-upscaled; the next one matches.
	if (s_dlss.dlssq != s_dlss.frameQuality)
		return false;

	const bool fDx = s_dlss.backend == BACKEND_D3D12;
	const bool fFgActive = fDx && s_dlss.mainSwap != nullptr;

	GLint previousFbo = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFbo);

	if (!EnsureSizedResources(rw, rh, ow, oh))
	{
		// Creation failed: DLSS has been disabled. Put the engine back on its
		// normal path for the following frames.
		glBindFramebuffer(GL_FRAMEBUFFER, previousFbo);
		ApplyInternalResolutionSettings();
		return false;
	}

	GLSTATESAVE state;
	state.Save();

	// --- 1. Copy scene color into the shared input -----------------------------
	// (D3D12 images are top-down, so that copy is flipped.)
	if (fDx)
		BlitFlip(sceneColorTex, rw, rh, s_dlss.color.glTexture, rw, rh);
	else
		glCopyImageSubData(sceneColorTex, GL_TEXTURE_2D, 0, 0, 0, 0,
			s_dlss.color.glTexture, GL_TEXTURE_2D, 0, 0, 0, 0, rw, rh, 1);

	// --- 2. Depth + camera motion vectors ---------------------------------------
	if (FDetectCameraCut(cam))
		s_dlss.fResetHistory = true;

	const glm::mat4 prevWorldToClip = s_dlss.fHavePrevCamera ? s_dlss.matPrevWorldToClip : cam.matWorldToClip;
	const glm::mat4 clipToPrevClip = prevWorldToClip * glm::inverse(cam.matWorldToClip);

	glBindFramebuffer(GL_FRAMEBUFFER, s_dlss.prepFbo);
	glViewport(0, 0, rw, rh);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDepthMask(GL_FALSE);
	glUseProgram(s_dlss.prepProgram);
	glUniformMatrix4fv(s_dlss.uClipToPrevClip, 1, GL_FALSE, &clipToPrevClip[0][0]);
	glUniform2f(s_dlss.uJitterNdc, s_dlss.jitterNdc.x, s_dlss.jitterNdc.y);
	glUniform2f(s_dlss.uRenderSize, static_cast<float>(rw), static_cast<float>(rh));
	glUniform1i(s_dlss.uFlipY, fDx ? 1 : 0);
	glUniform1i(s_dlss.uStaticRectCount, s_dlss.staticRectCount);
	if (s_dlss.staticRectCount > 0)
		glUniform4iv(s_dlss.uStaticRects, s_dlss.staticRectCount, &s_dlss.staticRects[0].x);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
	glBindVertexArray(s_dlss.prepVao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindTexture(GL_TEXTURE_2D, 0);

	// --- 3. Streamline inputs ---------------------------------------------------
	const sl::FrameToken& token = *s_dlss.token;
	const sl::ViewportHandle viewportHandle(0);

	sl::DLSSOptions options{};
	options.mode = SlModeFromQuality(s_dlss.dlssq);
	options.outputWidth = static_cast<uint32_t>(ow);
	options.outputHeight = static_cast<uint32_t>(oh);
	options.colorBuffersHDR = sl::Boolean::eFalse;      // LDR RGBA8 scene
	options.useAutoExposure = sl::Boolean::eFalse;
	const sl::DLSSPreset preset = static_cast<sl::DLSSPreset>(s_dlss.preset);
	options.dlaaPreset = preset;
	options.qualityPreset = preset;
	options.balancedPreset = preset;
	options.performancePreset = preset;
	options.ultraPerformancePreset = preset;
	options.ultraQualityPreset = preset;
	s_sl.DLSSSetOptions(viewportHandle, options);

	const glm::mat4 view = glm::inverse(cam.matProj) * cam.matWorldToClip;
	const glm::mat4 eye = glm::inverse(view);

	// Top-down images (D3D12) see clip space with Y flipped.
	const glm::mat4 flipY = fDx ? glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f)) : glm::mat4(1.0f);
	const glm::mat4 viewToClip = flipY * cam.matProj;
	const glm::mat4 clipToPrevClipSl = flipY * clipToPrevClip * flipY;

	sl::Constants consts{};
	consts.cameraViewToClip = ToSl(viewToClip);
	consts.clipToCameraView = ToSl(glm::inverse(viewToClip));
	consts.clipToLensClip = ToSl(glm::mat4(1.0f));
	consts.clipToPrevClip = ToSl(clipToPrevClipSl);
	consts.prevClipToClip = ToSl(glm::inverse(clipToPrevClipSl));
	consts.jitterOffset = sl::float2(s_dlss.jitterPixels.x * s_dlss.jitterSignX,
		s_dlss.jitterPixels.y * s_dlss.jitterSignY * (fDx ? -1.0f : 1.0f));
	consts.mvecScale = sl::float2(1.0f, 1.0f);
	consts.cameraPinholeOffset = sl::float2(0.0f, 0.0f);
	consts.cameraPos = sl::float3(cam.pos.x, cam.pos.y, cam.pos.z);
	consts.cameraRight = sl::float3(eye[0].x, eye[0].y, eye[0].z);
	consts.cameraUp = sl::float3(eye[1].x, eye[1].y, eye[1].z);
	consts.cameraFwd = sl::float3(-eye[2].x, -eye[2].y, -eye[2].z);
	consts.cameraNear = cam.sNearClip;
	consts.cameraFar = cam.sFarClip;
	consts.cameraFOV = cam.radFov;
	consts.cameraAspectRatio = static_cast<float>(rw) / static_cast<float>(rh);
	consts.depthInverted = sl::Boolean::eFalse;
	consts.cameraMotionIncluded = sl::Boolean::eTrue;
	consts.motionVectors3D = sl::Boolean::eFalse;
	consts.reset = s_dlss.fResetHistory ? sl::Boolean::eTrue : sl::Boolean::eFalse;
	consts.orthographicProjection = sl::Boolean::eFalse;
	consts.motionVectorsDilated = sl::Boolean::eFalse;
	consts.motionVectorsJittered = sl::Boolean::eFalse;
	s_sl.SetConstants(consts, token, viewportHandle);

	sl::Extent renderExtent{ 0, 0, static_cast<uint32_t>(rw), static_cast<uint32_t>(rh) };
	sl::Extent outputExtent{ 0, 0, static_cast<uint32_t>(ow), static_cast<uint32_t>(oh) };

	// -------------------------------------------------------------------------
	// DLSS 5 (neural rendering) hook point.
	//
	// NVIDIA has announced that DLSS 5 ships as a Streamline feature, but no
	// public plugin/header exists yet (Streamline 2.14.1). When it does:
	//   1. add its sl::Feature to the feature lists in LoadStreamline(),
	//   2. check it with slIsFeatureSupported() in InitializeDlss(),
	//   3. set its options here and call s_sl.EvaluateFeature() for it after
	//      the DLSS-SR evaluate below, reusing the same frame token and tags
	//      (color, depth and motion vectors are already provided).
	// -------------------------------------------------------------------------

	const sl::BaseStructure* inputs[] = { &viewportHandle };
	sl::Result evalResult = sl::Result::eOk;
	bool fSubmitted = false;

	const GLuint sharedTextures[4] =
	{
		s_dlss.color.glTexture, s_dlss.depth.glTexture, s_dlss.motion.glTexture, s_dlss.output.glTexture
	};

	if (fDx)
	{
		// --- 4a. D3D12 ------------------------------------------------------------
		GlSignalDx(sharedTextures, 4);

		ID3D12GraphicsCommandList* list = nullptr;
		{
			DLSSTIMER timer("evaluate: wait for command allocator");
			list = BeginDxList(false);
		}

		auto makeRes = [](const SHAREDIMAGE& img)
		{
			sl::Resource r(sl::ResourceType::eTex2d, img.d3dResource,
				static_cast<uint32_t>(D3D12_RESOURCE_STATE_COMMON));
			r.width = static_cast<uint32_t>(img.width);
			r.height = static_cast<uint32_t>(img.height);
			r.nativeFormat = static_cast<uint32_t>(img.dxgiFormat);
			r.mipLevels = 1;
			r.arrayLayers = 1;
			return r;
		};
		sl::Resource colorIn = makeRes(s_dlss.color);
		sl::Resource depthIn = makeRes(s_dlss.depth);
		sl::Resource motionIn = makeRes(s_dlss.motion);
		sl::Resource colorOut = makeRes(s_dlss.output);

		// With Frame Generation on, depth/motion are read again at Present on
		// Streamline's own queue, after OpenGL may already be rendering the
		// next frame into them, so let Streamline take copies now.
		const sl::ResourceLifecycle fgInputs = fFgActive
			? sl::ResourceLifecycle::eOnlyValidNow
			: sl::ResourceLifecycle::eValidUntilPresent;
		const sl::ResourceTag tags[] =
		{
			sl::ResourceTag(&colorIn, sl::kBufferTypeScalingInputColor, sl::ResourceLifecycle::eValidUntilPresent, &renderExtent),
			sl::ResourceTag(&depthIn, sl::kBufferTypeDepth, fgInputs, &renderExtent),
			sl::ResourceTag(&motionIn, sl::kBufferTypeMotionVectors, fgInputs, &renderExtent),
			sl::ResourceTag(&colorOut, sl::kBufferTypeScalingOutputColor, sl::ResourceLifecycle::eValidUntilPresent, &outputExtent),
		};
		s_sl.SetTagForFrame(token, viewportHandle, tags, static_cast<uint32_t>(std::size(tags)),
			reinterpret_cast<sl::CommandBuffer*>(list));

		{
			DLSSTIMER timer("evaluate: slSetTag/slEvaluateFeature");
			evalResult = s_sl.EvaluateFeature(sl::kFeatureDLSS, token, inputs,
				static_cast<uint32_t>(std::size(inputs)), reinterpret_cast<sl::CommandBuffer*>(list));
		}

		ExecuteDxList(list);
		DxSignalGl(sharedTextures, 4);
		CurrentDxFrame().fenceValue = s_dlss.fenceValue;
		fSubmitted = true;

		// Back to GL orientation for the HUD.
		BlitFlip(s_dlss.output.glTexture, ow, oh, s_dlss.composeTex, ow, oh);
	}
	else
	{
		// --- 4b. Vulkan -----------------------------------------------------------
		const GLenum layouts[4] =
		{
			kGL_LAYOUT_GENERAL_EXT, kGL_LAYOUT_GENERAL_EXT, kGL_LAYOUT_GENERAL_EXT, kGL_LAYOUT_GENERAL_EXT
		};
		s_gl.SignalSemaphore(s_dlss.glSemGlDone, 0, nullptr, 4, sharedTextures, layouts);
		glFlush();

		FRAMESLOT& slot = s_dlss.frames[s_dlss.frameIndex % kFramesInFlight];
		s_vk.vkWaitForFences(s_dlss.device, 1, &slot.fence, VK_TRUE, UINT64_MAX);
		s_vk.vkResetFences(s_dlss.device, 1, &slot.fence);
		s_vk.vkResetCommandBuffer(slot.cmd, 0);

		VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		s_vk.vkBeginCommandBuffer(slot.cmd, &bi);

		sl::Resource colorIn = MakeSlResource(s_dlss.color);
		sl::Resource depthIn = MakeSlResource(s_dlss.depth);
		sl::Resource motionIn = MakeSlResource(s_dlss.motion);
		sl::Resource colorOut = MakeSlResource(s_dlss.output);

		// The inputs are not touched again by GL until after this frame's VK work
		// (GL waits on semVkDone below), so they are valid until present and
		// Streamline does not need to make copies.
		const sl::ResourceTag tags[] =
		{
			sl::ResourceTag(&colorIn, sl::kBufferTypeScalingInputColor, sl::ResourceLifecycle::eValidUntilPresent, &renderExtent),
			sl::ResourceTag(&depthIn, sl::kBufferTypeDepth, sl::ResourceLifecycle::eValidUntilPresent, &renderExtent),
			sl::ResourceTag(&motionIn, sl::kBufferTypeMotionVectors, sl::ResourceLifecycle::eValidUntilPresent, &renderExtent),
			sl::ResourceTag(&colorOut, sl::kBufferTypeScalingOutputColor, sl::ResourceLifecycle::eValidUntilPresent, &outputExtent),
		};
		s_sl.SetTagForFrame(token, viewportHandle, tags, static_cast<uint32_t>(std::size(tags)),
			reinterpret_cast<sl::CommandBuffer*>(slot.cmd));

		evalResult = s_sl.EvaluateFeature(sl::kFeatureDLSS, token, inputs,
			static_cast<uint32_t>(std::size(inputs)), reinterpret_cast<sl::CommandBuffer*>(slot.cmd));

		s_vk.vkEndCommandBuffer(slot.cmd);

		const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		VkSubmitInfo si{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
		si.waitSemaphoreCount = 1;
		si.pWaitSemaphores = &s_dlss.semGlDone;
		si.pWaitDstStageMask = &waitStage;
		si.commandBufferCount = 1;
		si.pCommandBuffers = &slot.cmd;
		si.signalSemaphoreCount = 1;
		si.pSignalSemaphores = &s_dlss.semVkDone;
		const VkResult submitResult = s_vk.vkQueueSubmit(s_dlss.queue, 1, &si, slot.fence);
		fSubmitted = submitResult == VK_SUCCESS;
		if (!fSubmitted)
			DlssLog("vkQueueSubmit failed (%d)", static_cast<int>(submitResult));

		RunPresentHook();

		if (fSubmitted)
			s_gl.WaitSemaphore(s_dlss.glSemVkDone, 0, nullptr, 4, sharedTextures, layouts);
	}

	s_dlss.lastResult = static_cast<int>(evalResult);
	state.Restore();

	// Remember this frame's camera for next frame's motion vectors.
	s_dlss.matPrevWorldToClip = cam.matWorldToClip;
	s_dlss.posPrevCamera = cam.pos;
	s_dlss.dirPrevForward = -glm::normalize(glm::vec3(eye[2]));
	s_dlss.fHavePrevCamera = true;
	s_dlss.fResetHistory = false;

	if (!fSubmitted || evalResult != sl::Result::eOk)
	{
		if (evalResult != sl::Result::eOk)
			DlssLog("slEvaluateFeature failed (%d)", static_cast<int>(evalResult));
		glBindFramebuffer(GL_FRAMEBUFFER, previousFbo);
		return false;
	}

	if ((s_dlss.frameIndex % 120) == 0 && s_sl.DLSSGetState != nullptr)
	{
		sl::DLSSState dlssState{};
		if (s_sl.DLSSGetState(viewportHandle, dlssState) == sl::Result::eOk)
			s_dlss.vramBytes = dlssState.estimatedVRAMUsageInBytes;
	}

	s_dlss.fUpscaledThisFrame = true;

	// --- 5. Leave the upscaled output bound for the HUD -------------------------
	// TV portraits use their own camera and animate inside a screen-fixed
	// rectangle; temporal history darkens/smears them, so show them as rendered.
	BlitDlssStaticRects(sceneColorTex, rw, rh, s_dlss.outputFbo, ow, oh);
	glBindFramebuffer(GL_FRAMEBUFFER, s_dlss.outputFbo);
	glViewport(0, 0, ow, oh);
	glClearDepth(1.0);
	glClearStencil(0);
	glStencilMask(0xFF);
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glDepthMask(state.depthMask);
	// The HUD must see the scene's depth/stencil (binocucom TV portraits).
	BlitSceneDepthStencil(sceneDepthTex, rw, rh, s_dlss.outputFbo, ow, oh);
	return true;
}

// =============================================================================
// Presentation, Frame Generation, Reflex markers, FPS counter
// =============================================================================
namespace
{
D3D12_RESOURCE_BARRIER TransitionBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER b{};
	b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	b.Transition.pResource = resource;
	b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	b.Transition.StateBefore = before;
	b.Transition.StateAfter = after;
	return b;
}

void SetReflex(bool fOn)
{
	if (s_sl.ReflexSetOptions == nullptr || s_dlss.fReflexOn == fOn)
		return;
	sl::ReflexOptions options{};
	options.mode = fOn ? sl::ReflexMode::eLowLatency : sl::ReflexMode::eOff;
	if (s_sl.ReflexSetOptions(options) == sl::Result::eOk)
		s_dlss.fReflexOn = fOn;
}

void SetPclMarker(sl::PCLMarker marker)
{
	if (s_dlss.backend == BACKEND_D3D12 && s_sl.PCLSetMarker != nullptr && s_dlss.token != nullptr)
		s_sl.PCLSetMarker(marker, *s_dlss.token);
}

bool s_fFgFailed = false;   // swapchain creation failed: stop retrying every frame
int s_viewLeft = 0;
int s_viewTop = 0;
int s_viewWidth = 0;
int s_viewHeight = 0;

bool FWantFrameGen()
{
	return s_dlss.backend == BACKEND_D3D12 && s_dlss.fSupported && s_dlss.fFgSupported &&
		s_dlss.frameGen > 1 && s_dlss.dlssq != DLSSQ_Off && s_dlss.hwndGame != nullptr && !s_fFgFailed;
}

bool EnableFrameGenPresent(int width, int height)
{
	// DLSS-G supports one swapchain: drop the helper one before loading it.
	DestroyHiddenSwapDx();
	if (s_sl.SetFeatureLoaded != nullptr)
		s_sl.SetFeatureLoaded(sl::kFeatureDLSS_G, true);

	HWND hwndTarget = s_dlss.hwndGame;
	if (s_dlss.fFgChildWindow)
	{
		HWND hwndChild = CreatePresentChildWindow(width, height);
		if (hwndChild != nullptr)
			hwndTarget = hwndChild;
	}
	s_dlss.mainSwap = CreateSwapchainDx(hwndTarget, width, height);
	if (s_dlss.mainSwap == nullptr)
	{
		DestroyPresentChildWindow();
		if (s_sl.SetFeatureLoaded != nullptr)
			s_sl.SetFeatureLoaded(sl::kFeatureDLSS_G, false);
		CreateHiddenSwapDx();
		DlssLog("Frame Generation: could not create the game window swapchain");
		return false;
	}
	s_dlss.swapWidth = width;
	s_dlss.swapHeight = height;
	// The OpenGL window's own (driver-managed) swapchain may still own the
	// screen in independent-flip mode, in which case Windows keeps showing its
	// last frame and never shows ours (Alt+Tab "fixes" it). A few more OpenGL
	// presents with our child window on top make Windows recompose.
	s_dlss.glKickFrames = (hwndTarget != s_dlss.hwndGame) ? s_dlss.glKickSetting : 0;
	s_dlss.fgFramesLogged = 0;
	SetReflex(true);
	DlssLog("Frame Generation on (%dx), presenting through DXGI (%s)", s_dlss.frameGen,
		hwndTarget == s_dlss.hwndGame ? "game window" : "child window");
	return true;
}

void DisableFrameGenPresent()
{
	if (s_dlss.mainSwap == nullptr)
		return;
	if (s_sl.DLSSGSetOptions != nullptr)
	{
		sl::DLSSGOptions options{};
		options.mode = sl::DLSSGMode::eOff;
		s_sl.DLSSGSetOptions(sl::ViewportHandle(0), options);
	}
	ReleaseMainSwapDx();
	if (s_sl.SetFeatureLoaded != nullptr)
		s_sl.SetFeatureLoaded(sl::kFeatureDLSS_G, false);
	CreateHiddenSwapDx();
	SetReflex(false);
	s_dlss.fgPresentedFactor = 1;
	DlssLog("Frame Generation off, presenting through OpenGL");
}

// When frame generation starts on a window that covers its whole monitor
// and OpenGL was presenting to it (Fullscreen mode), Windows keeps the old
// OpenGL surface in its fullscreen fast path and never shows our frames until
// the window is disturbed (Alt+Tab). Do that disturbance ourselves: shrink the
// window by two pixels for a few frames (or minimize/restore it).
bool FWindowCoversMonitor(HWND hwnd)
{
	RECT rcWindow{};
	if (hwnd == nullptr || !GetWindowRect(hwnd, &rcWindow))
		return false;
	MONITORINFO mi{};
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfoW(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &mi))
		return false;
	return EqualRect(&rcWindow, &mi.rcMonitor) != FALSE;
}

void StartFgUnstick()
{
	if (s_dlss.unstickMode == 0 || s_dlss.unstickFrames > 0 || !FWindowCoversMonitor(s_dlss.hwndGame))
		return;
	if (s_dlss.unstickMode == 2)
	{
		DlssLog("Frame Generation: minimize/restore to leave the fullscreen fast path");
		ShowWindow(s_dlss.hwndGame, SW_MINIMIZE);
		ShowWindow(s_dlss.hwndGame, SW_RESTORE);
		SetForegroundWindow(s_dlss.hwndGame);
		return;
	}
	GetWindowRect(s_dlss.hwndGame, &s_dlss.unstickRect);
	const RECT& rc = s_dlss.unstickRect;
	SetWindowPos(s_dlss.hwndGame, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top - 2,
		SWP_NOZORDER | SWP_NOACTIVATE);
	s_dlss.unstickFrames = 6;
	DlssLog("Frame Generation: briefly resizing the window to leave the fullscreen fast path");
}

void UpdateFgUnstick()
{
	if (s_dlss.unstickFrames <= 0 || --s_dlss.unstickFrames > 0)
		return;
	const RECT& rc = s_dlss.unstickRect;
	SetWindowPos(s_dlss.hwndGame, nullptr, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
		SWP_NOZORDER | SWP_NOACTIVATE);
}

bool EnsurePresentTargets(int width, int height)
{
	if (s_dlss.presentGlTex != 0 && s_dlss.presentW == width && s_dlss.presentH == height)
		return true;

	WaitDxIdle();
	glFinish();
	DestroySharedImageDx(s_dlss.presentShared);
	if (s_dlss.presentFbo != 0) glDeleteFramebuffers(1, &s_dlss.presentFbo);
	if (s_dlss.presentGlTex != 0) glDeleteTextures(1, &s_dlss.presentGlTex);
	s_dlss.presentFbo = s_dlss.presentGlTex = 0;
	s_dlss.presentW = s_dlss.presentH = 0;

	if (!CreateSharedImageDx(s_dlss.presentShared, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, GL_RGBA8))
	{
		DestroySharedImageDx(s_dlss.presentShared);
		return false;
	}

	glGenTextures(1, &s_dlss.presentGlTex);
	glBindTexture(GL_TEXTURE_2D, s_dlss.presentGlTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &s_dlss.presentFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, s_dlss.presentFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_dlss.presentGlTex, 0);
	const bool ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (!ok)
		return false;

	s_dlss.presentW = width;
	s_dlss.presentH = height;
	return true;
}
} // namespace

bool FDlssNeedsBorderlessFullscreen()
{
	return FWantFrameGen();
}

void DlssMarkRenderStart()
{
	SetPclMarker(sl::PCLMarker::eSimulationEnd);
	SetPclMarker(sl::PCLMarker::eRenderSubmitStart);
}

GLuint DlssBeginPresent(int framebufferWidth, int framebufferHeight,
	int viewX, int viewY, int viewWidth, int viewHeight)
{
	if (s_dlss.backend != BACKEND_D3D12 || !s_dlss.fSupported)
		return 0;

	UpdateFgUnstick();

	const bool fVisible = framebufferWidth > 0 && framebufferHeight > 0;
	const bool fWant = FWantFrameGen();
	if (fWant && fVisible && s_dlss.mainSwap == nullptr)
	{
		if (!EnableFrameGenPresent(framebufferWidth, framebufferHeight))
			s_fFgFailed = true;
		else
			StartFgUnstick();
	}
	else if (!fWant && s_dlss.mainSwap != nullptr)
	{
		DisableFrameGenPresent();
	}

	if (s_dlss.mainSwap == nullptr)
		return 0;

	if (fVisible && (s_dlss.swapWidth != framebufferWidth || s_dlss.swapHeight != framebufferHeight))
	{
		// Frame generation must be off while the swapchain changes size.
		if (s_sl.DLSSGSetOptions != nullptr)
		{
			sl::DLSSGOptions options{};
			options.mode = sl::DLSSGMode::eOff;
			options.flags = sl::DLSSGFlags::eRetainResourcesWhenOff;
			s_sl.DLSSGSetOptions(sl::ViewportHandle(0), options);
		}
		WaitDxIdle();
		if (s_dlss.hwndPresent != nullptr)
			SetWindowPos(s_dlss.hwndPresent, nullptr, 0, 0, framebufferWidth, framebufferHeight,
				SWP_NOZORDER | SWP_NOACTIVATE);
		const HRESULT hr = s_dlss.mainSwap->ResizeBuffers(0, static_cast<UINT>(framebufferWidth),
			static_cast<UINT>(framebufferHeight), DXGI_FORMAT_UNKNOWN,
			s_dlss.fTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
		if (FAILED(hr))
			DlssLog("ResizeBuffers failed (0x%08X)", static_cast<unsigned>(hr));
		s_dlss.swapWidth = framebufferWidth;
		s_dlss.swapHeight = framebufferHeight;
	}

	if (fVisible && !EnsurePresentTargets(framebufferWidth, framebufferHeight))
	{
		DlssLog("Frame Generation: present targets unavailable; turning it off");
		s_fFgFailed = true;
		DisableFrameGenPresent();
		return 0;
	}

	// Game viewport inside the window, in top-down coordinates.
	s_viewLeft = viewX;
	s_viewWidth = viewWidth;
	s_viewHeight = viewHeight;
	s_viewTop = s_dlss.presentH - (viewY + viewHeight);
	return s_dlss.presentFbo;
}

bool FDlssPresentFrame(bool fGameplay)
{
	if (s_dlss.backend != BACKEND_D3D12 || !s_dlss.fSupported)
		return false;

	SetPclMarker(sl::PCLMarker::eRenderSubmitEnd);

	if (s_dlss.mainSwap == nullptr)
	{
		// OpenGL presents. Present the hidden helper swapchain so Streamline's
		// per-frame bookkeeping and garbage collection still run.
		SetPclMarker(sl::PCLMarker::ePresentStart);
		if (s_dlss.hiddenSwap != nullptr)
			s_dlss.hiddenSwap->Present(0, s_dlss.fTearing ? DXGI_PRESENT_ALLOW_TEARING : 0);
		SetPclMarker(sl::PCLMarker::ePresentEnd);
		return false;
	}

	if (s_dlss.presentW <= 0 || s_dlss.presentH <= 0)
		return true;   // minimized: nothing to show, but OpenGL must not present either

	// Final frame (scene + HUD + menus) -> top-down shared texture -> backbuffer.
	BlitFlip(s_dlss.presentGlTex, s_dlss.presentW, s_dlss.presentH,
		s_dlss.presentShared.glTexture, s_dlss.presentW, s_dlss.presentH);
	const GLuint sharedTex[1] = { s_dlss.presentShared.glTexture };
	GlSignalDx(sharedTex, 1);

	ID3D12GraphicsCommandList* list = nullptr;
	{
		DLSSTIMER timer("present: wait for command allocator");
		list = BeginDxList(true);
	}

	UINT backBufferIndex = 0;
	ID3D12Resource* backBuffer = nullptr;
	{
		DLSSTIMER timer("present: GetCurrentBackBufferIndex/GetBuffer");
		backBufferIndex = s_dlss.mainSwap->GetCurrentBackBufferIndex();
		s_dlss.mainSwap->GetBuffer(backBufferIndex, __uuidof(ID3D12Resource), reinterpret_cast<void**>(&backBuffer));
	}
	if (backBuffer != nullptr)
	{
		D3D12_RESOURCE_BARRIER before[2] =
		{
			TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST),
			TransitionBarrier(s_dlss.presentShared.d3dResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE)
		};
		list->ResourceBarrier(2, before);
		list->CopyResource(backBuffer, s_dlss.presentShared.d3dResource);
		D3D12_RESOURCE_BARRIER after[2] =
		{
			TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT),
			TransitionBarrier(s_dlss.presentShared.d3dResource, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON)
		};
		list->ResourceBarrier(2, after);
	}

	// Frame generation only on real game frames (not menus, loading, FMVs).
	const bool fGameFrame = fGameplay && s_dlss.fUpscaledThisFrame && s_dlss.token != nullptr;
	if (s_sl.DLSSGSetOptions != nullptr)
	{
		sl::DLSSGOptions options{};
		options.mode = fGameFrame ? sl::DLSSGMode::eOn : sl::DLSSGMode::eOff;
		options.numFramesToGenerate = static_cast<uint32_t>(std::clamp(s_dlss.frameGen - 1, 1, s_dlss.fgMaxGenerated));
		options.flags = sl::DLSSGFlags::eRetainResourcesWhenOff;
		s_sl.DLSSGSetOptions(sl::ViewportHandle(0), options);
	}

	if (fGameFrame)
	{
		// Hud-less color = the DLSS output (before the HUD was drawn), which is
		// exactly the game viewport rectangle of the backbuffer.
		sl::Resource hudless(sl::ResourceType::eTex2d, s_dlss.output.d3dResource,
			static_cast<uint32_t>(D3D12_RESOURCE_STATE_COMMON));
		hudless.width = static_cast<uint32_t>(s_dlss.output.width);
		hudless.height = static_cast<uint32_t>(s_dlss.output.height);
		hudless.nativeFormat = static_cast<uint32_t>(s_dlss.output.dxgiFormat);
		hudless.mipLevels = 1;
		hudless.arrayLayers = 1;

		sl::Extent hudlessExtent{ 0, 0, static_cast<uint32_t>(s_dlss.output.width), static_cast<uint32_t>(s_dlss.output.height) };
		sl::Extent backbufferExtent{
			static_cast<uint32_t>((std::max)(0, s_viewTop)), static_cast<uint32_t>((std::max)(0, s_viewLeft)),
			static_cast<uint32_t>(s_dlss.output.width), static_cast<uint32_t>(s_dlss.output.height) };

		const sl::ResourceTag tags[] =
		{
			sl::ResourceTag(&hudless, sl::kBufferTypeHUDLessColor, sl::ResourceLifecycle::eOnlyValidNow, &hudlessExtent),
			sl::ResourceTag(nullptr, sl::kBufferTypeBackbuffer, sl::ResourceLifecycle::eValidUntilPresent, &backbufferExtent),
		};
		s_sl.SetTagForFrame(*s_dlss.token, sl::ViewportHandle(0), tags, static_cast<uint32_t>(std::size(tags)),
			reinterpret_cast<sl::CommandBuffer*>(list));
	}

	ExecuteDxList(list);

	SetPclMarker(sl::PCLMarker::ePresentStart);
	const UINT syncInterval = g_fVsync ? 1u : 0u;
	// No tearing by default: with sync interval 0 and no tearing flag, a
	// flip-model swapchain shows the newest finished frame at each refresh
	// (G-Sync/VRR still applies below the refresh rate) without capping the
	// frame rate like V-Sync does. FgAllowTearing=1 restores tearing.
	const UINT presentFlags = (!g_fVsync && s_dlss.fTearing && s_dlss.fFgAllowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0u;
	HRESULT hr = S_OK;
	{
		DLSSTIMER timer("present: IDXGISwapChain::Present");
		hr = s_dlss.mainSwap->Present(syncInterval, presentFlags);
	}
	SetPclMarker(sl::PCLMarker::ePresentEnd);
	if (backBuffer != nullptr)
		backBuffer->Release();
	if (FAILED(hr))
		DlssLog("Present failed (0x%08X)", static_cast<unsigned>(hr));

	if (s_dlss.glKickFrames > 0)
	{
		--s_dlss.glKickFrames;
		HDC hdc = GetDC(s_dlss.hwndGame);
		if (hdc != nullptr)
		{
			SwapBuffers(hdc);
			ReleaseDC(s_dlss.hwndGame, hdc);
		}
	}
	if (s_dlss.fgFramesLogged < 20)
	{
		++s_dlss.fgFramesLogged;
		DlssLog("FG frame %u: present hr 0x%08X, game frame %d, fence %llu (completed %llu)",
			s_dlss.frameIndex, static_cast<unsigned>(hr), fGameFrame ? 1 : 0,
			static_cast<unsigned long long>(s_dlss.fenceValue),
			static_cast<unsigned long long>(s_dlss.d3dFence != nullptr ? s_dlss.d3dFence->GetCompletedValue() : 0));
	}

	// OpenGL must not overwrite the shared frame before D3D12 copied it.
	DxSignalGl(sharedTex, 1);
	CurrentDxFrame().fenceValue = s_dlss.fenceValue;

	if (s_sl.DLSSGGetState != nullptr)
	{
		sl::DLSSGState state{};
		if (s_sl.DLSSGGetState(sl::ViewportHandle(0), state, nullptr) == sl::Result::eOk)
		{
			s_dlss.fgPresentedFactor = fGameFrame ? (std::max)(1u, state.numFramesActuallyPresented) : 1;
			if (fGameFrame && static_cast<int>(state.status) != s_dlss.fgStatus)
			{
				s_dlss.fgStatus = static_cast<int>(state.status);
				if (s_dlss.fgStatus != 0)
					DlssLog("Frame Generation status 0x%x (see sl.log)", s_dlss.fgStatus);
			}
		}
	}
	return true;
}

bool FDlssFrameGenAvailable()
{
	return s_dlss.backend == BACKEND_D3D12 && s_dlss.fSupported && s_dlss.fFgSupported;
}

int DlssFrameGenMultiplier()
{
	return s_dlss.frameGen;
}

void SelectNextDlssFrameGen()
{
	if (!FDlssFrameGenAvailable())
		return;
	const int maxMultiplier = (std::min)(4, s_dlss.fgMaxGenerated + 1);
	s_dlss.frameGen = s_dlss.frameGen >= maxMultiplier ? 1 : s_dlss.frameGen + 1;
	s_fFgFailed = false;
	SaveDlssSettings();
}

const char* PchzDlssFrameGen()
{
	static char buffer[48];
	if (!FDlssFrameGenAvailable())
		return "Unavailable";
	if (s_dlss.frameGen <= 1)
		return "Off";
	if (s_dlss.dlssq == DLSSQ_Off)
		std::snprintf(buffer, sizeof(buffer), "%dx (turn on DLSS)", s_dlss.frameGen);
	else
		std::snprintf(buffer, sizeof(buffer), "%dx", s_dlss.frameGen);
	return buffer;
}

bool FDlssFpsCounter()
{
	return s_dlss.fpsMode != 0;
}

bool FDlssFpsCounterDetailed()
{
	return s_dlss.fpsMode == 2;
}

const char* PchzDlssFpsCounter()
{
	static const char* const s_apchz[] = { "Off", "On", "Detailed" };
	return s_apchz[std::clamp(s_dlss.fpsMode, 0, 2)];
}

void ToggleDlssFpsCounter()
{
	s_dlss.fpsMode = (s_dlss.fpsMode + 1) % 3;
	if (s_dlss.exeDirectory.empty())
		s_dlss.exeDirectory = ExeDirectory();
	SaveDlssSettings();
}

void DrawDlssOverlay()
{
	// Count every rendered frame, even when the counter is hidden, so it is
	// correct the moment it is turned on.
	const double now = glfwGetTime();
	if (s_dlss.tFpsWindow <= 0.0)
		s_dlss.tFpsWindow = now;
	++s_dlss.cFpsFrames;
	const double elapsed = now - s_dlss.tFpsWindow;
	if (elapsed >= 0.5)
	{
		const float baseFps = static_cast<float>(s_dlss.cFpsFrames / elapsed);
		const bool fFg = s_dlss.mainSwap != nullptr && s_dlss.fgPresentedFactor > 1;
		// With frame generation the screen shows several frames per rendered one.
		s_dlss.fpsShown = fFg ? baseFps * static_cast<float>(s_dlss.fgPresentedFactor) : baseFps;
		s_dlss.fFpsShownIsFg = fFg;
		s_dlss.cFpsFrames = 0;
		s_dlss.tFpsWindow = now;
	}

	if (s_dlss.fpsMode == 0 || ImGui::GetCurrentContext() == nullptr)
		return;

	// "MFG" = frame generation, "FSR" = AMD FSR upscaling, "FPS" otherwise.
	const bool fFsr = !s_dlss.fFpsShownIsFg && FFsrActive();
	char text[32];
	std::snprintf(text, sizeof(text), "%d %s", static_cast<int>(s_dlss.fpsShown + 0.5f),
		s_dlss.fFpsShownIsFg ? "MFG" : (fFsr ? "FSR" : "FPS"));

	ImDrawList* draw = ImGui::GetForegroundDrawList();
	const ImVec2 display = ImGui::GetIO().DisplaySize;
	const float scale = (std::max)(1.0f, display.y / 720.0f);
	const float fontSize = ImGui::GetFontSize() * 1.25f * scale;
	const ImVec2 size = ImGui::GetFont()->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text);
	const float pad = 8.0f * scale;
	const ImVec2 pos(display.x - size.x - 16.0f * scale, 16.0f * scale);
	draw->AddRectFilled(ImVec2(pos.x - pad, pos.y - pad), ImVec2(pos.x + size.x + pad, pos.y + size.y + pad),
		IM_COL32(0, 0, 0, 170), 6.0f * scale);
	draw->AddText(ImGui::GetFont(), fontSize, pos,
		s_dlss.fFpsShownIsFg ? IM_COL32(118, 185, 0, 255) :
		(fFsr ? IM_COL32(237, 28, 36, 255) : IM_COL32(255, 255, 255, 255)), text);

	// Detailed: CPU/GPU frame breakdown below the counter.
	if (s_dlss.fpsMode == 2 && CPerfLines() > 0)
	{
		const float smallSize = ImGui::GetFontSize() * 0.9f * scale;
		float widest = 0.0f;
		for (int i = 0; i < CPerfLines(); ++i)
			widest = (std::max)(widest, ImGui::GetFont()->CalcTextSizeA(smallSize, FLT_MAX, 0.0f, PchzPerfLine(i)).x);
		const float lineHeight = smallSize * 1.15f;
		const float top = pos.y + size.y + pad * 2.5f;
		const float left = display.x - widest - 16.0f * scale;
		draw->AddRectFilled(ImVec2(left - pad, top - pad),
			ImVec2(left + widest + pad, top + lineHeight * CPerfLines() + pad),
			IM_COL32(0, 0, 0, 170), 6.0f * scale);
		for (int i = 0; i < CPerfLines(); ++i)
			draw->AddText(ImGui::GetFont(), smallSize, ImVec2(left, top + lineHeight * i),
				IM_COL32(230, 230, 230, 255), PchzPerfLine(i));
	}
}

void DrawDlssDebugMenu()
{
	if (!ImGui::BeginMenu("DLSS"))
		return;

	ImGui::Text("Status: %s", s_dlss.status.c_str());
	if (!s_dlss.featureVersion.empty())
		ImGui::Text("%s", s_dlss.featureVersion.c_str());
	if (s_dlss.backend == BACKEND_D3D12)
	{
		ImGui::Text("Frame Generation: %s (max %dx), %s, factor %d, status 0x%x",
			s_dlss.fFgSupported ? "supported" : "unsupported", s_dlss.fgMaxGenerated + 1,
			s_dlss.mainSwap != nullptr ? "presenting via DXGI" : "presenting via OpenGL",
			s_dlss.fgPresentedFactor, s_dlss.fgStatus);
		ImGui::Text("Reflex: %s", s_dlss.fReflexOn ? "low latency" : "off");
	}

	if (s_dlss.fSupported)
	{
		ImGui::Separator();
		for (int i = 0; i < DLSSQ_Max; ++i)
		{
			if (ImGui::MenuItem(s_apchzQuality[i], nullptr, s_dlss.dlssq == i))
				SetDlssQuality(static_cast<DLSSQ>(i));
		}

		ImGui::Separator();
		if (ImGui::BeginMenu("Preset"))
		{
			struct PRESETNAME { int value; const char* name; };
			static const PRESETNAME presets[] =
			{
				{ 0, "NVIDIA default" }, { 10, "J (transformer)" }, { 11, "K (transformer)" },
				{ 12, "L (transformer 2)" }, { 13, "M (transformer 2)" }
			};
			for (const PRESETNAME& p : presets)
			{
				if (ImGui::MenuItem(p.name, nullptr, s_dlss.preset == p.value))
				{
					s_dlss.preset = p.value;
					s_dlss.fResetHistory = true;
					SaveDlssSettings();
				}
			}
			ImGui::EndMenu();
		}

		ImGui::Separator();
		ImGui::Text("Render %dx%d -> %dx%d", s_dlss.renderWidth, s_dlss.renderHeight,
			s_dlss.outputWidth, s_dlss.outputHeight);
		ImGui::Text("Jitter (%.3f, %.3f)  last result %d", s_dlss.jitterPixels.x, s_dlss.jitterPixels.y, s_dlss.lastResult);
		ImGui::Text("DLSS VRAM %.1f MB  present hook %s", s_dlss.vramBytes / (1024.0 * 1024.0),
			s_dlss.fPresentHook ? "on" : "off");

		if (ImGui::MenuItem("Jitter", nullptr, s_dlss.fJitterEnabled))
		{
			s_dlss.fJitterEnabled = !s_dlss.fJitterEnabled;
			s_dlss.fResetHistory = true;
		}
		if (ImGui::MenuItem("Flip jitter X", nullptr, s_dlss.jitterSignX < 0.0f))
		{
			s_dlss.jitterSignX = -s_dlss.jitterSignX;
			s_dlss.fResetHistory = true;
			SaveDlssSettings();
		}
		if (ImGui::MenuItem("Flip jitter Y", nullptr, s_dlss.jitterSignY < 0.0f))
		{
			s_dlss.jitterSignY = -s_dlss.jitterSignY;
			s_dlss.fResetHistory = true;
			SaveDlssSettings();
		}
		if (ImGui::MenuItem("Camera cut detection", nullptr, s_dlss.fCameraCutDetection))
			s_dlss.fCameraCutDetection = !s_dlss.fCameraCutDetection;
		if (ImGui::MenuItem("Reset history now"))
			s_dlss.fResetHistory = true;
	}

	ImGui::EndMenu();
}

// Options > Reset to Default: DLSS off, default model, no frame generation,
// FPS counter off. Advanced DLSS.ini keys (backend, logging, ...) are kept.
void ResetDlssUserSettings()
{
	s_dlss.preset = 0;
	s_dlss.frameGen = 1;
	s_dlss.fpsMode = 0;
	s_fFgFailed = false;
	SetDlssQuality(DLSSQ_Off);
	SaveDlssSettings();
}

// Settings import: re-read DLSS.ini and apply the user-facing options.
void ReloadDlssUserSettings()
{
	LoadDlssSettings();
	s_fFgFailed = false;
	SetDlssQuality(s_dlss.dlssq);
}

// Changed-setting markers: 0 = DLSS mode, 1 = model, 2 = frame generation, 3 = FPS counter.
bool FDlssOptionDefault(int which)
{
	switch (which)
	{
	case 0: return s_dlss.dlssq == DLSSQ_Off;
	case 1: return s_dlss.preset == 0;
	case 2: return s_dlss.frameGen <= 1;
	case 3: return s_dlss.fpsMode == 0;
	default: return true;
	}
}

void ResetDlssOption(int which)
{
	switch (which)
	{
	case 0: SetDlssQuality(DLSSQ_Off); return;
	case 1: s_dlss.preset = 0; s_dlss.fResetHistory = true; break;
	case 2: s_dlss.frameGen = 1; s_fFgFailed = false; break;
	case 3: s_dlss.fpsMode = 0; break;
	default: return;
	}
	SaveDlssSettings();
}
