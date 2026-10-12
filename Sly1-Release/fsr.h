#pragma once

// -----------------------------------------------------------------------------
// AMD FidelityFX Super Resolution for ProjectCane.
//
// Two backends, picked automatically:
//
//   * AMD FSR SDK (FidelityFX SDK 2.x, "ffx-api", D3D12)
//       FSR 4 (machine-learning upscaler) on Radeon RX 7000 / 9000 GPUs,
//       FSR 3.1 (temporal, analytical) on every other DX12 GPU. The loader
//       picks the best version the GPU and driver support.
//       Like the DLSS path, a small D3D12 device on the same GPU shares
//       textures with OpenGL (GL_EXT_memory_object_win32 / _semaphore_win32):
//         GL scene (render res, jittered) -> shared color / depth / motion
//         D3D12 + FSR                      -> shared output (present res)
//         GL HUD + present                 <- shared output
//       Needs amd_fidelityfx_loader_dx12.dll + amd_fidelityfx_upscaler_dx12.dll
//       next to the exe (copied by the post-build step).
//
//   * FSR 1 fallback (pure OpenGL EASU + RCAS) when the SDK DLLs, D3D12 or
//     the OpenGL interop extensions are missing.
//
// FSR and DLSS are alternatives: when DLSS is active, FSR is ignored.
// Settings are stored in FSR.ini next to the executable.
// -----------------------------------------------------------------------------

#include <glad/glad.h>
#include "dlss.h"   // DLSSCAMERA

enum FSRQ
{
	FSRQ_Off = 0,
	FSRQ_NativeAA,          // 1.0x (anti-aliasing only)
	FSRQ_Quality,           // 1.5x per axis
	FSRQ_Balanced,          // 1.7x
	FSRQ_Performance,       // 2.0x
	FSRQ_UltraPerformance,  // 3.0x
	FSRQ_Max
};

// Lifetime -------------------------------------------------------------------
void StartupFsr();           // Loads FSR.ini. Call after the GL context exists.
void ShutdownFsr();          // Call before the GL context is destroyed.

// State / settings -----------------------------------------------------------
bool FFsrActive();           // A mode other than Off is selected and DLSS is not active.
FSRQ FsrQuality();
void SetFsrQuality(FSRQ fsrq);      // Applies (resizes render targets) and saves FSR.ini.
void SelectNextFsrQuality();        // Video menu. Turns DLSS off when FSR is turned on.
const char* PchzFsrQuality();       // "Quality", "Off", ...
const char* PchzFsrVersion();       // "FSR 4", "FSR 3.1", "FSR 1" or "FSR" (not started yet)
void SelectNextFsrSharpness();      // Video menu.
const char* PchzFsrSharpness();

// Render-size negotiation. Returns false when FSR is not active.
bool FFsrRenderSize(int outputWidth, int outputHeight, int* pRenderWidth, int* pRenderHeight);

// Frame flow -----------------------------------------------------------------
// 1) Right after BeginDlssFrame(): picks this frame's sub-pixel jitter for the
//    temporal (SDK) backend and hands it to the shared camera upload.
void BeginFsrFrame(bool fUseFsr, int renderWidth, int renderHeight, int outputWidth, int outputHeight);

// 2) After the 3D scene and BEFORE the HUD. sceneColorTex must be the resolved
//    (non-MSAA) scene color and sceneDepthTex the scene depth texture. On
//    success the FSR output framebuffer (present resolution, with
//    depth/stencil) is bound with a matching viewport so the HUD can be drawn
//    on top of the upscaled image.
bool FEvaluateFsr(const DLSSCAMERA& cam, GLuint sceneColorTex, GLuint sceneDepthTex,
	int renderWidth, int renderHeight, int outputWidth, int outputHeight);
GLuint FsrOutputTexture();
void ResetFsrHistory();      // Camera cut / level load / teleport.
bool FFsrTemporal();         // The SDK backend (FSR 4 / 3.1) is in use: MSAA is not needed.

// Options > Reset to Default.
void ResetFsrSettings();
void ReloadFsrSettings();    // settings import
bool FFsrOptionDefault(int which);   // 0 = mode, 1 = sharpness
void ResetFsrOption(int which);
