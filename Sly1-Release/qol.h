#pragma once

// -----------------------------------------------------------------------------
// Quality-of-life options (QOL.ini next to the exe):
//
//   * Anisotropic filtering   Video menu, Off / 2x / 4x / 8x / 16x
//   * Field of view           Video menu, Default / +10% ... +40% (rendering
//                             only; the gameplay camera logic is unchanged)
//   * Camera sensitivity      Controls menu, 50% ... 200% (right-stick orbit
//                             and binocular / turret look)
//   * Screenshots             F12 = with HUD, Shift+F12 = without HUD.
//                             PNGs go to Screenshots\ next to the exe.
// -----------------------------------------------------------------------------

#include <glad/glad.h>

void StartupQol();

// Anisotropic filtering ------------------------------------------------------
void QolRegisterTexture(GLuint texture);   // call once per game texture after creation
void SelectNextQolAnisotropy();
const char* PchzQolAnisotropy();

// Field of view ----------------------------------------------------------------
float QolFovScale();                       // tan(fov/2) multiplier, 1 = default
void SelectNextQolFov();
const char* PchzQolFov();

// Camera sensitivity -------------------------------------------------------------
float QolCameraSensitivity();              // 1 = default
void SelectNextQolCameraSensitivity();
const char* PchzQolCameraSensitivity();
int QolCameraSensitivityPercent();         // 10 .. 400
int QolFovPercent();                       // 0 .. 60 (extra field of view)
void SetQolFovPercent(int percent);
int QolScreenshotKey();                    // GLFW key or mouse binding (mouseinput.h)
void SetQolScreenshotKey(int key);
void SetQolCameraSensitivityPercent(int percent);

// Options > Reset to Default.
void ResetQolSettings();
// Controller Settings > Button Icons: 0 Auto, 1 Xbox, 2 PlayStation, 3 Nintendo.
int QolButtonIcons();
void SetQolButtonIcons(int setting);
void SelectNextQolButtonIcons();
const char* PchzQolButtonIcons();
bool FQolAnisotropyDefault();
void ResetQolAnisotropy();
void ReloadQolSettings();                  // re-read QOL.ini (settings import)
void QolShowNotice(const char* text, double seconds);   // small on-screen message

// Screenshots --------------------------------------------------------------------
void UpdateQolInput();                     // once per frame (screenshot hotkeys)
// Call right before the HUD is drawn, with the framebuffer that holds the 3D
// image bound for drawing (captures "without HUD" requests).
void QolCaptureBeforeHud(int width, int height);
// Call after the final image (with HUD, before overlays) is in drawFbo.
void QolCaptureFinal(GLuint drawFbo, int x, int y, int width, int height);
bool FQolScreenshotPending();

// Pause / mute while the window is in the background (Options menu) ---------------
// Retail Timing (Options > Gameplay, default on): the game updates exactly like
// the PS2 - one 1/59.94 s step per frame, at most two - so physics and
// speedrun glitches match the console. Off: steps follow the Frame Rate option.
bool FRetailTiming();
void SetRetailTiming(bool fOn);

bool FQolPauseOnFocusLoss();
void ToggleQolPauseOnFocusLoss();
bool FQolFocusLossPause();                 // true while the game should open the pause menu
void UpdateQolFocus();                     // once per frame (mutes audio while unfocused)
void DrawQolOverlay();                     // "Screenshot saved" notice (ImGui)
