#pragma once
#include <cmath>
#include <cstdint>

inline int16_t CaneFloatToPcm16(float sample)
{
    if (!std::isfinite(sample)) return 0;
    if (sample <= -1.0f) return INT16_MIN;
    if (sample >= 1.0f) return INT16_MAX;
    const auto value=std::lrint(sample * 32768.0f);
    return int16_t(value>INT16_MAX ? INT16_MAX : value<INT16_MIN ? INT16_MIN : value);
}
