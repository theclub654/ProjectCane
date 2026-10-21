#pragma once

#include <cstdint>
#include <vector>

class CBinaryInputStream;

enum class OptionKind : std::uint8_t {
    Bool, Float, Vector3, Vector4, Int32, Float2, UInt32, Int16, TypedInt16
};

struct OptionSchema {
    OptionKind kind{OptionKind::Int16};
    std::uint32_t flags{};
    std::uint16_t argumentCount{};
    bool present{};
};

const std::vector<OptionSchema>& GetOptionSchema();
void SkipOptionValue(CBinaryInputStream& stream, OptionKind kind);
