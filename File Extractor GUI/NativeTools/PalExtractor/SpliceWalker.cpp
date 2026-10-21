#include "BrxWalker.h"

#include <stdexcept>

namespace {
constexpr unsigned kMaximumPairDepth = 4096;
}

void BrxWalker::ReadSerializedPair(CBinaryInputStream& s, bool debugInfo,
    unsigned depth) {
    if (depth > kMaximumPairDepth)
        throw std::runtime_error("Serialized pair nesting is unreasonably deep");

    const auto tagByte = s.U8Read();
    if (tagByte == 0xff) return;

    const auto tag = static_cast<std::uint8_t>(tagByte & 0x7f);
    const bool hasNext = (tagByte & 0x80) != 0;

    if (tag > 36)
        throw std::runtime_error("Unknown serialized pair tag " + std::to_string(tag));

    if (debugInfo)
        for (int i = 0; i < 5; ++i) s.U16Read();

    switch (tag) {
    case 20: s.S32Read(); break;                 // TAGK_S32
    case 21: s.F32Read(); break;                 // TAGK_F32
    case 22: s.ReadVector(); break;              // TAGK_Vector
    case 23: for (int i = 0; i < 16; ++i) s.F32Read(); break;
    case 24: s.F32Read(); s.F32Read(); s.F32Read(); break;
    case 25: s.F32Read(); s.F32Read(); break;
    case 26: s.F32Read(); s.F32Read(); s.F32Read(); break;
    case 27: case 28: case 29: case 32: s.U32Read(); break;
    case 30: ReadSerializedPair(s, debugInfo, depth + 1); break;
    default: break;
    }

    if (hasNext)
        ReadSerializedPair(s, debugInfo, depth + 1);
}

void BrxWalker::LoadSwSpliceFromBrx(CBinaryInputStream& s, bool debugInfo) {
    const auto begin = s.GetOffset();
    const auto spliceCount = s.U32Read();
    const auto symbolCount = s.U32Read();
    const auto extraOptionCount = s.U32Read();

    if (spliceCount > 1'000'000 || symbolCount > 10'000'000 ||
        extraOptionCount > 10'000'000)
        throw std::runtime_error("Implausible splice table counts");

    for (std::uint32_t i = 0; i < spliceCount; ++i) {
        ReadSerializedPair(s, debugInfo);
        if (debugInfo) {
            s.ReadStringSw();
            s.ReadStringSw();
            s.ReadStringSw();
        }
    }

    if (debugInfo) {
        for (std::uint32_t i = 0; i < symbolCount; ++i) s.ReadStringSw();
        for (std::uint32_t i = 0; i < extraOptionCount; ++i) s.ReadStringSw();
    }

    Trace("LoadSwSpliceFromBrx", begin, s.GetOffset());
}
