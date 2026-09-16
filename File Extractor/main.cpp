#include "main.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <array>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

constexpr uint64_t SCUS_ISO_OFFSET = 0x0008A000;

constexpr uint32_t LEVEL_TABLE_VADDR = 0x00247AF0;
constexpr uint32_t SOUND_BANK_TABLE_VADDR = 0x00244F40;
constexpr uint32_t COMMENTARY_TABLE_VADDR = 0x00248748;
constexpr uint32_t JLO_AMBIENT_TABLE_VADDR = 0x002482D8;
constexpr uint32_t CUTSCENE_TABLE_VADDR = 0x00248490;
constexpr uint32_t PROMPT_VAG_INFO_VADDR = 0x0026A118;
constexpr uint32_t ELF_SEGMENT_VADDR = 0x00100000;
constexpr uint32_t ELF_SEGMENT_OFFSET = 0x00001000;

constexpr uint32_t LEVEL_TABLE_EXE_OFFSET = LEVEL_TABLE_VADDR - ELF_SEGMENT_VADDR + ELF_SEGMENT_OFFSET;
constexpr uint64_t LEVEL_TABLE_ISO_OFFSET = SCUS_ISO_OFFSET + LEVEL_TABLE_EXE_OFFSET;
constexpr uint32_t SOUND_BANK_TABLE_EXE_OFFSET = SOUND_BANK_TABLE_VADDR - ELF_SEGMENT_VADDR + ELF_SEGMENT_OFFSET;
constexpr uint64_t SOUND_BANK_TABLE_ISO_OFFSET = SCUS_ISO_OFFSET + SOUND_BANK_TABLE_EXE_OFFSET;
constexpr uint32_t COMMENTARY_TABLE_EXE_OFFSET = COMMENTARY_TABLE_VADDR - ELF_SEGMENT_VADDR + ELF_SEGMENT_OFFSET;
constexpr uint64_t COMMENTARY_TABLE_ISO_OFFSET = SCUS_ISO_OFFSET + COMMENTARY_TABLE_EXE_OFFSET;
constexpr uint32_t JLO_AMBIENT_TABLE_EXE_OFFSET = JLO_AMBIENT_TABLE_VADDR - ELF_SEGMENT_VADDR + ELF_SEGMENT_OFFSET;
constexpr uint64_t JLO_AMBIENT_TABLE_ISO_OFFSET = SCUS_ISO_OFFSET + JLO_AMBIENT_TABLE_EXE_OFFSET;
constexpr uint32_t CUTSCENE_TABLE_EXE_OFFSET = CUTSCENE_TABLE_VADDR - ELF_SEGMENT_VADDR + ELF_SEGMENT_OFFSET;
constexpr uint64_t CUTSCENE_TABLE_ISO_OFFSET = SCUS_ISO_OFFSET + CUTSCENE_TABLE_EXE_OFFSET;
constexpr uint32_t PROMPT_VAG_INFO_EXE_OFFSET = PROMPT_VAG_INFO_VADDR - ELF_SEGMENT_VADDR + ELF_SEGMENT_OFFSET;
constexpr uint64_t PROMPT_VAG_INFO_ISO_OFFSET = SCUS_ISO_OFFSET + PROMPT_VAG_INFO_EXE_OFFSET;

constexpr std::size_t LEVEL_COUNT = 46;
constexpr std::size_t SOUND_BANK_COUNT = 7;
constexpr std::size_t COMMENTARY_COUNT = 33;
constexpr std::size_t JLO_AMBIENT_COUNT = 6;
constexpr std::size_t CUTSCENE_COUNT = 17;
constexpr std::size_t EXTRACTED_CUTSCENE_COUNT = CUTSCENE_COUNT + 1;
constexpr uint32_t STARTUP_SPLASH_SECTOR = 0x000C8F03;
constexpr uint32_t STARTUP_SPLASH_SIZE = 0x00AB4004;
constexpr uint32_t STARTUP_SPLASH_IDENTITY = 0x00000344;
constexpr uint64_t ISO_SECTOR_SIZE = 0x800;

#pragma pack(push, 1)

struct FCLDisk
{
    uint32_t isector;
    uint32_t cb;
};

struct CFileLocationDisk
{
    FCLDisk fcl;
};

struct LEVELINFODisk
{
    CFileLocationDisk fileInfo;
    uint32_t searchValue;
    uint32_t field2_0x0c;
    uint32_t field3_0x10;
    uint32_t lsnKey;
    uint32_t searchCipher;
    uint32_t sizeKey;
    uint32_t levelId;
    uint32_t levelName;
    uint32_t task;
};

struct VAGINFODisk
{
    uint32_t encryptedSector;
    uint32_t encryptedSize;
    uint32_t encryptedId;
    uint32_t field0C;
    uint32_t field10;
    uint32_t sectorKey;
    uint32_t idKey;
    uint32_t sizeKey;
};

struct COMMENTARYINFODisk
{
    uint32_t levelId;
    float audioDuration;
    VAGINFODisk audio;
    uint32_t speakerNames;
};

struct MUSICINFODisk
{
    const char* levelName;
    uint32_t musicId;
    uint32_t sector;
    uint32_t size;
};

enum class DialogAudioSource
{
    DIALOG,
    ASEG,
};

struct DIALOGAUDIOINFO
{
    uint32_t levelId;
    uint32_t sector;
    uint32_t size;
    uint32_t identity;
    DialogAudioSource source;
};

#pragma pack(pop)

static_assert(sizeof(FCLDisk) == 8);
static_assert(sizeof(CFileLocationDisk) == 8);
static_assert(sizeof(LEVELINFODisk) == 44);
static_assert(sizeof(VAGINFODisk) == 32);
static_assert(sizeof(COMMENTARYINFODisk) == 44);

constexpr MUSICINFODisk MUSIC_TABLE[] =
{
    { "Paris, France",             0x55, 0xC8E0E, 0x7A6A0 },
    { "The Hideout",              0x43, 0xD3411, 0x6F7A0 },
    { "A Cunning Disguise",       0x47, 0xC4602, 0x72324 },
    { "A Stealthy Approach",      0x51, 0xD44ED, 0x745F8 },
    { "High Class Heist",         0x59, 0xCDE0B, 0x74674 },
    { "Into the Machine",         0x3F, 0xC839D, 0x7B414 },
    { "Prowling the Grounds",     0x44, 0xBFA16, 0x746D4 },
    { "The Eye of the Storm",     0x57, 0xC354B, 0x7A0C4 },
    { "The Fire Down Below",      0x46, 0xC130C, 0x775E0 },
    { "The Gunboat Graveyard",    0x42, 0xCAC3D, 0x70D4C },
    { "Treasure in the Depths",   0x3E, 0xD4E9D, 0x748F8 },
    { "A Rocky Start",            0x50, 0xBC6B4, 0x64A34 },
    { "At the Dog Track",         0x4E, 0xCFBE5, 0x726F8 },
    { "Back Alley Heist",         0x4C, 0xC1B99, 0x77978 },
    { "Boneyard Casino",          0x49, 0xCF341, 0x71A18 },
    { "Last Call",                0x48, 0xC02DE, 0x731B8 },
    { "Muggshot's Turf",          0x40, 0xCE9BB, 0x5DB18 },
    { "Murray's Big Gamble",      0x4A, 0xCD592, 0x767B8 },
    { "Straight to the Top",      0x4C, 0xD04EF, 0x77978 },
    { "Two to Tango",             0x4D, 0xD5A8B, 0x788FC },
    { "A Deadly Dance",           0x68, 0xBE35F, 0x771E8 },
    { "A Ghastly Voyage",         0x65, 0xC6338, 0x76D9C },
    { "A Grave Undertaking",      0x55, 0xCCD68, 0x7A6A0 },
    { "Descent into Danger",      0x64, 0xC25C6, 0x6EBB8 },
    { "Down Home Cooking",        0x62, 0xC50BB, 0x73978 },
    { "Piranha Lake",             0x67, 0xD4AFA, 0x73978 },
    { "The Dread Swamp Path",     0x60, 0xC6D86, 0x76A60 },
    { "The Lair of the Beast",    0x64, 0xCB6A5, 0x6EBB8 },
    { "The Swamp's Dark Center",  0x64, 0xD1852, 0x6EBB8 },
    { "A Desperate Race",         0x4E, 0xCBD9B, 0x726F8 },
    { "A Perilous Ascent",        0x53, 0xD2424, 0x773CC },
    { "Duel by the Dragon",       0x5B, 0xBD2A0, 0x6DA90 },
    { "Flame Fu!",                0x56, 0xC4B48, 0x6EA6C },
    { "Flaming Temple of Flame",  0x54, 0xBEF0F, 0x7737C },
    { "Inside the Stronghold",    0x5B, 0xC7870, 0x6DA90 },
    { "Rapid Fire Assault",       0x5D, 0xC2F89, 0x6C2BC },
    { "The King of the Hill",     0x5C, 0xD2D42, 0x78064 },
    { "The Unseen Foe",           0x44, 0xC5A33, 0x746D4 },
    { "A Daring Rescue",          0x38, 0xBD8B1, 0x788F8 },
    { "A Hazardous Path",         0x35, 0xC3CB5, 0x77C7C },
    { "A Strange Reunion",        0x34, 0xC0AAC, 0x95774 },
    { "A Temporary Truce",        0x39, 0xD0BD7, 0x74DAC },
    { "Bentley Comes Through",    0x36, 0xD50F1, 0x8C528 },
    { "Burning Rubber",           0x3A, 0xCC1CA, 0x77614 },
    { "Sinking Peril",            0x37, 0xD3A9E, 0x77F08 },
};

constexpr std::array<const char*, SOUND_BANK_COUNT> SOUND_BANK_NAMES =
{
    "Common",
    "Intro",
    "Snow",
    "Underwater",
    "Muggshot",
    "Voodoo",
    "Clockwerk",
};

constexpr DIALOGAUDIOINFO DIALOG_AUDIO_TABLE[] =
{
#include "DialogAudioTable.inc"
};

static void WaitForExit()
{
    std::cout << "\nPress Enter to close...";
    std::cin.clear();
    std::cin.get();
}

static int Finish(int result)
{
    WaitForExit();
    return result;
}

static fs::path GetExecutableDirectory()
{
    std::wstring path(32768, L'\0');

    const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));

    if (length == 0 || length >= path.size())
        return fs::current_path();

    path.resize(length);

    return fs::path(path).parent_path();
}

static bool ReadExactAt(std::ifstream& file, uint64_t offset, void* destination, std::size_t size)
{
    file.clear();
    file.seekg(static_cast<std::streamoff>(offset), std::ios::beg);

    if (!file)
        return false;

    if (size == 0)
        return true;

    file.read(static_cast<char*>(destination), static_cast<std::streamsize>(size));

    return file.gcount() == static_cast<std::streamsize>(size);
}

static uint16_t ReadBe16(const std::vector<uint8_t>& data, std::size_t offset)
{
    return static_cast<uint16_t>((data[offset] << 8) | data[offset + 1]);
}

static uint32_t ReadBe32(const std::vector<uint8_t>& data, std::size_t offset)
{
    return (static_cast<uint32_t>(data[offset]) << 24) |
           (static_cast<uint32_t>(data[offset + 1]) << 16) |
           (static_cast<uint32_t>(data[offset + 2]) << 8) |
           static_cast<uint32_t>(data[offset + 3]);
}

static uint32_t ReadLe32(const std::vector<uint8_t>& data, std::size_t offset)
{
    return static_cast<uint32_t>(data[offset]) |
           (static_cast<uint32_t>(data[offset + 1]) << 8) |
           (static_cast<uint32_t>(data[offset + 2]) << 16) |
           (static_cast<uint32_t>(data[offset + 3]) << 24);
}

struct DemuxedPss
{
    std::vector<uint8_t> video;
    std::vector<uint8_t> adpcm;
    uint32_t audioFormat = 0;
    uint32_t sampleRate = 0;
    uint8_t channels = 0;
    uint32_t interleave = 0x800;
    std::string error;
};

static bool AppendBytes(const std::vector<uint8_t>& source, std::size_t offset,
                        std::size_t size, std::vector<uint8_t>& destination)
{
    if (offset > source.size() || size > source.size() - offset)
        return false;

    destination.insert(destination.end(), source.begin() + offset, source.begin() + offset + size);
    return true;
}

static bool DemuxPss(const std::vector<uint8_t>& pss, DemuxedPss& result)
{
    constexpr uint32_t MPEG_PROGRAM_END = 0x000001B9;
    constexpr uint32_t MPEG_PACK_HEADER = 0x000001BA;
    constexpr uint32_t MPEG_SYSTEM_HEADER = 0x000001BB;
    constexpr uint32_t MPEG_PRIVATE_STREAM_1 = 0x000001BD;
    constexpr uint32_t MPEG_PADDING = 0x000001BE;
    constexpr uint32_t MPEG_VIDEO = 0x000001E0;
    constexpr uint8_t PRIVATE_VAG = 0x01;

    bool firstAudioPacket = true;
    std::vector<uint8_t> sonyAds;
    std::size_t offset = 0;

    auto Fail = [&](const char* reason)
    {
        result.error = reason;
        result.error += " at PSS offset 0x";
        char address[32]{};
        std::snprintf(address, sizeof(address), "%llX",
                      static_cast<unsigned long long>(offset));
        result.error += address;
        return false;
    };

    while (offset + 4 <= pss.size())
    {
        const uint32_t streamId = ReadBe32(pss, offset);

        if (streamId == MPEG_PROGRAM_END)
            break;

        if (streamId == MPEG_PACK_HEADER)
        {
            if (offset + 14 > pss.size())
                return Fail("truncated pack header");
            const std::size_t packSize = 14 + (pss[offset + 13] & 7U);
            if (packSize > pss.size() - offset)
                return Fail("invalid pack-header stuffing");
            offset += packSize;
            continue;
        }

        if (streamId == MPEG_SYSTEM_HEADER || streamId == MPEG_PADDING)
        {
            if (offset + 6 > pss.size())
                return Fail("truncated system/padding packet");
            const std::size_t packetSize = ReadBe16(pss, offset + 4);
            if (packetSize > pss.size() - offset - 6)
                return Fail("invalid system/padding packet size");
            offset += 6 + packetSize;
            continue;
        }

        if (streamId != MPEG_VIDEO && streamId != MPEG_PRIVATE_STREAM_1)
        {
            // Disc sectors can contain zero fill before the first pack and after
            // MPEG_PROGRAM_END. Resynchronize on the next MPEG start-code prefix.
            ++offset;
            continue;
        }

        if (offset + 9 > pss.size())
            return Fail("truncated PES packet");

        const std::size_t packetSize = ReadBe16(pss, offset + 4);
        if (packetSize > pss.size() - offset - 6)
            return Fail("invalid PES packet size");

        const std::size_t extensionSize = pss[offset + 8];
        if (packetSize < 3 + extensionSize)
            return Fail("invalid PES extension size");

        const std::size_t extensionEnd = offset + 9 + extensionSize;

        if (streamId == MPEG_VIDEO)
        {
            if (!AppendBytes(pss, extensionEnd, packetSize - 3 - extensionSize, result.video))
                return Fail("truncated video payload");
        }
        else
        {
            if (extensionEnd + 4 > offset + 6 + packetSize)
                return Fail("truncated private-stream header");

            const uint8_t privateStreamId = pss[extensionEnd + 3];

            // Standard Sony PSS movies carry an ADS stream in private_stream_1.
            // Its four-byte packet tag is repeated at the start of every PES.
            const std::size_t privatePayloadSize = packetSize - 3 - extensionSize;
            if (privatePayloadSize > 4 &&
                !AppendBytes(pss, extensionEnd + 4, privatePayloadSize - 4, sonyAds))
                return Fail("truncated Sony ADS payload");

            if (privateStreamId == PRIVATE_VAG)
            {
                std::size_t audioOffset = extensionEnd + 4;
                std::size_t audioSize = packetSize - 3 - extensionSize - 4;

                if (firstAudioPacket)
                {
                    constexpr std::size_t PRIVATE_HEADER_SIZE = 16;
                    constexpr std::size_t FIRST_PACKET_ALIGNMENT = 0x800 - 0x10;

                    if (audioSize < PRIVATE_HEADER_SIZE + FIRST_PACKET_ALIGNMENT)
                        return Fail("truncated first VAG packet");

                    result.sampleRate = ReadBe16(pss, audioOffset + 6);
                    result.channels = pss[audioOffset + 8];
                    result.audioFormat = 2;
                    audioOffset += PRIVATE_HEADER_SIZE + FIRST_PACKET_ALIGNMENT;
                    audioSize -= PRIVATE_HEADER_SIZE + FIRST_PACKET_ALIGNMENT;
                    firstAudioPacket = false;
                }

                if (!AppendBytes(pss, audioOffset, audioSize, result.adpcm))
                    return Fail("truncated VAG payload");
            }
        }

        offset += 6 + packetSize;
    }

    // Sly uses Sony's ADS container rather than Zoe's stream-0x01 wrapper.
    // Locate its little-endian SShd/SSbd headers after joining the PES chunks.
    if (result.adpcm.empty() && sonyAds.size() >= 40)
    {
        const std::array<uint8_t, 4> sshd{ 'S', 'S', 'h', 'd' };
        const std::array<uint8_t, 4> ssbd{ 'S', 'S', 'b', 'd' };
        const auto headerIt = std::search(sonyAds.begin(), sonyAds.end(), sshd.begin(), sshd.end());

        if (headerIt != sonyAds.end())
        {
            const std::size_t headerOffset = static_cast<std::size_t>(headerIt - sonyAds.begin());
            if (headerOffset + 32 <= sonyAds.size())
            {
                result.audioFormat = ReadLe32(sonyAds, headerOffset + 8);
                result.sampleRate = ReadLe32(sonyAds, headerOffset + 12);
                const uint32_t channelCount = ReadLe32(sonyAds, headerOffset + 16);
                result.channels = channelCount <= 255 ? static_cast<uint8_t>(channelCount) : 0;
                result.interleave = ReadLe32(sonyAds, headerOffset + 20);

                const auto bodyIt = std::search(headerIt + 8, sonyAds.end(), ssbd.begin(), ssbd.end());
                if (bodyIt != sonyAds.end())
                {
                    const std::size_t bodyOffset = static_cast<std::size_t>(bodyIt - sonyAds.begin());
                    if (bodyOffset + 8 <= sonyAds.size())
                    {
                        const std::size_t declaredSize = ReadLe32(sonyAds, bodyOffset + 4);
                        const std::size_t availableSize = sonyAds.size() - bodyOffset - 8;
                        const std::size_t audioSize = (std::min)(declaredSize, availableSize);
                        AppendBytes(sonyAds, bodyOffset + 8, audioSize, result.adpcm);
                    }
                }
            }
        }
    }

    if (result.video.empty())
        result.error = "no MPEG-2 video stream was found";
    else if (result.adpcm.empty())
        result.error = "no Zoe VAG or Sony SShd/SSbd audio stream was found";
    else if (result.channels != 1 && result.channels != 2)
        result.error = "unsupported VAG channel count " + std::to_string(result.channels);
    else if (result.sampleRate == 0)
        result.error = "VAG sample rate is zero";
    else
        return true;

    return false;
}

static std::vector<int16_t> DecodeVagChannel(const std::vector<uint8_t>& data)
{
    static constexpr int filters[5][2] =
    {
        { 0, 0 }, { 60, 0 }, { 115, -52 }, { 98, -55 }, { 122, -60 }
    };

    std::vector<int16_t> pcm;
    int history1 = 0;
    int history2 = 0;

    for (std::size_t offset = 0; offset + 16 <= data.size(); offset += 16)
    {
        const uint8_t* frame = data.data() + offset;
        const int predictor = frame[0] >> 4;
        const int shift = frame[0] & 0x0F;

        if (predictor >= 5)
            continue;

        bool endMarker = frame[0] == 0 && frame[1] == 1;
        for (int i = 2; endMarker && i < 16; ++i)
            endMarker = frame[i] == 0x77;
        if (endMarker)
            break;

        for (int sampleIndex = 0; sampleIndex < 28; ++sampleIndex)
        {
            const uint8_t packed = frame[2 + sampleIndex / 2];
            int nibble = (sampleIndex & 1) != 0 ? packed >> 4 : packed & 0x0F;
            if (nibble >= 8)
                nibble -= 16;

            int sample = (nibble << 12) >> shift;
            sample += (history1 * filters[predictor][0] + history2 * filters[predictor][1] + 32) >> 6;
            sample = (std::max)(-32768, (std::min)(32767, sample));
            history2 = history1;
            history1 = sample;
            pcm.push_back(static_cast<int16_t>(sample));
        }
    }

    return pcm;
}

static void WriteLe16(std::vector<uint8_t>& data, uint16_t value)
{
    data.push_back(static_cast<uint8_t>(value));
    data.push_back(static_cast<uint8_t>(value >> 8));
}

static void WriteLe32(std::vector<uint8_t>& data, uint32_t value)
{
    data.push_back(static_cast<uint8_t>(value));
    data.push_back(static_cast<uint8_t>(value >> 8));
    data.push_back(static_cast<uint8_t>(value >> 16));
    data.push_back(static_cast<uint8_t>(value >> 24));
}

static bool WritePcm16Wav(const std::vector<int16_t>& left,
                          const std::vector<int16_t>& right,
                          uint16_t channels, uint32_t sampleRate,
                          std::vector<uint8_t>& wav)
{
    const std::size_t sampleFrames = channels == 2 ? (std::min)(left.size(), right.size()) : left.size();
    if (sampleFrames == 0 || (channels != 1 && channels != 2) || sampleRate == 0)
        return false;

    const uint32_t dataSize = static_cast<uint32_t>(sampleFrames * channels * sizeof(int16_t));
    wav.clear();
    wav.reserve(44 + dataSize);
    wav.insert(wav.end(), { 'R','I','F','F' });
    WriteLe32(wav, 36 + dataSize);
    wav.insert(wav.end(), { 'W','A','V','E','f','m','t',' ' });
    WriteLe32(wav, 16);
    WriteLe16(wav, 1);
    WriteLe16(wav, channels);
    WriteLe32(wav, sampleRate);
    WriteLe32(wav, sampleRate * channels * sizeof(int16_t));
    WriteLe16(wav, static_cast<uint16_t>(channels * sizeof(int16_t)));
    WriteLe16(wav, 16);
    wav.insert(wav.end(), { 'd','a','t','a' });
    WriteLe32(wav, dataSize);

    for (std::size_t i = 0; i < sampleFrames; ++i)
    {
        WriteLe16(wav, static_cast<uint16_t>(left[i]));
        if (channels == 2)
            WriteLe16(wav, static_cast<uint16_t>(right[i]));
    }

    return true;
}

struct Sony989Wave
{
    uint32_t streamOffset = 0;
    uint32_t streamSize = 0;
    uint32_t sampleRate = 0;
    uint16_t flags = 0;
    uint8_t centerNote = 0;
    uint8_t centerFine = 0;
    int8_t mapLow = 0;
    int8_t mapHigh = 0;
    int8_t pitchBendLow = 0;
    int8_t pitchBendHigh = 0;
    uint16_t program = 0;
    uint16_t tone = 0;
    int8_t priority = 0;
    int8_t volume = 0;
    int16_t pan = 0;
    uint16_t adsr1 = 0;
    uint16_t adsr2 = 0;
};

struct Sony989Program
{
    uint16_t index = 0;
    uint8_t toneCount = 0;
    int8_t volume = 0;
    int16_t pan = 0;
};

struct Sony989Cue
{
    int32_t type = 0;
    uint32_t midiId = 0;
    int16_t volume = 0;
    int8_t repeats = 0;
    int8_t volumeGroup = 0;
    int16_t pan = 0;
};

static bool WriteBinaryFile(const fs::path& path, const std::vector<uint8_t>& data);

static uint16_t ReadLe16(const std::vector<uint8_t>& data, std::size_t offset)
{
    if (offset + 2 > data.size())
        return 0;
    return static_cast<uint16_t>(data[offset]) |
        static_cast<uint16_t>(data[offset + 1] << 8);
}

static uint32_t Sony989SampleRate(uint8_t encodedCenterNote, uint8_t centerFine)
{
    // 989SND uses the SPU2 note representation. A high-bit center note is
    // stored as its two's-complement magnitude; 0xC4 therefore means note 60.
    const int centerNote = (encodedCenterNote & 0x80) != 0
        ? 0x100 - encodedCenterNote
        : encodedCenterNote;
    const double semitones = 60.0 - static_cast<double>(centerNote)
        + static_cast<double>(centerFine) / 128.0;
    const double rate = 48000.0 * std::pow(2.0, semitones / 12.0);
    return static_cast<uint32_t>((std::max)(4000.0, (std::min)(192000.0, std::round(rate))));
}

static bool IsSony989Terminator(const uint8_t* frame)
{
    bool allZero = true;
    for (int i = 0; i < 16; ++i)
        allZero = allZero && frame[i] == 0;
    if (allZero)
        return true;

    if (frame[0] != 0 || frame[1] != 7)
        return false;

    bool zeroTail = true;
    bool sevenTail = true;
    for (int i = 2; i < 8; ++i)
    {
        zeroTail = zeroTail && frame[i] == 0;
        sevenTail = sevenTail && frame[i] == 0x77;
    }
    return zeroTail || sevenTail;
}

static bool ParseSony989SbV2(const std::vector<uint8_t>& data,
                             std::vector<Sony989Wave>& waves,
                             uint32_t& audioDataOffset,
                             std::vector<Sony989Cue>& cues,
                             std::vector<Sony989Program>& programs,
                             uint32_t& sequenceOffset,
                             uint32_t& sequenceSize,
                             std::string& error)
{
    if (data.size() < 0x30 || ReadLe32(data, 0x00) != 1)
    {
        error = "not a version-1 989SND container";
        return false;
    }

    const uint32_t sectionCount = ReadLe32(data, 0x04);
    const uint32_t sblkOffset = ReadLe32(data, 0x08);
    audioDataOffset = ReadLe32(data, 0x10);
    const uint32_t audioDataSize = ReadLe32(data, 0x14);
    sequenceOffset = sectionCount >= 3 ? ReadLe32(data, 0x18) : 0;
    sequenceSize = sectionCount >= 3 ? ReadLe32(data, 0x1C) : 0;
    if (sectionCount < 2 || sectionCount > 3 || sblkOffset + 0x30 > data.size() ||
        std::memcmp(data.data() + sblkOffset, "SBv2", 4) != 0 ||
        ReadLe32(data, sblkOffset + 4) != 2)
    {
        error = "missing supported SBv2 bank";
        return false;
    }
    if (audioDataOffset >= data.size() ||
        static_cast<uint64_t>(audioDataOffset) + audioDataSize > data.size())
    {
        error = "989SND audio section is outside the file";
        return false;
    }

    const uint16_t grainCount = ReadLe16(data, sblkOffset + 0x16);
    const uint16_t soundCount = ReadLe16(data, sblkOffset + 0x14);
    const uint32_t soundTable = sblkOffset + ReadLe32(data, sblkOffset + 0x1C);
    const uint32_t grainTable = sblkOffset + ReadLe32(data, sblkOffset + 0x20);
    const uint32_t waveTable = sblkOffset + ReadLe32(data, sblkOffset + 0x24);
    if (static_cast<uint64_t>(soundTable) + static_cast<uint64_t>(soundCount) * 0x1C > data.size() ||
        static_cast<uint64_t>(grainTable) + static_cast<uint64_t>(grainCount) * 8 > data.size() ||
        waveTable >= data.size())
    {
        error = "invalid SBv2 table offsets";
        return false;
    }

    for (uint16_t index = 0; index < soundCount; ++index)
    {
        const std::size_t entry = soundTable + static_cast<std::size_t>(index) * 0x1C;
        Sony989Cue cue{};
        cue.type = static_cast<int32_t>(ReadLe32(data, entry));
        cue.midiId = ReadLe32(data, entry + 0x0C);
        cue.volume = static_cast<int16_t>(ReadLe16(data, entry + 0x10));
        cue.repeats = static_cast<int8_t>(data[entry + 0x12]);
        cue.volumeGroup = static_cast<int8_t>(data[entry + 0x13]);
        cue.pan = static_cast<int16_t>(ReadLe16(data, entry + 0x14));
        cues.push_back(cue);
    }

    if (sequenceSize != 0 &&
        (sequenceOffset >= data.size() || static_cast<uint64_t>(sequenceOffset) + sequenceSize > data.size()))
    {
        error = "989SND sequence section is outside the file";
        return false;
    }

    for (uint16_t grain = 0; grain < grainCount; ++grain)
    {
        const std::size_t grainEntry = grainTable + static_cast<std::size_t>(grain) * 8;
        const uint8_t waveCount = data[grainEntry];
        Sony989Program program{};
        program.index = grain;
        program.toneCount = waveCount;
        program.volume = static_cast<int8_t>(data[grainEntry + 1]);
        program.pan = static_cast<int16_t>(ReadLe16(data, grainEntry + 2));
        programs.push_back(program);
        const uint32_t waveOffsetFromSblk = ReadLe32(data, grainEntry + 4);
        for (uint8_t index = 0; index < waveCount; ++index)
        {
            const uint64_t toneOffset = static_cast<uint64_t>(sblkOffset) + waveOffsetFromSblk
                + static_cast<uint64_t>(index) * 0x18;
            if (toneOffset + 0x18 > data.size())
            {
                error = "SBv2 tone entry is outside the file";
                return false;
            }

            Sony989Wave wave{};
            wave.centerNote = data[static_cast<std::size_t>(toneOffset) + 2];
            wave.centerFine = data[static_cast<std::size_t>(toneOffset) + 3];
            wave.program = grain;
            wave.tone = index;
            wave.priority = static_cast<int8_t>(data[static_cast<std::size_t>(toneOffset)]);
            wave.volume = static_cast<int8_t>(data[static_cast<std::size_t>(toneOffset) + 1]);
            wave.pan = static_cast<int16_t>(ReadLe16(data, static_cast<std::size_t>(toneOffset) + 4));
            wave.mapLow = static_cast<int8_t>(data[static_cast<std::size_t>(toneOffset) + 6]);
            wave.mapHigh = static_cast<int8_t>(data[static_cast<std::size_t>(toneOffset) + 7]);
            wave.pitchBendLow = static_cast<int8_t>(data[static_cast<std::size_t>(toneOffset) + 8]);
            wave.pitchBendHigh = static_cast<int8_t>(data[static_cast<std::size_t>(toneOffset) + 9]);
            wave.adsr1 = ReadLe16(data, static_cast<std::size_t>(toneOffset) + 0x0A);
            wave.adsr2 = ReadLe16(data, static_cast<std::size_t>(toneOffset) + 0x0C);
            wave.flags = ReadLe16(data, static_cast<std::size_t>(toneOffset) + 0x0E);
            wave.streamOffset = ReadLe32(data, static_cast<std::size_t>(toneOffset) + 0x10);
            wave.sampleRate = Sony989SampleRate(wave.centerNote, wave.centerFine);

            if ((wave.flags & 0x80) != 0 || wave.streamOffset >= audioDataSize)
                continue;

            const std::size_t start = audioDataOffset + wave.streamOffset;
            const std::size_t limit = audioDataOffset + audioDataSize;
            std::size_t end = start;
            while (end + 16 <= limit)
            {
                if (end != start && IsSony989Terminator(data.data() + end))
                    break;
                end += 16;
            }
            wave.streamSize = static_cast<uint32_t>(end - start);
            if (wave.streamSize != 0)
                waves.push_back(wave);
        }
    }

    if (waves.empty())
    {
        error = "SBv2 contains no supported PS-ADPCM waves";
        return false;
    }
    return true;
}

static bool ExtractSony989MusicWaves(const std::vector<uint8_t>& musicData,
                                     const fs::path& outputDirectory,
                                     std::string& error)
{
    std::vector<Sony989Wave> waves;
    std::vector<Sony989Cue> cues;
    std::vector<Sony989Program> programs;
    uint32_t audioDataOffset = 0;
    uint32_t sequenceOffset = 0;
    uint32_t sequenceSize = 0;
    if (!ParseSony989SbV2(musicData, waves, audioDataOffset, cues, programs,
        sequenceOffset, sequenceSize, error))
        return false;

    std::error_code directoryError;
    fs::create_directories(outputDirectory, directoryError);
    if (directoryError)
    {
        error = "could not create 989SND output directory";
        return false;
    }

    std::ofstream manifest(outputDirectory / "Waves.txt");
    if (!manifest.is_open())
    {
        error = "could not create 989SND wave manifest";
        return false;
    }

    if (sequenceSize != 0)
    {
        std::vector<uint8_t> sequence(musicData.begin() + sequenceOffset,
            musicData.begin() + sequenceOffset + sequenceSize);
        if (!WriteBinaryFile(outputDirectory / "Sequence.mmid", sequence))
        {
            error = "could not write Sequence.mmid";
            return false;
        }
    }

    std::ofstream bank(outputDirectory / "Bank.json");
    if (!bank.is_open())
    {
        error = "could not create 989SND bank metadata";
        return false;
    }
    bank << "{\n  \"format\": \"989SND SBv2\",\n  \"sequence\": \"Sequence.mmid\",\n  \"cues\": [\n";
    for (std::size_t index = 0; index < cues.size(); ++index)
    {
        const Sony989Cue& cue = cues[index];
        bank << "    {\"index\": " << index << ", \"type\": " << cue.type
            << ", \"midiId\": " << cue.midiId << ", \"volume\": " << cue.volume
            << ", \"repeats\": " << static_cast<int>(cue.repeats)
            << ", \"volumeGroup\": " << static_cast<int>(cue.volumeGroup)
            << ", \"pan\": " << cue.pan << "}" << (index + 1 < cues.size() ? "," : "") << '\n';
    }
    bank << "  ],\n  \"programs\": [\n";
    for (std::size_t index = 0; index < programs.size(); ++index)
    {
        const Sony989Program& program = programs[index];
        bank << "    {\"index\": " << program.index
            << ", \"toneCount\": " << static_cast<unsigned>(program.toneCount)
            << ", \"volume\": " << static_cast<int>(program.volume)
            << ", \"pan\": " << program.pan << "}"
            << (index + 1 < programs.size() ? "," : "") << '\n';
    }
    bank << "  ],\n  \"tones\": [\n";

    for (std::size_t index = 0; index < waves.size(); ++index)
    {
        const Sony989Wave& wave = waves[index];
        const std::size_t start = audioDataOffset + wave.streamOffset;
        std::vector<uint8_t> encoded(musicData.begin() + start,
            musicData.begin() + start + wave.streamSize);
        const std::vector<int16_t> pcm = DecodeVagChannel(encoded);
        std::vector<uint8_t> wav;
        if (!WritePcm16Wav(pcm, {}, 1, wave.sampleRate, wav))
        {
            error = "PS-ADPCM decoder produced no samples for wave " + std::to_string(index + 1);
            return false;
        }

        char fileName[32]{};
        std::snprintf(fileName, sizeof(fileName), "Wave_%03zu.wav", index + 1);
        if (!WriteBinaryFile(outputDirectory / fileName, wav))
        {
            error = "could not write " + std::string(fileName);
            return false;
        }

        manifest << fileName
            << " | offset 0x" << std::hex << std::uppercase << wave.streamOffset
            << " | bytes 0x" << wave.streamSize
            << " | flags 0x" << wave.flags
            << std::nouppercase << std::dec
            << " | rate " << wave.sampleRate
            << " | loop " << ((wave.flags & 0x40) != 0 ? "yes" : "no") << '\n';

        bank << "    {\"wave\": \"" << fileName << "\", \"program\": " << wave.program
            << ", \"tone\": " << wave.tone
            << ", \"priority\": " << static_cast<int>(wave.priority)
            << ", \"volume\": " << static_cast<int>(wave.volume)
            << ", \"pan\": " << wave.pan
            << ", \"centerNote\": " << static_cast<unsigned>(wave.centerNote)
            << ", \"centerFine\": " << static_cast<unsigned>(wave.centerFine)
            << ", \"mapLow\": " << static_cast<int>(wave.mapLow)
            << ", \"mapHigh\": " << static_cast<int>(wave.mapHigh)
            << ", \"pitchBendLow\": " << static_cast<int>(wave.pitchBendLow)
            << ", \"pitchBendHigh\": " << static_cast<int>(wave.pitchBendHigh)
            << ", \"sampleRate\": " << wave.sampleRate
            << ", \"flags\": " << wave.flags
            << ", \"adsr1\": " << wave.adsr1
            << ", \"adsr2\": " << wave.adsr2 << "}"
            << (index + 1 < waves.size() ? "," : "") << '\n';
    }

    bank << "  ]\n}\n";

    return true;
}

static bool DecodeStandaloneVagToWav(const std::vector<uint8_t>& vag,
                                     std::vector<uint8_t>& wav,
                                     std::string& error)
{
    constexpr std::size_t VAG_HEADER_SIZE = 0x30;
    if (vag.size() < VAG_HEADER_SIZE ||
        vag[0] != 'V' || vag[1] != 'A' || vag[2] != 'G' || vag[3] != 'p')
    {
        error = "missing Sony VAGp header";
        return false;
    }

    const uint32_t declaredDataSize = ReadBe32(vag, 0x0C);
    const uint32_t sampleRate = ReadBe32(vag, 0x10);
    if (sampleRate < 4000 || sampleRate > 192000)
    {
        error = "invalid VAG sample rate " + std::to_string(sampleRate);
        return false;
    }

    const std::size_t availableDataSize = vag.size() - VAG_HEADER_SIZE;
    std::size_t encodedDataSize = declaredDataSize == 0
        ? availableDataSize
        : (std::min)(static_cast<std::size_t>(declaredDataSize), availableDataSize);
    encodedDataSize &= ~std::size_t(15);
    if (encodedDataSize == 0)
    {
        error = "VAG contains no complete ADPCM frames";
        return false;
    }

    const std::vector<uint8_t> encoded(
        vag.begin() + VAG_HEADER_SIZE,
        vag.begin() + VAG_HEADER_SIZE + encodedDataSize);
    const std::vector<int16_t> pcm = DecodeVagChannel(encoded);
    if (!WritePcm16Wav(pcm, {}, 1, sampleRate, wav))
    {
        error = "VAG ADPCM decoder produced no samples";
        return false;
    }

    return true;
}

static bool DecodePssAudioToWav(const DemuxedPss& pss, std::vector<uint8_t>& wav)
{
    std::array<std::vector<uint8_t>, 2> channelData;

    if (pss.channels == 1)
        channelData[0] = pss.adpcm;
    else
    {
        const std::size_t vagInterleave = pss.interleave != 0 ? pss.interleave : 0x800;
        std::size_t offset = 0;
        int channel = 0;

        while (offset < pss.adpcm.size())
        {
            const std::size_t size = (std::min)(vagInterleave, pss.adpcm.size() - offset);
            channelData[channel].insert(channelData[channel].end(),
                                        pss.adpcm.begin() + offset, pss.adpcm.begin() + offset + size);
            offset += size;
            channel ^= 1;
        }
    }

    std::vector<int16_t> left;
    std::vector<int16_t> right;

    if (pss.audioFormat == 0 || pss.audioFormat == 1)
    {
        auto DecodePcm16 = [&](const std::vector<uint8_t>& bytes)
        {
            std::vector<int16_t> samples;
            samples.reserve(bytes.size() / 2);
            for (std::size_t i = 0; i + 1 < bytes.size(); i += 2)
            {
                const uint16_t sample = pss.audioFormat == 1
                    ? static_cast<uint16_t>(bytes[i] | (bytes[i + 1] << 8))
                    : static_cast<uint16_t>((bytes[i] << 8) | bytes[i + 1]);
                samples.push_back(static_cast<int16_t>(sample));
            }
            return samples;
        };

        left = DecodePcm16(channelData[0]);
        if (pss.channels == 2)
            right = DecodePcm16(channelData[1]);
    }
    else if (pss.audioFormat == 2 || pss.audioFormat == 0x10)
    {
        left = DecodeVagChannel(channelData[0]);
        if (pss.channels == 2)
            right = DecodeVagChannel(channelData[1]);
    }
    else
    {
        return false;
    }
    return WritePcm16Wav(left, right, pss.channels, pss.sampleRate, wav);
}

static bool WriteBinaryFile(const fs::path& path, const std::vector<uint8_t>& data)
{
    std::ofstream output(path, std::ios::binary);
    if (!output.is_open())
        return false;
    if (!data.empty())
        output.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    return static_cast<bool>(output);
}

#if 0 // Retained temporarily for reference; MP4 conversion is disabled.
class MediaFoundationSession
{
public:
    MediaFoundationSession()
    {
        const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        m_uninitializeCom = SUCCEEDED(comResult);
        m_result = MFStartup(MF_VERSION, MFSTARTUP_FULL);
    }

    ~MediaFoundationSession()
    {
        if (SUCCEEDED(m_result))
            MFShutdown();
        if (m_uninitializeCom)
            CoUninitialize();
    }

    HRESULT Result() const { return m_result; }

private:
    HRESULT m_result = E_FAIL;
    bool m_uninitializeCom = false;
};

static bool CreateCutsceneMp4(const fs::path& videoPath, const fs::path& audioPath,
                              const fs::path& outputPath, HRESULT& failure,
                              const char*& failureStage)
{
    ComPtr<IMFSourceReader> videoReader;
    ComPtr<IMFSourceReader> audioReader;
    ComPtr<IMFSinkWriter> writer;
    ComPtr<IMFMediaType> type;
    ComPtr<IMFMediaType> decodedVideo;
    ComPtr<IMFMediaType> decodedAudio;

    failureStage = "opening the MPEG program stream";
    ComPtr<IMFAttributes> videoReaderAttributes;
    failure = MFCreateAttributes(&videoReaderAttributes, 2);
    if (FAILED(failure)) return false;
    videoReaderAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
    videoReaderAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    failure = MFCreateSourceReaderFromURL(videoPath.c_str(), videoReaderAttributes.Get(), &videoReader);
    if (FAILED(failure)) return false;
    failureStage = "opening the PCM WAV stream";
    failure = MFCreateSourceReaderFromURL(audioPath.c_str(), nullptr, &audioReader);
    if (FAILED(failure)) return false;

    const GUID videoFormats[] =
    {
        MFVideoFormat_NV12,
        MFVideoFormat_YUY2,
        MFVideoFormat_IYUV,
        MFVideoFormat_YV12,
        MFVideoFormat_RGB32
    };

    failureStage = "negotiating an uncompressed MPEG-2 decoder output";
    for (const GUID& videoFormat : videoFormats)
    {
        type.Reset();
        failure = MFCreateMediaType(&type);
        if (FAILED(failure)) return false;
        type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        type->SetGUID(MF_MT_SUBTYPE, videoFormat);
        failure = videoReader->SetCurrentMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, type.Get());
        if (SUCCEEDED(failure))
            break;
    }
    if (FAILED(failure)) return false;
    failure = videoReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &decodedVideo);
    if (FAILED(failure)) return false;

    type.Reset();
    failure = MFCreateMediaType(&type);
    if (FAILED(failure)) return false;
    failure = type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    if (FAILED(failure)) return false;
    failure = type->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    if (FAILED(failure)) return false;
    failureStage = "configuring the WAV decoder for PCM audio";
    failure = audioReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, type.Get());
    if (FAILED(failure)) return false;
    failure = audioReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &decodedAudio);
    if (FAILED(failure)) return false;

    std::error_code removeError;
    fs::remove(outputPath, removeError);
    failureStage = "creating the MP4 sink writer";
    failure = MFCreateSinkWriterFromURL(outputPath.c_str(), nullptr, nullptr, &writer);
    if (FAILED(failure)) return false;

    UINT32 width = 0, height = 0, frameRateNumerator = 0, frameRateDenominator = 0;
    MFGetAttributeSize(decodedVideo.Get(), MF_MT_FRAME_SIZE, &width, &height);
    if (FAILED(MFGetAttributeRatio(decodedVideo.Get(), MF_MT_FRAME_RATE,
                                   &frameRateNumerator, &frameRateDenominator)))
    {
        frameRateNumerator = 30000;
        frameRateDenominator = 1001;
    }

    ComPtr<IMFMediaType> outputVideo;
    failure = MFCreateMediaType(&outputVideo);
    if (FAILED(failure)) return false;
    outputVideo->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    outputVideo->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
    outputVideo->SetUINT32(MF_MT_AVG_BITRATE, 8000000);
    outputVideo->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(outputVideo.Get(), MF_MT_FRAME_SIZE, width, height);
    MFSetAttributeRatio(outputVideo.Get(), MF_MT_FRAME_RATE, frameRateNumerator, frameRateDenominator);
    MFSetAttributeRatio(outputVideo.Get(), MF_MT_PIXEL_ASPECT_RATIO, 1, 1);

    DWORD videoStream = 0;
    failureStage = "adding the H.264 output stream";
    failure = writer->AddStream(outputVideo.Get(), &videoStream);
    if (FAILED(failure)) return false;
    failureStage = "configuring the H.264 encoder input";
    failure = writer->SetInputMediaType(videoStream, decodedVideo.Get(), nullptr);
    if (FAILED(failure)) return false;

    UINT32 sampleRate = 0, channels = 0, bitsPerSample = 16;
    decodedAudio->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
    decodedAudio->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
    decodedAudio->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);

    ComPtr<IMFMediaType> outputAudio;
    failure = MFCreateMediaType(&outputAudio);
    if (FAILED(failure)) return false;
    outputAudio->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    outputAudio->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
    outputAudio->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, sampleRate);
    outputAudio->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channels);
    outputAudio->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample);
    outputAudio->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, 24000);
    outputAudio->SetUINT32(MF_MT_AAC_PAYLOAD_TYPE, 0);
    outputAudio->SetUINT32(MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION, 0x29);

    DWORD audioStream = 0;
    failureStage = "adding the AAC output stream";
    failure = writer->AddStream(outputAudio.Get(), &audioStream);
    if (FAILED(failure)) return false;
    failureStage = "configuring the AAC encoder input";
    failure = writer->SetInputMediaType(audioStream, decodedAudio.Get(), nullptr);
    if (FAILED(failure)) return false;
    failureStage = "starting the MP4 sink writer";
    failure = writer->BeginWriting();
    if (FAILED(failure)) return false;

    auto CopyStream = [&](IMFSourceReader* reader, DWORD sourceStream, DWORD destinationStream)
    {
        while (true)
        {
            DWORD flags = 0;
            LONGLONG timestamp = 0;
            ComPtr<IMFSample> sample;
            failureStage = sourceStream == MF_SOURCE_READER_FIRST_VIDEO_STREAM
                ? "decoding MPEG-2 video" : "reading PCM audio";
            failure = reader->ReadSample(sourceStream, 0, nullptr, &flags, &timestamp, &sample);
            if (FAILED(failure)) return false;
            if ((flags & MF_SOURCE_READERF_STREAMTICK) != 0)
                writer->SendStreamTick(destinationStream, timestamp);
            if (sample)
            {
                sample->SetSampleTime(timestamp);
                failureStage = sourceStream == MF_SOURCE_READER_FIRST_VIDEO_STREAM
                    ? "encoding H.264 video" : "encoding AAC audio";
                failure = writer->WriteSample(destinationStream, sample.Get());
                if (FAILED(failure)) return false;
            }
            if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0)
                return true;
        }
    };

    if (!CopyStream(videoReader.Get(), MF_SOURCE_READER_FIRST_VIDEO_STREAM, videoStream)) return false;
    if (!CopyStream(audioReader.Get(), MF_SOURCE_READER_FIRST_AUDIO_STREAM, audioStream)) return false;
    failureStage = "finalizing the MP4 file";
    failure = writer->Finalize();
    return SUCCEEDED(failure) && fs::is_regular_file(outputPath) && fs::file_size(outputPath) != 0;
}
#endif

static bool VirtualAddressToExeOffset(uint32_t virtualAddress, uint64_t& executableOffset)
{
    if (virtualAddress < ELF_SEGMENT_VADDR)
        return false;

    executableOffset = static_cast<uint64_t>(virtualAddress - ELF_SEGMENT_VADDR) + ELF_SEGMENT_OFFSET;

    return true;
}

static bool ReadExeStringFromIso(std::ifstream& iso, uint32_t virtualAddress, std::string& result)
{
    uint64_t executableOffset{};

    if (!VirtualAddressToExeOffset(virtualAddress, executableOffset))
        return false;

    const uint64_t isoOffset = SCUS_ISO_OFFSET + executableOffset;

    iso.clear();
    iso.seekg(static_cast<std::streamoff>(isoOffset), std::ios::beg);

    if (!iso)
        return false;

    result.clear();

    constexpr std::size_t MAX_LEVEL_NAME_LENGTH = 256;

    for (std::size_t index = 0; index < MAX_LEVEL_NAME_LENGTH; ++index)
    {
        char character{};

        iso.read(&character, 1);

        if (!iso)
            return false;

        if (character == '\0')
            return true;

        result.push_back(character);
    }

    return false;
}

static std::string SanitizeFileName(std::string name)
{
    for (char& character : name)
    {
        const unsigned char value = static_cast<unsigned char>(character);

        if (character == '<' ||
            character == '>' ||
            character == ':' ||
            character == '"' ||
            character == '/' ||
            character == '\\' ||
            character == '|' ||
            character == '?' ||
            character == '*' ||
            std::iscntrl(value))
        {
            character = '_';
        }
    }

    while (!name.empty() && (name.back() == ' ' || name.back() == '.'))
        name.pop_back();

    if (name.empty())
        name = "Unnamed Level";

    return name;
}

static fs::path GetLevelOutputDirectory(const fs::path& worldsDirectory, uint32_t levelId)
{
    switch (levelId >> 8)
    {
    case 1:
        return worldsDirectory / "Tide of Terror";
    case 2:
        return worldsDirectory / "Sunset Snake Eyes";
    case 3:
        return worldsDirectory / "Vicious Voodoo";
    case 4:
        return worldsDirectory / "Fire in the sky";
    case 5:
        return worldsDirectory / "The Cold Heart of Hate";
    default:
        return worldsDirectory;
    }
}

static fs::path GetDialogWorldDirectory(const fs::path& dialogDirectory, uint32_t levelId)
{
    switch (levelId >> 8)
    {
    case 1:
        return dialogDirectory / "Tide of Terror";
    case 2:
        return dialogDirectory / "Sunset Snake Eyes";
    case 3:
        return dialogDirectory / "Vicious Voodoo";
    case 4:
        return dialogDirectory / "Fire in the sky";
    case 5:
        return dialogDirectory / "The Cold Heart of Hate";
    default:
        return dialogDirectory;
    }
}
static bool ValidateIso(std::ifstream& iso)
{
    std::array<uint8_t, 4> elfMagic{};

    if (!ReadExactAt(iso, SCUS_ISO_OFFSET, elfMagic.data(), elfMagic.size()))
        return false;

    return elfMagic[0] == 0x7F && elfMagic[1] == 'E' && elfMagic[2] == 'L' && elfMagic[3] == 'F';
}

std::vector<uint8_t> Decompress(std::span<const uint8_t> input)
{
    std::vector<uint8_t> output;
    output.reserve(input.size() * 3);

    std::array<uint8_t, CHUNK_SIZE + MAX_MATCH_LENGTH> work{};

    std::size_t inputPosition = 0;
    std::size_t outputPosition = 0;

    uint32_t flags = 0;

    while (inputPosition < input.size())
    {
        flags >>= 1;

        if ((flags & 0x100U) == 0)
        {
            if (inputPosition >= input.size())
                break;

            flags = 0xFF00U | input[inputPosition++];
        }

        if (inputPosition >= input.size())
            break;

        const uint8_t firstByte = input[inputPosition++];

        if ((flags & 1U) != 0)
        {
            work[outputPosition++] = firstByte;
        }
        else
        {
            if (inputPosition >= input.size())
                break;

            const uint8_t secondByte = input[inputPosition++];
            const uint16_t token = static_cast<uint16_t>(firstByte) | static_cast<uint16_t>(static_cast<uint16_t>(secondByte) << 8);

            std::size_t sourcePosition = token & 0x1FFFU;
            const std::size_t copyLength = static_cast<std::size_t>(token >> 13) + 3U;

            for (std::size_t index = 0; index < copyLength; ++index)
            {
                work[outputPosition++] = work[sourcePosition & 0x1FFFU];
                ++sourcePosition;
            }
        }

        if (outputPosition >= CHUNK_SIZE)
        {
            output.insert(output.end(), work.begin(), work.begin() + CHUNK_SIZE);

            const std::size_t spillCount = outputPosition - CHUNK_SIZE;

            if (spillCount > 0)
                std::memmove(work.data(), work.data() + CHUNK_SIZE, spillCount);

            outputPosition = spillCount;
        }
    }

    output.insert(output.end(), work.begin(), work.begin() + outputPosition);

    return output;
}

int main(int argc, char** argv)
{
    if (argc < 2 || argv[1] == nullptr)
    {
        std::cerr << "Drag the Sly 1 NTSC ISO onto this executable.\n";
        return Finish(EXIT_FAILURE);
    }

    const fs::path isoPath = fs::path(argv[1]);
    const fs::path outputDirectory = GetExecutableDirectory();
    const fs::path worldsDirectory = outputDirectory / "Worlds";
    const fs::path cutsceneDirectory = outputDirectory / "Cutscenes";
    const fs::path musicDirectory = worldsDirectory / "Music";
    const fs::path soundBankDirectory = worldsDirectory / "Sound Banks";
    const fs::path commentaryDirectory = worldsDirectory / "Sounds" / "Commentary";
    const fs::path dialogAudioDirectory = worldsDirectory / "Sounds" / "Dialog";
    const fs::path jloAmbientDirectory = worldsDirectory / "Sounds" / "JLO Ambient";
    const fs::path promptSoundDirectory = worldsDirectory / "Sounds" / "Prompt";

    if (!fs::exists(isoPath))
    {
        std::cerr << "ISO does not exist:\n" << isoPath << '\n';
        return Finish(EXIT_FAILURE);
    }

    std::ifstream iso(isoPath, std::ios::binary);

    if (!iso.is_open())
    {
        std::cerr << "Could not open ISO:\n" << isoPath << '\n';
        return Finish(EXIT_FAILURE);
    }

    if (!ValidateIso(iso))
    {
        std::cerr << "Invalid Sly 1 NTSC ISO.\n";
        std::cerr << "The embedded SCUS ELF was not found at ISO offset 0x" << std::hex << SCUS_ISO_OFFSET << std::dec << ".\n";
        return Finish(EXIT_FAILURE);
    }

    std::array<LEVELINFODisk, LEVEL_COUNT> levelTable{};

    if (!ReadExactAt(iso, LEVEL_TABLE_ISO_OFFSET, levelTable.data(), sizeof(levelTable)))
    {
        std::cerr << "Failed to read LEVELINFO table at ISO offset 0x" << std::hex << LEVEL_TABLE_ISO_OFFSET << std::dec << ".\n";
        return Finish(EXIT_FAILURE);
    }

    std::array<VAGINFODisk, SOUND_BANK_COUNT> soundBankTable{};

    if (!ReadExactAt(iso, SOUND_BANK_TABLE_ISO_OFFSET, soundBankTable.data(), sizeof(soundBankTable)))
    {
        std::cerr << "Failed to read sound bank table at ISO offset 0x" << std::hex
                  << SOUND_BANK_TABLE_ISO_OFFSET << std::dec << ".\n";
        return Finish(EXIT_FAILURE);
    }

    std::array<COMMENTARYINFODisk, COMMENTARY_COUNT> commentaryTable{};

    if (!ReadExactAt(iso, COMMENTARY_TABLE_ISO_OFFSET, commentaryTable.data(), sizeof(commentaryTable)))
    {
        std::cerr << "Failed to read commentary table at ISO offset 0x" << std::hex
                  << COMMENTARY_TABLE_ISO_OFFSET << std::dec << ".\n";
        return Finish(EXIT_FAILURE);
    }

    std::array<VAGINFODisk, JLO_AMBIENT_COUNT> jloAmbientTable{};

    if (!ReadExactAt(iso, JLO_AMBIENT_TABLE_ISO_OFFSET, jloAmbientTable.data(), sizeof(jloAmbientTable)))
    {
        std::cerr << "Failed to read JLO ambient VAG table at ISO offset 0x" << std::hex
                  << JLO_AMBIENT_TABLE_ISO_OFFSET << std::dec << ".\n";
        return Finish(EXIT_FAILURE);
    }

    std::array<VAGINFODisk, CUTSCENE_COUNT> cutsceneTable{};

    if (!ReadExactAt(iso, CUTSCENE_TABLE_ISO_OFFSET, cutsceneTable.data(), sizeof(cutsceneTable)))
    {
        std::cerr << "Failed to read cutscene table at ISO offset 0x" << std::hex
                  << CUTSCENE_TABLE_ISO_OFFSET << std::dec << ".\n";
        return Finish(EXIT_FAILURE);
    }

    VAGINFODisk promptVagInfo{};

    if (!ReadExactAt(iso, PROMPT_VAG_INFO_ISO_OFFSET, &promptVagInfo, sizeof(promptVagInfo)))
    {
        std::cerr << "Failed to read prompt VAG descriptor at ISO offset 0x" << std::hex
                  << PROMPT_VAG_INFO_ISO_OFFSET << std::dec << ".\n";
        return Finish(EXIT_FAILURE);
    }

    std::cout << "ISO: " << isoPath << '\n';
    std::cout << "Output directory: " << worldsDirectory << '\n';
    std::cout << "LEVELINFO table: 0x" << std::hex << LEVEL_TABLE_ISO_OFFSET << std::dec << "\n\n";

    std::vector<std::pair<uint32_t, std::string>> levelNames;
    levelNames.reserve(levelTable.size());

    for (std::size_t index = 0; index < levelTable.size(); ++index)
    {
        const LEVELINFODisk& levelInfo = levelTable[index];

        const uint32_t sector = levelInfo.fileInfo.fcl.isector ^ levelInfo.lsnKey;
        const uint32_t compressedSize = levelInfo.fileInfo.fcl.cb ^ levelInfo.sizeKey;
        const uint32_t searchValue = levelInfo.searchValue ^ levelInfo.searchCipher;
        const uint64_t dataOffset = static_cast<uint64_t>(sector) * ISO_SECTOR_SIZE;

        std::string levelName;

        if (!ReadExeStringFromIso(iso, levelInfo.levelName, levelName))
            levelName = "Level_" + std::to_string(index);

        levelName = SanitizeFileName(levelName);

        if ((levelInfo.levelId & 0xFFFFU) == 0x0003U)
            levelName = "Paris";

        levelNames.emplace_back(levelInfo.levelId, levelName);

        const fs::path levelOutputDirectory = GetLevelOutputDirectory(worldsDirectory, levelInfo.levelId);
        std::error_code directoryError;
        fs::create_directories(levelOutputDirectory, directoryError);

        if (directoryError)
        {
            std::cerr << "Could not create output directory:\n" << levelOutputDirectory << '\n';
            return Finish(EXIT_FAILURE);
        }

        const fs::path outputPath = levelOutputDirectory / (levelName + ".brx");

        std::cout
            << '[' << index << "] Extracting " << levelName << '\n'
            << "    sector: " << sector << '\n'
            << "    ISO offset: 0x" << std::hex << dataOffset << std::dec << '\n'
            << "    compressed size: " << compressedSize << '\n'
            << "    search value: 0x" << std::hex << searchValue << std::dec << '\n'
            << "    level ID: " << levelInfo.levelId << '\n'
            << "    task: " << levelInfo.task << '\n';

        std::vector<uint8_t> compressed(compressedSize);

        if (!ReadExactAt(iso, dataOffset, compressed.data(), compressed.size()))
        {
            std::cerr << "Failed to read " << compressedSize << " bytes from ISO offset 0x" << std::hex << dataOffset << std::dec << " for " << levelName << ".\n";
            return Finish(EXIT_FAILURE);
        }

        const std::vector<uint8_t> decompressed = Decompress(compressed);

        std::ofstream output(outputPath, std::ios::binary);

        if (!output.is_open())
        {
            std::cerr << "Could not create output file:\n" << outputPath << '\n';
            return Finish(EXIT_FAILURE);
        }

        if (!decompressed.empty())
            output.write(reinterpret_cast<const char*>(decompressed.data()), static_cast<std::streamsize>(decompressed.size()));

        if (!output)
        {
            std::cerr << "Failed writing output file:\n" << outputPath << '\n';
            return Finish(EXIT_FAILURE);
        }

        output.close();

        std::cout
            << "    decompressed size: " << decompressed.size() << '\n'
            << "    output: " << outputPath << "\n\n";
    }

    std::error_code musicDirectoryError;
    fs::create_directories(musicDirectory, musicDirectoryError);

    if (musicDirectoryError)
    {
        std::cerr << "Could not create music output directory:\n" << musicDirectory << '\n';
        return Finish(EXIT_FAILURE);
    }

    std::ofstream musicManifest(musicDirectory / "MusicMap.txt");
    if (!musicManifest.is_open())
    {
        std::cerr << "Could not create music manifest:\n" << (musicDirectory / "MusicMap.txt") << '\n';
        return Finish(EXIT_FAILURE);
    }

    std::vector<uint32_t> extractedMusicIds;
    for (const MUSICINFODisk& musicInfo : MUSIC_TABLE)
    {
        musicManifest << musicInfo.levelName
            << " | MUSID 0x" << std::hex << std::uppercase << musicInfo.musicId
            << " | sector 0x" << musicInfo.sector
            << " | bytes 0x" << musicInfo.size
            << std::nouppercase << std::dec << '\n';

        if (std::find(extractedMusicIds.begin(), extractedMusicIds.end(), musicInfo.musicId)
            != extractedMusicIds.end())
        {
            continue;
        }

        const uint64_t dataOffset = static_cast<uint64_t>(musicInfo.sector) * ISO_SECTOR_SIZE;
        std::vector<uint8_t> musicData(musicInfo.size);
        if (!ReadExactAt(iso, dataOffset, musicData.data(), musicData.size()))
        {
            std::cerr << "Failed to read MUSID 0x" << std::hex << musicInfo.musicId
                << " from ISO offset 0x" << dataOffset << std::dec << ".\n";
            return Finish(EXIT_FAILURE);
        }

        char musicFileName[32]{};
        std::snprintf(musicFileName, sizeof(musicFileName), "MUSID_%02X.mus", musicInfo.musicId);
        const fs::path musicOutputPath = musicDirectory / musicFileName;
        if (!WriteBinaryFile(musicOutputPath, musicData))
        {
            std::cerr << "Failed writing music output:\n" << musicOutputPath << '\n';
            return Finish(EXIT_FAILURE);
        }

		extractedMusicIds.push_back(musicInfo.musicId);
		std::cout << "[Music] " << musicInfo.levelName
			<< " -> MUSID 0x" << std::hex << std::uppercase << musicInfo.musicId
			<< std::nouppercase << std::dec
			<< " -> " << musicOutputPath << '\n';
    }

    std::cout << "Extracted " << extractedMusicIds.size()
        << " unique music files into:\n" << musicDirectory << "\n\n";

    std::error_code soundBankDirectoryError;
    fs::create_directories(soundBankDirectory, soundBankDirectoryError);

    if (soundBankDirectoryError)
    {
        std::cerr << "Could not create sound bank output directory:\n" << soundBankDirectory << '\n';
        return Finish(EXIT_FAILURE);
    }

    for (std::size_t index = 0; index < soundBankTable.size(); ++index)
    {
        const VAGINFODisk& descriptor = soundBankTable[index];
        const uint32_t sector = descriptor.encryptedSector ^ descriptor.sectorKey;
        const uint32_t size = descriptor.encryptedSize ^ descriptor.sizeKey;
        const uint32_t identity = descriptor.encryptedId ^ descriptor.idKey;
        const uint64_t dataOffset = static_cast<uint64_t>(sector) * ISO_SECTOR_SIZE;

        if (sector == 0 || size == 0)
        {
            std::cerr << "Invalid sound bank descriptor for IBNK " << index << " ("
                      << SOUND_BANK_NAMES[index] << ").\n";
            return Finish(EXIT_FAILURE);
        }

        std::vector<uint8_t> bankData(size);
        if (!ReadExactAt(iso, dataOffset, bankData.data(), bankData.size()))
        {
            std::cerr << "Failed to read sound bank " << SOUND_BANK_NAMES[index]
                      << " from ISO offset 0x" << std::hex << dataOffset << std::dec << ".\n";
            return Finish(EXIT_FAILURE);
        }

        const fs::path bankOutputPath = soundBankDirectory /
            (std::string(SOUND_BANK_NAMES[index]) + ".bnk");
        if (!WriteBinaryFile(bankOutputPath, bankData))
        {
            std::cerr << "Failed writing sound bank:\n" << bankOutputPath << '\n';
            return Finish(EXIT_FAILURE);
        }

        std::cout << "[Sound Bank] IBNK " << index << " " << SOUND_BANK_NAMES[index]
                  << " | identity 0x" << std::hex << std::uppercase << identity
                  << " | sector 0x" << sector
                  << " | bytes 0x" << size
                  << std::nouppercase << std::dec
                  << " -> " << bankOutputPath << '\n';
    }

    std::cout << "Extracted " << soundBankTable.size()
              << " sound banks into:\n" << soundBankDirectory << "\n\n";

    std::error_code commentaryDirectoryError;
    fs::create_directories(commentaryDirectory, commentaryDirectoryError);

    if (commentaryDirectoryError)
    {
        std::cerr << "Could not create commentary output directory:\n" << commentaryDirectory << '\n';
        return Finish(EXIT_FAILURE);
    }

    const fs::path commentaryManifestPath = commentaryDirectory / "CommentaryMap.txt";
    std::ofstream commentaryManifest(commentaryManifestPath);
    if (!commentaryManifest.is_open())
    {
        std::cerr << "Could not create commentary manifest:\n" << commentaryManifestPath << '\n';
        return Finish(EXIT_FAILURE);
    }

    std::size_t commentaryExtracted = 0;
    for (std::size_t index = 0; index < commentaryTable.size(); ++index)
    {
        const COMMENTARYINFODisk& commentary = commentaryTable[index];
        const VAGINFODisk& descriptor = commentary.audio;
        const uint32_t sector = descriptor.encryptedSector ^ descriptor.sectorKey;
        const uint32_t size = descriptor.encryptedSize ^ descriptor.sizeKey;
        const uint32_t identity = descriptor.encryptedId ^ descriptor.idKey;

        std::string speakerNames;
        if (!ReadExeStringFromIso(iso, commentary.speakerNames, speakerNames))
            speakerNames = "Unknown";

        if (sector == 0 || size == 0)
        {
            std::cout << "[Commentary] Skipping empty descriptor for level ID 0x"
                      << std::hex << std::uppercase << commentary.levelId
                      << std::nouppercase << std::dec << '.\n';
            continue;
        }

        const uint64_t dataOffset = static_cast<uint64_t>(sector) * ISO_SECTOR_SIZE;
        std::vector<uint8_t> commentaryData(size);
        if (!ReadExactAt(iso, dataOffset, commentaryData.data(), commentaryData.size()))
        {
            std::cerr << "Failed to read commentary for level ID 0x" << std::hex
                      << commentary.levelId << " from ISO offset 0x" << dataOffset << std::dec << ".\n";
            return Finish(EXIT_FAILURE);
        }

        char commentaryFileName[40]{};
        std::snprintf(commentaryFileName, sizeof(commentaryFileName),
            "Commentary_%04X.wav", commentary.levelId);
        const fs::path commentaryOutputPath = commentaryDirectory / commentaryFileName;
        std::vector<uint8_t> commentaryWav;
        std::string commentaryConversionError;
        if (!DecodeStandaloneVagToWav(commentaryData, commentaryWav, commentaryConversionError) ||
            !WriteBinaryFile(commentaryOutputPath, commentaryWav))
        {
            if (commentaryConversionError.empty())
                commentaryConversionError = "failed writing WAV file";
            std::cerr << "Failed converting commentary audio to WAV: "
                      << commentaryConversionError << "\n" << commentaryOutputPath << '\n';
            return Finish(EXIT_FAILURE);
        }

        commentaryManifest << commentaryFileName
            << " | levelId 0x" << std::hex << std::uppercase << commentary.levelId
            << " | duration " << std::dec << commentary.audioDuration
            << " | identity 0x" << std::hex << identity
            << " | sector 0x" << sector
            << " | bytes 0x" << size
            << " | speakers " << speakerNames
            << std::nouppercase << std::dec << '\n';

        ++commentaryExtracted;
        std::cout << "[Commentary] level ID 0x" << std::hex << std::uppercase
                  << commentary.levelId << " | identity 0x" << identity
                  << " | sector 0x" << sector << " | bytes 0x" << size
                  << std::nouppercase << std::dec
                  << " | " << speakerNames << " -> " << commentaryOutputPath << '\n';
    }

    std::cout << "Extracted " << commentaryExtracted
              << " commentary audio files into:\n" << commentaryDirectory << "\n\n";

    std::error_code dialogAudioDirectoryError;
    fs::create_directories(dialogAudioDirectory, dialogAudioDirectoryError);

    if (dialogAudioDirectoryError)
    {
        std::cerr << "Could not create dialog audio output directory:\n" << dialogAudioDirectory << '\n';
        return Finish(EXIT_FAILURE);
    }

    std::vector<uint32_t> initializedDialogManifests;
    std::size_t dialogAudioExtracted = 0;

    for (const DIALOGAUDIOINFO& dialogAudio : DIALOG_AUDIO_TABLE)
    {
        std::string levelName = "Level_" + std::to_string(dialogAudio.levelId);
        for (const auto& level : levelNames)
        {
            if (level.first == dialogAudio.levelId)
            {
                levelName = level.second;
                break;
            }
        }

        if (dialogAudio.levelId == 0x3)
            levelName = "Paris";
        else if (dialogAudio.levelId == 0x4)
            levelName = "The Hideout";

        const fs::path levelDialogDirectory =
            GetDialogWorldDirectory(dialogAudioDirectory, dialogAudio.levelId) /
            SanitizeFileName(levelName);

        std::error_code levelDialogDirectoryError;
        fs::create_directories(levelDialogDirectory, levelDialogDirectoryError);
        if (levelDialogDirectoryError)
        {
            std::cerr << "Could not create level dialog directory:\n" << levelDialogDirectory << '\n';
            return Finish(EXIT_FAILURE);
        }

        const bool hasIdentityCollision = std::count_if(
            std::begin(DIALOG_AUDIO_TABLE), std::end(DIALOG_AUDIO_TABLE),
            [&dialogAudio](const DIALOGAUDIOINFO& candidate)
            {
                return candidate.levelId == dialogAudio.levelId &&
                    candidate.identity == dialogAudio.identity;
            }) > 1;

        char audioFileName[64]{};
        if (hasIdentityCollision)
        {
            std::snprintf(audioFileName, sizeof(audioFileName),
                "VAG_%04X_%06X.wav", dialogAudio.identity, dialogAudio.sector);
        }
        else
        {
            std::snprintf(audioFileName, sizeof(audioFileName),
                "VAG_%04X.wav", dialogAudio.identity);
        }

        const uint64_t dataOffset = static_cast<uint64_t>(dialogAudio.sector) * ISO_SECTOR_SIZE;
        std::vector<uint8_t> audioData(dialogAudio.size);
        if (!ReadExactAt(iso, dataOffset, audioData.data(), audioData.size()))
        {
            std::cerr << "Failed to read dialog VAG 0x" << std::hex << dialogAudio.identity
                      << " from ISO offset 0x" << dataOffset << std::dec << ".\n";
            return Finish(EXIT_FAILURE);
        }

        std::vector<uint8_t> wavData;
        std::string conversionError;
        const fs::path wavOutputPath = levelDialogDirectory / audioFileName;
        const bool decodedToWav = DecodeStandaloneVagToWav(audioData, wavData, conversionError);
        const bool convertedToWav = decodedToWav && WriteBinaryFile(wavOutputPath, wavData);
        if (!convertedToWav)
        {
            if (decodedToWav)
                conversionError = "failed writing WAV file";
            std::cerr << "Could not convert dialog VAG 0x" << std::hex << std::uppercase
                      << dialogAudio.identity << std::nouppercase << std::dec
                      << " to WAV: " << conversionError << '\n';
            return Finish(EXIT_FAILURE);
        }

        const bool initializeManifest = std::find(
            initializedDialogManifests.begin(), initializedDialogManifests.end(), dialogAudio.levelId)
            == initializedDialogManifests.end();
        if (initializeManifest)
            initializedDialogManifests.push_back(dialogAudio.levelId);

        std::ofstream dialogManifest(
            levelDialogDirectory / "DialogMap.txt",
            initializeManifest ? std::ios::out : (std::ios::out | std::ios::app));
        if (!dialogManifest.is_open())
        {
            std::cerr << "Could not write dialog manifest in:\n" << levelDialogDirectory << '\n';
            return Finish(EXIT_FAILURE);
        }

        dialogManifest << audioFileName
            << " | levelId 0x" << std::hex << std::uppercase << dialogAudio.levelId
            << " | identity 0x" << dialogAudio.identity
            << " | sector 0x" << dialogAudio.sector
            << " | bytes 0x" << dialogAudio.size
            << " | source " << (dialogAudio.source == DialogAudioSource::DIALOG ? "DIALOG" : "ASEG")
            << std::nouppercase << std::dec << '\n';

        ++dialogAudioExtracted;
        std::cout << "[Dialog Audio] " << levelName
                  << " | VAG 0x" << std::hex << std::uppercase << dialogAudio.identity
                  << " | sector 0x" << dialogAudio.sector
                  << " | bytes 0x" << dialogAudio.size
                  << std::nouppercase << std::dec
                  << " -> " << wavOutputPath << '\n';
    }

    std::cout << "Extracted " << dialogAudioExtracted
              << " dialog WAV files into:\n" << dialogAudioDirectory << ".\n\n";

    std::error_code cutsceneDirectoryError;
    fs::create_directories(cutsceneDirectory, cutsceneDirectoryError);

    if (cutsceneDirectoryError)
    {
        std::cerr << "Could not create cutscene output directory:\n" << cutsceneDirectory << '\n';
        return Finish(EXIT_FAILURE);
    }

    std::cout << "Cutscene table: 0x" << std::hex << CUTSCENE_TABLE_ISO_OFFSET << std::dec << "\n\n";

    for (std::size_t index = 0; index < EXTRACTED_CUTSCENE_COUNT; ++index)
    {
        const bool isStartupSplash = index == CUTSCENE_COUNT;
        const VAGINFODisk* cutsceneInfo = isStartupSplash ? nullptr : &cutsceneTable[index];
        const uint32_t sector = isStartupSplash ? STARTUP_SPLASH_SECTOR :
            cutsceneInfo->encryptedSector ^ cutsceneInfo->sectorKey;
        const uint32_t size = isStartupSplash ? STARTUP_SPLASH_SIZE :
            cutsceneInfo->encryptedSize ^ cutsceneInfo->sizeKey;
        const uint32_t identity = isStartupSplash ? STARTUP_SPLASH_IDENTITY :
            cutsceneInfo->encryptedId ^ cutsceneInfo->idKey;
        const uint64_t dataOffset = static_cast<uint64_t>(sector) * ISO_SECTOR_SIZE;

        // The recovered descriptor at 0x00248490 precedes the sixteen entries
        // used by the FMV menu. Preserve 00-15 and map identity 0x344 to 16.
        const std::size_t outputIndex = isStartupSplash ? 17 : (index == 0 ? 16 : index - 1);
        std::string fileName = "Cutscene_";

        if (outputIndex < 10)
            fileName += '0';

        fileName += std::to_string(outputIndex);
        const fs::path videoOutputPath = cutsceneDirectory / (fileName + ".m2v");
        const fs::path audioOutputPath = cutsceneDirectory / (fileName + ".wav");

        if (sector == 0 || size == 0)
        {
            std::cerr << "Invalid decrypted cutscene descriptor " << index << ".\n";
            return Finish(EXIT_FAILURE);
        }

        std::vector<uint8_t> movieData(size);

        if (!ReadExactAt(iso, dataOffset, movieData.data(), movieData.size()))
        {
            std::cerr << "Failed to read cutscene " << index << " from ISO offset 0x"
                      << std::hex << dataOffset << std::dec << ".\n";
            return Finish(EXIT_FAILURE);
        }

        DemuxedPss demuxed;
        std::vector<uint8_t> wavData;

        if (!DemuxPss(movieData, demuxed))
        {
            std::cerr << "Failed to demux cutscene " << index << ": "
                      << demuxed.error << ".\n";
            return Finish(EXIT_FAILURE);
        }

        if (!DecodePssAudioToWav(demuxed, wavData))
        {
            std::cerr << "Unsupported or invalid cutscene audio in cutscene " << index
                      << " (format " << demuxed.audioFormat << ").\n";
            return Finish(EXIT_FAILURE);
        }

        if (!WriteBinaryFile(videoOutputPath, demuxed.video) ||
            !WriteBinaryFile(audioOutputPath, wavData))
        {
            std::cerr << "Failed writing cutscene outputs:\n"
                      << videoOutputPath << '\n' << audioOutputPath << '\n';
            return Finish(EXIT_FAILURE);
        }

        std::cout
            << "[Cutscene " << index << "]\n"
            << "    identity: 0x" << std::hex << identity << std::dec << '\n'
            << "    sector: " << sector << '\n'
            << "    ISO offset: 0x" << std::hex << dataOffset << std::dec << '\n'
            << "    PSS size: " << size << '\n'
            << "    audio source: format " << demuxed.audioFormat << ", "
            << demuxed.sampleRate << " Hz, " << static_cast<int>(demuxed.channels)
            << " channels, 0x" << std::hex << demuxed.interleave << std::dec
            << " interleave\n"
            << "    video: " << videoOutputPath << '\n'
            << "    audio: " << audioOutputPath << "\n\n";
    }

    std::error_code soundDirectoryError;
    fs::create_directories(jloAmbientDirectory, soundDirectoryError);

    if (soundDirectoryError)
    {
        std::cerr << "Could not create sound output directory:\n" << jloAmbientDirectory << '\n';
        return Finish(EXIT_FAILURE);
    }

    std::cout << "JLO ambient VAG table: 0x" << std::hex << JLO_AMBIENT_TABLE_ISO_OFFSET << std::dec << "\n\n";

    for (std::size_t index = 0; index < jloAmbientTable.size(); ++index)
    {
        const VAGINFODisk& vagInfo = jloAmbientTable[index];
        const uint32_t sector = vagInfo.encryptedSector ^ vagInfo.sectorKey;
        const uint32_t size = vagInfo.encryptedSize ^ vagInfo.sizeKey;
        const uint64_t dataOffset = static_cast<uint64_t>(sector) * ISO_SECTOR_SIZE;
        const fs::path outputPath = jloAmbientDirectory / ("JloAmbient_" + std::to_string(index + 1) + ".wav");

        if (sector == 0 || size == 0)
        {
            std::cerr << "Invalid decrypted JLO ambient descriptor " << index << ".\n";
            return Finish(EXIT_FAILURE);
        }

        std::vector<uint8_t> vagData(size);

        if (!ReadExactAt(iso, dataOffset, vagData.data(), vagData.size()))
        {
            std::cerr << "Failed to read JLO ambient " << index + 1 << " from ISO offset 0x"
                      << std::hex << dataOffset << std::dec << ".\n";
            return Finish(EXIT_FAILURE);
        }

        std::vector<uint8_t> wavData;
        std::string conversionError;
        if (!DecodeStandaloneVagToWav(vagData, wavData, conversionError) ||
            !WriteBinaryFile(outputPath, wavData))
        {
            if (conversionError.empty())
                conversionError = "failed writing WAV file";
            std::cerr << "Failed converting JLO ambient " << index + 1
                      << " to WAV: " << conversionError << '\n';
            return Finish(EXIT_FAILURE);
        }

        std::cout
            << "[JLO ambient " << index + 1 << "]\n"
            << "    sector: " << sector << '\n'
            << "    ISO offset: 0x" << std::hex << dataOffset << std::dec << '\n'
            << "    size: " << size << '\n'
            << "    output: " << outputPath << "\n\n";
    }

    std::error_code promptDirectoryError;
    fs::create_directories(promptSoundDirectory, promptDirectoryError);

    if (promptDirectoryError)
    {
        std::cerr << "Could not create prompt sound output directory:\n" << promptSoundDirectory << '\n';
        return Finish(EXIT_FAILURE);
    }

    const uint32_t promptSector = promptVagInfo.encryptedSector ^ promptVagInfo.sectorKey;
    const uint32_t promptSize = promptVagInfo.encryptedSize ^ promptVagInfo.sizeKey;
    const uint64_t promptDataOffset = static_cast<uint64_t>(promptSector) * ISO_SECTOR_SIZE;
    const fs::path promptOutputPath = promptSoundDirectory / "PromptPrk18.wav";

    if (promptSector == 0 || promptSize == 0)
    {
        std::cerr << "Invalid decrypted prompt VAG descriptor.\n";
        return Finish(EXIT_FAILURE);
    }

    std::vector<uint8_t> promptVagData(promptSize);

    if (!ReadExactAt(iso, promptDataOffset, promptVagData.data(), promptVagData.size()))
    {
        std::cerr << "Failed to read prompt VAG from ISO offset 0x" << std::hex
                  << promptDataOffset << std::dec << ".\n";
        return Finish(EXIT_FAILURE);
    }

    std::vector<uint8_t> promptWavData;
    std::string promptConversionError;
    if (!DecodeStandaloneVagToWav(promptVagData, promptWavData, promptConversionError) ||
        !WriteBinaryFile(promptOutputPath, promptWavData))
    {
        if (promptConversionError.empty())
            promptConversionError = "failed writing WAV file";
        std::cerr << "Failed converting prompt VAG to WAV: "
                  << promptConversionError << '\n';
        return Finish(EXIT_FAILURE);
    }

    std::cout
        << "[Prompt PRK 18 WAV]\n"
        << "    descriptor: 0x" << std::hex << PROMPT_VAG_INFO_ISO_OFFSET << std::dec << '\n'
        << "    sector: " << promptSector << '\n'
        << "    ISO offset: 0x" << std::hex << promptDataOffset << std::dec << '\n'
        << "    size: " << promptSize << '\n'
        << "    output: " << promptOutputPath << "\n\n";

    std::cout << "Finished extracting " << LEVEL_COUNT << " BRX files, "
              << extractedMusicIds.size() << " unique music files, "
              << EXTRACTED_CUTSCENE_COUNT << " cutscene M2V/WAV pairs, " << JLO_AMBIENT_COUNT
              << " JLO ambient WAV files, and the prompt WAV into:\n"
              << worldsDirectory << '\n';

    return Finish(EXIT_SUCCESS);
}
