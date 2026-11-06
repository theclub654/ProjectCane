#include "iso_extractor.h"
#include "../../File Extractor GUI/NativeTools/BrxRebuilder/BrxWalker.h"
#include <future>
#include "../../File Extractor GUI/NativeTools/PalExtractor/PalIsoExtractor.h"
#include <atomic>
#include <set>
#include <mutex>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <zlib.h>
#ifdef __SWITCH__
#include "../switch/file_io.h"
#endif
#include <array>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <span>
#include <sstream>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
namespace
{
struct Session;
thread_local Session* activeSession = nullptr;
struct Session
{
    const CaneExtract::ProgressCallback& callback;
    CaneExtract::Progress progress;
    uint64_t isoSize;
    std::chrono::steady_clock::time_point last{};
    Session(const CaneExtract::ProgressCallback& fn, uint64_t size) : callback(fn), isoSize(size)
    {
        if (activeSession) throw std::runtime_error("An extraction is already running");
        activeSession = this;
    }
    ~Session() { activeSession = nullptr; }
    void Pulse(bool force = false)
    {
        if (!callback) return;
        const auto now = std::chrono::steady_clock::now();
        if (!force && now - last < std::chrono::milliseconds(50)) return;
        last = now;
        if (!callback(progress)) throw CaneExtract::Cancelled();
    }
    void Step(const char* stage, const std::string& file)
    {
        ++progress.completed;
        progress.stage = stage;
        progress.file = file;
        Pulse(true);
    }
};
void Pulse() { if (activeSession) activeSession->Pulse(); }
void CheckIsoRange(uint64_t offset, uint64_t size)
{
    if (!activeSession || offset > activeSession->isoSize || size > activeSession->isoSize - offset || size > 512 * 1024 * 1024)
        throw std::runtime_error("The ISO is incomplete or damaged. Please select a complete game ISO.");
}
constexpr std::size_t CHUNK_SIZE = 0x2000, MAX_MATCH_LENGTH = 10;
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
#include "../../File Extractor GUI/Data/DialogAudioTable.inc"
};

static bool ReadExactAt(std::ifstream& file, uint64_t offset, void* destination, std::size_t size)
{
    if (activeSession && (offset > activeSession->isoSize || size > activeSession->isoSize - offset))
        return false;
    file.clear();
    file.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!file) return false;
    auto* output = static_cast<char*>(destination);
    while (size)
    {
        const auto chunk = (std::min)(size, std::size_t(1024 * 1024));
        file.read(output, static_cast<std::streamsize>(chunk));
        if (file.gcount() != static_cast<std::streamsize>(chunk)) return false;
        output += chunk;
        size -= chunk;
        Pulse();
    }
    return true;
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

static uint16_t ReadLe16(const std::vector<uint8_t>& data, std::size_t offset)
{
    return static_cast<uint16_t>(data[offset] | (data[offset + 1] << 8));
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
        if ((offset & 0x7ff) == 0) Pulse();
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
        if ((offset & 0xffff) == 0) Pulse();
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
    if (!output) return false;
    for (size_t offset = 0; offset < data.size();)
    {
        const size_t chunk = (std::min)(data.size() - offset, size_t(1024 * 1024));
        output.write(reinterpret_cast<const char*>(data.data() + offset), static_cast<std::streamsize>(chunk));
        if (!output) return false;
        offset += chunk;
        Pulse();
    }
    output.close();
    return bool(output);
}

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

static fs::path GetDialogOutputDirectory(const fs::path& worldsDirectory, uint32_t levelId)
{
    return GetLevelOutputDirectory(worldsDirectory, levelId);
}
static bool ValidateIso(std::ifstream& iso)
{
    std::vector<uint8_t> volume(ISO_SECTOR_SIZE);
    if (!ReadExactAt(iso, 16 * ISO_SECTOR_SIZE, volume.data(), volume.size()) ||
        volume[0] != 1 || std::memcmp(volume.data() + 1, "CD001", 5) ||
        ReadLe16(volume, 128) != ISO_SECTOR_SIZE)
        return false;
    const uint32_t sector = ReadLe32(volume, 158), bytes = ReadLe32(volume, 166);
    if (bytes < 34 || bytes > 1024 * 1024) return false;
    std::vector<uint8_t> root(bytes);
    if (!ReadExactAt(iso, uint64_t(sector) * ISO_SECTOR_SIZE, root.data(), root.size())) return false;
    bool haveExecutable = false;
    for (size_t at = 0; at < root.size();)
    {
        const unsigned length = root[at];
        if (!length) { at = (at / ISO_SECTOR_SIZE + 1) * ISO_SECTOR_SIZE; continue; }
        if (length < 34 || length > root.size() - at || root[at + 32] > length - 33) return false;
        const std::string name(reinterpret_cast<const char*>(root.data() + at + 33), root[at + 32]);
        if (name == "SCUS_971.98;1" && uint64_t(ReadLe32(root, at + 2)) * ISO_SECTOR_SIZE == SCUS_ISO_OFFSET)
            haveExecutable = true;
        at += length;
    }
    std::array<uint8_t, 24> header{};
    if (!haveExecutable || !ReadExactAt(iso, SCUS_ISO_OFFSET, header.data(), header.size())) return false;
    return std::memcmp(header.data(), "\x7f" "ELF", 4) == 0 && header[4] == 1 && header[5] == 1 &&
        header[16] == 2 && header[17] == 0 && header[18] == 8 && header[19] == 0;
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
            if ((output.size() & 0x3ffff) == 0) Pulse();
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
using DiscRegion = cane::pal::DiscRegion;

void WriteManifest(const fs::path& directory, DiscRegion region, int version)
{
    const bool japanese = region == DiscRegion::Japanese, pal = region == DiscRegion::Pal;
    nlohmann::json manifest{
        {"schemaVersion", 1}, {"game", japanese ? "Kaitou Sly Cooper" : pal ? "Sly Raccoon" : "Sly Cooper and the Thievius Raccoonus"},
        {"region", japanese ? "NTSC-J" : pal ? "PAL" : "NTSC-U"},
        {"serial", japanese ? "SCPS-150.36" : pal ? "SCES-509.17" : "SCUS-971.98"},
        {"levelFormat", "PCBRX"}, {"levelFormatVersion", version},
        {"languages", japanese ? nlohmann::json{"Japanese"} : pal ?
            nlohmann::json{"English", "French", "German", "Spanish", "Italian"} : nlohmann::json{"English"}}
    };
    if (japanese) manifest["voiceLanguages"] = {"Japanese", "English"};
    std::ofstream output(directory / "extraction_manifest.json", std::ios::binary);
    output << manifest.dump(2) << '\n';
    output.close();
    if (!output) throw std::runtime_error("Cannot write the content manifest");
}

std::vector<uint8_t> JapaneseSourceTables(const fs::path& path)
{
    CBinaryInputStream input(path);
    input.Skip(8);
    const bool debug = input.U8Read() != 0;
    input.Skip(2);
    const auto names = input.U32Read();
    if (debug)
        for (uint32_t i = 1; i < names; ++i) input.Skip(input.U16Read());
    const auto start = input.GetOffset();
    for (unsigned table = 0; table < 2; ++table)
    {
        const auto count = input.U16Read();
        for (unsigned i = 0; i < count; ++i) input.Skip(size_t(input.U16Read()) * 2);
    }
    std::vector<uint8_t> bytes(input.GetOffset() - start);
    input.Seek(start); input.ReadBytes(bytes.data(), bytes.size());
    return bytes;
}

void PackLocalizedLevel(const std::vector<fs::path>& paths, const fs::path& outputPath,
    bool japanese, const std::function<void()>& poll,
    const std::function<std::vector<uint8_t>(const fs::path&)>& read)
{
    static constexpr auto gearTable = [] {
        std::array<uint64_t, 256> values{};
        for (size_t i = 0; i < values.size(); ++i)
        {
            uint64_t gear = uint64_t(i) + 0x9E3779B97F4A7C15ull;
            gear = (gear ^ (gear >> 30)) * 0xBF58476D1CE4E5B9ull;
            gear = (gear ^ (gear >> 27)) * 0x94D049BB133111EBull;
            values[i] = gear ^ (gear >> 31);
        }
        return values;
    }();
    struct Chunk { std::vector<uint8_t> bytes; uint32_t crc; };
    std::vector<Chunk> chunks;
    std::unordered_map<uint64_t, std::vector<uint32_t>> chunkIds;
    std::vector<std::vector<uint32_t>> references(paths.size());
    std::vector<uint64_t> sizes;
    std::vector<uint8_t> japaneseLocalization;
    for (size_t language = 0; language < paths.size(); ++language)
    {
        const auto bytes = read(paths[language]);
        if (bytes.size() < 8 || bytes.size() > 256 * 1024 * 1024)
            throw std::runtime_error("Invalid level size");
        if (ReadLe32(bytes, 0) != 0x52424350 || ReadLe32(bytes, 4) != 17 ||
            ReadLe32(bytes, bytes.size() - 8) != 0x434f4c50)
            throw std::runtime_error("Converted level has no localization table");
        const auto localizationSize = ReadLe32(bytes, bytes.size() - 4);
        if (localizationSize > bytes.size() - 8) throw std::runtime_error("Invalid localization table");
        if (japanese)
        {
            std::vector<uint8_t> localization(bytes.end() - 8 - localizationSize, bytes.end() - 8);
            if (language == 0) japaneseLocalization = std::move(localization);
            else if (localization != japaneseLocalization)
                throw std::runtime_error("Japanese voice variants contain different subtitles");
        }
        for (size_t at = 0; at + 4 <= bytes.size(); ++at)
        {
            if (std::memcmp(bytes.data() + at, "VAG_", 4) != 0) continue;
            size_t end = at;
            const auto limit = (std::min)(bytes.size(), at + 32);
            while (end < limit && bytes[end]) ++end;
            if (end == limit) continue;
            const std::string name(reinterpret_cast<const char*>(bytes.data() + at), end - at);
            if (name.ends_with(".wav") &&
                !fs::is_regular_file(paths[language].parent_path() / "Dialogs" / name) &&
                !fs::is_regular_file(paths[language].parent_path() / "Commentary" / name))
                throw std::runtime_error("Missing localized audio: " + name);
            at = end;
        }
        sizes.push_back(bytes.size());
        size_t start = 0;
        uint64_t hash = 0;
        for (size_t i = 0; i < bytes.size(); ++i)
        {
            hash = (hash << 1) + gearTable[bytes[i]];
            const auto size = i - start + 1;
            if (i + 1 != bytes.size() && (size < 16 * 1024 ||
                (size < 256 * 1024 && (hash & 0xffff) != 0))) continue;
            const auto crc = uint32_t(crc32(0, bytes.data() + start, static_cast<uInt>(size)));
            auto& candidates = chunkIds[(uint64_t(crc) << 32) | size];
            auto found = std::find_if(candidates.begin(), candidates.end(), [&](uint32_t id) {
                return std::equal(chunks[id].bytes.begin(), chunks[id].bytes.end(), bytes.begin() + start);
            });
            if (found == candidates.end())
            {
                candidates.push_back(static_cast<uint32_t>(chunks.size()));
                chunks.push_back({{bytes.begin() + start, bytes.begin() + i + 1}, crc});
                references[language].push_back(candidates.back());
            }
            else references[language].push_back(*found);
            start = i + 1; hash = 0;
            poll();
        }
    }
    size_t fileSize = 28 + paths.size() * 16 + chunks.size() * 8;
    for (const auto& refs : references) fileSize += refs.size() * 4;
    for (const auto& chunk : chunks) fileSize += chunk.bytes.size();
    std::vector<char> buffer(fileSize + 64);
    std::ofstream output;
    output.rdbuf()->pubsetbuf(buffer.data(), buffer.size());
    output.open(outputPath, std::ios::binary);
    const auto u32 = [&](uint32_t value) {
        const uint8_t bytes[] = {uint8_t(value), uint8_t(value >> 8), uint8_t(value >> 16), uint8_t(value >> 24)};
        output.write(reinterpret_cast<const char*>(bytes), sizeof(bytes));
    };
    u32(0x52424350); u32(19); u32(0x494e5550); u32(1);
    u32(paths.size()); u32(0); u32(chunks.size());
    for (size_t language = 0; language < paths.size(); ++language)
    {
        u32(language); u32(uint32_t(sizes[language])); u32(uint32_t(sizes[language] >> 32));
        u32(references[language].size());
        for (const auto chunk : references[language]) u32(chunk);
    }
    for (const auto& chunk : chunks)
    {
        u32(chunk.bytes.size()); u32(chunk.crc);
        output.write(reinterpret_cast<const char*>(chunk.bytes.data()), chunk.bytes.size());
        poll();
    }
    output.close();
    if (!output) throw std::runtime_error("Cannot write " + outputPath.string());
}

void ExtractLocalizedGame(const fs::path& isoPath, const fs::path& directory,
    DiscRegion region, Session& session)
{
    const auto staging = directory / ".ProjectCane.extracting";
    fs::create_directories(directory);
    const auto marker = directory / ".ProjectCane.installing";
    { std::ofstream output(marker); output << '1'; output.close();
      if (!output) throw std::runtime_error("Cannot write to the game folder"); }
    fs::remove_all(staging);
    fs::create_directories(staging);
    std::mutex mutex;
    std::atomic<bool> cancelled = false;
    CaneExtract::Progress progress{"Checking your ISO", isoPath.filename().string(), 0, 1};
    const auto report = [&](const CaneExtract::Progress& update) {
        if (cancelled.load(std::memory_order_relaxed)) throw CaneExtract::Cancelled();
        std::lock_guard lock(mutex); progress = update;
    };
    auto task = std::async(std::launch::async, [&] {
        size_t extracted = 0;
        cane::pal::PalIsoExtractor extractor([&](const cane::pal::ExtractionProgress& update) {
            extracted = update.total;
            report({update.stage, update.file.filename().string(), update.completed, update.total + LEVEL_COUNT});
        });
        extractor.ExtractDetectedRegion(isoPath, staging);
        const bool japanese = region == DiscRegion::Japanese;
        const std::vector<std::string> languages = japanese ? std::vector<std::string>{"Japanese", "English"} :
            std::vector<std::string>{"English", "French", "German", "Spanish", "Italian"};
        std::set<fs::path> groups;
        size_t variants = 0;
        for (const auto& entry : fs::recursive_directory_iterator(staging / "Worlds"))
            if (entry.is_regular_file() && entry.path().extension() == ".brx")
            {
                const auto level = entry.path().parent_path().parent_path() / entry.path().filename();
                groups.insert(level);
                ++variants;
            }
        if (groups.size() != LEVEL_COUNT || variants != LEVEL_COUNT * languages.size())
            throw std::runtime_error("Some localized levels are missing");
        size_t completed = extracted;
        for (const auto& output : groups)
        {
            const auto update = CaneExtract::Progress{"Preparing worlds", output.filename().string(), completed, extracted + LEVEL_COUNT};
            report(update);
            const auto poll = [&] { report(update); };
            std::vector<fs::path> sources;
            std::vector<uint8_t> sourceTables;
            for (const auto& language : languages)
            {
                const auto source = output.parent_path() / language / output.filename();
                if (japanese)
                {
                    const auto tables = JapaneseSourceTables(source);
                    if (sources.empty()) sourceTables = tables;
                    else if (tables != sourceTables)
                        throw std::runtime_error("Japanese voice variants contain different localization tables");
                }
                sources.push_back(source);
                poll();
            }
            PackLocalizedLevel(sources, output, japanese, poll, [&](const fs::path& source) {
                BrxWalker walker(StringEncoding::Auto, japanese ? BrxSourceRegion::Japanese : BrxSourceRegion::Pal, false);
                if (!japanese) walker.LoadLevelIdentityMap(staging / ".pal-level-identities.txt");
                return walker.RebuildFile(source);
            });
            for (const auto& path : sources) fs::remove(path);
            ++completed;
        }
        if (!japanese) fs::remove(staging / ".pal-level-identities.txt");
        WriteManifest(staging, region, 19);
        report({"Finishing installation", "", completed, completed});
    });
    try
    {
        while (task.wait_for(std::chrono::milliseconds(50)) != std::future_status::ready)
        {
            { std::lock_guard lock(mutex); session.progress = progress; }
            session.Pulse();
        }
        task.get();
        session.progress.stage = "Finishing installation";
        session.Pulse(true);
    }
    catch (...)
    {
        cancelled.store(true, std::memory_order_relaxed);
        if (task.valid()) task.wait();
        throw;
    }
    for (const char* component : {"Worlds", "Cutscenes", "extraction_manifest.json"})
    {
        fs::remove_all(directory / component);
        fs::rename(staging / component, directory / component);
    }
    fs::remove(staging);
    fs::remove(marker);
}

bool CurrentLevel(const fs::path& path)
{
    std::ifstream input(path, std::ios::binary);
    uint32_t header[2]{};
    return input.read(reinterpret_cast<char*>(header), sizeof(header)) &&
        header[0] == 0x52424350 && header[1] >= 17 && header[1] <= 19;
}
void RebuildLevel(const fs::path& path)
{
    auto rebuild = std::async(std::launch::async, [path] {
        BrxWalker(StringEncoding::Auto, BrxSourceRegion::NtscU, false).WalkFile(path);
    });
    while (rebuild.wait_for(std::chrono::milliseconds(50)) != std::future_status::ready) Pulse();
    rebuild.get();
    auto output = path.parent_path() / (path.stem().string() + "_Rebuilt.brx");
    if (!CurrentLevel(output)) throw std::runtime_error("Level conversion failed: " + path.string());
#ifdef __SWITCH__
    if (!CaneIo::AtomicReplace(output, path)) throw std::runtime_error("Cannot replace " + path.string());
#else
    fs::rename(output, path);
#endif
}
std::vector<fs::path> RetailLevels(const fs::path& directory)
{
    std::vector<fs::path> levels;
    std::error_code error;
    if (!fs::is_directory(directory / "Worlds", error)) return levels;
#ifdef __SWITCH__
    for (const auto& entry : fs::recursive_directory_iterator(directory / "Worlds"))
        if (entry.path().extension() == ".previous" && entry.path().stem().extension() == ".brx")
            CaneIo::RecoverFile(entry.path().parent_path() / entry.path().stem());
#endif
    for (const auto& entry : fs::recursive_directory_iterator(directory / "Worlds"))
        if (entry.is_regular_file() && entry.path().extension() == ".brx" &&
            !entry.path().stem().string().ends_with("_Rebuilt") && !CurrentLevel(entry.path()))
            levels.push_back(entry.path());
    return levels;
}
}

bool CaneExtract::NeedsRebuild(const fs::path& directory)
{
    const auto levels = RetailLevels(directory);
    return fs::is_regular_file(directory / "Worlds/Splash/Splash.brx") &&
        !fs::exists(directory / ".ProjectCane.installing") && !levels.empty();
}

void CaneExtract::RebuildGame(const fs::path& directory, const ProgressCallback& callback)
{
    const auto levels = RetailLevels(directory);
    Session session(callback, 0);
    session.progress.total = levels.size();
    for (const auto& path : levels)
    {
        session.Step("Updating game files", path.filename().string());
        RebuildLevel(path);
    }
}

bool CaneExtract::IsReady(const fs::path& directory)
{
    std::error_code error;
    if (fs::exists(directory / ".ProjectCane.installing", error) || error) return false;
    if (!fs::is_directory(directory / "Worlds", error) || error) return false;
    if (CurrentLevel(directory / "Worlds" / "Splash" / "Splash.brx") && RetailLevels(directory).empty())
        return true;
    error.clear();
    for (fs::recursive_directory_iterator it(directory / "Worlds", fs::directory_options::skip_permission_denied, error), end;
         it != end && !error; it.increment(error))
        if (it->path().filename() == "Splash.brx" && CurrentLevel(it->path()) && RetailLevels(directory).empty())
            return true;
    return false;
}

void CaneExtract::ExtractGame(const fs::path& isoPath, const fs::path& outputDirectory, const ProgressCallback& callback)
{
    const fs::path worldsDirectory = outputDirectory / "Worlds";
    const fs::path cutsceneDirectory = outputDirectory / "Cutscenes";
    const fs::path musicDirectory = worldsDirectory / "Music";
    const fs::path soundBankDirectory = worldsDirectory / "Sound Banks";
    const fs::path commentaryDirectory = worldsDirectory / "Sounds" / "Commentary";
    const fs::path jloAmbientDirectory = worldsDirectory / "Sounds" / "JLO Ambient";
    const fs::path promptSoundDirectory = worldsDirectory / "Sounds" / "Prompt";

    Session session(callback, fs::file_size(isoPath));
    session.progress.total = LEVEL_COUNT + std::size(MUSIC_TABLE) + SOUND_BANK_COUNT +
        COMMENTARY_COUNT + std::size(DIALOG_AUDIO_TABLE) + EXTRACTED_CUTSCENE_COUNT + JLO_AMBIENT_COUNT + 1;
    session.progress.stage = "Checking your ISO";
    session.progress.file = isoPath.filename().string();
    session.Pulse(true);

    if (!fs::exists(isoPath))
    {
        throw std::runtime_error((std::ostringstream{} << "ISO does not exist:\n" << isoPath << '\n').str());
    }

    std::ifstream iso(isoPath, std::ios::binary);

    if (!iso.is_open())
    {
        throw std::runtime_error((std::ostringstream{} << "Could not open ISO:\n" << isoPath << '\n').str());
    }

    const auto region = cane::pal::PalIsoExtractor::DetectRegion(isoPath);
    if (region != DiscRegion::NtscU)
    {
        iso.close();
        ExtractLocalizedGame(isoPath, outputDirectory, region, session);
        return;
    }

    if (!ValidateIso(iso))
    {
        throw std::runtime_error("This USA ISO is unsupported or damaged.");
    }

    fs::create_directories(outputDirectory);
    const fs::path marker = outputDirectory / ".ProjectCane.installing";
    std::ofstream incomplete(marker, std::ios::binary);
    incomplete << "1";
    incomplete.close();
    if (!incomplete) throw std::runtime_error("Cannot write to the game folder. Check the SD card.");

    std::array<LEVELINFODisk, LEVEL_COUNT> levelTable{};

    if (!ReadExactAt(iso, LEVEL_TABLE_ISO_OFFSET, levelTable.data(), sizeof(levelTable)))
    {
        throw std::runtime_error((std::ostringstream{} << "Failed to read LEVELINFO table at ISO offset 0x" << std::hex << LEVEL_TABLE_ISO_OFFSET << std::dec << ".\n").str());
    }

    std::array<VAGINFODisk, SOUND_BANK_COUNT> soundBankTable{};

    if (!ReadExactAt(iso, SOUND_BANK_TABLE_ISO_OFFSET, soundBankTable.data(), sizeof(soundBankTable)))
    {
        throw std::runtime_error((std::ostringstream{} << "Failed to read sound bank table at ISO offset 0x" << std::hex
                  << SOUND_BANK_TABLE_ISO_OFFSET << std::dec << ".\n").str());
    }

    std::array<COMMENTARYINFODisk, COMMENTARY_COUNT> commentaryTable{};

    if (!ReadExactAt(iso, COMMENTARY_TABLE_ISO_OFFSET, commentaryTable.data(), sizeof(commentaryTable)))
    {
        throw std::runtime_error((std::ostringstream{} << "Failed to read commentary table at ISO offset 0x" << std::hex
                  << COMMENTARY_TABLE_ISO_OFFSET << std::dec << ".\n").str());
    }

    std::array<VAGINFODisk, JLO_AMBIENT_COUNT> jloAmbientTable{};

    if (!ReadExactAt(iso, JLO_AMBIENT_TABLE_ISO_OFFSET, jloAmbientTable.data(), sizeof(jloAmbientTable)))
    {
        throw std::runtime_error((std::ostringstream{} << "Failed to read JLO ambient VAG table at ISO offset 0x" << std::hex
                  << JLO_AMBIENT_TABLE_ISO_OFFSET << std::dec << ".\n").str());
    }

    std::array<VAGINFODisk, CUTSCENE_COUNT> cutsceneTable{};

    if (!ReadExactAt(iso, CUTSCENE_TABLE_ISO_OFFSET, cutsceneTable.data(), sizeof(cutsceneTable)))
    {
        throw std::runtime_error((std::ostringstream{} << "Failed to read cutscene table at ISO offset 0x" << std::hex
                  << CUTSCENE_TABLE_ISO_OFFSET << std::dec << ".\n").str());
    }

    VAGINFODisk promptVagInfo{};

    if (!ReadExactAt(iso, PROMPT_VAG_INFO_ISO_OFFSET, &promptVagInfo, sizeof(promptVagInfo)))
    {
        throw std::runtime_error((std::ostringstream{} << "Failed to read prompt VAG descriptor at ISO offset 0x" << std::hex
                  << PROMPT_VAG_INFO_ISO_OFFSET << std::dec << ".\n").str());
    }

    std::vector<std::pair<uint32_t, std::string>> levelNames;
    levelNames.reserve(levelTable.size());

    for (std::size_t index = 0; index < levelTable.size(); ++index)
    {
        session.Step("Extracting worlds", "World " + std::to_string(index + 1) + " of " + std::to_string(levelTable.size()));
        const LEVELINFODisk& levelInfo = levelTable[index];

        const uint32_t sector = levelInfo.fileInfo.fcl.isector ^ levelInfo.lsnKey;
        const uint32_t compressedSize = levelInfo.fileInfo.fcl.cb ^ levelInfo.sizeKey;

        const uint64_t dataOffset = static_cast<uint64_t>(sector) * ISO_SECTOR_SIZE;

        std::string levelName;

        if (!ReadExeStringFromIso(iso, levelInfo.levelName, levelName))
            levelName = "Level_" + std::to_string(index);

        levelName = SanitizeFileName(levelName);

        if ((levelInfo.levelId & 0xFFFFU) == 0x0003U)
            levelName = "Paris";

        levelNames.emplace_back(levelInfo.levelId, levelName);

        const fs::path levelOutputDirectory =
            GetLevelOutputDirectory(worldsDirectory, levelInfo.levelId) / levelName;
        std::error_code directoryError;
        fs::create_directories(levelOutputDirectory, directoryError);

        if (directoryError)
        {
            throw std::runtime_error((std::ostringstream{} << "Could not create output directory:\n" << levelOutputDirectory << '\n').str());
        }

        const fs::path outputPath = levelOutputDirectory / (levelName + ".brx");

        CheckIsoRange(dataOffset, compressedSize);
        std::vector<uint8_t> compressed(compressedSize);

        if (!ReadExactAt(iso, dataOffset, compressed.data(), compressed.size()))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed to read " << compressedSize << " bytes from ISO offset 0x" << std::hex << dataOffset << std::dec << " for " << levelName << ".\n").str());
        }

        const std::vector<uint8_t> decompressed = Decompress(compressed);

        if (!WriteBinaryFile(outputPath, decompressed))
            throw std::runtime_error("Cannot write " + outputPath.string() + ". Check available SD space.");
        session.progress.stage = "Preparing worlds";
        session.progress.file = levelName;
        RebuildLevel(outputPath);
    }

    std::error_code musicDirectoryError;
    fs::create_directories(musicDirectory, musicDirectoryError);

    if (musicDirectoryError)
    {
        throw std::runtime_error((std::ostringstream{} << "Could not create music output directory:\n" << musicDirectory << '\n').str());
    }

    std::ofstream musicManifest(musicDirectory / "MusicMap.txt");
    if (!musicManifest.is_open())
    {
        throw std::runtime_error((std::ostringstream{} << "Could not create music manifest:\n" << (musicDirectory / "MusicMap.txt") << '\n').str());
    }

    std::vector<uint32_t> extractedMusicIds;
    for (const MUSICINFODisk& musicInfo : MUSIC_TABLE)
    {
        session.Step("Extracting music", musicInfo.levelName);
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
        CheckIsoRange(dataOffset, musicInfo.size);
        std::vector<uint8_t> musicData(musicInfo.size);
        if (!ReadExactAt(iso, dataOffset, musicData.data(), musicData.size()))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed to read MUSID 0x" << std::hex << musicInfo.musicId
                << " from ISO offset 0x" << dataOffset << std::dec << ".\n").str());
        }

        char musicFileName[32]{};
        std::snprintf(musicFileName, sizeof(musicFileName), "MUSID_%02X.mus", musicInfo.musicId);
        const fs::path musicOutputPath = musicDirectory / musicFileName;
        if (!WriteBinaryFile(musicOutputPath, musicData))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed writing music output:\n" << musicOutputPath << '\n').str());
        }

		extractedMusicIds.push_back(musicInfo.musicId);
    }

    std::error_code soundBankDirectoryError;
    fs::create_directories(soundBankDirectory, soundBankDirectoryError);

    if (soundBankDirectoryError)
    {
        throw std::runtime_error((std::ostringstream{} << "Could not create sound bank output directory:\n" << soundBankDirectory << '\n').str());
    }

    for (std::size_t index = 0; index < soundBankTable.size(); ++index)
    {
        session.Step("Extracting sound banks", SOUND_BANK_NAMES[index]);
        const VAGINFODisk& descriptor = soundBankTable[index];
        const uint32_t sector = descriptor.encryptedSector ^ descriptor.sectorKey;
        const uint32_t size = descriptor.encryptedSize ^ descriptor.sizeKey;
        const uint64_t dataOffset = static_cast<uint64_t>(sector) * ISO_SECTOR_SIZE;

        if (sector == 0 || size == 0)
        {
            throw std::runtime_error((std::ostringstream{} << "Invalid sound bank descriptor for IBNK " << index << " ("
                      << SOUND_BANK_NAMES[index] << ").\n").str());
        }

        CheckIsoRange(dataOffset, size);
        std::vector<uint8_t> bankData(size);
        if (!ReadExactAt(iso, dataOffset, bankData.data(), bankData.size()))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed to read sound bank " << SOUND_BANK_NAMES[index]
                      << " from ISO offset 0x" << std::hex << dataOffset << std::dec << ".\n").str());
        }

        const fs::path bankOutputPath = soundBankDirectory /
            (std::string(SOUND_BANK_NAMES[index]) + ".bnk");
        if (!WriteBinaryFile(bankOutputPath, bankData))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed writing sound bank:\n" << bankOutputPath << '\n').str());
        }
    }

    std::error_code commentaryDirectoryError;
    fs::create_directories(commentaryDirectory, commentaryDirectoryError);

    if (commentaryDirectoryError)
    {
        throw std::runtime_error((std::ostringstream{} << "Could not create commentary output directory:\n" << commentaryDirectory << '\n').str());
    }

    const fs::path commentaryManifestPath = commentaryDirectory / "CommentaryMap.txt";
    std::ofstream commentaryManifest(commentaryManifestPath);
    if (!commentaryManifest.is_open())
    {
        throw std::runtime_error((std::ostringstream{} << "Could not create commentary manifest:\n" << commentaryManifestPath << '\n').str());
    }

    for (std::size_t index = 0; index < commentaryTable.size(); ++index)
    {
        session.Step("Extracting commentary", "Track " + std::to_string(index + 1));
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

            continue;
        }

        const uint64_t dataOffset = static_cast<uint64_t>(sector) * ISO_SECTOR_SIZE;
        CheckIsoRange(dataOffset, size);
        std::vector<uint8_t> commentaryData(size);
        if (!ReadExactAt(iso, dataOffset, commentaryData.data(), commentaryData.size()))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed to read commentary for level ID 0x" << std::hex
                      << commentary.levelId << " from ISO offset 0x" << dataOffset << std::dec << ".\n").str());
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
            throw std::runtime_error((std::ostringstream{} << "Failed converting commentary audio to WAV: "
                      << commentaryConversionError << "\n" << commentaryOutputPath << '\n').str());
        }

        commentaryManifest << commentaryFileName
            << " | levelId 0x" << std::hex << std::uppercase << commentary.levelId
            << " | duration " << std::dec << commentary.audioDuration
            << " | identity 0x" << std::hex << identity
            << " | sector 0x" << sector
            << " | bytes 0x" << size
            << " | speakers " << speakerNames
            << std::nouppercase << std::dec << '\n';
    }

    std::vector<uint32_t> initializedDialogManifests;

    for (const DIALOGAUDIOINFO& dialogAudio : DIALOG_AUDIO_TABLE)
    {
        session.Step("Extracting dialogue", "Track " + std::to_string(session.progress.completed - LEVEL_COUNT - std::size(MUSIC_TABLE) - SOUND_BANK_COUNT - COMMENTARY_COUNT + 1));
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
            GetDialogOutputDirectory(worldsDirectory, dialogAudio.levelId) /
            SanitizeFileName(levelName) / "Dialog";

        std::error_code levelDialogDirectoryError;
        fs::create_directories(levelDialogDirectory, levelDialogDirectoryError);
        if (levelDialogDirectoryError)
        {
            throw std::runtime_error((std::ostringstream{} << "Could not create level dialog directory:\n" << levelDialogDirectory << '\n').str());
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
        uint32_t audioSize = dialogAudio.size;
        if (audioSize == 0)
        {
            std::vector<uint8_t> vagHeader(0x30);
            if (!ReadExactAt(iso, dataOffset, vagHeader.data(), vagHeader.size()) ||
                vagHeader[0] != 'V' || vagHeader[1] != 'A' ||
                vagHeader[2] != 'G' || vagHeader[3] != 'p')
            {
                throw std::runtime_error((std::ostringstream{} << "Failed reading dialog VAG header at ISO sector 0x"
                          << std::hex << dialogAudio.sector << std::dec << ".\n").str());
            }
            audioSize = ReadBe32(vagHeader, 0x0C) + 0x30;
        }
        CheckIsoRange(dataOffset, audioSize);
        std::vector<uint8_t> audioData(audioSize);
        if (!ReadExactAt(iso, dataOffset, audioData.data(), audioData.size()))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed to read dialog VAG 0x" << std::hex << dialogAudio.identity
                      << " from ISO offset 0x" << dataOffset << std::dec << ".\n").str());
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
            throw std::runtime_error((std::ostringstream{} << "Could not convert dialog VAG 0x" << std::hex << std::uppercase
                      << dialogAudio.identity << std::nouppercase << std::dec
                      << " to WAV: " << conversionError << '\n').str());
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
            throw std::runtime_error((std::ostringstream{} << "Could not write dialog manifest in:\n" << levelDialogDirectory << '\n').str());
        }

        dialogManifest << audioFileName
            << " | levelId 0x" << std::hex << std::uppercase << dialogAudio.levelId
            << " | identity 0x" << dialogAudio.identity
            << " | sector 0x" << dialogAudio.sector
            << " | bytes 0x" << audioSize
            << " | source " << (dialogAudio.source == DialogAudioSource::DIALOG ? "DIALOG" : "ASEG")
            << std::nouppercase << std::dec << '\n';
    }

    std::error_code cutsceneDirectoryError;
    fs::create_directories(cutsceneDirectory, cutsceneDirectoryError);

    if (cutsceneDirectoryError)
    {
        throw std::runtime_error((std::ostringstream{} << "Could not create cutscene output directory:\n" << cutsceneDirectory << '\n').str());
    }

    for (std::size_t index = 0; index < EXTRACTED_CUTSCENE_COUNT; ++index)
    {
        session.Step("Preparing cutscenes", "Cutscene " + std::to_string(index + 1) + " of " + std::to_string(EXTRACTED_CUTSCENE_COUNT));
        const bool isStartupSplash = index == CUTSCENE_COUNT;
        const VAGINFODisk* cutsceneInfo = isStartupSplash ? nullptr : &cutsceneTable[index];
        const uint32_t sector = isStartupSplash ? STARTUP_SPLASH_SECTOR :
            cutsceneInfo->encryptedSector ^ cutsceneInfo->sectorKey;
        const uint32_t size = isStartupSplash ? STARTUP_SPLASH_SIZE :
            cutsceneInfo->encryptedSize ^ cutsceneInfo->sizeKey;
        const uint64_t dataOffset = static_cast<uint64_t>(sector) * ISO_SECTOR_SIZE;

        const std::size_t outputIndex = isStartupSplash ? 17 : (index == 0 ? 16 : index - 1);
        std::string fileName = "Cutscene_";

        if (outputIndex < 10)
            fileName += '0';

        fileName += std::to_string(outputIndex);
        const fs::path videoOutputPath = cutsceneDirectory / (fileName + ".m2v");
        const fs::path audioOutputPath = cutsceneDirectory / (fileName + ".wav");

        if (sector == 0 || size == 0)
        {
            throw std::runtime_error((std::ostringstream{} << "Invalid decrypted cutscene descriptor " << index << ".\n").str());
        }

        CheckIsoRange(dataOffset, size);
        std::vector<uint8_t> movieData(size);

        if (!ReadExactAt(iso, dataOffset, movieData.data(), movieData.size()))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed to read cutscene " << index << " from ISO offset 0x"
                      << std::hex << dataOffset << std::dec << ".\n").str());
        }

        DemuxedPss demuxed;
        std::vector<uint8_t> wavData;

        if (!DemuxPss(movieData, demuxed))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed to demux cutscene " << index << ": "
                      << demuxed.error << ".\n").str());
        }

        if (!DecodePssAudioToWav(demuxed, wavData))
        {
            throw std::runtime_error((std::ostringstream{} << "Unsupported or invalid cutscene audio in cutscene " << index
                      << " (format " << demuxed.audioFormat << ").\n").str());
        }

        if (!WriteBinaryFile(videoOutputPath, demuxed.video) ||
            !WriteBinaryFile(audioOutputPath, wavData))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed writing cutscene outputs:\n"
                      << videoOutputPath << '\n' << audioOutputPath << '\n').str());
        }
    }

    std::error_code soundDirectoryError;
    fs::create_directories(jloAmbientDirectory, soundDirectoryError);

    if (soundDirectoryError)
    {
        throw std::runtime_error((std::ostringstream{} << "Could not create sound output directory:\n" << jloAmbientDirectory << '\n').str());
    }

    for (std::size_t index = 0; index < jloAmbientTable.size(); ++index)
    {
        session.Step("Extracting ambient sounds", "Track " + std::to_string(index + 1));
        const VAGINFODisk& vagInfo = jloAmbientTable[index];
        const uint32_t sector = vagInfo.encryptedSector ^ vagInfo.sectorKey;
        const uint32_t size = vagInfo.encryptedSize ^ vagInfo.sizeKey;
        const uint64_t dataOffset = static_cast<uint64_t>(sector) * ISO_SECTOR_SIZE;
        const fs::path outputPath = jloAmbientDirectory / ("JloAmbient_" + std::to_string(index + 1) + ".wav");

        if (sector == 0 || size == 0)
        {
            throw std::runtime_error((std::ostringstream{} << "Invalid decrypted JLO ambient descriptor " << index << ".\n").str());
        }

        CheckIsoRange(dataOffset, size);
        std::vector<uint8_t> vagData(size);

        if (!ReadExactAt(iso, dataOffset, vagData.data(), vagData.size()))
        {
            throw std::runtime_error((std::ostringstream{} << "Failed to read JLO ambient " << index + 1 << " from ISO offset 0x"
                      << std::hex << dataOffset << std::dec << ".\n").str());
        }

        std::vector<uint8_t> wavData;
        std::string conversionError;
        if (!DecodeStandaloneVagToWav(vagData, wavData, conversionError) ||
            !WriteBinaryFile(outputPath, wavData))
        {
            if (conversionError.empty())
                conversionError = "failed writing WAV file";
            throw std::runtime_error((std::ostringstream{} << "Failed converting JLO ambient " << index + 1
                      << " to WAV: " << conversionError << '\n').str());
        }
    }

    std::error_code promptDirectoryError;
    fs::create_directories(promptSoundDirectory, promptDirectoryError);

    if (promptDirectoryError)
    {
        throw std::runtime_error((std::ostringstream{} << "Could not create prompt sound output directory:\n" << promptSoundDirectory << '\n').str());
    }

    session.Step("Finishing audio", "Menu sounds");
    const uint32_t promptSector = promptVagInfo.encryptedSector ^ promptVagInfo.sectorKey;
    const uint32_t promptSize = promptVagInfo.encryptedSize ^ promptVagInfo.sizeKey;
    const uint64_t promptDataOffset = static_cast<uint64_t>(promptSector) * ISO_SECTOR_SIZE;
    const fs::path promptOutputPath = promptSoundDirectory / "PromptPrk18.wav";

    if (promptSector == 0 || promptSize == 0)
    {
        throw std::runtime_error((std::ostringstream{} << "Invalid decrypted prompt VAG descriptor.\n").str());
    }

    CheckIsoRange(promptDataOffset, promptSize);
    std::vector<uint8_t> promptVagData(promptSize);

    if (!ReadExactAt(iso, promptDataOffset, promptVagData.data(), promptVagData.size()))
    {
        throw std::runtime_error((std::ostringstream{} << "Failed to read prompt VAG from ISO offset 0x" << std::hex
                  << promptDataOffset << std::dec << ".\n").str());
    }

    std::vector<uint8_t> promptWavData;
    std::string promptConversionError;
    if (!DecodeStandaloneVagToWav(promptVagData, promptWavData, promptConversionError) ||
        !WriteBinaryFile(promptOutputPath, promptWavData))
    {
        if (promptConversionError.empty())
            promptConversionError = "failed writing WAV file";
        throw std::runtime_error((std::ostringstream{} << "Failed converting prompt VAG to WAV: "
                  << promptConversionError << '\n').str());
    }

    musicManifest.close();
    commentaryManifest.close();
    if (!musicManifest || !commentaryManifest)
        throw std::runtime_error("Cannot finish game files. Check available SD space.");
    session.progress.completed = session.progress.total;
    session.progress.stage = "Game ready";
    session.Pulse(true);
    WriteManifest(outputDirectory, DiscRegion::NtscU, 17);
    fs::remove(marker);
}
