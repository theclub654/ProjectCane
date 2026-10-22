#include "PalIsoExtractor.h"
#include "BrxWalker.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace {
constexpr std::uint32_t kSectorSize = 0x800;
constexpr std::uint32_t kPalLevelTableVa = 0x002490F0;
constexpr std::uint32_t kJapaneseLevelTableVa = 0x00249330;
constexpr std::uint32_t kPalCutsceneTableVa = 0x0024B0F0;
constexpr std::uint32_t kJapaneseCutsceneTableVa = 0x0024A210;
constexpr std::size_t kLevelCount = 46;
constexpr std::size_t kCutsceneCount = 16;
constexpr std::size_t kLanguageCount = 5;
constexpr std::size_t kDescriptorSize = 32;
constexpr std::size_t kRecordSize = 172;
constexpr std::size_t kJapaneseRecordSize = 76;
constexpr std::size_t kJapaneseSlotCount = 2;
constexpr std::size_t kMaximumCompressedSize = 256u * 1024u * 1024u;

struct SoundBankLocation {
    std::string_view name;
    std::uint32_t sector;
};

constexpr std::array<SoundBankLocation, 7> kPalSoundBanks = {{
    {"Common.bnk", 0x06A932},
    {"Intro.bnk", 0x0CEA72},
    {"Underwater.bnk", 0x06E559},
    {"Muggshot.bnk", 0x0719C2},
    {"Voodoo.bnk", 0x0809D7},
    {"Snow.bnk", 0x08568F},
    {"Clockwerk.bnk", 0x065D9D}
}};

constexpr std::array<SoundBankLocation, 7> kJapaneseSoundBanks = {{
    {"Common.bnk", 0x0A32F2},
    {"Intro.bnk", 0x0CD19A},
    {"Underwater.bnk", 0x0A4835},
    {"Muggshot.bnk", 0x0A5721},
    {"Voodoo.bnk", 0x0AC27B},
    {"Snow.bnk", 0x0AEAD9},
    {"Clockwerk.bnk", 0x0A0A93}
}};

std::uint16_t U16(std::span<const std::byte> data, std::size_t at) {
    if (at + 2 > data.size()) throw std::runtime_error("Unexpected end of data");
    return static_cast<std::uint16_t>(std::to_integer<unsigned>(data[at]) |
        (std::to_integer<unsigned>(data[at + 1]) << 8));
}

std::uint32_t U32(std::span<const std::byte> data, std::size_t at) {
    if (at + 4 > data.size()) throw std::runtime_error("Unexpected end of data");
    return static_cast<std::uint32_t>(std::to_integer<unsigned>(data[at])) |
        (static_cast<std::uint32_t>(std::to_integer<unsigned>(data[at + 1])) << 8) |
        (static_cast<std::uint32_t>(std::to_integer<unsigned>(data[at + 2])) << 16) |
        (static_cast<std::uint32_t>(std::to_integer<unsigned>(data[at + 3])) << 24);
}

std::vector<std::byte> ReadFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) throw std::runtime_error("Could not open ISO");
    const auto end = file.tellg();
    if (end < 0) throw std::runtime_error("Could not determine ISO size");
    std::vector<std::byte> bytes(static_cast<std::size_t>(end));
    file.seekg(0);
    if (!bytes.empty() && !file.read(reinterpret_cast<char*>(bytes.data()), end))
        throw std::runtime_error("Could not read ISO");
    return bytes;
}

std::string IsoName(std::span<const std::byte> bytes) {
    std::string value;
    for (const auto byte : bytes) value.push_back(static_cast<char>(std::to_integer<unsigned>(byte)));
    if (const auto semicolon = value.find(';'); semicolon != std::string::npos) value.resize(semicolon);
    return value;
}

struct IsoFile { std::string name; std::uint32_t sector{}, size{}; };

std::vector<IsoFile> RootFiles(std::span<const std::byte> iso) {
    const std::size_t pvd = 16u * kSectorSize;
    if (pvd + kSectorSize > iso.size() || U32(iso, pvd + 1) != 0x30304443u)
        throw std::runtime_error("Not an ISO9660 image");
    const auto rootSector = U32(iso, pvd + 156 + 2);
    const auto rootSize = U32(iso, pvd + 156 + 10);
    const std::size_t rootOffset = static_cast<std::size_t>(rootSector) * kSectorSize;
    if (rootOffset + rootSize > iso.size()) throw std::runtime_error("Invalid ISO root directory");
    std::vector<IsoFile> files;
    std::size_t pos = rootOffset;
    const std::size_t end = rootOffset + rootSize;
    while (pos < end) {
        const auto length = std::to_integer<unsigned>(iso[pos]);
        if (length == 0) { pos = ((pos / kSectorSize) + 1) * kSectorSize; continue; }
        if (pos + length > end || length < 34) break;
        const auto nameLength = std::to_integer<unsigned>(iso[pos + 32]);
        if (pos + 33 + nameLength <= end && (std::to_integer<unsigned>(iso[pos + 25]) & 2u) == 0)
            files.push_back({IsoName(iso.subspan(pos + 33, nameLength)), U32(iso, pos + 2), U32(iso, pos + 10)});
        pos += length;
    }
    return files;
}

const IsoFile& FindGameElf(const std::vector<IsoFile>& files) {
    const auto found = std::find_if(files.begin(), files.end(), [](const IsoFile& file) {
        return file.name.starts_with("SCES_") || file.name.starts_with("SCES-") ||
               file.name.starts_with("SCED_") || file.name.starts_with("SCED-") ||
               file.name.starts_with("SCPS_") || file.name.starts_with("SCPS-");
    });
    if (found == files.end())
        throw std::runtime_error("PAL/Japanese executable (SCES/SCED/SCPS) was not found");
    return *found;
}

bool IsJapaneseElf(const IsoFile& elf) {
    return elf.name.starts_with("SCPS_") || elf.name.starts_with("SCPS-");
}

std::size_t VaToIsoOffset(std::span<const std::byte> iso, const IsoFile& elf, std::uint32_t va) {
    const std::size_t base = static_cast<std::size_t>(elf.sector) * kSectorSize;
    if (base + 52 > iso.size() || U32(iso, base) != 0x464C457Fu)
        throw std::runtime_error("PAL executable is not a 32-bit little-endian ELF");
    const auto phoff = U32(iso, base + 28);
    const auto phentsize = U16(iso, base + 42);
    const auto phnum = U16(iso, base + 44);
    for (std::size_t i = 0; i < phnum; ++i) {
        const std::size_t ph = base + phoff + i * phentsize;
        const auto fileOffset = U32(iso, ph + 4);
        const auto vaddr = U32(iso, ph + 8);
        const auto fileSize = U32(iso, ph + 16);
        if (va >= vaddr && va - vaddr < fileSize) return base + fileOffset + (va - vaddr);
    }
    throw std::runtime_error("Virtual address is not backed by an ELF segment");
}

std::string ReadName(std::span<const std::byte> iso, const IsoFile& elf, std::uint32_t pointer) {
    const auto at = VaToIsoOffset(iso, elf, pointer);
    if (at + 4 < iso.size() && iso[at + 1] == std::byte{} && iso[at + 3] == std::byte{}) {
        std::string utf8;
        for (std::size_t i = 0; i < 255 && at + i * 2 + 1 < iso.size(); ++i) {
            const auto ch = U16(iso, at + i * 2);
            if (ch == 0) break;
            if (ch < 0x80) utf8.push_back(static_cast<char>(ch));
            else if (ch < 0x800) {
                utf8.push_back(static_cast<char>(0xC0 | (ch >> 6)));
                utf8.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
            } else {
                utf8.push_back(static_cast<char>(0xE0 | (ch >> 12)));
                utf8.push_back(static_cast<char>(0x80 | ((ch >> 6) & 0x3F)));
                utf8.push_back(static_cast<char>(0x80 | (ch & 0x3F)));
            }
        }
        return utf8;
    }
    std::string narrow;
    for (std::size_t i = 0; i < 255 && at + i < iso.size() && iso[at + i] != std::byte{}; ++i)
        narrow.push_back(static_cast<char>(std::to_integer<unsigned>(iso[at + i])));
    return narrow;
}

std::string SafeName(std::string name, std::size_t index) {
    for (auto& ch : name)
        if (static_cast<unsigned char>(ch) < 32 || std::string_view("<>:\"/\\|?*").find(ch) != std::string_view::npos) ch = '_';
    while (!name.empty() && (name.back() == ' ' || name.back() == '.')) name.pop_back();
    return name.empty() ? "Level_" + std::to_string(index) : name;
}

std::string LevelFileName(std::uint32_t levelId) {
    std::ostringstream name;
    name << "Level_" << std::uppercase << std::hex << std::setfill('0')
         << std::setw(3) << levelId;
    return name.str();
}

std::vector<std::byte> Decompress(std::span<const std::byte> input) {
    constexpr std::size_t chunk = 0x2000;
    std::array<std::byte, chunk + 10> work{};
    std::vector<std::byte> output;
    output.reserve(input.size() * 3);
    std::size_t in = 0, out = 0;
    std::uint32_t flags = 0;
    while (in < input.size()) {
        flags >>= 1;
        if ((flags & 0x100u) == 0) flags = 0xFF00u | std::to_integer<unsigned>(input[in++]);
        if (in >= input.size()) break;
        const auto first = std::to_integer<unsigned>(input[in++]);
        if ((flags & 1u) != 0) work[out++] = static_cast<std::byte>(first);
        else {
            if (in >= input.size()) break;
            const auto token = first | (std::to_integer<unsigned>(input[in++]) << 8);
            auto source = token & 0x1FFF;
            const auto count = (token >> 13) + 3;
            for (unsigned i = 0; i < count; ++i) work[out++] = work[source++ & 0x1FFF];
        }
        if (out >= chunk) {
            output.insert(output.end(), work.begin(), work.begin() + chunk);
            const auto spill = out - chunk;
            std::move(work.begin() + chunk, work.begin() + out, work.begin());
            out = spill;
        }
    }
    output.insert(output.end(), work.begin(), work.begin() + out);
    return output;
}

std::uint32_t Be32(std::span<const std::byte> data, std::size_t at) {
    if (at + 4 > data.size()) throw std::runtime_error("Unexpected end of VAG data");
    return (static_cast<std::uint32_t>(std::to_integer<unsigned>(data[at])) << 24) |
        (static_cast<std::uint32_t>(std::to_integer<unsigned>(data[at + 1])) << 16) |
        (static_cast<std::uint32_t>(std::to_integer<unsigned>(data[at + 2])) << 8) |
        static_cast<std::uint32_t>(std::to_integer<unsigned>(data[at + 3]));
}

std::uint16_t Be16(std::span<const std::byte> data, std::size_t at) {
    if (at + 2 > data.size()) throw std::runtime_error("Unexpected end of big-endian data");
    return static_cast<std::uint16_t>((std::to_integer<unsigned>(data[at]) << 8) |
        std::to_integer<unsigned>(data[at + 1]));
}

void Write16(std::ofstream& output, std::uint16_t value) {
    const char bytes[] = {static_cast<char>(value), static_cast<char>(value >> 8)};
    output.write(bytes, sizeof(bytes));
}

void Write32(std::ofstream& output, std::uint32_t value) {
    const char bytes[] = {static_cast<char>(value), static_cast<char>(value >> 8),
        static_cast<char>(value >> 16), static_cast<char>(value >> 24)};
    output.write(bytes, sizeof(bytes));
}

std::vector<std::int16_t> DecodeAdpcm(std::span<const std::byte> source) {
    static constexpr int coefficients[5][2] = {{0, 0}, {60, 0}, {115, -52}, {98, -55}, {122, -60}};
    std::vector<std::int16_t> pcm;
    pcm.reserve((source.size() / 16) * 28);
    int history1 = 0, history2 = 0;
    for (std::size_t at = 0; at + 16 <= source.size(); at += 16) {
        const auto predictorShift = std::to_integer<unsigned>(source[at]);
        const auto predictor = predictorShift >> 4;
        const auto shift = predictorShift & 0x0F;
        if (predictor >= 5) continue;
        bool end = predictorShift == 0 && source[at + 1] == std::byte{1};
        for (std::size_t i = 2; end && i < 16; ++i) end = source[at + i] == std::byte{0x77};
        if (end) break;
        for (std::size_t i = 0; i < 28; ++i) {
            const auto packed = std::to_integer<unsigned>(source[at + 2 + i / 2]);
            int nibble = (i & 1) == 0 ? (packed & 0x0F) : (packed >> 4);
            if (nibble >= 8) nibble -= 16;
            int sample = (nibble << 12) >> shift;
            sample += (history1 * coefficients[predictor][0] + history2 * coefficients[predictor][1] + 32) >> 6;
            sample = std::clamp(sample, -32768, 32767);
            history2 = history1;
            history1 = sample;
            pcm.push_back(static_cast<std::int16_t>(sample));
        }
    }
    return pcm;
}

std::vector<std::int16_t> DecodePcm(std::span<const std::byte> source, bool littleEndian) {
    std::vector<std::int16_t> pcm(source.size() / 2);
    for (std::size_t i = 0; i < pcm.size(); ++i) {
        const auto a = std::to_integer<unsigned>(source[i * 2]);
        const auto b = std::to_integer<unsigned>(source[i * 2 + 1]);
        pcm[i] = static_cast<std::int16_t>(littleEndian ? (a | (b << 8)) : ((a << 8) | b));
    }
    return pcm;
}

std::vector<std::int16_t> DecodeVag(std::span<const std::byte> source, std::uint32_t& sampleRate) {
    if (source.size() < 0x30 || std::to_integer<unsigned>(source[0]) != 'V' ||
        std::to_integer<unsigned>(source[1]) != 'A' || std::to_integer<unsigned>(source[2]) != 'G' ||
        std::to_integer<unsigned>(source[3]) != 'p')
        throw std::runtime_error("Audio stream does not have a VAGp header");
    const auto declaredSize = Be32(source, 0x0C);
    sampleRate = Be32(source, 0x10);
    if (sampleRate < 4000 || sampleRate > 192000) throw std::runtime_error("Invalid VAG sample rate");
    auto encodedSize = declaredSize == 0 ? source.size() - 0x30 :
        std::min<std::size_t>(declaredSize, source.size() - 0x30);
    encodedSize &= ~std::size_t{15};
    return DecodeAdpcm(source.subspan(0x30, encodedSize));
}

void WriteWave(const std::filesystem::path& path, std::span<const std::int16_t> pcm,
    std::uint32_t sampleRate) {
    std::ofstream output(path, std::ios::binary);
    if (!output) throw std::runtime_error("Could not create " + path.string());
    const auto dataSize = static_cast<std::uint32_t>(pcm.size() * sizeof(std::int16_t));
    output.write("RIFF", 4); Write32(output, 36 + dataSize); output.write("WAVE", 4);
    output.write("fmt ", 4); Write32(output, 16); Write16(output, 1); Write16(output, 1);
    Write32(output, sampleRate); Write32(output, sampleRate * 2); Write16(output, 2); Write16(output, 16);
    output.write("data", 4); Write32(output, dataSize);
    if (!pcm.empty()) output.write(reinterpret_cast<const char*>(pcm.data()), dataSize);
    if (!output) throw std::runtime_error("Could not write " + path.string());
}

void WriteWave(const std::filesystem::path& path, std::span<const std::int16_t> left,
    std::span<const std::int16_t> right, std::uint16_t channels, std::uint32_t sampleRate) {
    const auto frames = channels == 2 ? std::min(left.size(), right.size()) : left.size();
    if (frames == 0 || channels < 1 || channels > 2)
        throw std::runtime_error("Audio decoder produced no samples");
    std::ofstream output(path, std::ios::binary);
    if (!output) throw std::runtime_error("Could not create " + path.string());
    const auto dataSize = static_cast<std::uint32_t>(frames * channels * sizeof(std::int16_t));
    output.write("RIFF", 4); Write32(output, 36 + dataSize); output.write("WAVE", 4);
    output.write("fmt ", 4); Write32(output, 16); Write16(output, 1); Write16(output, channels);
    Write32(output, sampleRate); Write32(output, sampleRate * channels * 2);
    Write16(output, channels * 2); Write16(output, 16);
    output.write("data", 4); Write32(output, dataSize);
    for (std::size_t i = 0; i < frames; ++i) {
        Write16(output, static_cast<std::uint16_t>(left[i]));
        if (channels == 2) Write16(output, static_cast<std::uint16_t>(right[i]));
    }
    if (!output) throw std::runtime_error("Could not write " + path.string());
}

std::string AudioFileName(const AudioDescriptor& descriptor) {
    std::ostringstream name;
    name << "VAG_" << std::uppercase << std::hex << std::setfill('0')
         << std::setw(4) << descriptor.identity << '_' << std::setw(6) << descriptor.sector << ".wav";
    return name.str();
}

void ExtractAudio(std::span<const std::byte> iso, const std::filesystem::path& dialogDirectory,
    std::span<const AudioDescriptor> descriptors) {
    std::unordered_set<std::uint64_t> extracted;
    for (const auto& descriptor : descriptors) {
        const auto key = (static_cast<std::uint64_t>(descriptor.sector) << 32) | descriptor.identity;
        if (!extracted.insert(key).second) continue;
        const auto offset = static_cast<std::size_t>(descriptor.sector) * kSectorSize;
        if (offset > iso.size() || descriptor.size > iso.size() - offset)
            throw std::runtime_error("Audio descriptor points outside the PAL ISO");
        std::filesystem::create_directories(dialogDirectory);
        std::uint32_t sampleRate{};
        const auto pcm = DecodeVag(iso.subspan(offset, descriptor.size), sampleRate);
        const auto path = dialogDirectory / AudioFileName(descriptor);
        WriteWave(path, pcm, sampleRate);
        std::cout << "    audio: " << path.string() << " (" << sampleRate << " Hz)\n";
    }
}

void ExtractMusic(std::span<const std::byte> iso, const std::filesystem::path& musicDirectory,
    std::span<const AudioDescriptor> descriptors) {
    for (const auto& descriptor : descriptors) {
        const auto offset = static_cast<std::size_t>(descriptor.sector) * kSectorSize;
        if (offset > iso.size() || descriptor.size > iso.size() - offset)
            throw std::runtime_error("Music descriptor points outside the PAL ISO");
        std::ostringstream name;
        name << "MUSID_" << std::uppercase << std::hex << std::setfill('0')
             << std::setw(2) << descriptor.identity << ".mus";
        std::filesystem::create_directories(musicDirectory);
        const auto path = musicDirectory / name.str();
        if (std::filesystem::exists(path)) continue;
        std::ofstream output(path, std::ios::binary);
        if (!output || !output.write(reinterpret_cast<const char*>(iso.data() + offset), descriptor.size))
            throw std::runtime_error("Could not write " + path.string());
        std::cout << "    music: " << path.string() << '\n';
    }
}

void ExtractSoundBanks(std::span<const std::byte> iso,
    const std::filesystem::path& soundBankDirectory,
    std::span<const SoundBankLocation> banks, std::string_view regionName) {
    std::filesystem::create_directories(soundBankDirectory);
    for (const auto& bank : banks) {
        const auto offset = static_cast<std::size_t>(bank.sector) * kSectorSize;
        if (offset > iso.size() || iso.size() - offset < 0x20)
            throw std::runtime_error("Sound bank header points outside the " +
                std::string(regionName) + " ISO");
        if (std::to_integer<unsigned>(iso[offset + 0x18]) != 'S' ||
            std::to_integer<unsigned>(iso[offset + 0x19]) != 'B' ||
            std::to_integer<unsigned>(iso[offset + 0x1A]) != 'l' ||
            std::to_integer<unsigned>(iso[offset + 0x1B]) != 'k')
            throw std::runtime_error("Invalid SBlk header for " + std::string(bank.name));
        const auto headerSize = U32(iso, offset + 0x10);
        const auto sampleDataSize = U32(iso, offset + 0x14);
        const auto size = static_cast<std::size_t>(headerSize) + sampleDataSize;
        if (size < 0x20 || size > iso.size() - offset)
            throw std::runtime_error("Invalid sound bank size for " + std::string(bank.name));
        const auto path = soundBankDirectory / bank.name;
        std::ofstream output(path, std::ios::binary);
        if (!output || !output.write(reinterpret_cast<const char*>(iso.data() + offset),
                static_cast<std::streamsize>(size)))
            throw std::runtime_error("Could not write " + path.string());
        std::cout << "Sound bank: " << path.string() << " (" << size << " bytes)\n";
    }
}

struct PssStreams {
    std::vector<std::byte> video;
    struct Audio {
        std::vector<std::int16_t> left;
        std::vector<std::int16_t> right;
        std::uint32_t sampleRate{};
        std::uint16_t channels{};
        std::uint8_t streamId{};
    };
    std::vector<Audio> audio;
};

std::size_t FindBytes(std::span<const std::byte> data, std::string_view pattern,
    std::size_t start = 0) {
    for (auto at = start; at + pattern.size() <= data.size(); ++at) {
        bool match = true;
        for (std::size_t i = 0; i < pattern.size(); ++i)
            match = match && data[at + i] == static_cast<std::byte>(pattern[i]);
        if (match) return at;
    }
    return std::string_view::npos;
}

PssStreams DemuxPss(std::span<const std::byte> programStream) {
    constexpr std::uint32_t end = 0x000001B9, pack = 0x000001BA, system = 0x000001BB;
    constexpr std::uint32_t privateStream = 0x000001BD, padding = 0x000001BE, videoStream = 0x000001E0;
    PssStreams streams;
    streams.video.reserve(programStream.size());
    std::array<std::vector<std::byte>, 256> sonyAds;
    std::size_t at = 0;
    while (at + 4 <= programStream.size()) {
        const auto id = Be32(programStream, at);
        if (id == end) break;
        if (id == pack) {
            if (at + 14 > programStream.size()) throw std::runtime_error("Truncated PSS pack header");
            at += 14 + (std::to_integer<unsigned>(programStream[at + 13]) & 7u);
            continue;
        }
        if (id == system || id == padding) {
            if (at + 6 > programStream.size()) throw std::runtime_error("Truncated PSS packet");
            at += 6 + Be16(programStream, at + 4);
            continue;
        }
        if (id != videoStream && id != privateStream) { ++at; continue; }
        if (at + 9 > programStream.size()) throw std::runtime_error("Truncated PSS PES packet");
        const auto packetSize = Be16(programStream, at + 4);
        const auto packetEnd = at + 6 + packetSize;
        if (packetEnd > programStream.size()) throw std::runtime_error("Invalid PSS packet size");
        const auto extension = std::to_integer<unsigned>(programStream[at + 8]);
        const auto payload = at + 9 + extension;
        if (packetSize < 3 + extension || payload > packetEnd)
            throw std::runtime_error("Invalid PSS payload");
        const auto payloadSize = static_cast<std::size_t>(packetSize - 3 - extension);
        if (id == videoStream) {
            streams.video.insert(streams.video.end(), programStream.begin() + payload,
                programStream.begin() + payload + payloadSize);
        } else if (payloadSize > 4) {
            const auto audioStreamId = std::to_integer<std::uint8_t>(programStream[payload + 3]);
            sonyAds[audioStreamId].insert(sonyAds[audioStreamId].end(), programStream.begin() + payload + 4,
                programStream.begin() + payload + payloadSize);
        }
        at = packetEnd;
    }

    for (std::size_t streamId = 0; streamId < sonyAds.size(); ++streamId) {
        const auto& ads = sonyAds[streamId];
        if (ads.empty()) continue;
        const auto header = FindBytes(ads, "SShd");
        const auto body = header == std::string_view::npos ? header : FindBytes(ads, "SSbd", header + 8);
        if (header == std::string_view::npos || body == std::string_view::npos ||
            header + 32 > ads.size() || body + 8 > ads.size())
            throw std::runtime_error("PSS audio headers were not found for stream " +
                std::to_string(streamId));
        const auto format = U32(ads, header + 8);
        PssStreams::Audio audio;
        audio.streamId = static_cast<std::uint8_t>(streamId);
        audio.sampleRate = U32(ads, header + 12);
        const auto channelCount = U32(ads, header + 16);
        if (channelCount > 255) throw std::runtime_error("Invalid PSS channel count");
        audio.channels = static_cast<std::uint16_t>(channelCount);
        const auto interleave = U32(ads, header + 20);
        const auto size = std::min<std::size_t>(U32(ads, body + 4), ads.size() - body - 8);
        std::vector<std::byte> encoded(ads.begin() + body + 8, ads.begin() + body + 8 + size);
        if (encoded.empty() || audio.channels < 1 || audio.channels > 2 || audio.sampleRate == 0)
            throw std::runtime_error("PSS audio stream " + std::to_string(streamId) + " is incomplete");
        std::array<std::vector<std::byte>, 2> channelBytes;
        if (audio.channels == 1) channelBytes[0] = std::move(encoded);
        else {
        const auto block = static_cast<std::size_t>(interleave == 0 ? 0x800 : interleave);
        std::size_t position = 0, channel = 0;
        while (position < encoded.size()) {
            const auto count = std::min(block, encoded.size() - position);
            channelBytes[channel].insert(channelBytes[channel].end(), encoded.begin() + position,
                encoded.begin() + position + count);
            position += count;
            channel ^= 1;
        }
        }
        if (format == 2 || format == 0x10) {
            audio.left = DecodeAdpcm(channelBytes[0]);
            if (audio.channels == 2) audio.right = DecodeAdpcm(channelBytes[1]);
        } else if (format == 0 || format == 1) {
            audio.left = DecodePcm(channelBytes[0], format == 1);
            if (audio.channels == 2) audio.right = DecodePcm(channelBytes[1], format == 1);
        } else {
            throw std::runtime_error("Unsupported PSS audio format " + std::to_string(format));
        }
        streams.audio.push_back(std::move(audio));
    }
    if (streams.video.empty() || streams.audio.empty())
        throw std::runtime_error("The PSS streams are incomplete");
    return streams;
}

void ExtractCutscenes(std::span<const std::byte> iso, const IsoFile& elf,
    const std::filesystem::path& cutsceneDirectory, std::uint32_t tableVa,
    std::span<const std::string_view> languageNames, std::string_view regionName,
    std::size_t precedingSpecialDescriptorCount) {
    const auto table = VaToIsoOffset(iso, elf, tableVa);
    if (table < precedingSpecialDescriptorCount * kDescriptorSize ||
        table + kCutsceneCount * kDescriptorSize > iso.size())
        throw std::runtime_error(std::string(regionName) +
            " cutscene table lies outside the executable");
    std::filesystem::create_directories(cutsceneDirectory);
    const auto descriptorCount = kCutsceneCount + precedingSpecialDescriptorCount;
    for (std::size_t index = 0; index < descriptorCount; ++index) {
        // Number the ordinary table as 0..15, followed by special descriptors
        // working backward from the table. PAL has one such entry. SCPS has
        // identity 0x12 at table-0x20 (movie 16) and its play-once startup
        // movie identity 0x08 at table-0x40 (movie 17).
        const auto descriptor = index < kCutsceneCount
            ? table + index * kDescriptorSize
            : table - (index - kCutsceneCount + 1) * kDescriptorSize;
        const auto sector = U32(iso, descriptor) ^ U32(iso, descriptor + 20);
        const auto size = U32(iso, descriptor + 4) ^ U32(iso, descriptor + 28);
        const auto identity = U32(iso, descriptor + 8) ^ U32(iso, descriptor + 24);
        const auto offset = static_cast<std::size_t>(sector) * kSectorSize;
        if (size == 0 || offset > iso.size() || size > iso.size() - offset)
            throw std::runtime_error("Invalid " + std::string(regionName) +
                " cutscene descriptor " + std::to_string(index));
        const auto streams = DemuxPss(iso.subspan(offset, size));
        std::ostringstream baseName;
        baseName << "Cutscene_" << std::setfill('0') << std::setw(2) << index;
        const auto videoPath = cutsceneDirectory / (baseName.str() + ".m2v");
        std::ofstream output(videoPath, std::ios::binary);
        if (!output || !output.write(reinterpret_cast<const char*>(streams.video.data()),
                static_cast<std::streamsize>(streams.video.size())))
            throw std::runtime_error("Could not write " + videoPath.string());
        output.close();
        for (const auto& audio : streams.audio) {
            const auto languageIndex = audio.streamId == 0 ? languageNames.size() :
                static_cast<std::size_t>(audio.streamId - 1);
            const auto audioDirectory = languageIndex < languageNames.size() ?
                cutsceneDirectory / languageNames[languageIndex] : cutsceneDirectory;
            std::filesystem::create_directories(audioDirectory);
            const auto audioPath = audioDirectory / (baseName.str() + ".wav");
            WriteWave(audioPath, audio.left, audio.right, audio.channels, audio.sampleRate);
        }
        std::cout << "Cutscene " << index << ": identity=0x" << std::hex << identity
                  << std::dec << ' ' << videoPath.string() << " (" << streams.video.size()
                  << " bytes), " << streams.audio.size() << " language audio stream(s)\n";
    }
}

void ExtractPalCutscenes(std::span<const std::byte> iso, const IsoFile& elf,
    const std::filesystem::path& cutsceneDirectory) {
    static constexpr std::array<std::string_view, 5> languages = {
        "English", "French", "German", "Spanish", "Italian"
    };
    ExtractCutscenes(iso, elf, cutsceneDirectory, kPalCutsceneTableVa,
        languages, "PAL", 1);
}

void ExtractJapaneseCutscenes(std::span<const std::byte> iso, const IsoFile& elf,
    const std::filesystem::path& cutsceneDirectory) {
    static constexpr std::array<std::string_view, 2> languages = {
        "Japanese", "English"
    };
    ExtractCutscenes(iso, elf, cutsceneDirectory, kJapaneseCutsceneTableVa,
        languages, "Japanese", 2);
}

constexpr std::array<std::string_view, kLevelCount> kLevelNames = {
    "Splash", "Paris", "The Hideout",
    "A Stealthy Approach", "Prowling the Grounds", "High Class Heist",
    "Into the Machine", "A Cunning Disguise", "The Fire Down Below",
    "Treasure in the Depths", "The Gunboat Graveyard", "The Eye of the Storm",
    "A Rocky Start", "Muggshot's Turf", "Boneyard Casino", "Murray's Big Gamble",
    "At the Dog Track", "Two to Tango", "Straight to the Top", "Back Alley Heist",
    "Last Call", "The Dread Swamp Path", "The Swamp's Dark Center",
    "The Lair of the Beast", "A Grave Undertaking", "Piranha Lake",
    "Descent into Danger", "A Ghastly Voyage", "Down Home Cooking", "A Deadly Dance",
    "A Perilous Ascent", "Inside the Stronghold", "Flaming Temple of Flame",
    "The Unseen Foe", "The King of the Hill", "Rapid Fire Assault",
    "Duel by the Dragon", "A Desperate Race", "Flame Fu!", "A Hazardous Path",
    "Burning Rubber", "A Daring Rescue", "Bentley Comes Through",
    "A Temporary Truce", "Sinking Peril", "A Strange Reunion"
};

constexpr std::array<std::string_view, kLanguageCount> kLanguageNames = {
    "English", "French", "German", "Spanish", "Italian"
};

std::filesystem::path LevelDirectory(const std::filesystem::path& worldsDirectory,
    std::size_t levelIndex) {
    const auto levelName = std::string(kLevelNames[levelIndex]);
    if (levelIndex <= 2) return worldsDirectory / levelName;
    if (levelIndex <= 11) return worldsDirectory / "Tide of Terror" / levelName;
    if (levelIndex <= 20) return worldsDirectory / "Sunset Snake Eyes" / levelName;
    if (levelIndex <= 29) return worldsDirectory / "Vicious Voodoo" / levelName;
    if (levelIndex <= 38) return worldsDirectory / "Fire in the sky" / levelName;
    return worldsDirectory / "The Cold Heart of Hate" / levelName;
}

void WritePalLevelIdentityMap(std::span<const std::byte> iso, std::size_t table,
    const std::filesystem::path& outputDirectory) {
    std::filesystem::create_directories(outputDirectory);
    const auto path = outputDirectory / ".pal-level-identities.txt";
    std::ofstream output(path, std::ios::trunc);
    if (!output) throw std::runtime_error("Could not write " + path.string());
    std::unordered_set<std::uint32_t> identities;
    for (std::size_t level = 0; level < kLevelCount; ++level) {
        const auto record = table + level * kRecordSize;
        const std::string filename = std::string(kLevelNames[level]) + ".brx";
        for (std::size_t slot = 0; slot < kLanguageCount; ++slot) {
            const auto descriptor = record + slot * kDescriptorSize;
            const auto identity = U32(iso, descriptor + 8) ^ U32(iso, descriptor + 24);
            if (!identities.insert(identity).second)
                throw std::runtime_error("Duplicate PAL level descriptor identity " +
                    std::to_string(identity));
            output << identity << ' ' << std::quoted(filename) << '\n';
        }
    }
    if (!output) throw std::runtime_error("Could not finish " + path.string());
}

void ExtractSlot(std::span<const std::byte> iso, const IsoFile& elf, std::size_t table,
    const std::filesystem::path& outputDirectory, std::size_t slot) {
    if (slot >= kLanguageCount) throw std::runtime_error("PAL language slot must be 0 through 4");
    const auto worldsDirectory = outputDirectory / "Worlds";
    for (std::size_t i = 0; i < kLevelCount; ++i) {
        const auto record = table + i * kRecordSize;
        const auto descriptor = record + slot * kDescriptorSize;
        const auto sector = U32(iso, descriptor) ^ U32(iso, descriptor + 20);
        const auto compressedSize = U32(iso, descriptor + 4) ^ U32(iso, descriptor + 28);
        const auto levelId = U32(iso, record + 160);
        const auto searchKey = U32(iso, record + 164);
        const auto task = U32(iso, record + 168);
        const auto source = static_cast<std::size_t>(sector) * kSectorSize;
        if (compressedSize == 0 || compressedSize > kMaximumCompressedSize ||
            source > iso.size() || compressedSize > iso.size() - source)
            throw std::runtime_error("Invalid PAL descriptor at table index " + std::to_string(i) +
                ", language " + std::string(kLanguageNames[slot]));
        const auto decoded = Decompress(iso.subspan(source, compressedSize));
        const auto levelLanguageDirectory = LevelDirectory(worldsDirectory, i) /
            std::string(kLanguageNames[slot]);
        std::filesystem::create_directories(levelLanguageDirectory);
        const auto path = levelLanguageDirectory / (std::string(kLevelNames[i]) + ".brx");
        std::ofstream output(path, std::ios::binary);
        if (!output || !output.write(reinterpret_cast<const char*>(decoded.data()), static_cast<std::streamsize>(decoded.size())))
            throw std::runtime_error("Could not write " + path.string());
        output.close();
        BrxWalker walker(StringEncoding::Auto, BrxRegion::Pal);
        walker.WalkFile(path);
        const auto dialogDirectory = levelLanguageDirectory / "Dialogs";
        ExtractAudio(iso, dialogDirectory, walker.AudioDescriptors());
        const auto commentaryDirectory = levelLanguageDirectory / "Commentary";
        ExtractAudio(iso, commentaryDirectory, walker.CommentaryDescriptors());
        ExtractMusic(iso, worldsDirectory / "Music", walker.MusicDescriptors());
        std::cout << '[' << kLanguageNames[slot] << ' ' << (i + 1) << '/' << kLevelCount
                  << "] id=0x" << std::hex << levelId << " search=0x" << searchKey
                  << " task=0x" << task << std::dec << ' ' << path.string() << '\n';
    }
}

void ExtractJapaneseSlot(std::span<const std::byte> iso, std::size_t table,
    const std::filesystem::path& outputDirectory, std::size_t slot) {
    if (slot >= kJapaneseSlotCount)
        throw std::runtime_error("Japanese descriptor slot must be 0 or 1");

    static constexpr std::array<std::string_view, 2> slotNames = {
        "Japanese", "English"
    };
    const auto slotName = slotNames[slot];
    const auto worldsDirectory = outputDirectory / "Worlds";
    for (std::size_t i = 0; i < kLevelCount; ++i) {
        const auto record = table + i * kJapaneseRecordSize;
        const auto descriptor = record + slot * kDescriptorSize;
        const auto sector = U32(iso, descriptor) ^ U32(iso, descriptor + 20);
        const auto compressedSize = U32(iso, descriptor + 4) ^ U32(iso, descriptor + 28);
        const auto identity = U32(iso, descriptor + 8) ^ U32(iso, descriptor + 24);
        const auto levelId = U32(iso, record + 64);
        const auto searchKey = U32(iso, record + 68);
        const auto flags = U32(iso, record + 72);
        const auto source = static_cast<std::size_t>(sector) * kSectorSize;
        if (compressedSize == 0 || compressedSize > kMaximumCompressedSize ||
            source > iso.size() || compressedSize > iso.size() - source) {
            throw std::runtime_error("Invalid Japanese descriptor at table index " +
                std::to_string(i) + ", slot " + std::to_string(slot));
        }

        const auto decoded = Decompress(iso.subspan(source, compressedSize));
        const auto levelDirectory = LevelDirectory(worldsDirectory, i) / slotName;
        std::filesystem::create_directories(levelDirectory);
        const auto path = levelDirectory / (std::string(kLevelNames[i]) + ".brx");
        std::ofstream output(path, std::ios::binary);
        if (!output || !output.write(reinterpret_cast<const char*>(decoded.data()),
            static_cast<std::streamsize>(decoded.size()))) {
            throw std::runtime_error("Could not write " + path.string());
        }
        output.close();

        BrxWalker walker(StringEncoding::Auto, BrxRegion::Japanese);
        walker.WalkFile(path);
        ExtractAudio(iso, levelDirectory / "Dialogs", walker.AudioDescriptors());
        ExtractAudio(iso, levelDirectory / "Commentary", walker.CommentaryDescriptors());
        ExtractMusic(iso, worldsDirectory / "Music", walker.MusicDescriptors());
        std::cout << '[' << slotName << ' ' << (i + 1) << '/' << kLevelCount
                  << "] id=0x" << std::hex << levelId << " search=0x" << searchKey
                  << " flags=0x" << flags << " file=0x" << identity << std::dec
                  << ' ' << path.string() << '\n';
    }
}
} // namespace

void PalIsoExtractor::Extract(const std::filesystem::path& isoPath,
    const std::filesystem::path& outputDirectory, PalLanguageSlot language) const {
    const auto iso = ReadFile(isoPath);
    const auto rootFiles = RootFiles(iso);
    const auto& elf = FindGameElf(rootFiles);
    if (IsJapaneseElf(elf))
        throw std::runtime_error("A PAL language slot cannot be selected for a Japanese SCPS ISO");
    const auto table = VaToIsoOffset(iso, elf, kPalLevelTableVa);
    const auto slot = static_cast<std::size_t>(language);
    if (table + kLevelCount * kRecordSize > iso.size()) throw std::runtime_error("PAL level table lies outside the executable");
    WritePalLevelIdentityMap(iso, table, outputDirectory);
    ExtractSoundBanks(iso, outputDirectory / "Worlds" / "Sound Banks", kPalSoundBanks, "PAL");
    ExtractPalCutscenes(iso, elf, outputDirectory / "Cutscenes");
    ExtractSlot(iso, elf, table, outputDirectory, slot);
}

void PalIsoExtractor::ExtractAllLanguages(const std::filesystem::path& isoPath,
    const std::filesystem::path& outputDirectory) const {
    const auto iso = ReadFile(isoPath);
    const auto rootFiles = RootFiles(iso);
    const auto& elf = FindGameElf(rootFiles);
    if (IsJapaneseElf(elf))
        throw std::runtime_error("Use automatic extraction for a Japanese SCPS ISO");
    const auto table = VaToIsoOffset(iso, elf, kPalLevelTableVa);
    if (table + kLevelCount * kRecordSize > iso.size()) throw std::runtime_error("PAL level table lies outside the executable");
    WritePalLevelIdentityMap(iso, table, outputDirectory);
    ExtractSoundBanks(iso, outputDirectory / "Worlds" / "Sound Banks", kPalSoundBanks, "PAL");
    ExtractPalCutscenes(iso, elf, outputDirectory / "Cutscenes");
    for (std::size_t slot = 0; slot < kLanguageCount; ++slot)
        ExtractSlot(iso, elf, table, outputDirectory, slot);
}

void PalIsoExtractor::ExtractDetectedRegion(const std::filesystem::path& isoPath,
    const std::filesystem::path& outputDirectory) const {
    const auto iso = ReadFile(isoPath);
    const auto rootFiles = RootFiles(iso);
    const auto& elf = FindGameElf(rootFiles);

    if (!IsJapaneseElf(elf)) {
        ExtractAllLanguages(isoPath, outputDirectory);
        return;
    }

    const auto table = VaToIsoOffset(iso, elf, kJapaneseLevelTableVa);
    if (table + kLevelCount * kJapaneseRecordSize > iso.size())
        throw std::runtime_error("Japanese level table lies outside the executable");

    std::cout << "Detected Japanese SCPS release; extracting both voice slots.\n";
    ExtractSoundBanks(iso, outputDirectory / "Worlds" / "Sound Banks",
        kJapaneseSoundBanks, "Japanese");
    ExtractJapaneseCutscenes(iso, elf, outputDirectory / "Cutscenes");
    for (std::size_t slot = 0; slot < kJapaneseSlotCount; ++slot)
        ExtractJapaneseSlot(iso, table, outputDirectory, slot);
}
