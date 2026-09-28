#pragma once

#include "gl.h"
#include <algorithm>

struct GuiScale
{
    float x;
    float y;
};

inline GuiScale GetGuiScale(float scaleSetting)
{
    constexpr float kPs2CanvasHeight = 492.80002f;

    if (g_guiStyle == GuiStyle_PS2)
    {
        // Preserve the proportions of artwork authored for the PS2 canvas.
        // Screen-edge anchoring can still respond to framebuffer width, but
        // changing only the host aspect ratio must not stretch fonts, blots,
        // prompts, or icons horizontally.
        const float uniformScale = (g_gl.height / kPs2CanvasHeight) *
            scaleSetting;
        return { uniformScale, uniformScale };
    }

    // Modern UI keeps the original assets and authored layout relationships,
    // but treats them as a compact, uniformly-scaled HUD.  A 720p density
    // makes the HUD roughly 60% of the apparent PS2 size at 1080p and avoids
    // stretching glyphs on widescreen and ultrawide displays.
    constexpr float kModernReferenceHeight = 720.0f;
    constexpr float kModernDensity = 0.90f;
    const float uniformScale = (g_gl.height / kModernReferenceHeight) *
        kModernDensity * scaleSetting;
    return { uniformScale, uniformScale };
}

inline GuiScale GetGuiScale()
{
    return GetGuiScale(g_guiScale);
}

inline bool FModernGui()
{
    return g_guiStyle == GuiStyle_Modern;
}
