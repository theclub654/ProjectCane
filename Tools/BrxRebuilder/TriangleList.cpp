#include "TriangleList.h"

#include <stdexcept>

std::vector<Triangle> ConvertTriangleStrip(const std::vector<StripVertex>& strip)
{
    std::vector<Triangle> triangles;
    if (strip.size() < 3)
        return triangles;

    triangles.reserve(strip.size() - 2);

    std::uint32_t first = 0;
    for (std::size_t vertex = 2; vertex < strip.size(); ++vertex, ++first)
    {
        // This is the same test used by BuildSubGlob. The flag belongs to the
        // third vertex of the candidate triangle.
        if ((strip[first + 2].misc & 0x80u) != 0)
            continue;

        Triangle triangle{};
        triangle.v0 = first;
        if ((vertex & 1u) == 0)
        {
            triangle.v1 = first + 1;
            triangle.v2 = first + 2;
        }
        else
        {
            triangle.v1 = first + 2;
            triangle.v2 = first + 1;
        }

        if (triangle.v0 >= strip.size() || triangle.v1 >= strip.size() ||
            triangle.v2 >= strip.size())
        {
            throw std::runtime_error("triangle-strip conversion produced an out-of-range index");
        }

        triangles.push_back(triangle);
    }

    return triangles;
}

