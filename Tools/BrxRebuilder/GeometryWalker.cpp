#include "BrxWalker.h"

#include <stdexcept>

GeometrySummary BrxWalker::ReadGeom(CBinaryInputStream& stream) {
    const auto begin = stream.GetOffset();
    stream.F32Read();

    GeometrySummary geometry{};
    geometry.positions = stream.U16Read();
    for (std::uint16_t i = 0; i < geometry.positions; ++i)
        stream.ReadVector();

    geometry.surfaces = stream.U16Read();
    geometry.edges = stream.U16Read();
    std::uint32_t edgesRead = 0;

    for (std::uint16_t surface = 0; surface < geometry.surfaces; ++surface) {
        for (int vertex = 0; vertex < 3; ++vertex) {
            const auto index = static_cast<std::int16_t>(stream.U16Read());
            if (index < 0 || index >= geometry.positions)
                throw std::runtime_error("ReadGeom position index is out of range");
        }

        stream.F32Read();
        stream.ReadVector();
        const auto edgeCount = static_cast<std::int8_t>(stream.U8Read());
        if (edgeCount < 0)
            throw std::runtime_error("ReadGeom contains a negative edge count");

        for (int edge = 0; edge < edgeCount; ++edge) {
            const auto position0 = static_cast<std::int16_t>(stream.U16Read());
            const auto position1 = static_cast<std::int16_t>(stream.U16Read());
            const auto otherSurface = static_cast<std::int16_t>(stream.U16Read());

            if (position0 < 0 || position0 >= geometry.positions ||
                position1 < 0 || position1 >= geometry.positions ||
                otherSurface < 0 || otherSurface >= geometry.surfaces)
                throw std::runtime_error("ReadGeom edge reference is out of range");

            ++edgesRead;
        }
    }

    if (edgesRead != geometry.edges)
        throw std::runtime_error("ReadGeom edge total differs from its header");

    ++stats_.geometries;
    Trace("ReadGeom", begin, stream.GetOffset());
    return geometry;
}

void BrxWalker::ReadBspc(CBinaryInputStream& stream,
    const GeometrySummary& geometry) {
    const auto begin = stream.GetOffset();
    const auto count = stream.U16Read();
    stream.U16Read();

    for (std::uint16_t i = 0; i < count; ++i) {
        const auto surface = stream.U16Read();
        const auto negative = stream.U16Read();
        const auto positive = stream.U16Read();

        if (surface >= geometry.surfaces ||
            (negative != 0xffff && negative >= count) ||
            (positive != 0xffff && positive >= count))
            throw std::runtime_error("ReadBspc reference is out of range");
    }

    stats_.bspNodes += count;
    Trace("ReadBspc", begin, stream.GetOffset());
}
