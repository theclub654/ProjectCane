#pragma once
// The fullscreen checks of the display sweep (sweep_child.cpp, ModeDisplay),
// kept free of GLFW and CoreGraphics so the units (sweep_units.cpp) can feed
// them a Retina display this Mac may not have connected.
//
// On macOS GLFW reports the window size and the monitor's video mode in
// points (CGDisplayModeGetWidth/Height) and the framebuffer in pixels, so on
// a 2x display a correct fullscreen window has a framebuffer twice the mode.

#include <cmath>

namespace sweep_display
{
// A display mode as CoreGraphics reports it (CGDisplayCopyDisplayMode).
struct DisplayMode
{
    bool valid = false;
    int width = 0;       // points
    int height = 0;
    int pixelWidth = 0;
    int pixelHeight = 0;
    double refreshRate = 0.0;
};

// Fullscreen must never change the display mode.
inline bool SameMode(const DisplayMode& a, const DisplayMode& b)
{
    return a.valid && b.valid && a.width == b.width && a.height == b.height &&
        a.pixelWidth == b.pixelWidth && a.pixelHeight == b.pixelHeight &&
        std::fabs(a.refreshRate - b.refreshRate) < 0.01;
}

struct FullscreenGeometry
{
    int monitorX = 0, monitorY = 0;   // glfwGetMonitorPos (points)
    int modeWidth = 0, modeHeight = 0; // glfwGetVideoMode (points)
    int windowX = 0, windowY = 0;     // glfwGetWindowPos (points)
    int windowWidth = 0, windowHeight = 0; // glfwGetWindowSize (points)
    int framebufferWidth = 0, framebufferHeight = 0; // glfwGetFramebufferSize (pixels)
    float scaleX = 1.0f, scaleY = 1.0f; // glfwGetWindowContentScale
};

// The window covers the whole monitor, in points.
inline bool WindowCoversMonitor(const FullscreenGeometry& g)
{
    return g.windowX == g.monitorX && g.windowY == g.monitorY &&
        g.windowWidth == g.modeWidth && g.windowHeight == g.modeHeight;
}

// The framebuffer is the window at the display's backing scale, in pixels.
inline bool FramebufferMatchesScale(const FullscreenGeometry& g)
{
    return g.framebufferWidth == static_cast<int>(std::lround(g.windowWidth * g.scaleX)) &&
        g.framebufferHeight == static_cast<int>(std::lround(g.windowHeight * g.scaleY));
}
}
