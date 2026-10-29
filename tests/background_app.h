#pragma once
// Keeps macOS test processes in the background. GLFW turns an unbundled
// process into a regular app when it creates its first window (Dock icon,
// menu bar) and activates it when a window is focused, which takes focus from
// whatever the user is doing (the user may be playing the game). Test
// processes call MakeBackgroundOnly() before glfwInit: the process becomes
// NSApplicationActivationPolicyProhibited (no Dock icon, never active) and
// any later request for another policy or for activation is ignored.
// Rendering to the hidden test windows is unaffected.
//
// Implemented in background_app.cpp in plain C++ through the Objective-C
// runtime (kept out of this header: objc.h's BOOL clashes with the game's).

namespace cane_test
{
// NSApplicationActivationPolicy values.
constexpr long kPolicyRegular = 0;
constexpr long kPolicyAccessory = 1;
constexpr long kPolicyProhibited = 2;

// The process's activation policy, or -1 when AppKit is not loaded.
long ActivationPolicy();
void MakeBackgroundOnly();
}
