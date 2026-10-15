#pragma once

#include <cstdint>
#include <vector>

// The four-byte render-vertex reference serialized by the original PS2 BRX.
// The high bit of misc suppresses the triangle that would end at this vertex.
struct StripVertex
{
    std::uint8_t position{};
    std::uint8_t normal{};
    std::uint8_t texcoord{};
    std::uint8_t misc{};
};

struct Triangle
{
    std::uint32_t v0{};
    std::uint32_t v1{};
    std::uint32_t v2{};
};

// Mirrors ProjectCane's BuildSubGlob winding and strip-break behavior exactly.
std::vector<Triangle> ConvertTriangleStrip(const std::vector<StripVertex>& strip);

