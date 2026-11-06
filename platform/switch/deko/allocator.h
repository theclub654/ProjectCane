#pragma once
#include <algorithm>
#include <cstdint>
#include <map>
#include <optional>
#include <stdexcept>

namespace CaneDeko
{
inline uint32_t Align(uint32_t value, uint32_t alignment)
{
    if (!alignment) throw std::logic_error("Zero GPU allocation alignment");
    const uint64_t result = (uint64_t(value) + alignment - 1) / alignment * alignment;
    if (result > UINT32_MAX) throw std::overflow_error("GPU allocation alignment overflow");
    return uint32_t(result);
}

class RangeAllocator
{
    uint32_t capacity;
    std::map<uint32_t, uint32_t> free;
public:
    explicit RangeAllocator(uint32_t size) : capacity(size), free{{0, size}} {}
    std::optional<uint32_t> Allocate(uint32_t size, uint32_t alignment)
    {
        if (!size || !alignment) return {};
        for (auto it = free.begin(); it != free.end(); ++it)
        {
            const uint32_t start = Align(it->first, alignment);
            const uint64_t end = uint64_t(start) + size;
            if (end > uint64_t(it->first) + it->second) continue;
            const auto [oldStart, oldSize] = *it;
            free.erase(it);
            if (start > oldStart) free.emplace(oldStart, start - oldStart);
            if (end < uint64_t(oldStart) + oldSize) free.emplace(uint32_t(end), oldStart + oldSize - uint32_t(end));
            return start;
        }
        return {};
    }
    void Release(uint32_t start, uint32_t size)
    {
        if (!size || uint64_t(start) + size > capacity) throw std::logic_error("Invalid GPU allocation release");
        auto next = free.lower_bound(start);
        if (next != free.end() && uint64_t(start) + size > next->first) throw std::logic_error("Overlapping GPU allocation release");
        if (next != free.begin())
        {
            auto previous = std::prev(next);
            if (uint64_t(previous->first) + previous->second > start) throw std::logic_error("Duplicate GPU allocation release");
            if (previous->first + previous->second == start)
            {
                start = previous->first; size += previous->second; free.erase(previous);
            }
        }
        if (next != free.end() && uint64_t(start) + size == next->first)
        {
            size += next->second; free.erase(next);
        }
        free.emplace(start, size);
    }
    uint64_t FreeBytes() const
    {
        uint64_t result = 0;
        for (const auto& [offset, size] : free) result += size;
        return result;
    }
};
}
