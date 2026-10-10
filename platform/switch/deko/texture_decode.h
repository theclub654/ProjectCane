#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

namespace CaneDeko
{
inline std::vector<uint8_t> DecodeBc(const uint8_t* source, unsigned width, unsigned height, bool bc3)
{
    std::vector<uint8_t> pixels(size_t(width) * height * 4);
    for (unsigned by = 0; by < height; by += 4)
        for (unsigned bx = 0; bx < width; bx += 4, source += bc3 ? 16 : 8)
        {
            const uint8_t* color = source + (bc3 ? 8 : 0);
            uint16_t endpoints[2];
            std::memcpy(endpoints, color, sizeof(endpoints));
            std::array<std::array<uint8_t, 4>, 4> palette{};
            for (unsigned i = 0; i < 2; ++i)
            {
                const unsigned r = endpoints[i] >> 11, g = (endpoints[i] >> 5) & 63, b = endpoints[i] & 31;
                palette[i] = {uint8_t((r << 3) | (r >> 2)), uint8_t((g << 2) | (g >> 4)),
                    uint8_t((b << 3) | (b >> 2)), 255};
            }
            for (unsigned c = 0; c < 3; ++c)
            {
                if (bc3 || endpoints[0] > endpoints[1])
                {
                    palette[2][c] = (2 * palette[0][c] + palette[1][c]) / 3;
                    palette[3][c] = (palette[0][c] + 2 * palette[1][c]) / 3;
                }
                else palette[2][c] = (palette[0][c] + palette[1][c]) / 2;
            }
            palette[2][3] = palette[3][3] = 255;
            std::array<uint8_t, 8> alpha{source[0], source[1]};
            uint64_t alphaIndices = 0;
            if (bc3)
            {
                if (alpha[0] > alpha[1])
                    for (unsigned i = 2; i < 8; ++i)
                        alpha[i] = ((8 - i) * alpha[0] + (i - 1) * alpha[1]) / 7;
                else
                {
                    for (unsigned i = 2; i < 6; ++i)
                        alpha[i] = ((6 - i) * alpha[0] + (i - 1) * alpha[1]) / 5;
                    alpha[7] = 255;
                }
                for (unsigned i = 0; i < 6; ++i) alphaIndices |= uint64_t(source[2 + i]) << (8 * i);
            }
            uint32_t indices;
            std::memcpy(&indices, color + 4, sizeof(indices));
            for (unsigned y = 0; y < 4; ++y)
                for (unsigned x = 0; x < 4; ++x)
                {
                    const unsigned index = y * 4 + x;
                    if (bx + x >= width || by + y >= height) continue;
                    auto rgba = palette[(indices >> (2 * index)) & 3];
                    if (bc3) rgba[3] = alpha[(alphaIndices >> (3 * index)) & 7];
                    std::memcpy(pixels.data() + (size_t(by + y) * width + bx + x) * 4, rgba.data(), 4);
                }
        }
    return pixels;
}
}
