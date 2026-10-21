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
    if (g_guiStyle == GuiStyle_PS2)
    {
        // Keep the PS2 canvas uniformly fitted when the presentation aspect
        // changes. The retail raster applies a fixed 4:3 horizontal pixel-
        // aspect correction to this UI; it is part of the artwork's displayed
        // shape, not a scale derived from the host window width.
        constexpr float kPs2HorizontalPixelAspect = 4.0f / 3.0f;
        const float canvasScale = g_gl.uiScale * scaleSetting;
        return {
            canvasScale * kPs2HorizontalPixelAspect,
            canvasScale
        };
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
