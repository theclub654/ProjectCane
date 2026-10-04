#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <vector>
namespace cane {
inline constexpr std::size_t CHUNK_SIZE = 0x2000, MAX_MATCH_LENGTH = 10;
inline std::vector<uint8_t> decompressBrx(std::span<const uint8_t> input,
                                          std::size_t outputLimit = 512ull * 1024 * 1024) {
    std::vector<uint8_t> output;
    if (input.empty() || input.size() > 256ull * 1024 * 1024)
        throw std::runtime_error("Invalid compressed asset size");
    output.reserve(input.size());

    std::array<uint8_t, CHUNK_SIZE + MAX_MATCH_LENGTH> work{};

    std::size_t inputPosition = 0;
    std::size_t outputPosition = 0;

    uint32_t flags = 0;

    while (inputPosition < input.size()) {
        flags >>= 1;

        if ((flags & 0x100U) == 0) {
            if (inputPosition >= input.size())
                throw std::runtime_error("Truncated compressed asset");

            flags = 0xFF00U | input[inputPosition++];
        }

        if (inputPosition >= input.size()) break;

        const uint8_t firstByte = input[inputPosition++];

        if ((flags & 1U) != 0) {
            work[outputPosition++] = firstByte;
        } else {
            if (inputPosition >= input.size())
                throw std::runtime_error("Truncated compressed asset");

            const uint8_t secondByte = input[inputPosition++];
            const uint16_t token = static_cast<uint16_t>(firstByte) |
                                   static_cast<uint16_t>(static_cast<uint16_t>(secondByte) << 8);

            std::size_t sourcePosition = token & 0x1FFFU;
            const std::size_t copyLength = static_cast<std::size_t>(token >> 13) + 3U;

            for (std::size_t index = 0; index < copyLength; ++index) {
                work[outputPosition++] = work[sourcePosition & 0x1FFFU];
                ++sourcePosition;
            }
        }

        if (output.size() + outputPosition > outputLimit)
            throw std::runtime_error("Decompressed asset exceeds limit");
        if (outputPosition >= CHUNK_SIZE) {
            output.insert(output.end(), work.begin(), work.begin() + CHUNK_SIZE);

            const std::size_t spillCount = outputPosition - CHUNK_SIZE;

            if (spillCount > 0) std::memmove(work.data(), work.data() + CHUNK_SIZE, spillCount);

            outputPosition = spillCount;
        }
    }

    output.insert(output.end(), work.begin(), work.begin() + outputPosition);

    return output;
}

}  // namespace cane
