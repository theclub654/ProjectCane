#include "BrxWalker.h"

#include <algorithm>
#include <stdexcept>

namespace {
void ReadSaa(CBinaryInputStream& s) {
    const auto type = s.U16Read();
    if (type == 0 || type == 0xffff) return;
    if (type > 8) throw std::runtime_error("Unknown shader animation type");
    s.S16Read();
    s.U16Read();
    s.Skip(24); // All retail SAAK loaders consume a 24-byte subtype record.
}
}

void BrxWalker::LoadShadersFromBrx(CBinaryInputStream& s) {
    const auto begin = s.GetOffset();
    clutBlocks_.clear();
    bitmapBlocks_.clear();

    const auto clutCount = s.U16Read();
    clutBlocks_.reserve(clutCount);
    for (std::uint16_t i = 0; i < clutCount; ++i) {
        s.U32Read();
        const auto colors = s.U16Read();
        const auto colorSize = s.U16Read();
        const auto offset = s.U32Read();
        (void)colorSize;
        // The original loader stores CLUT payload size in 16-byte GS units:
        // (numColors * 4 + 15) >> 4. Keep bytes here for bulk-data extents.
        const auto bytes = (static_cast<std::uint32_t>(colors) * 4u + 15u) & ~15u;
        clutBlocks_.push_back({offset, bytes});
    }

    const auto bitmapCount = s.U16Read();
    bitmapBlocks_.reserve(bitmapCount);
    for (std::uint16_t i = 0; i < bitmapCount; ++i) {
        s.U16Read(); s.U16Read(); s.U32Read(); s.S8Read(); s.S8Read();
        s.U16Read();
        const auto size = s.U32Read();
        const auto offset = s.U32Read();
        bitmapBlocks_.push_back({offset, size});
    }

    const auto shaderCount = s.U16Read();
    s.U16Read(); // total shader-animation count
    for (std::uint16_t shader = 0; shader < shaderCount; ++shader) {
        s.U8Read(); s.U8Read(); s.S16Read();
        s.Skip(8); // base RGBA + volume RGBA
        s.U32Read(); s.U16Read(); s.U8Read();
        const auto textureCount = s.U8Read();
        ReadSaa(s);

        for (std::uint8_t texture = 0; texture < textureCount; ++texture) {
            s.U16Read(); s.S16Read();
            const auto bitmapIndices = s.U8Read();
            const auto clutIndices = s.U8Read();
            for (std::uint8_t i = 0; i < bitmapIndices; ++i) {
                // The PS2 loader relocates these indices directly and performs
                // no range check. PAL files contain references beyond the local
                // serialized count, so validating here rejects valid streams.
                s.U16Read();
            }
            for (std::uint8_t i = 0; i < clutIndices; ++i) {
                s.U16Read(); // 0xFFFF is the original loader's null sentinel.
            }
        }
    }

    const auto fontCount = s.U16Read();
    for (std::uint16_t font = 0; font < fontCount; ++font) {
        const auto bitmap = s.U16Read();
        const auto clut = s.S16Read();
        if (bitmap >= bitmapCount || (clut >= 0 && clut >= clutCount))
            throw std::runtime_error("Font texture reference is out of range");
        s.U8Read(); s.U8Read(); s.U8Read(); s.U8Read(); s.F32Read();
        const auto glyphCount = s.U32Read();
        if (glyphCount > 0x10000u)
            throw std::runtime_error("Implausible font glyph count");
        s.Skip(static_cast<std::size_t>(glyphCount) * 8u);
    }

    Trace("LoadShadersFromBrx", begin, s.GetOffset());
}

void BrxWalker::LoadTexturesFromBrx(CBinaryInputStream& s) {
    const auto begin = s.GetOffset();
    std::uint64_t extent = 0;
    for (const auto& block : clutBlocks_)
        extent = std::max(extent, static_cast<std::uint64_t>(block.offset) + block.size);
    for (const auto& block : bitmapBlocks_)
        extent = std::max(extent, static_cast<std::uint64_t>(block.offset) + block.size);
    if (extent > s.Remaining())
        throw std::runtime_error("Texture payload extends past EOF");

    // PAL's FUN_001dc720 (LoadBulkDataFromBrx) aligns the stream and copies
    // DAT_0027ee94 bytes. This is the final serialized section of the BRX.
    // Individual bitmap/CLUT extents need not reach the end because the bulk
    // allocation can contain additional packed data and padding.
    s.Skip(s.Remaining());
    Trace("LoadBulkDataFromBrx", begin, s.GetOffset());
}
