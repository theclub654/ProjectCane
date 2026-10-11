#pragma once

// -----------------------------------------------------------------------------
// NVIDIA DLSS (via NVIDIA Streamline) for ProjectCane.
//
// ProjectCane renders with OpenGL 4.3, but Streamline/DLSS only run on
// D3D11/D3D12/Vulkan. This module creates a small Vulkan device on the same
// GPU through Streamline's interposer, shares four images with OpenGL
// (GL_EXT_memory_object_win32 + GL_EXT_semaphore_win32) and runs DLSS there:
//
//   GL scene (render res, jittered)  -> shared color / depth / motion vectors
//   Vulkan + Streamline DLSS         -> shared output (present res)
//   GL HUD + present                 <- shared output
//
// Everything degrades gracefully: when Streamline, the DLLs, the GL interop
// extensions or an RTX GPU are missing, DLSS reports "not available" and the
// game keeps its regular OpenGL path.
// -----------------------------------------------------------------------------

#include <glad/glad.h>
#include <glm/glm.hpp>

enum DLSSQ
{
	DLSSQ_Off = 0,
	DLSSQ_DLAA,
	DLSSQ_Quality,
	DLSSQ_Balanced,
	DLSSQ_Performance,
	DLSSQ_UltraPerformance,
	DLSSQ_Max
};

// Per-frame camera data handed to DLSS. Matrices are the UNJITTERED
// OpenGL matrices (clip = matWorldToClip * world).
struct DLSSCAMERA
{
	glm::mat4 matWorldToClip;
	glm::mat4 matProj;
	glm::vec3 pos;
	float radFov;
	float sNearClip;
	float sFarClip;
};

// Lifetime -------------------------------------------------------------------
// Call first thing in Startup(), BEFORE the OpenGL context exists: loads
// optional user-supplied compatibility DLLs that must be in the process before
// the graphics driver (DLSS.ini [DLSS] PreloadModules=, comma separated;
// defaults to version.dll when dlssg_sm86.ini sits next to the exe).
void PreloadDlssModules();

// Call once, right after the GL context exists. hwndGame is the game window
// (HWND); the D3D12 backend presents there while Frame Generation is on.
void StartupDlss(void* hwndGame);
void ShutdownDlss();         // Call before the GL context is destroyed.

// State / settings -----------------------------------------------------------
bool FDlssSupported();       // Streamline + DLSS + GL interop all working.
bool FDlssActive();          // Supported and a mode other than Off is selected.
DLSSQ DlssQuality();
void SetDlssQuality(DLSSQ dlssq);   // Applies, resizes render targets, saves DLSS.ini.
void SelectNextDlssQuality();       // Cycles modes (used by the in-game Video menu).
const char* PchzDlssQuality(DLSSQ dlssq);
const char* PchzDlssStatus();       // Human readable reason when unsupported.
void SelectNextDlssPreset();        // NVIDIA default -> J -> K -> L -> M (Video menu).
const char* PchzDlssPreset();       // Name of the current DLSS model preset.

// Render-size negotiation. Returns false when DLSS is not active, in which
// case the caller keeps its own render size.
bool FDlssRenderSize(int outputWidth, int outputHeight, int* pRenderWidth, int* pRenderHeight);

// Frame flow -----------------------------------------------------------------
// 1) Call once per frame BEFORE any 3D camera upload. Picks this frame's
//    sub-pixel jitter (zero when fUseDlss is false).
void BeginDlssFrame(bool fUseDlss, int renderWidth, int renderHeight, int outputWidth, int outputHeight);

// 2) Replacement for "glBufferSubData(cmUBO, ..., &pcm->matWorldToClip)".
//    pcmgl points at a CMGL-layout block (mat4 worldToClip + vec4 cameraPos);
//    the jitter is applied to the uploaded copy only, never to game state.
void UploadCmUbo(const void* pcmgl, unsigned int cb);

// 2b) Another temporal upscaler (AMD FSR) supplies this frame's projection
//     jitter in OpenGL clip space instead. Call after BeginDlssFrame.
void SetDlssExternalJitter(float jitterNdcX, float jitterNdcY);

// 3) Optional: mark a screen-fixed rectangle in render-target pixels (GL,
//    bottom-left origin) whose motion vectors must be zero this frame, e.g.
//    the dialogue TV portrait that is rendered with its own camera.
void AddDlssStaticRect(int x, int y, int width, int height);

// Copies this frame's screen-fixed rectangles (TV portraits) straight from the
// rendered scene into an upscaled output, so temporal upscalers (DLSS, FSR)
// never blend them with history. Used internally by DLSS and by fsr.cpp.
void BlitDlssStaticRects(GLuint sceneColorTex, int renderWidth, int renderHeight,
	GLuint dstFbo, int outputWidth, int outputHeight);

// Copies the scene's depth + stencil (scaled) into an upscaled output so the
// HUD drawn there sees the same depth/stencil it would see in the scene FBO.
// (The binocucom's dark background relies on the TV portraits being sealed at
// depth 0 to leave the faces uncovered.)
void BlitSceneDepthStencil(GLuint sceneDepthTex, int renderWidth, int renderHeight,
	GLuint dstFbo, int outputWidth, int outputHeight);

// 4) After the 3D scene and BEFORE the HUD. Copies the scene into the shared
//    images, builds depth + motion vectors and runs DLSS. On success the DLSS
//    output framebuffer (present resolution) is bound with a matching
//    viewport so the HUD can be drawn on top of the upscaled image.
bool FEvaluateDlss(const DLSSCAMERA& cam, GLuint sceneColorTex, GLuint sceneDepthTex);

GLuint DlssOutputTexture();
void ResetDlssHistory();     // Camera cut / level load / teleport.

// Presentation (D3D12 backend / Frame Generation) ------------------------------
// After the game update, before rendering the scene (Reflex markers).
void DlssMarkRenderStart();
// Returns the framebuffer the final frame (scene + HUD + menus) must be drawn
// into, or 0 for the default framebuffer. view* is the game viewport inside
// the window (GL, bottom-left origin).
GLuint DlssBeginPresent(int framebufferWidth, int framebufferHeight,
	int viewX, int viewY, int viewWidth, int viewHeight);
// Presents the frame through DXGI when Frame Generation owns the window.
// Returns false when OpenGL must present it (glfwSwapBuffers).
bool FDlssPresentFrame(bool fGameplay);

// Frame Generation setting (1 = off, 2..4 = 2x..4x).
bool FDlssFrameGenAvailable();
// Frame generation presents through DXGI, which cannot show anything while
// OpenGL holds the monitor in exclusive fullscreen. While this returns true,
// "Fullscreen" is applied as a borderless window covering the whole monitor.
bool FDlssNeedsBorderlessFullscreen();
int DlssFrameGenMultiplier();
void SelectNextDlssFrameGen();
const char* PchzDlssFrameGen();

// FPS counter ("240 FPS", or "240 MFG" while frame generation multiplies frames).
bool FDlssFpsCounter();             // counter visible (On or Detailed)
bool FDlssFpsCounterDetailed();     // Detailed: also CPU/GPU frame breakdown
const char* PchzDlssFpsCounter();   // "Off", "On", "Detailed"
void ToggleDlssFpsCounter();        // Off -> On -> Detailed -> Off
void DrawDlssOverlay();   // every frame, between ImGui::NewFrame and ImGui::Render

// Debug ImGui menu (developer menu bar).
void DrawDlssDebugMenu();

// When non-zero, UI code that converts UI pixels to render-target pixels
// (e.g. text scissor rectangles) must use this size instead of the scene
// render size because the HUD is being drawn into the DLSS output.
extern int g_uiTargetWidth;
extern int g_uiTargetHeight;

// Options > Reset to Default (user-facing DLSS options only).
void ResetDlssUserSettings();
void ReloadDlssUserSettings();   // settings import
bool FDlssOptionDefault(int which);   // 0 = mode, 1 = model, 2 = frame gen, 3 = FPS counter
void ResetDlssOption(int which);
