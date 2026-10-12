#pragma once
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <utility>
#include <vector>

struct SoundWaveFormat
{
    uint16_t formatTag = 1;
    uint16_t channels = 0;
    uint32_t samplesPerSec = 0;
    uint32_t avgBytesPerSec = 0;
    uint16_t blockAlign = 0;
    uint16_t bitsPerSample = 0;
};

namespace CaneWave
{
inline uint16_t Le16(const uint8_t* p)
{ return uint16_t(p[0]) | uint16_t(p[1]) << 8; }
inline uint32_t Le32(const uint8_t* p)
{ return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24; }

inline bool Load(const std::filesystem::path& path, SoundWaveFormat& format, std::vector<uint8_t>& pcm)
{
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) return false;
    const auto length = input.tellg();
    if (length < 12) return false;
    input.seekg(0);
    uint8_t header[12];
    if (!input.read(reinterpret_cast<char*>(header), sizeof(header)) ||
        std::memcmp(header, "RIFF", 4) || std::memcmp(header + 8, "WAVE", 4)) return false;
    const uint64_t end = uint64_t(Le32(header + 4)) + 8;
    if (end < 12 || end > uint64_t(length)) return false;

    SoundWaveFormat parsed{};
    bool foundFormat = false;
    uint64_t dataOffset = 0;
    uint32_t dataSize = 0;
    for (uint64_t offset = 12; offset < end;)
    {
        if (end - offset < 8) return false;
        input.seekg(std::streamoff(offset));
        uint8_t chunk[8];
        if (!input.read(reinterpret_cast<char*>(chunk), sizeof(chunk))) return false;
        const uint32_t size = Le32(chunk + 4);
        const uint64_t payload = offset + 8;
        if (size > end - payload) return false;
        if (!std::memcmp(chunk, "fmt ", 4))
        {
            uint8_t fields[16];
            if (size < sizeof(fields) || !input.read(reinterpret_cast<char*>(fields), sizeof(fields))) return false;
            parsed.formatTag = Le16(fields);
            parsed.channels = Le16(fields + 2);
            parsed.samplesPerSec = Le32(fields + 4);
            parsed.avgBytesPerSec = Le32(fields + 8);
            parsed.blockAlign = Le16(fields + 12);
            parsed.bitsPerSample = Le16(fields + 14);
            foundFormat = parsed.formatTag == 1;
            if (parsed.formatTag == 0xfffe)
            {
                uint8_t extension[24];
                if (size < 40 || !input.read(reinterpret_cast<char*>(extension), sizeof(extension))) return false;
                static constexpr uint8_t pcmSubtype[16] =
                { 1, 0, 0, 0, 0, 0, 0x10, 0, 0x80, 0, 0, 0xaa, 0, 0x38, 0x9b, 0x71 };
                const auto validBits = Le16(extension + 2);
                foundFormat = Le16(extension) >= 22 &&
                    !std::memcmp(extension + 8, pcmSubtype, sizeof(pcmSubtype)) &&
                    (validBits == 0 || validBits <= parsed.bitsPerSample);
                if (foundFormat) parsed.formatTag = 1;
            }
        }
        else if (!std::memcmp(chunk, "data", 4))
        {
            dataOffset = payload;
            dataSize = size;
        }
        offset = payload + size + (size & 1);
        if (offset > end) return false;
    }
    if (!foundFormat || parsed.formatTag != 1 || parsed.channels < 1 || parsed.channels > 64 ||
        parsed.samplesPerSec < 1000 || parsed.samplesPerSec > 200000 ||
        (parsed.bitsPerSample != 8 && parsed.bitsPerSample != 16 &&
         parsed.bitsPerSample != 24 && parsed.bitsPerSample != 32) ||
        parsed.blockAlign != uint32_t(parsed.channels) * (parsed.bitsPerSample / 8) ||
        uint64_t(parsed.avgBytesPerSec) != uint64_t(parsed.samplesPerSec) * parsed.blockAlign ||
        !dataSize || dataSize > 64 * 1024 * 1024 || dataSize % parsed.blockAlign) return false;

    std::vector<uint8_t> data(dataSize);
    input.seekg(std::streamoff(dataOffset));
    if (!input.read(reinterpret_cast<char*>(data.data()), data.size())) return false;
    format = parsed;
    pcm = std::move(data);
    return true;
}
}
