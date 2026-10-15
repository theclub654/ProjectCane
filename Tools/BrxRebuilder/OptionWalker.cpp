#include "OptionWalker.h"
#include "BinaryInputStream.h"
#include "BrxWalker.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <utility>

namespace {
bool IsPackedColorOption(std::int16_t id) {
    switch (id) {
    case 95: case 96: case 528: case 535:
    case 988: case 1133: case 1134: case 1135:
        return true;
    default:
        return false;
    }
}

bool IsNormalizedWorldLightingOption(std::int16_t id) {
    return id == 526 || id == 527;
}

void AppendF32(std::vector<std::uint8_t>& bytes, float value) {
    std::uint32_t bits = 0;
    static_assert(sizeof(bits) == sizeof(value));
    std::memcpy(&bits, &value, sizeof(bits));
    bytes.push_back(static_cast<std::uint8_t>(bits));
    bytes.push_back(static_cast<std::uint8_t>(bits >> 8));
    bytes.push_back(static_cast<std::uint8_t>(bits >> 16));
    bytes.push_back(static_cast<std::uint8_t>(bits >> 24));
}
}

const std::vector<OptionSchema>& GetOptionSchema() {
    static const std::vector<OptionSchema> schema = [] {
        std::vector<OptionSchema> result(1184);
#define BRX_OPTION(id, kind, flags, argc) result.at(id) = OptionSchema{kind, static_cast<std::uint32_t>(flags), static_cast<std::uint16_t>(argc), true};
#include "OptionSchema.generated.inc"
#undef BRX_OPTION
        return result;
    }();
    return schema;
}

void SkipOptionValue(CBinaryInputStream& s, OptionKind kind) {
    switch (kind) {
    case OptionKind::Bool: s.U8Read(); break;
    case OptionKind::Float: s.F32Read(); break;
    case OptionKind::Vector3: s.ReadVector(); break;
    case OptionKind::Vector4: s.ReadVector4(); break;
    case OptionKind::Int32: s.S32Read(); break;
    case OptionKind::Float2: s.F32Read(); s.F32Read(); break;
    case OptionKind::UInt32: s.U32Read(); break;
    case OptionKind::Int16: s.S16Read(); break;
    case OptionKind::TypedInt16: s.S16Read(); s.S16Read(); break;
    }
}

void BrxWalker::LoadOptionsFromBrx(CBinaryInputStream& s) {
    const auto& schema=GetOptionSchema();
    lastOptionListStart_ = s.GetOffset();
    lastOptionListEnd_ = lastOptionListStart_;
    lastOptionTerminator_ = lastOptionListStart_;
    lastOptionStart_ = lastOptionListStart_;
    lastOptionEnd_ = lastOptionListStart_;
    lastOptionId_ = -1;
    haveLastOption_ = false;
    bool havePrevious = false;
    std::int16_t previousId = -1;
    std::size_t previousStart = 0;
    std::size_t previousEnd = 0;
    while (true) {
        const auto optionStart = s.GetOffset();
        const auto id=s.S16Read();
        if(id == -1) {
            lastOptionTerminator_ = optionStart;
            lastOptionListEnd_ = s.GetOffset();
            return;
        }
        if (std::size_t(id)>=schema.size() || !schema[id].present) {
            std::ostringstream out;
            out << "Unknown EOPID " << id << " at offset 0x" << std::hex << optionStart;
            if (havePrevious)
                out << "; previous EOPID=" << std::dec << previousId << " covered 0x"
                    << std::hex << previousStart << "..0x" << previousEnd;
            throw std::runtime_error(out.str());
        }
        const auto& option=schema[id];
        if ((option.flags&0x400u)!=0) {
            for(std::uint16_t arg=0;arg<option.argumentCount;++arg) {
                const auto typeOffset = s.GetOffset();
                const auto type=s.S16Read();
                if(type==0) SkipOptionValue(s,OptionKind::Bool);
                else if(type==1) SkipOptionValue(s,OptionKind::Float);
                else if(type==2) SkipOptionValue(s,OptionKind::Vector3);
                else if(type==3||type==12) SkipOptionValue(s,OptionKind::Vector4);
                else if(type==4) SkipOptionValue(s,OptionKind::Vector3);
                else if(type==5) SkipOptionValue(s,OptionKind::Int32);
                else if(type==6||type==7||type==14||type==15||type==16||
                        type==17||type==20)
                    SkipOptionValue(s,OptionKind::Int16);
                else if(type==9||type==11) SkipOptionValue(s,OptionKind::Vector3);
                else if(type==10) SkipOptionValue(s,OptionKind::Float2);
                else if(type==13) SkipOptionValue(s,OptionKind::UInt32);
                else if((std::uint32_t(type)&0x7fffffffu)>>12==1) SkipOptionValue(s,OptionKind::Int16);
                else {
                    std::ostringstream out;
                    out << "Unknown dynamic OTYP " << type << " for EOPID " << id
                        << " argument " << arg << '/' << option.argumentCount
                        << " at offset 0x" << std::hex << typeOffset;
                    throw std::runtime_error(out.str());
                }
            }
        } else if (IsPackedColorOption(id)) {
            const auto colorBegin = s.GetOffset();
            const std::uint32_t rgba = s.U32Read();
            const float rgbScale = id == 988 ? (1.0f / 128.0f) : (1.0f / 255.0f);
            std::vector<std::uint8_t> replacement;
            replacement.reserve(16);
            AppendF32(replacement, std::min(static_cast<float>((rgba >> 0) & 0xff) * rgbScale, 1.0f));
            AppendF32(replacement, std::min(static_cast<float>((rgba >> 8) & 0xff) * rgbScale, 1.0f));
            AppendF32(replacement, std::min(static_cast<float>((rgba >> 16) & 0xff) * rgbScale, 1.0f));
            AppendF32(replacement, static_cast<float>((rgba >> 24) & 0xff) / 255.0f);
            RecordReplacement(colorBegin, s.GetOffset(), std::move(replacement));
        } else if (IsNormalizedWorldLightingOption(id)) {
            const auto valueBegin = s.GetOffset();
            const float authoredValue = s.F32Read();
            std::vector<std::uint8_t> replacement;
            replacement.reserve(sizeof(float));
            AppendF32(replacement, authoredValue * (1.0f / 255.0f));
            RecordReplacement(valueBegin, s.GetOffset(), std::move(replacement));
        } else if (captureEmitOrigin_ && !emitOriginCaptured_ && (id==366 || id==421) && option.kind==OptionKind::Int16) {
            emitOriginKind_=s.S16Read(); emitOriginCaptured_=true;
        } else SkipOptionValue(s,option.kind);
        ++stats_.options;
        havePrevious = true;
        previousId = id;
        previousStart = optionStart;
        previousEnd = s.GetOffset();
        lastOptionId_ = id;
        lastOptionStart_ = optionStart;
        lastOptionEnd_ = s.GetOffset();
        lastOptionListEnd_ = s.GetOffset();
        haveLastOption_ = true;
    }
}
