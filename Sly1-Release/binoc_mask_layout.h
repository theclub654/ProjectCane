#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace CaneBinoc
{
    struct MaskRegion
    {
        int clipLeft;
        int clipRight;
        float scaleX;
        float translateX;
    };

    struct MaskLayout
    {
        std::array<MaskRegion, 3> regions;
        int clipHeight;
    };

    inline MaskLayout LayoutMask(float screenWidth, float scale,
        int rasterWidth, int rasterHeight)
    {
        constexpr float virtualWidth = 640.0f;
        constexpr float centreLeft = 240.0f;
        constexpr float centreRight = 400.0f;
        const float centreOrigin = screenWidth * 0.5f - virtualWidth * 0.5f * scale;
        const float leftJoin = std::clamp(centreOrigin + centreLeft * scale, 0.0f, screenWidth);
        const float rightJoin = std::clamp(centreOrigin + centreRight * scale, leftJoin, screenWidth);

        const float rasterScaleX = static_cast<float>(rasterWidth) / screenWidth;
        const int leftCut = std::clamp(static_cast<int>(std::lround(leftJoin * rasterScaleX)),
            0, rasterWidth);
        const int rightCut = std::clamp(static_cast<int>(std::lround(rightJoin * rasterScaleX)),
            leftCut, rasterWidth);
        const float leftScale = leftJoin / centreLeft;
        const float rightScale = (screenWidth - rightJoin) / (virtualWidth - centreRight);

        return { {{
            { 0, leftCut, leftScale, 0.0f },
            { leftCut, rightCut, scale, centreOrigin },
            { rightCut, rasterWidth, rightScale, rightJoin - centreRight * rightScale }
        }}, rasterHeight };
    }
}
