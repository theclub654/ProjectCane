#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "sound.h"
#include "alo.h"
#include "lo.h"
#include "po.h"
#include "sw.h"
#include "clock.h"
#include "cm.h"
#include "game.h"
#include "util.h"
#include <Windows.h>
#include <mmsystem.h>
#include <xaudio2.h>
#include <xaudio2fx.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cwchar>
#include <cwctype>
#include <cstdlib>
#include <iterator>
#include <array>
#include <chrono>
#include <cmath>
#include <memory>
#include <algorithm>
#include <nlohmann/json.hpp>

const glm::vec3* PposSoundEar()
{
	PO* ppo = PpoCur();
	if (ppo != nullptr && FIsPoSoundBase(ppo))
		return &ppo->xf.posWorld;
	return g_pcm != nullptr ? &g_pcm->pos : nullptr;
}

template <typename T>
bool ReadBankValue(const std::vector<uint8_t>& bytes, std::size_t offset, T& value)
{
	if (offset > bytes.size() || sizeof(T) > bytes.size() - offset)
		return false;
	std::memcpy(&value, bytes.data() + offset, sizeof(T));
	return true;
}

bool Parse989SfxBank(EFFECTBANK& bank)
{
	uint32_t fileType = 0;
	uint32_t chunkCount = 0;
	if (!ReadBankValue(bank.bytes, 0, fileType) || fileType != 3 ||
		!ReadBankValue(bank.bytes, 4, chunkCount) || chunkCount < 2)
		return false;

	uint32_t metadataOffset = 0;
	uint32_t metadataSize = 0;
	if (!ReadBankValue(bank.bytes, 8, metadataOffset) ||
		!ReadBankValue(bank.bytes, 12, metadataSize) ||
		!ReadBankValue(bank.bytes, 16, bank.sampleOffset) ||
		!ReadBankValue(bank.bytes, 20, bank.sampleSize))
		return false;
	if (metadataOffset > bank.bytes.size() || metadataSize > bank.bytes.size() - metadataOffset ||
		bank.sampleOffset > bank.bytes.size() || bank.sampleSize > bank.bytes.size() - bank.sampleOffset)
		return false;

	constexpr uint32_t kSblk =
		static_cast<uint32_t>('S') | (static_cast<uint32_t>('B') << 8) |
		(static_cast<uint32_t>('l') << 16) | (static_cast<uint32_t>('k') << 24);
	uint32_t dataId = 0;
	uint16_t soundCount = 0;
	uint32_t firstSound = 0;
	uint32_t firstGrain = 0;
	if (!ReadBankValue(bank.bytes, metadataOffset, dataId) || dataId != kSblk ||
		!ReadBankValue(bank.bytes, metadataOffset + 4, bank.version) ||
		!ReadBankValue(bank.bytes, metadataOffset + 12, bank.bankId) ||
		!ReadBankValue(bank.bytes, metadataOffset + 22, soundCount) ||
		!ReadBankValue(bank.bytes, metadataOffset + 28, firstSound) ||
		!ReadBankValue(bank.bytes, metadataOffset + 32, firstGrain))
		return false;

	constexpr std::size_t kSoundRecordSize = 12;
	const std::size_t soundTable = static_cast<std::size_t>(metadataOffset) + firstSound;
	if (soundTable > bank.bytes.size() ||
		static_cast<std::size_t>(soundCount) > (bank.bytes.size() - soundTable) / kSoundRecordSize)
		return false;

	bank.sounds.clear();
	bank.sounds.reserve(soundCount);
	for (uint16_t i = 0; i < soundCount; ++i)
	{
		const std::size_t record = soundTable + static_cast<std::size_t>(i) * kSoundRecordSize;
		EFFECTBANK::SOUND sound{};
		int8_t grainCount = 0;
		uint32_t relativeGrain = 0;
		if (!ReadBankValue(bank.bytes, record + 0, sound.volume) ||
			!ReadBankValue(bank.bytes, record + 1, sound.volumeGroup) ||
			!ReadBankValue(bank.bytes, record + 2, sound.pan) ||
			!ReadBankValue(bank.bytes, record + 4, grainCount) || grainCount < 0 ||
			!ReadBankValue(bank.bytes, record + 5, sound.instanceLimit) ||
			!ReadBankValue(bank.bytes, record + 6, sound.flags) ||
			!ReadBankValue(bank.bytes, record + 8, relativeGrain))
			return false;
		sound.grainCount = static_cast<uint8_t>(grainCount);
		sound.firstGrainOffset = firstGrain + relativeGrain;
		bank.sounds.push_back(sound);
	}

	constexpr std::size_t kV1GrainSize = 0x28;
	for (EFFECTBANK::SOUND& sound : bank.sounds)
	{
		if (sound.grainCount == 0)
			continue;
		const std::size_t grainTable = static_cast<std::size_t>(metadataOffset) + sound.firstGrainOffset;
		if (grainTable > metadataOffset + metadataSize ||
			static_cast<std::size_t>(sound.grainCount) >
			(metadataOffset + metadataSize - grainTable) / kV1GrainSize)
			return false;

		sound.grains.reserve(sound.grainCount);
		for (uint8_t igrain = 0; igrain < sound.grainCount; ++igrain)
		{
			const std::size_t record = grainTable + static_cast<std::size_t>(igrain) * kV1GrainSize;
			EFFECTBANK::GRAIN grain{};
			if (!ReadBankValue(bank.bytes, record, grain.type) ||
				!ReadBankValue(bank.bytes, record + 4, grain.delay))
				return false;
			std::memcpy(grain.data.data(), bank.bytes.data() + record + 8, grain.data.size());
			if (grain.type == 1 || grain.type == 9) // TONE / TONE2
			{
				grain.hasTone = true;
				const std::size_t tone = record + 8;
				if (!ReadBankValue(bank.bytes, tone + 0, grain.tone.priority) ||
					!ReadBankValue(bank.bytes, tone + 1, grain.tone.volume) ||
					!ReadBankValue(bank.bytes, tone + 2, grain.tone.centerNote) ||
					!ReadBankValue(bank.bytes, tone + 3, grain.tone.centerFine) ||
					!ReadBankValue(bank.bytes, tone + 4, grain.tone.pan) ||
					!ReadBankValue(bank.bytes, tone + 6, grain.tone.mapLow) ||
					!ReadBankValue(bank.bytes, tone + 7, grain.tone.mapHigh) ||
					!ReadBankValue(bank.bytes, tone + 8, grain.tone.pitchBendLow) ||
					!ReadBankValue(bank.bytes, tone + 9, grain.tone.pitchBendHigh) ||
					!ReadBankValue(bank.bytes, tone + 10, grain.tone.adsr1) ||
					!ReadBankValue(bank.bytes, tone + 12, grain.tone.adsr2) ||
					!ReadBankValue(bank.bytes, tone + 14, grain.tone.flags) ||
					!ReadBankValue(bank.bytes, tone + 16, grain.tone.sampleOffset) ||
					!ReadBankValue(bank.bytes, tone + 20, grain.tone.reserved))
					return false;
			}
			sound.grains.push_back(grain);
		}
	}
	return true;
}


std::filesystem::path ExecutableDirectory()
{
    wchar_t executablePath[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
        return std::filesystem::current_path();
    return std::filesystem::path(executablePath).parent_path();
}

bool IsReadablePath(const char* value)
{
    if (!value || !value[0])
        return false;

    // Encrypted descriptors are exactly 32 bytes, so never probe beyond that
    // boundary while deciding whether this is a descriptor or a C string.
    for (std::size_t i = 0; i < 32; ++i)
    {
        if (value[i] == '\0')
            return i != 0;
        const unsigned char ch = static_cast<unsigned char>(value[i]);
        if (ch < 0x20 || ch > 0x7e)
            return false;
    }
    return true;
}

bool MatchesInsensitive(const std::filesystem::path& path, const std::wstring& name)
{
    std::wstring candidate = path.filename().wstring();
    if (candidate.size() != name.size())
        return false;

    for (std::size_t i = 0; i < name.size(); ++i)
    {
        if (std::towlower(candidate[i]) != std::towlower(name[i]))
            return false;
    }
    return true;
}

std::filesystem::path FindExtractedWav(const std::wstring& fileName)
{
    const std::filesystem::path executableDirectory = ExecutableDirectory();
    const std::filesystem::path repositoryDirectory = std::filesystem::current_path();
    const std::filesystem::path executableReposDirectory =
        executableDirectory.parent_path().parent_path().parent_path();
    const std::filesystem::path roots[] =
    {
        executableDirectory / L"Worlds" / L"Sounds",
        std::filesystem::current_path() / L"Worlds" / L"Sounds",
        std::filesystem::current_path() / L"Sly1-Release" / L"Worlds" / L"Sounds",
        // Development fallback: allow ProjectCane to use a freshly generated
        // extractor asset tree without duplicating roughly 500 MB of dialog.
        repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" /
            L"Debug" / L"Worlds" / L"Sounds",
        repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" /
            L"Release" / L"Worlds" / L"Sounds",
        executableReposDirectory / L"Sly1 File Extractor" / L"x64" /
            L"Debug" / L"Worlds" / L"Sounds",
        executableReposDirectory / L"Sly1 File Extractor" / L"x64" /
            L"Release" / L"Worlds" / L"Sounds"
    };

    for (const std::filesystem::path& root : roots)
    {
        std::error_code error;
        if (!std::filesystem::is_directory(root, error))
            continue;

        for (std::filesystem::recursive_directory_iterator iter(
                 root, std::filesystem::directory_options::skip_permission_denied, error), end;
             iter != end; iter.increment(error))
        {
            if (error)
            {
                error.clear();
                continue;
            }
            if (iter->is_regular_file(error) && MatchesInsensitive(iter->path(), fileName))
                return iter->path();
        }
    }

    return {};
}

std::filesystem::path FindCommentaryWav(uint32_t identity, uint32_t sector)
{
    const std::filesystem::path mapPath = FindExtractedWav(L"CommentaryMap.txt");
    if (mapPath.empty())
        return {};

    std::ifstream map(mapPath);
    std::string line;
    while (std::getline(map, line))
    {
        char fileName[260]{};
        unsigned int mapIdentity = 0;
        unsigned int mapSector = 0;
        if (std::sscanf(line.c_str(),
                "%259s | levelId 0x%*x | duration %*f | identity 0x%x | sector 0x%x",
                fileName, &mapIdentity, &mapSector) != 3)
        {
            continue;
        }

        if (mapIdentity != identity || mapSector != sector)
            continue;

        const std::filesystem::path commentaryPath = mapPath.parent_path() / fileName;
        std::error_code error;
        if (std::filesystem::is_regular_file(commentaryPath, error))
            return commentaryPath;

        return FindExtractedWav(commentaryPath.filename().wstring());
    }

    return {};
}

std::filesystem::path FindEffectBank(int ibnk)
{
	if (ibnk < 0 || ibnk >= static_cast<int>(kEffectBankNames.size()))
		return {};
	const std::filesystem::path executableDirectory = ExecutableDirectory();
	const std::filesystem::path repositoryDirectory = std::filesystem::current_path();
	const std::filesystem::path executableReposDirectory =
		executableDirectory.parent_path().parent_path().parent_path();
	const std::filesystem::path roots[] =
	{
		executableDirectory / L"Worlds" / L"Sound Banks",
		repositoryDirectory / L"Worlds" / L"Sound Banks",
		repositoryDirectory / L"Sly1-Release" / L"Worlds" / L"Sound Banks",
		repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" / L"Debug" / L"Worlds" / L"Sound Banks",
		repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" / L"Release" / L"Worlds" / L"Sound Banks",
		executableReposDirectory / L"Sly1 File Extractor" / L"x64" / L"Debug" / L"Worlds" / L"Sound Banks",
		executableReposDirectory / L"Sly1 File Extractor" / L"x64" / L"Release" / L"Worlds" / L"Sound Banks"
	};
	for (const std::filesystem::path& root : roots)
	{
		const std::filesystem::path candidate = root / kEffectBankNames[ibnk];
		std::error_code error;
		if (std::filesystem::is_regular_file(candidate, error))
			return candidate;
	}
	return {};
}

std::filesystem::path FindMusicBank(MUSID musid)
{
	wchar_t bankName[32]{};
	std::swprintf(bankName, std::size(bankName), L"MUSID_%02X", static_cast<unsigned>(musid));
	const std::filesystem::path executableDirectory = ExecutableDirectory();
	const std::filesystem::path repositoryDirectory = std::filesystem::current_path();
	const std::filesystem::path executableReposDirectory =
		executableDirectory.parent_path().parent_path().parent_path();
	const std::filesystem::path roots[] =
	{
		executableDirectory / L"Worlds" / L"Music",
		repositoryDirectory / L"Worlds" / L"Music",
		repositoryDirectory / L"Sly1-Release" / L"Worlds" / L"Music",
		repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" / L"Debug" / L"Worlds" / L"Music",
		repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" / L"Release" / L"Worlds" / L"Music",
		executableReposDirectory / L"Sly1 File Extractor" / L"x64" / L"Debug" / L"Worlds" / L"Music",
		executableReposDirectory / L"Sly1 File Extractor" / L"x64" / L"Release" / L"Worlds" / L"Music"
	};

	for (const std::filesystem::path& root : roots)
	{
		const std::filesystem::path raw = root / (std::wstring(bankName) + L".mus");
		std::error_code error;
		if (std::filesystem::is_regular_file(raw, error))
			return raw;
		const std::filesystem::path wave = root / bankName;
		if (std::filesystem::is_regular_file(wave / L"Bank.json", error) &&
			std::filesystem::is_regular_file(wave / L"Sequence.mmid", error))
			return wave;
	}
	return {};
}

bool LoadPcmWave(const std::filesystem::path& path, SoundWaveFormat& format,
	std::vector<uint8_t>& pcm)
{
	std::ifstream input(path, std::ios::binary);
	if (!input.is_open())
		return false;
	std::vector<uint8_t> file((std::istreambuf_iterator<char>(input)), {});
	if (file.size() < 12 || std::memcmp(file.data(), "RIFF", 4) != 0 ||
		std::memcmp(file.data() + 8, "WAVE", 4) != 0)
		return false;

	bool foundFormat = false;
	bool foundData = false;
	for (std::size_t offset = 12; offset + 8 <= file.size();)
	{
		uint32_t size = 0;
		std::memcpy(&size, file.data() + offset + 4, sizeof(size));
		const std::size_t payload = offset + 8;
		if (payload + size > file.size())
			return false;
		if (std::memcmp(file.data() + offset, "fmt ", 4) == 0 && size >= 16)
		{
			format = {};
			std::memcpy(&format.formatTag, file.data() + payload, sizeof(format.formatTag));
			std::memcpy(&format.channels, file.data() + payload + 2, sizeof(format.channels));
			std::memcpy(&format.samplesPerSec, file.data() + payload + 4, sizeof(format.samplesPerSec));
			std::memcpy(&format.avgBytesPerSec, file.data() + payload + 8, sizeof(format.avgBytesPerSec));
			std::memcpy(&format.blockAlign, file.data() + payload + 12, sizeof(format.blockAlign));
			std::memcpy(&format.bitsPerSample, file.data() + payload + 14, sizeof(format.bitsPerSample));
			foundFormat = format.formatTag == 1;
		}
		else if (std::memcmp(file.data() + offset, "data", 4) == 0)
		{
			pcm.assign(file.begin() + payload, file.begin() + payload + size);
			foundData = !pcm.empty();
		}
		offset = payload + size + (size & 1);
	}
	return foundFormat && foundData;
}

bool EnsureEffectReverb()
{
	if (s_effectReverbVoice)
		return true;
	if (!s_xaudio2 || !s_musicMasterVoice)
		return false;
	IUnknown* reverb = nullptr;
	if (FAILED(XAudio2CreateReverb(&reverb)) || !reverb)
		return false;
	XAUDIO2_EFFECT_DESCRIPTOR effect{};
	effect.pEffect = reverb;
	effect.InitialState = TRUE;
	effect.OutputChannels = 2;
	XAUDIO2_EFFECT_CHAIN chain{};
	chain.EffectCount = 1;
	chain.pEffectDescriptors = &effect;
	const HRESULT result = s_xaudio2->CreateSubmixVoice(&s_effectReverbVoice, 2, 48000,
		0, 0, nullptr, &chain);
	reverb->Release();
	if (SUCCEEDED(result))
		ApplyCurrentEffectReverb();
	return SUCCEEDED(result) && s_effectReverbVoice != nullptr;
}

void ApplyCurrentEffectReverb()
{
	if (!s_effectReverbVoice)
		return;
	static const XAUDIO2FX_REVERB_I3DL2_PARAMETERS presets[] =
	{
		XAUDIO2FX_I3DL2_PRESET_DEFAULT,
		XAUDIO2FX_I3DL2_PRESET_SMALLROOM,
		XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM,
		XAUDIO2FX_I3DL2_PRESET_LARGEROOM,
		XAUDIO2FX_I3DL2_PRESET_SEWERPIPE,
		XAUDIO2FX_I3DL2_PRESET_ROOM
	};
	const int preset = std::clamp(static_cast<int>(s_effectReverbKind),
		static_cast<int>(REVERBK_None), static_cast<int>(REVERBK_Max) - 1);
	XAUDIO2FX_REVERB_I3DL2_PARAMETERS i3dl2 = presets[preset];
	i3dl2.WetDryMix = s_effectReverbKind == REVERBK_None ? 0.0f :
		100.0f * std::clamp(s_effectReverbDepth, 0, 127) / 127.0f;
	XAUDIO2FX_REVERB_PARAMETERS native{};
	ReverbConvertI3DL2ToNative(&i3dl2, &native, FALSE);
	s_effectReverbVoice->SetEffectParameters(0, &native, sizeof(native));
	s_effectReverbVoice->SetVolume(
		s_effectReverbKind == REVERBK_None || s_effectReverbDepth <= 0 ? 0.0f : 1.0f);
}

bool EnsureMusicEngine()
{
	if (s_xaudio2)
		return true;
	if (FAILED(XAudio2Create(&s_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
		return false;
	if (FAILED(s_xaudio2->CreateMasteringVoice(&s_musicMasterVoice)))
	{
		s_xaudio2->Release();
		s_xaudio2 = nullptr;
		return false;
	}
	s_musicMasterVoice->SetVolume(s_masterVolume);
	EnsureEffectReverb();
	return true;
}

void RecalculateChannelVolume(int channel)
{
	if (channel < 0 || channel >= static_cast<int>(s_channelVolumes.size()))
		return;
	float volume = channel == 1 ? 0.5f : 1.0f;
	for (const auto& group : s_relativeVolumes)
		volume *= group[channel];
	s_channelVolumes[channel] = (std::max)(0.0f, (std::min)(1.0f, volume));

	const int musicChannel = std::clamp(s_music989.cueVolumeGroup, 0,
		static_cast<int>(s_channelVolumes.size()) - 1);
	if (channel == musicChannel)
	{
		for (MusicVoice& active : s_music989.voices)
			if (active.voice) active.voice->SetVolume(
				active.baseVolume * (active.envelope.level / 32767.0f) *
				s_channelVolumes[musicChannel] * s_musicVolume * s_dialogMusicDuck);
	}
	// Retail VAG/dialog streams use mixer channel 2. Channel 1 is music and is
	// independently controlled by the prompt's Music option.
	if (channel == 2 && s_vagStream.loaded)
	{
		if (s_pambVagAmbient)
			SetPambVol(s_pambVagAmbient, s_pambVagAmbient->uVolAtSource);
		else if (s_vagStream.voice)
			s_vagStream.voice->SetVolume(s_channelVolumes[2]);
	}
}

void CloseMusicVoice()
{
	for (MusicVoice& active : s_music989.voices)
	{
		if (active.voice)
		{
			active.voice->Stop();
			active.voice->DestroyVoice();
		}
	}
	s_music989 = {};
	s_musicStream = {};
}

std::filesystem::path ResolveVagWav(const char* descriptorOrPath)
{
    if (!descriptorOrPath)
        return {};

    if (IsReadablePath(descriptorOrPath))
    {
        std::filesystem::path path(descriptorOrPath);
        path.replace_extension(".wav");
        std::error_code error;
        if (std::filesystem::is_regular_file(path, error))
            return path;

        if (std::filesystem::path found = FindExtractedWav(path.filename().wstring()); !found.empty())
            return found;
    }

    uint32_t words[8]{};
    std::memcpy(words, descriptorOrPath, sizeof(words));
    const uint32_t sector = words[0] ^ words[5];
    const uint32_t identity = words[2] ^ words[6];

    wchar_t exactName[64]{};
    std::swprintf(exactName, std::size(exactName), L"VAG_%04X_%06X.wav", identity, sector);
    if (std::filesystem::path found = FindExtractedWav(exactName); !found.empty())
        return found;

    wchar_t identityName[64]{};
    std::swprintf(identityName, std::size(identityName), L"VAG_%04X.wav", identity);
    if (std::filesystem::path found = FindExtractedWav(identityName); !found.empty())
        return found;

    // Commentary exports use level-based names instead of the generic VAG
    // identity name. Resolve the retail descriptor through the extractor's
    // metadata map so the BRX-provided file location remains authoritative.
    return FindCommentaryWav(identity, sector);
}

void CloseVagStream()
{
	const bool wasCommentary = s_vagStream.commentary;
	if (s_vagStream.voice)
	{
		s_vagStream.voice->Stop();
		s_vagStream.voice->FlushSourceBuffers();
		s_vagStream.voice->DestroyVoice();
	}
    s_vagStream = {};
	s_hvagAmbient = 0;
	s_dialogMusicDuck = 1.0f;
	for (MusicVoice& active : s_music989.voices)
		if (active.voice) active.voice->SetVolume(
			active.baseVolume * (active.envelope.level / 32767.0f) *
				s_channelVolumes[std::clamp(s_music989.cueVolumeGroup, 0,
					static_cast<int>(s_channelVolumes.size()) - 1)] * s_musicVolume);

	if (wasCommentary)
	{
		SetMvgkRvol(1.0f, 8, 1);
		SetMvgkRvol(1.0f, 8, 0);
		SetMvgkRvol(1.0f, 8, 3);
	}
}

uint16_t MusicLe16(const uint8_t* p)
{
	return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

uint32_t MusicLe32(const uint8_t* p)
{
	return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
		(static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

bool ParseMusicSequence()
{
	if (s_music989.sequence.size() < 0x20)
		return false;
	const uint8_t* file = s_music989.sequence.data();
	const size_t fileSize = s_music989.sequence.size();
	size_t mmidOffset = 0;
	if (MusicLe32(file) == 2 && MusicLe32(file + 4) != 0)
		mmidOffset = MusicLe32(file + 8);
	if (mmidOffset + 16 > fileSize || std::memcmp(file + mmidOffset, "MMID", 4) != 0)
		return false;
	const uint8_t count = file[mmidOffset + 7];
	if (count == 0 || mmidOffset + 16 + static_cast<size_t>(count) * 4 > fileSize)
		return false;
	for (uint8_t i = 0; i < count; ++i)
	{
		const uint32_t offset = static_cast<uint32_t>(mmidOffset) +
			MusicLe32(file + mmidOffset + 16 + i * 4);
		if (offset + 44 > fileSize || std::memcmp(file + offset, "MID ", 4) != 0)
			return false;
		s_music989.segmentOffsets.push_back(offset);
		s_music989.segmentHandles.push_back(MusicLe32(file + offset + 40));
	}
	return true;
}

bool DecodeMusicAdpcm(const std::vector<uint8_t>& data, std::size_t begin,
	std::size_t end, MusicTone& tone)
{
	// SPU2 treats both halves of the ADPCM header as full four-bit fields.
	// Predictor IDs above 4 use the zero-filled tail of its coefficient table.
	static constexpr int coefficients[16][2] =
	{
		{ 0, 0 }, { 60, 0 }, { 115, -52 }, { 98, -55 }, { 122, -60 }
	};
	std::vector<int16_t> pcm;
	int history1 = 0;
	int history2 = 0;
	UINT32 loopStart = 0;
	std::size_t loopBlockStart = 0;
	std::size_t loopBlockEnd = 0;
	bool sawLoopStart = false;
	for (std::size_t block = begin; block + 16 <= end; block += 16)
	{
		const uint8_t predictorShift = data[block];
		const uint8_t flags = data[block + 1];
		const int shift = predictorShift & 0x0f;
		const int filter = (predictorShift >> 4) & 0x0f;
		if ((flags & 4) != 0)
		{
			loopStart = static_cast<UINT32>(pcm.size());
			loopBlockStart = block;
			sawLoopStart = true;
		}
		for (int sample = 0; sample < 28; ++sample)
		{
			const uint8_t packed = data[block + 2 + sample / 2];
			int nibble = (sample & 1) ? packed >> 4 : packed & 0x0f;
			if ((nibble & 8) != 0) nibble -= 16;
			int value = (nibble << 12) >> shift;
			value += (coefficients[filter][0] * history1 +
				coefficients[filter][1] * history2 + 32) >> 6;
			value = std::clamp(value, -32768, 32767);
			history2 = history1;
			history1 = value;
			pcm.push_back(static_cast<int16_t>(value));
		}
		if ((flags & 1) != 0)
		{
			loopBlockEnd = block + 16;
			tone.loop = (flags & 2) != 0 && sawLoopStart;
			if (tone.loop)
			{
				tone.loopBegin = loopStart;
				tone.loopLength = static_cast<UINT32>(pcm.size()) - loopStart;
			}
			break;
		}
	}
	// The SPU2 preserves its ADPCM predictor history when it jumps from a loop
	// end back to the loop-start block. XAudio loops decoded PCM instead, so a
	// short loop decoded only with the intro history clicks on every iteration.
	// Decode the encoded loop repeatedly and retain its converged PCM cycle.
	if (tone.loop && loopBlockStart < loopBlockEnd)
	{
		std::vector<int16_t> steadyLoop;
		for (int iteration = 0; iteration < 32; ++iteration)
		{
			steadyLoop.clear();
			for (std::size_t block = loopBlockStart; block < loopBlockEnd; block += 16)
			{
				const uint8_t predictorShift = data[block];
				const int shift = predictorShift & 0x0f;
				const int filter = (predictorShift >> 4) & 0x0f;
				for (int sample = 0; sample < 28; ++sample)
				{
					const uint8_t packed = data[block + 2 + sample / 2];
					int nibble = (sample & 1) ? packed >> 4 : packed & 0x0f;
					if ((nibble & 8) != 0) nibble -= 16;
					int value = (nibble << 12) >> shift;
					value += (coefficients[filter][0] * history1 +
						coefficients[filter][1] * history2 + 32) >> 6;
					value = std::clamp(value, -32768, 32767);
					history2 = history1;
					history1 = value;
					steadyLoop.push_back(static_cast<int16_t>(value));
				}
			}
		}
		if (steadyLoop.size() == tone.loopLength)
			std::copy(steadyLoop.begin(), steadyLoop.end(), pcm.begin() + loopStart);
	}
	if (pcm.empty())
		return false;
	tone.format = {};
	tone.format.formatTag = 1;
	tone.format.channels = 1;
	tone.format.samplesPerSec = 48000;
	tone.format.bitsPerSample = 16;
	tone.format.blockAlign = sizeof(int16_t);
	tone.format.avgBytesPerSec = tone.format.samplesPerSec * tone.format.blockAlign;
	tone.pcm.resize(pcm.size() * sizeof(int16_t));
	std::memcpy(tone.pcm.data(), pcm.data(), tone.pcm.size());
	return true;
}

bool LoadMusic989Mus(const std::filesystem::path& path)
{
	std::ifstream input(path, std::ios::binary);
	if (!input.is_open())
		return false;
	const std::vector<uint8_t> data((std::istreambuf_iterator<char>(input)), {});
	if (data.size() < 0x20 || MusicLe32(data.data()) != 1)
		return false;
	const uint32_t sectionCount = MusicLe32(data.data() + 4);
	const uint32_t sblkOffset = MusicLe32(data.data() + 8);
	const uint32_t audioOffset = MusicLe32(data.data() + 0x10);
	const uint32_t audioSize = MusicLe32(data.data() + 0x14);
	const uint32_t sequenceOffset = sectionCount >= 3 ? MusicLe32(data.data() + 0x18) : 0;
	const uint32_t sequenceSize = sectionCount >= 3 ? MusicLe32(data.data() + 0x1c) : 0;
	if (sectionCount < 3 || sblkOffset + 0x30 > data.size() ||
		std::memcmp(data.data() + sblkOffset, "SBv2", 4) != 0 ||
		static_cast<uint64_t>(audioOffset) + audioSize > data.size() ||
		static_cast<uint64_t>(sequenceOffset) + sequenceSize > data.size())
		return false;

	const uint16_t cueCount = MusicLe16(data.data() + sblkOffset + 0x14);
	const uint16_t programCount = MusicLe16(data.data() + sblkOffset + 0x16);
	const uint32_t cueTable = sblkOffset + MusicLe32(data.data() + sblkOffset + 0x1c);
	const uint32_t programTable = sblkOffset + MusicLe32(data.data() + sblkOffset + 0x20);
	if (cueTable + static_cast<uint64_t>(cueCount) * 0x1c > data.size() ||
		programTable + static_cast<uint64_t>(programCount) * 8 > data.size())
		return false;
	if (cueCount != 0)
	{
		s_music989.cueVolume = static_cast<int16_t>(MusicLe16(data.data() + cueTable + 0x10));
		s_music989.cueRepeats = static_cast<int8_t>(data[cueTable + 0x12]);
		s_music989.cueVolumeGroup = static_cast<int8_t>(data[cueTable + 0x13]);
		s_music989.cuePan = static_cast<int16_t>(MusicLe16(data.data() + cueTable + 0x14));
	}
	for (uint16_t programIndex = 0; programIndex < programCount; ++programIndex)
	{
		const std::size_t entry = programTable + static_cast<std::size_t>(programIndex) * 8;
		MusicProgram program{};
		program.volume = static_cast<int8_t>(data[entry + 1]);
		program.pan = static_cast<int16_t>(MusicLe16(data.data() + entry + 2));
		s_music989.programs.push_back(program);
		const uint8_t toneCount = data[entry];
		const uint32_t toneTable = sblkOffset + MusicLe32(data.data() + entry + 4);
		for (uint8_t toneIndex = 0; toneIndex < toneCount; ++toneIndex)
		{
			const std::size_t toneEntry = toneTable + static_cast<std::size_t>(toneIndex) * 0x18;
			if (toneEntry + 0x18 > data.size())
				return false;
			const uint16_t flags = MusicLe16(data.data() + toneEntry + 0x0e);
			const uint32_t streamOffset = MusicLe32(data.data() + toneEntry + 0x10);
			if ((flags & 0x80) != 0 || streamOffset >= audioSize)
				continue;
			MusicTone tone{};
			tone.program = programIndex;
			tone.volume = static_cast<int8_t>(data[toneEntry + 1]);
			tone.centerNote = static_cast<int8_t>(data[toneEntry + 2]);
			tone.centerFine = static_cast<int8_t>(data[toneEntry + 3]);
			tone.pan = static_cast<int16_t>(MusicLe16(data.data() + toneEntry + 4));
			tone.mapLow = static_cast<int8_t>(data[toneEntry + 6]);
			tone.mapHigh = static_cast<int8_t>(data[toneEntry + 7]);
			tone.pitchBendLow = static_cast<int8_t>(data[toneEntry + 8]);
			tone.pitchBendHigh = static_cast<int8_t>(data[toneEntry + 9]);
			tone.adsr1 = MusicLe16(data.data() + toneEntry + 0x0a);
			tone.adsr2 = MusicLe16(data.data() + toneEntry + 0x0c);
			if (!DecodeMusicAdpcm(data, audioOffset + streamOffset,
				static_cast<std::size_t>(audioOffset) + audioSize, tone))
				return false;
			s_music989.tones.push_back(std::move(tone));
		}
	}
	s_music989.sequence.assign(data.begin() + sequenceOffset,
		data.begin() + sequenceOffset + sequenceSize);
	const bool parsed = !s_music989.tones.empty() && ParseMusicSequence();
	return parsed;
}

void LoadMusicAdpcmLoopPoints(const std::filesystem::path& directory)
{
	const std::filesystem::path musPath = directory.parent_path() /
		(directory.filename().wstring() + L".mus");
	std::ifstream input(musPath, std::ios::binary);
	if (!input.is_open())
		return;
	const std::vector<uint8_t> data((std::istreambuf_iterator<char>(input)), {});
	if (data.size() < 0x30 || MusicLe32(data.data()) != 1)
		return;

	const uint32_t sectionCount = MusicLe32(data.data() + 4);
	const uint32_t sblkOffset = MusicLe32(data.data() + 8);
	const uint32_t audioOffset = MusicLe32(data.data() + 0x10);
	const uint32_t audioSize = MusicLe32(data.data() + 0x14);
	if (sectionCount < 2 || sblkOffset + 0x30 > data.size() ||
		std::memcmp(data.data() + sblkOffset, "SBv2", 4) != 0 ||
		static_cast<uint64_t>(audioOffset) + audioSize > data.size())
		return;

	const uint16_t programCount = MusicLe16(data.data() + sblkOffset + 0x16);
	const uint32_t programTable = sblkOffset + MusicLe32(data.data() + sblkOffset + 0x20);
	if (programTable >= data.size())
		return;

	std::size_t extractedTone = 0;
	for (uint16_t program = 0; program < programCount && extractedTone < s_music989.tones.size(); ++program)
	{
		const std::size_t programEntry = programTable + static_cast<std::size_t>(program) * 8;
		if (programEntry + 8 > data.size())
			break;
		const uint8_t toneCount = data[programEntry];
		const uint32_t toneTable = sblkOffset + MusicLe32(data.data() + programEntry + 4);
		for (uint8_t toneIndex = 0; toneIndex < toneCount && extractedTone < s_music989.tones.size(); ++toneIndex)
		{
			const std::size_t toneEntry = toneTable + static_cast<std::size_t>(toneIndex) * 0x18;
			if (toneEntry + 0x18 > data.size())
				return;
			const uint16_t toneFlags = MusicLe16(data.data() + toneEntry + 0x0e);
			const uint32_t streamOffset = MusicLe32(data.data() + toneEntry + 0x10);
			if ((toneFlags & 0x80) != 0 || streamOffset >= audioSize)
				continue;

			MusicTone& tone = s_music989.tones[extractedTone++];
			tone.loop = false;
			tone.loopBegin = 0;
			tone.loopLength = 0;
			UINT32 loopStart = 0;
			bool foundStart = false;
			const std::size_t stream = audioOffset + streamOffset;
			for (std::size_t frame = 0; stream + frame * 16 + 16 <= audioOffset + audioSize; ++frame)
			{
				const uint8_t* block = data.data() + stream + frame * 16;
				bool terminator = true;
				for (int byte = 0; byte < 16; ++byte)
					terminator = terminator && block[byte] == 0;
				if (terminator)
					break;

				const uint8_t flags = block[1];
				if ((flags & 0x04) != 0)
				{
					loopStart = static_cast<UINT32>(frame * 28);
					foundStart = true;
				}
				if ((flags & 0x01) != 0)
				{
					const UINT32 loopEnd = static_cast<UINT32>((frame + 1) * 28);
					const UINT32 sampleCount = static_cast<UINT32>(tone.pcm.size() /
						(std::max)(1u, static_cast<unsigned>(tone.format.blockAlign)));
					if ((flags & 0x02) != 0 && foundStart && loopEnd > loopStart && loopEnd <= sampleCount)
					{
						tone.loop = true;
						tone.loopBegin = loopStart;
						tone.loopLength = loopEnd - loopStart;
					}
					break;
				}
			}
		}
	}
}

bool LoadMusic989Bank(const std::filesystem::path& directory)
{
	std::ifstream metadata(directory / L"Bank.json");
	if (!metadata.is_open())
		return false;
	nlohmann::json bank;
	try { metadata >> bank; }
	catch (...) { return false; }

	const auto& cues = bank.value("cues", nlohmann::json::array());
	if (!cues.empty())
	{
		s_music989.cueRepeats = cues[0].value("repeats", 0);
		s_music989.cueVolume = cues[0].value("volume", 127);
		s_music989.cuePan = cues[0].value("pan", 0);
		s_music989.cueVolumeGroup = cues[0].value("volumeGroup", 0);
	}

	for (const auto& programJson : bank.value("programs", nlohmann::json::array()))
	{
		MusicProgram program{};
		program.volume = programJson.value("volume", 127);
		program.pan = programJson.value("pan", 0);
		s_music989.programs.push_back(program);
	}
	for (const auto& toneJson : bank.value("tones", nlohmann::json::array()))
	{
		MusicTone tone{};
		tone.program = toneJson.value("program", 0);
		tone.mapLow = toneJson.value("mapLow", 0);
		tone.mapHigh = toneJson.value("mapHigh", 127);
		tone.volume = toneJson.value("volume", 127);
		tone.pan = toneJson.value("pan", 0);
		tone.centerNote = toneJson.value("centerNote", 60);
		tone.centerFine = toneJson.value("centerFine", 0);
		tone.pitchBendLow = toneJson.value("pitchBendLow", 0);
		tone.pitchBendHigh = toneJson.value("pitchBendHigh", 0);
		tone.adsr1 = static_cast<uint16_t>(toneJson.value("adsr1", 0));
		tone.adsr2 = static_cast<uint16_t>(toneJson.value("adsr2", 0));
		// Actual looping comes from the PS-ADPCM block flags in the original
		// .mus data, not from bit 0x40 of the SBv2 tone flags.
		tone.loop = false;
		const std::filesystem::path wave = directory / toneJson.value("wave", std::string());
		if (!LoadPcmWave(wave, tone.format, tone.pcm))
			return false;
		// The extractor stores the tone's native 989SND tuning in the WAV sample
		// rate. Preserve it and apply only note-relative pitch during playback.
		tone.wavePath = wave;
		s_music989.tones.push_back(std::move(tone));
	}
	LoadMusicAdpcmLoopPoints(directory);

	std::ifstream sequenceFile(directory / L"Sequence.mmid", std::ios::binary);
	if (!sequenceFile.is_open())
		return false;
	s_music989.sequence.assign(std::istreambuf_iterator<char>(sequenceFile), {});
	if (s_music989.sequence.size() < 0x20)
		return false;

	const uint8_t* file = s_music989.sequence.data();
	const size_t fileSize = s_music989.sequence.size();
	size_t mmidOffset = 0;
	if (MusicLe32(file) == 2 && MusicLe32(file + 4) != 0)
		mmidOffset = MusicLe32(file + 8);
	if (mmidOffset + 16 > fileSize || std::memcmp(file + mmidOffset, "MMID", 4) != 0)
		return false;
	const uint8_t count = file[mmidOffset + 7];
	if (count == 0 || mmidOffset + 16 + static_cast<size_t>(count) * 4 > fileSize)
		return false;
	for (uint8_t i = 0; i < count; ++i)
	{
		const uint32_t offset = static_cast<uint32_t>(mmidOffset) + MusicLe32(file + mmidOffset + 16 + i * 4);
		// 989SND calls the structure Midi internally, but the on-disk DataID is
		// "MID " (with a trailing space), not the conventional "MIDI" FourCC.
		if (offset + 44 > fileSize || std::memcmp(file + offset, "MID ", 4) != 0)
			return false;
		s_music989.segmentOffsets.push_back(offset);
		s_music989.segmentHandles.push_back(MusicLe32(file + offset + 40));
	}
	return !s_music989.tones.empty();
}

bool ReadMusicVlq(MusicSegment& segment, uint32_t& value)
{
	value = 0;
	for (int i = 0; i < 4 && segment.cursor < segment.end; ++i)
	{
		const uint8_t byte = *segment.cursor++;
		value = (value << 7) | (byte & 0x7f);
		if ((byte & 0x80) == 0)
			return true;
	}
	return false;
}

void QueueMusicSegment(uint32_t id)
{
	if (id < s_music989.segmentOffsets.size())
		s_music989.pendingStarts.push_back(id);
}

void StopMusicSegment(uint32_t id)
{
	for (MusicSegment& segment : s_music989.activeSegments)
		if (segment.id == id)
			segment.complete = true;
	for (MusicVoice& active : s_music989.voices)
	{
		if (active.voice && active.segment == id)
			ReleaseMusicEnvelope(active.envelope);
	}
}

void SetGlobalExciteValue(int excite)
{
	s_globalExcite = static_cast<uint8_t>(std::clamp(excite, 0, 127));

	// snd_SetGlobalExcite immediately reapplies every active AME channel group.
	// Merely changing the stored value leaves battle channels playing forever.
	for (MusicSegment& segment : s_music989.activeSegments)
	{
		for (const MusicAmeGroup& group : s_music989.groups)
		{
			if (!group.active || group.basis != 0)
				continue;
			for (uint8_t i = 0; i < group.channelCount; ++i)
			{
				const uint8_t channel = group.channels[i];
				if (channel >= segment.muted.size())
					continue;
				const bool mute = s_globalExcite < group.exciteMin[i] ||
					s_globalExcite > group.exciteMax[i];
				if (mute && !segment.muted[channel])
				{
					for (MusicVoice& voice : s_music989.voices)
						if (voice.voice && voice.segment == segment.id && voice.channel == channel)
							ReleaseMusicEnvelope(voice.envelope);
				}
				segment.muted[channel] = mute;
			}
		}
	}
}

void ReapplyRegisterAmeGroups(int registerIndex)
{
	for (MusicSegment& segment : s_music989.activeSegments)
	{
		for (const MusicAmeGroup& group : s_music989.groups)
		{
			if (!group.active || group.basis == 0 || group.basis - 1 != registerIndex)
				continue;
			const uint8_t value = s_music989.registers[registerIndex];
			for (uint8_t i = 0; i < group.channelCount; ++i)
			{
				const uint8_t channel = group.channels[i];
				if (channel >= segment.muted.size())
					continue;
				const bool mute = value < group.exciteMin[i] || value > group.exciteMax[i];
				if (mute && !segment.muted[channel])
				{
					for (MusicVoice& voice : s_music989.voices)
						if (voice.voice && voice.segment == segment.id && voice.channel == channel)
							ReleaseMusicEnvelope(voice.envelope);
				}
				segment.muted[channel] = mute;
			}
		}
	}
}

bool RunMusicAme(MusicSegment& midi, const uint8_t*& stream)
{
	int skip = 0;
	for (int safety = 0; safety < 1024 && stream < midi.end; ++safety)
	{
		const uint8_t op = *stream++;
		if (op == 0xf7)
			return true;
		// AME's conditional opcodes skip exactly one ordinary command.  A value
		// of two means an ELSE block is being skipped until opcode 05.  The old
		// implementation never cleared skip==1, causing every later segment
		// start/stop command in the packet to be ignored or misapplied.
		auto beginCommand = [&]()
		{
			if (skip == 0)
				return true;
			if (skip == 1)
				skip = 0;
			return false;
		};
		switch (op)
		{
		case 0x03: if (beginCommand()) StopMusicSegment(*stream); ++stream; break;
		case 0x04: if (skip == 1) skip = 2; break;
		case 0x05: if (skip == 2) skip = 0; break;
		case 0x06: if (beginCommand() && s_music989.registers[stream[0]] > stream[1] - 1) skip = 1; stream += 2; break;
		case 0x07: if (beginCommand() && s_music989.registers[stream[0]] < stream[1] + 1) skip = 1; stream += 2; break;
		case 0x0b:
		{
			const uint8_t macroIndex = *stream++;
			if (macroIndex >= s_music989.macros.size())
				return false;
			s_music989.macros[macroIndex] = stream;
			while (stream < midi.end && *stream++ != 0xf7) {}
			break;
		}
		case 0x0c:
			if (beginCommand() && s_music989.macros[*stream])
			{
				const uint8_t* macro = s_music989.macros[*stream];
				if (!RunMusicAme(midi, macro))
				{
					midi.complete = true;
					return false;
				}
			}
			++stream; break;
		case 0x0d: if (beginCommand()) { QueueMusicSegment(s_music989.registers[stream[0]] - 1); midi.complete = true; } ++stream; break;
		case 0x0e: if (beginCommand()) QueueMusicSegment(s_music989.registers[stream[0]] - 1); ++stream; break;
		case 0x11: if (beginCommand()) { QueueMusicSegment(*stream); midi.complete = true; } ++stream; break;
		case 0x12: if (beginCommand()) QueueMusicSegment(*stream); ++stream; break;
		case 0x13:
			if (beginCommand())
			{
				const int reg = stream[0];
				s_music989.registers[reg] = stream[1];
				ReapplyRegisterAmeGroups(reg);
			}
			stream += 2;
			break;
		case 0x14:
			if (beginCommand())
			{
				const int reg = *stream;
				if (s_music989.registers[reg] < 0x7f) ++s_music989.registers[reg];
				ReapplyRegisterAmeGroups(reg);
			}
			++stream;
			break;
		case 0x15:
			if (beginCommand())
			{
				const int reg = *stream;
				if (s_music989.registers[reg] > 0) --s_music989.registers[reg];
				ReapplyRegisterAmeGroups(reg);
			}
			++stream;
			break;
		case 0x16: if (beginCommand() && s_music989.registers[stream[0]] != stream[1]) skip = 1; stream += 2; break;
		case 0x00: if (beginCommand() && s_globalExcite <= static_cast<int>(stream[0]) + 1) skip = 1; ++stream; break;
		case 0x01: if (beginCommand() && s_globalExcite != static_cast<int>(stream[0]) + 1) skip = 1; ++stream; break;
		case 0x02: if (beginCommand() && s_globalExcite > static_cast<int>(stream[0]) + 1) skip = 1; ++stream; break;
		case 0x0f:
			if (skip)
			{
				while (stream < midi.end && *stream != 0x7f) ++stream;
				if (stream < midi.end) ++stream;
				if (skip == 1) skip = 0;
			}
			else
			{
				const uint8_t groupIndex = *stream++;
				if (groupIndex >= s_music989.groups.size()) return false;
				MusicAmeGroup& group = s_music989.groups[groupIndex];
				group = {};
				group.basis = *stream++;
				while (stream < midi.end && *stream != 0xf7)
				{
					if (group.channelCount >= 16 || stream + 3 > midi.end) return false;
					const uint8_t index = group.channelCount++;
					group.channels[index] = *stream++;
					group.exciteMin[index] = *stream++;
					group.exciteMax[index] = *stream++;
				}
				if (stream < midi.end) ++stream;
			}
			break;
		case 0x10:
			if (beginCommand())
			{
				const uint8_t groupIndex = *stream;
				if (groupIndex >= s_music989.groups.size()) return false;
				MusicAmeGroup& group = s_music989.groups[groupIndex];
				group.active = true;
				const uint8_t value = group.basis == 0 ? s_globalExcite : s_music989.registers[group.basis - 1];
				for (uint8_t i = 0; i < group.channelCount; ++i)
				{
					const uint8_t channel = group.channels[i];
					if (channel < 16)
						midi.muted[channel] = value < group.exciteMin[i] || value > group.exciteMax[i];
				}
			}
			++stream;
			break;
		default:
			return false;
		}
	}
	return false;
}

void ConfigureMusicEnvelope(MusicEnvelope& envelope)
{
	switch (envelope.phase)
	{
	case MusicEnvelopePhase::Attack:
		envelope.exponential = (envelope.bits & (1u << 15)) != 0;
		envelope.decreasing = false;
		envelope.shift = (envelope.bits >> 10) & 0x1f;
		envelope.step = 7 - static_cast<int>((envelope.bits >> 8) & 3);
		envelope.target = 0x7fff;
		break;
	case MusicEnvelopePhase::Decay:
		envelope.exponential = true;
		envelope.decreasing = true;
		envelope.shift = (envelope.bits >> 4) & 0x0f;
		envelope.step = -8;
		envelope.target = (static_cast<int>(envelope.bits & 0x0f) + 1) << 11;
		break;
	case MusicEnvelopePhase::Sustain:
		envelope.exponential = (envelope.bits & (1u << 31)) != 0;
		envelope.decreasing = (envelope.bits & (1u << 30)) != 0;
		envelope.shift = (envelope.bits >> 24) & 0x1f;
		envelope.step = envelope.decreasing
			? -8 + static_cast<int>((envelope.bits >> 22) & 3)
			: 7 - static_cast<int>((envelope.bits >> 22) & 3);
		envelope.target = 0;
		break;
	case MusicEnvelopePhase::Release:
		envelope.exponential = (envelope.bits & (1u << 21)) != 0;
		envelope.decreasing = true;
		envelope.shift = (envelope.bits >> 16) & 0x1f;
		envelope.step = -8;
		envelope.target = 0;
		break;
	default:
		break;
	}
}

void StartMusicEnvelope(MusicEnvelope& envelope, uint16_t adsr1, uint16_t adsr2)
{
	envelope = {};
	envelope.bits = static_cast<uint32_t>(adsr1) | (static_cast<uint32_t>(adsr2) << 16);
	envelope.phase = MusicEnvelopePhase::Attack;
	ConfigureMusicEnvelope(envelope);
}

void ReleaseMusicEnvelope(MusicEnvelope& envelope)
{
	if (envelope.phase == MusicEnvelopePhase::Stopped || envelope.phase == MusicEnvelopePhase::Release)
		return;
	envelope.phase = MusicEnvelopePhase::Release;
	envelope.counter = 0;
	ConfigureMusicEnvelope(envelope);
}

void StepMusicEnvelope(MusicEnvelope& envelope)
{
	if (envelope.phase == MusicEnvelopePhase::Stopped)
		return;
	uint32_t counterStep = 0x800000;
	const int adjustedShift = envelope.shift - 11;
	if (adjustedShift > 0)
		counterStep >>= adjustedShift;
	int sampleStep = envelope.step * (1 << (std::max)(0, 11 - envelope.shift));
	if (envelope.exponential)
	{
		if (!envelope.decreasing && envelope.level > 0x6000)
			counterStep >>= 2;
		if (envelope.decreasing)
			sampleStep = (sampleStep * envelope.level) >> 15;
	}
	envelope.counter += counterStep;
	if (envelope.counter >= 0x800000)
	{
		envelope.counter = 0;
		envelope.level = std::clamp(envelope.level + sampleStep, 0, 0x7fff);
	}
	if (envelope.phase == MusicEnvelopePhase::Sustain && envelope.decreasing && envelope.level <= 0)
	{
		envelope.phase = MusicEnvelopePhase::Stopped;
		return;
	}
	if (envelope.phase != MusicEnvelopePhase::Sustain &&
		((!envelope.decreasing && envelope.level >= envelope.target) ||
		 (envelope.decreasing && envelope.level <= envelope.target)))
	{
		if (envelope.phase == MusicEnvelopePhase::Attack)
			envelope.phase = MusicEnvelopePhase::Decay;
		else if (envelope.phase == MusicEnvelopePhase::Decay)
			envelope.phase = MusicEnvelopePhase::Sustain;
		else if (envelope.phase == MusicEnvelopePhase::Release)
			envelope.phase = MusicEnvelopePhase::Stopped;
		ConfigureMusicEnvelope(envelope);
	}
}

void UpdateMusicEnvelopes()
{
	for (MusicVoice& active : s_music989.voices)
	{
		if (!active.voice)
			continue;
		// OpenGOAL advances the SPU envelope once per 48 kHz output sample.
		// ProjectCane's music scheduler ticks at 240 Hz, hence 200 steps here.
		for (int sample = 0; sample < 200; ++sample)
			StepMusicEnvelope(active.envelope);
		if (active.envelope.phase == MusicEnvelopePhase::Stopped)
		{
			active.voice->Stop();
			active.voice->DestroyVoice();
			active.voice = nullptr;
		}
		else
		{
			const float envelopeVolume = active.envelope.level / 32767.0f;
			active.voice->SetVolume(active.baseVolume * envelopeVolume *
				s_channelVolumes[std::clamp(s_music989.cueVolumeGroup, 0,
					static_cast<int>(s_channelVolumes.size()) - 1)] *
				s_musicVolume * s_dialogMusicDuck);
		}
	}
}

void SetMusicVoicePan(IXAudio2SourceVoice* voice, int pan)
{
	if (!voice || !s_musicMasterVoice)
		return;
	XAUDIO2_VOICE_DETAILS sourceDetails{};
	XAUDIO2_VOICE_DETAILS destinationDetails{};
	voice->GetVoiceDetails(&sourceDetails);
	s_musicMasterVoice->GetVoiceDetails(&destinationDetails);
	if (sourceDetails.InputChannels != 1 || destinationDetails.InputChannels < 2)
		return;
	while (pan >= 360) pan -= 360;
	while (pan < 0) pan += 360;
	if (pan > 180) pan -= 360;
	const float position = std::sin(static_cast<float>(pan) * 3.14159265358979323846f / 180.0f);
	std::vector<float> matrix(destinationDetails.InputChannels, 0.0f);
	// 989SND applies its master/group curve after panning, which squares the
	// signed 15-bit voice volume.  For a PC float mixer that means using the
	// power pan coefficients themselves rather than their square roots.
	matrix[0] = s_stereoOutputEnabled ? (1.0f - position) * 0.5f : 0.5f;
	matrix[1] = s_stereoOutputEnabled ? (1.0f + position) * 0.5f : 0.5f;
	voice->SetOutputMatrix(s_musicMasterVoice, 1, destinationDetails.InputChannels, matrix.data());
}

void StartMusicTone(MusicSegment& segment, uint8_t channel, uint8_t note, uint8_t velocity)
{
	if (segment.muted[channel])
		return;
	const int programIndex = segment.programs[channel];
	for (MusicTone& tone : s_music989.tones)
	{
		if (tone.program != programIndex || note < tone.mapLow || note > tone.mapHigh)
			continue;
		IXAudio2SourceVoice* voice = nullptr;
		WAVEFORMATEX format{};
		format.wFormatTag = tone.format.formatTag;
		format.nChannels = tone.format.channels;
		format.nSamplesPerSec = tone.format.samplesPerSec;
		format.nAvgBytesPerSec = tone.format.avgBytesPerSec;
		format.nBlockAlign = tone.format.blockAlign;
		format.wBitsPerSample = tone.format.bitsPerSample;
		// 989SND can transpose an instrument by considerably more than one octave.
		// XAudio2's default MaxFrequencyRatio is only 2.0, which made higher notes
		// reject SetFrequencyRatio and silently play at the sample's base pitch.
		const HRESULT createResult = s_xaudio2->CreateSourceVoice(
			&voice, &format, 0, kSpuMaximumFrequencyRatio);
		if (FAILED(createResult) || !voice)
			continue;
		XAUDIO2_BUFFER buffer{};
		buffer.AudioBytes = static_cast<UINT32>(tone.pcm.size());
		buffer.pAudioData = tone.pcm.data();
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		if (tone.loop)
		{
			buffer.LoopBegin = tone.loopBegin;
			buffer.LoopLength = tone.loopLength;
			buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
		}
		const float programVolume = programIndex < static_cast<int>(s_music989.programs.size())
			? s_music989.programs[programIndex].volume / 127.0f : 1.0f;
		const int programPan = programIndex < static_cast<int>(s_music989.programs.size())
			? s_music989.programs[programIndex].pan : 0;
		const float linearVolume = (velocity / 127.0f) *
			(segment.volumes[channel] / 127.0f) * (tone.volume / 127.0f) * programVolume *
			(s_music989.cueVolume / 127.0f);
		// VoiceManager::AdjustVolToGroup in 989SND squares the composed voice
		// volume before it reaches the SPU.  Keeping this curve is important:
		// linear XAudio gains make a layered bank much too loud and mask its
		// upper-frequency instruments, producing the muddy sound we recorded.
		const float baseVolume = linearVolume * linearVolume;
		// 989SND stores centerNote as a signed byte. Negative values use their
		// magnitude for native PS2 tuning; non-negative values use PS1 tuning.
		const bool ps1Pitch = tone.centerNote >= 0;
		const int centerNote = ps1Pitch ? tone.centerNote : -tone.centerNote;
		const double baseSemitones = static_cast<double>(note) - centerNote +
			static_cast<double>(tone.centerFine) / 128.0;
		const float baseFrequencyRatio = static_cast<float>(std::pow(2.0,
			baseSemitones / 12.0) * (ps1Pitch ? 44100.0 / 48000.0 : 1.0));
		const int currentPitchBend = segment.pitchBends[channel];
		const double bendSemitones = currentPitchBend >= 0
			? tone.pitchBendHigh * static_cast<double>(currentPitchBend) / 32767.0
			: tone.pitchBendLow * static_cast<double>(currentPitchBend) / 32768.0;
		const float frequencyRatio = std::clamp(
			baseFrequencyRatio * static_cast<float>(std::pow(2.0, bendSemitones / 12.0)),
			XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio);
		voice->SetVolume(0.0f);
		const int finalPan = s_music989.cuePan + segment.pans[channel] + programPan + tone.pan;
		SetMusicVoicePan(voice, finalPan);
		const HRESULT pitchResult = voice->SetFrequencyRatio(frequencyRatio);
		if (FAILED(pitchResult))
		{
			voice->DestroyVoice();
			continue;
		}
		const HRESULT submitResult = voice->SubmitSourceBuffer(&buffer);
		const HRESULT startResult = SUCCEEDED(submitResult) ? voice->Start() : E_FAIL;
		if (SUCCEEDED(submitResult) && SUCCEEDED(startResult))
		{
			MusicVoice active{};
			active.voice = voice;
			active.segment = segment.id;
			active.channel = channel;
			active.note = note;
			active.baseVolume = baseVolume;
			active.baseFrequencyRatio = baseFrequencyRatio;
			active.pitchBendLow = tone.pitchBendLow;
			active.pitchBendHigh = tone.pitchBendHigh;
			StartMusicEnvelope(active.envelope, tone.adsr1, tone.adsr2);
			s_music989.voices.push_back(std::move(active));
		}
		else
		{
			voice->DestroyVoice();
		}
	}
}

void StopMusicTone(uint32_t segment, uint8_t channel, uint8_t note)
{
	for (MusicVoice& active : s_music989.voices)
	{
		if (active.voice && active.segment == segment && active.channel == channel && active.note == note)
			ReleaseMusicEnvelope(active.envelope);
	}
}

void ActivateQueuedMusicSegments()
{
	for (const uint32_t id : s_music989.pendingStarts)
	{
		bool exists = false;
		for (const MusicSegment& active : s_music989.activeSegments)
			exists = exists || (!active.complete && active.id == id);
		if (exists || id >= s_music989.segmentOffsets.size())
			continue;
		const uint8_t* base = s_music989.sequence.data() + s_music989.segmentOffsets[id];
		const uint32_t dataStart = MusicLe32(base + 24);
		MusicSegment segment{};
		segment.id = id;
		segment.begin = base + dataStart;
		segment.cursor = segment.begin;
		// Each MMID entry owns only the bytes up to the next MID structure.  Letting
		// a segment run to EOF makes malformed/running-status reads spill into the
		// following segment and starts music layers that were never requested.
		segment.end = id + 1 < s_music989.segmentOffsets.size()
			? s_music989.sequence.data() + s_music989.segmentOffsets[id + 1]
			: s_music989.sequence.data() + s_music989.sequence.size();
		segment.tempo = MusicLe32(base + 32);
		segment.ppq = static_cast<int32_t>(MusicLe32(base + 36));
		segment.repeatsRemaining = s_music989.cueRepeats;
		segment.volumes.fill(127);
		s_music989.activeSegments.push_back(segment);
	}
	s_music989.pendingStarts.clear();
}

void StepMusicSegment(MusicSegment& segment)
{
	if (segment.complete || segment.cursor >= segment.end)
		return;
	if (segment.ticksLeft > 0) { --segment.ticksLeft; return; }
	for (int safety = 0; safety < 256 && !segment.complete; ++safety)
	{
		if (segment.needDelta)
		{
			uint32_t delta = 0;
			if (!ReadMusicVlq(segment, delta)) { segment.complete = true; break; }
			// The 989SND sequencer runs at 240 Hz and carries the sub-tick rounding
			// error forward.  Ceil-ing every MIDI delta independently steadily slows
			// the song and eventually makes its loop point audibly wrong.
			const double exactTicks = delta * static_cast<double>(segment.tempo) * 240.0 /
				((std::max)(1, segment.ppq) * 1000000.0) + segment.tickError;
			segment.ticksLeft = static_cast<int>(std::ceil((std::max)(0.0, exactTicks)));
			segment.tickError = exactTicks - segment.ticksLeft;
			segment.needDelta = false;
			if (segment.ticksLeft > 0) { --segment.ticksLeft; break; }
		}
		if (segment.cursor >= segment.end) { segment.complete = true; break; }
		if ((*segment.cursor & 0x80) != 0) segment.status = *segment.cursor++;
		const uint8_t channel = segment.status & 0x0f;
		switch (segment.status >> 4)
		{
		case 0x8: StopMusicTone(segment.id, channel, segment.cursor[0]); segment.cursor += 2; break;
		case 0x9:
			if (segment.cursor[1] == 0) StopMusicTone(segment.id, channel, segment.cursor[0]);
			else StartMusicTone(segment, channel, segment.cursor[0], segment.cursor[1]);
			segment.cursor += 2; break;
		case 0xb:
			if (segment.cursor[0] == 7) segment.volumes[channel] = segment.cursor[1];
			else if (segment.cursor[0] == 10)
			{
				const uint8_t pan = segment.cursor[1];
				segment.pans[channel] = pan >= 64
					? static_cast<int16_t>(90 * (pan - 64) / 63)
					: static_cast<int16_t>(90 * pan / 64 + 270);
			}
			segment.cursor += 2; break;
		case 0xc: segment.programs[channel] = *segment.cursor++; break;
		case 0xd:
			// 989SND uses channel pressure's byte as a note and keys off matching voices.
			StopMusicTone(segment.id, channel, *segment.cursor++);
			break;
		case 0xe:
		{
			const uint16_t rawPitch = static_cast<uint16_t>((segment.cursor[0] & 0x7f) |
				((segment.cursor[1] & 0x7f) << 7));
			const int scaledPitch = 0xffff * static_cast<int>(rawPitch) / 0x3fff;
			segment.pitchBends[channel] = static_cast<int16_t>(scaledPitch + 0x8000);
			for (MusicVoice& active : s_music989.voices)
			{
				if (!active.voice || active.segment != segment.id || active.channel != channel)
					continue;
				const int bend = segment.pitchBends[channel];
				const double semitones = bend >= 0
					? active.pitchBendHigh * static_cast<double>(bend) / 32767.0
					: active.pitchBendLow * static_cast<double>(bend) / 32768.0;
				active.voice->SetFrequencyRatio(std::clamp(
					active.baseFrequencyRatio *
					static_cast<float>(std::pow(2.0, semitones / 12.0)),
					XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio));
			}
			segment.cursor += 2;
			break;
		}
		case 0xf:
			if (segment.status == 0xff)
			{
				const uint8_t type = *segment.cursor++;
				uint32_t length = 0; if (!ReadMusicVlq(segment, length)) { segment.complete = true; break; }
				if (type == 0x2f)
				{
					// A repeat count of zero is the 989SND convention for an infinite
					// music cue.  Positive values count complete track plays.
					if (segment.repeatsRemaining > 0 && --segment.repeatsRemaining == 0)
						segment.complete = true;
					else
					{
						segment.cursor = segment.begin;
						segment.status = 0;
					}
				}
				else { if (type == 0x51 && length >= 3) segment.tempo = (segment.cursor[0] << 16) | (segment.cursor[1] << 8) | segment.cursor[2]; segment.cursor += length; }
			}
			else if (segment.status == 0xf0 && segment.cursor < segment.end && *segment.cursor++ == 0x75)
			{
				const uint8_t* script = segment.cursor;
				if (!RunMusicAme(segment, script)) segment.complete = true;
				segment.cursor = script;
			}
			else segment.complete = true;
			break;
		default: segment.complete = true; break;
		}
		segment.needDelta = true;
	}
}

bool DecodeEffectAdpcm(const EFFECTBANK& bank, uint32_t sampleOffset, DecodedEffectSample& decoded)
{
	if (sampleOffset >= bank.sampleSize)
		return false;
	// Match SPU2's complete ADPCM header handling. Reserved predictor IDs have
	// zero coefficients instead of aliasing to IDs 0..7 or rejecting the tone.
	static constexpr int coefficients[16][2] =
	{
		{ 0, 0 }, { 60, 0 }, { 115, -52 }, { 98, -55 }, { 122, -60 }
	};
	const std::size_t begin = static_cast<std::size_t>(bank.sampleOffset) + sampleOffset;
	const std::size_t end = static_cast<std::size_t>(bank.sampleOffset) + bank.sampleSize;
	int history1 = 0;
	int history2 = 0;
	UINT32 loopStart = 0;
	std::size_t loopBlockStart = 0;
	std::size_t loopBlockEnd = 0;
	bool sawLoopStart = false;
	for (std::size_t block = begin; block + 16 <= end; block += 16)
	{
		const uint8_t predictorShift = bank.bytes[block];
		const uint8_t flags = bank.bytes[block + 1];
		const int shift = predictorShift & 0x0f;
		const int filter = (predictorShift >> 4) & 0x0f;
		if ((flags & 4) != 0)
		{
			loopStart = static_cast<UINT32>(decoded.pcm.size());
			loopBlockStart = block;
			sawLoopStart = true;
		}
		for (int sample = 0; sample < 28; ++sample)
		{
			const uint8_t packed = bank.bytes[block + 2 + sample / 2];
			int nibble = (sample & 1) ? (packed >> 4) : (packed & 0x0f);
			if ((nibble & 8) != 0) nibble -= 16;
			int value = (nibble << 12) >> shift;
			value += (coefficients[filter][0] * history1 +
				coefficients[filter][1] * history2 + 32) >> 6;
			value = std::clamp(value, -32768, 32767);
			history2 = history1;
			history1 = value;
			decoded.pcm.push_back(static_cast<int16_t>(value));
		}
		if ((flags & 1) != 0)
		{
			loopBlockEnd = block + 16;
			decoded.loop = (flags & 2) != 0 && sawLoopStart;
			if (decoded.loop)
			{
				decoded.loopBegin = loopStart;
				decoded.loopLength = static_cast<UINT32>(decoded.pcm.size()) - loopStart;
			}
			break;
		}
	}
	// The SPU2 carries the ADPCM predictor history across a loop jump. XAudio2
	// loops decoded PCM and cannot reproduce that state transition itself, so
	// converge the encoded loop and use its steady-state samples. Without this,
	// short continuous effects can become a loud buzz/static at every repeat.
	if (decoded.loop && loopBlockStart < loopBlockEnd)
	{
		std::vector<int16_t> steadyLoop;
		for (int iteration = 0; iteration < 32; ++iteration)
		{
			steadyLoop.clear();
			for (std::size_t block = loopBlockStart; block < loopBlockEnd; block += 16)
			{
				const uint8_t predictorShift = bank.bytes[block];
				const int shift = predictorShift & 0x0f;
				const int filter = (predictorShift >> 4) & 0x0f;
				for (int sample = 0; sample < 28; ++sample)
				{
					const uint8_t packed = bank.bytes[block + 2 + sample / 2];
					int nibble = (sample & 1) ? packed >> 4 : packed & 0x0f;
					if ((nibble & 8) != 0) nibble -= 16;
					int value = (nibble << 12) >> shift;
					value += (coefficients[filter][0] * history1 +
						coefficients[filter][1] * history2 + 32) >> 6;
					value = std::clamp(value, -32768, 32767);
					history2 = history1;
					history1 = value;
					steadyLoop.push_back(static_cast<int16_t>(value));
				}
			}
		}
		if (steadyLoop.size() == decoded.loopLength)
			std::copy(steadyLoop.begin(), steadyLoop.end(), decoded.pcm.begin() + loopStart);
	}
	return !decoded.pcm.empty();
}

void SetEffectVoicePan(IXAudio2SourceVoice* voice, int pan)
{
	if (!voice || !s_musicMasterVoice)
		return;
	XAUDIO2_VOICE_DETAILS destinationDetails{};
	s_musicMasterVoice->GetVoiceDetails(&destinationDetails);
	if (destinationDetails.InputChannels == 0)
		return;
	while (pan >= 360) pan -= 360;
	while (pan < 0) pan += 360;
	if (pan > 180) pan -= 360;
	const float position = std::sin(static_cast<float>(pan) * 3.14159265358979323846f / 180.0f);
	std::vector<float> matrix(destinationDetails.InputChannels, 0.0f);
	matrix[0] = s_stereoOutputEnabled ? (1.0f - position) * 0.5f : 0.5f;
	if (destinationDetails.InputChannels >= 2)
		matrix[1] = s_stereoOutputEnabled ? (1.0f + position) * 0.5f : 0.5f;
	voice->SetOutputMatrix(s_musicMasterVoice, 1, destinationDetails.InputChannels, matrix.data());
	if (s_effectReverbVoice)
	{
		const float reverbMatrix[2] = { matrix[0], destinationDetails.InputChannels >= 2 ? matrix[1] : matrix[0] };
		voice->SetOutputMatrix(s_effectReverbVoice, 1, 2, reverbMatrix);
	}
}

int EffectSpatialPanDegrees(float pan)
{
	// This is the same conversion used by retail RefreshPambVolPan before its
	// pan value is sent to 989SND.
	return pan > 0.0f
		? static_cast<int>((pan + 2.0f) * 179.5f) - 359
		: static_cast<int>((pan + 2.0f) * 179.5f);
}

float EffectVoiceVolume(const EFFECTVOICE& active, float applicationVolume)
{
	// 989SND first folds the 10-bit application volume into the authored
	// 0..127 sound volume and clamps it.  The tone volume and master/group
	// volume are then applied before AdjustVolToGroup squares the result.
	const float adjustedSoundVolume = std::clamp(
		active.soundVolume * (std::max)(0.0f, applicationVolume), 0.0f, 1.0f);
	const int group = std::clamp(active.volumeGroup, 0, 3);
	const float linearVolume = adjustedSoundVolume * active.toneVolume *
		s_channelVolumes[group];
	// 989SND's group curve is applied to the programmed voice volume first;
	// the SPU2 ADSR envelope then scales that result linearly.
	return linearVolume * linearVolume * (active.envelopeLevel / 32767.0f);
}

void ConfigureEffectEnvelope(EFFECTVOICE& active)
{
	switch (active.envelopePhase)
	{
	case EffectEnvelope_Attack:
		active.envelopeExponential = (active.envelopeBits & (1u << 15)) != 0;
		active.envelopeDecreasing = false;
		active.envelopeShift = (active.envelopeBits >> 10) & 0x1f;
		active.envelopeStep = 7 - static_cast<int>((active.envelopeBits >> 8) & 3);
		active.envelopeTarget = 0x7fff;
		break;
	case EffectEnvelope_Decay:
		active.envelopeExponential = true;
		active.envelopeDecreasing = true;
		active.envelopeShift = (active.envelopeBits >> 4) & 0x0f;
		active.envelopeStep = -8;
		active.envelopeTarget = (static_cast<int>(active.envelopeBits & 0x0f) + 1) << 11;
		break;
	case EffectEnvelope_Sustain:
		active.envelopeExponential = (active.envelopeBits & (1u << 31)) != 0;
		active.envelopeDecreasing = (active.envelopeBits & (1u << 30)) != 0;
		active.envelopeShift = (active.envelopeBits >> 24) & 0x1f;
		active.envelopeStep = active.envelopeDecreasing
			? -8 + static_cast<int>((active.envelopeBits >> 22) & 3)
			: 7 - static_cast<int>((active.envelopeBits >> 22) & 3);
		active.envelopeTarget = 0;
		break;
	case EffectEnvelope_Release:
		active.envelopeExponential = (active.envelopeBits & (1u << 21)) != 0;
		active.envelopeDecreasing = true;
		active.envelopeShift = (active.envelopeBits >> 16) & 0x1f;
		active.envelopeStep = -8;
		active.envelopeTarget = 0;
		break;
	default:
		break;
	}
}

void StartEffectEnvelope(EFFECTVOICE& active, uint16_t adsr1, uint16_t adsr2)
{
	active.envelopeBits = static_cast<uint32_t>(adsr1) |
		(static_cast<uint32_t>(adsr2) << 16);
	active.envelopeCounter = 0;
	active.envelopeLevel = 0;
	active.envelopePhase = EffectEnvelope_Attack;
	ConfigureEffectEnvelope(active);
}

void ReleaseEffectEnvelope(EFFECTVOICE& active)
{
	if (active.envelopePhase == EffectEnvelope_Stopped ||
		active.envelopePhase == EffectEnvelope_Release)
		return;
	active.envelopePhase = EffectEnvelope_Release;
	active.envelopeCounter = 0;
	ConfigureEffectEnvelope(active);
}

void StepEffectEnvelope(EFFECTVOICE& active)
{
	if (active.envelopePhase == EffectEnvelope_Stopped)
		return;
	uint32_t counterStep = 0x800000;
	const int adjustedShift = active.envelopeShift - 11;
	if (adjustedShift > 0)
		counterStep >>= adjustedShift;
	int sampleStep = active.envelopeStep *
		(1 << (std::max)(0, 11 - active.envelopeShift));
	if (active.envelopeExponential)
	{
		if (!active.envelopeDecreasing && active.envelopeLevel > 0x6000)
			counterStep >>= 2;
		if (active.envelopeDecreasing)
		{
			sampleStep = (sampleStep * active.envelopeLevel) >> 15;
			// Integer SPU envelope emulation must continue making progress near
			// zero. A rounded step of zero leaves a keyed-off looping voice alive
			// forever (notably Panda King SFX 665).
			if (sampleStep == 0 && active.envelopeLevel > 0)
				sampleStep = -1;
		}
	}
	active.envelopeCounter += counterStep;
	if (active.envelopeCounter >= 0x800000)
	{
		active.envelopeCounter = 0;
		active.envelopeLevel = std::clamp(active.envelopeLevel + sampleStep, 0, 0x7fff);
	}
	if (active.envelopePhase == EffectEnvelope_Sustain &&
		active.envelopeDecreasing && active.envelopeLevel <= 0)
	{
		active.envelopePhase = EffectEnvelope_Stopped;
		return;
	}
	if (active.envelopePhase != EffectEnvelope_Sustain &&
		((!active.envelopeDecreasing && active.envelopeLevel >= active.envelopeTarget) ||
		 (active.envelopeDecreasing && active.envelopeLevel <= active.envelopeTarget)))
	{
		if (active.envelopePhase == EffectEnvelope_Attack)
			active.envelopePhase = EffectEnvelope_Decay;
		else if (active.envelopePhase == EffectEnvelope_Decay)
			active.envelopePhase = EffectEnvelope_Sustain;
		else
			active.envelopePhase = EffectEnvelope_Stopped;
		ConfigureEffectEnvelope(active);
	}
}

bool PlayEffectTone(AMB& amb, const EFFECTBANK& bank, const EFFECTBANK::SOUND& sound,
	const EFFECTBANK::TONE& tone, float applicationVolume, float pitchMod)
{
	if ((tone.flags & 8) != 0 || !EnsureMusicEngine()) // SPU noise tone
	{
		return false;
	}
	DecodedEffectSample decoded;
	if (!DecodeEffectAdpcm(bank, tone.sampleOffset, decoded))
	{
		return false;
	}
	EFFECTVOICE active;
	active.pcm = std::move(decoded.pcm);
	active.loop = decoded.loop;

	WAVEFORMATEX format{};
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.nSamplesPerSec = 48000;
	format.wBitsPerSample = 16;
	format.nBlockAlign = 2;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	XAUDIO2_SEND_DESCRIPTOR sends[2]{};
	sends[0].pOutputVoice = s_musicMasterVoice;
	UINT32 sendCount = 1;
	if (s_effectReverbVoice)
		sends[sendCount++].pOutputVoice = s_effectReverbVoice;
	XAUDIO2_VOICE_SENDS sendList{};
	sendList.SendCount = sendCount;
	sendList.pSends = sends;
	const HRESULT createResult = s_xaudio2->CreateSourceVoice(&active.voice, &format, 0,
		kSpuMaximumFrequencyRatio, nullptr, &sendList, nullptr);
	if (FAILED(createResult) ||
		active.voice == nullptr)
	{
		return false;
	}

	XAUDIO2_BUFFER buffer{};
	buffer.AudioBytes = static_cast<UINT32>(active.pcm.size() * sizeof(int16_t));
	buffer.pAudioData = reinterpret_cast<const BYTE*>(active.pcm.data());
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	if (decoded.loop)
	{
		buffer.LoopBegin = decoded.loopBegin;
		buffer.LoopLength = decoded.loopLength;
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	int resolvedToneVolume = tone.volume;
	if (resolvedToneVolume < 0)
	{
		if (resolvedToneVolume >= -4) resolvedToneVolume = amb.registers[-resolvedToneVolume - 1];
		else if (resolvedToneVolume == -5) resolvedToneVolume = std::rand() % 0x7f;
		else if (-resolvedToneVolume - 6 < static_cast<int>(s_effectGlobalRegisters.size()))
			resolvedToneVolume = s_effectGlobalRegisters[-resolvedToneVolume - 6];
	}
	int resolvedTonePan = tone.pan;
	if (resolvedTonePan < 0)
	{
		int registerValue = 0;
		if (resolvedTonePan >= -4) registerValue = amb.registers[-resolvedTonePan - 1];
		else if (resolvedTonePan == -5) registerValue = std::rand() % 127;
		else if (-resolvedTonePan - 6 < static_cast<int>(s_effectGlobalRegisters.size()))
			registerValue = s_effectGlobalRegisters[-resolvedTonePan - 6];
		resolvedTonePan = 360 * (std::min)(std::abs(registerValue), 127) / 127;
	}
	const float soundVolume = std::clamp(static_cast<int>(sound.volume), 0, 127) / 127.0f;
	const float toneVolume = std::clamp(resolvedToneVolume, 0, 127) / 127.0f;
	active.soundVolume = soundVolume;
	active.toneVolume = toneVolume;
	active.volumeGroup = std::clamp(static_cast<int>(sound.volumeGroup), 0, 3);
	active.authoredPan = static_cast<int>(sound.pan) + resolvedTonePan;
	active.baseVolume = EffectVoiceVolume(active, 1.0f);
	active.voice->SetVolume(EffectVoiceVolume(active, applicationVolume));
	SetEffectVoicePan(active.voice, active.authoredPan + EffectSpatialPanDegrees(amb.pan));

	const bool ps1Pitch = tone.centerNote >= 0;
	const int centerNote = ps1Pitch ? tone.centerNote : -tone.centerNote;
	const double semitones = 60.0 - centerNote - tone.centerFine / 128.0 +
		(static_cast<double>(pitchMod) * 1524.0 / 128.0);
	active.baseFrequencyRatio = static_cast<float>(std::pow(2.0, semitones / 12.0) *
		(ps1Pitch ? 44100.0 / 48000.0 : 1.0));
	active.baseFrequencyRatio = std::clamp(active.baseFrequencyRatio,
		XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio);
	active.pitchBendLow = tone.pitchBendLow;
	active.pitchBendHigh = tone.pitchBendHigh;
	StartEffectEnvelope(active, tone.adsr1, tone.adsr2);
	active.voice->SetVolume(EffectVoiceVolume(active, applicationVolume));
	active.voice->SetFrequencyRatio(active.baseFrequencyRatio);
	const HRESULT submitResult = active.voice->SubmitSourceBuffer(&buffer);
	const HRESULT startResult = SUCCEEDED(submitResult) ? active.voice->Start() : E_FAIL;
	if (FAILED(submitResult) || FAILED(startResult))
	{
		active.voice->DestroyVoice();
		return false;
	}
	amb.voices.push_back(std::move(active));
	amb.voice = amb.voices.front().voice;
	amb.frequencyRatioBase = amb.voices.front().baseFrequencyRatio;
	return true;
}

int16_t EffectControlParam(const EFFECTBANK::GRAIN& grain, std::size_t index)
{
	int16_t value = 0;
	if (index < 4)
		std::memcpy(&value, grain.data.data() + index * sizeof(value), sizeof(value));
	return value;
}

void SetEffectControlParam(EFFECTBANK::GRAIN& grain, std::size_t index, int16_t value)
{
	if (index < 4)
		std::memcpy(grain.data.data() + index * sizeof(value), &value, sizeof(value));
}

void ApplyEffectPitchBend(AMB& amb, int bend)
{
	amb.currentPitchBend = std::clamp(bend, -32768, 32767);
	for (EFFECTVOICE& active : amb.voices)
	{
		if (!active.voice)
			continue;
		const double bendSemitones = amb.currentPitchBend >= 0
			? active.pitchBendHigh * static_cast<double>(amb.currentPitchBend) / 32767.0
			: active.pitchBendLow * static_cast<double>(amb.currentPitchBend) / 32768.0;
		const double pitchModSemitones = static_cast<double>(amb.lfoPitchMod) / 128.0;
		const float ratio = std::clamp(active.baseFrequencyRatio *
			static_cast<float>(std::pow(2.0, (bendSemitones + pitchModSemitones) / 12.0)),
			XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio);
		active.voice->SetFrequencyRatio(ratio);
	}
}

int EffectLfoValue(EFFECTLFO& lfo)
{
	const int step = lfo.nextStep >> 16;
	lfo.nextStep += static_cast<int32_t>(2u * lfo.stepSize);
	if (lfo.nextStep > 0x07ffffff)
		lfo.nextStep -= 0x08000000;
	int value = 0;
	switch (lfo.type)
	{
	case 1: value = static_cast<int>(std::sin(step * 3.14159265358979323846 / 1024.0) * 32767.0); break;
	case 2: value = step >= lfo.stateHold1 ? -32767 : 32767; break;
	case 3:
		if (step < 512) value = 32767 * step / 512;
		else if (step >= 1536) value = 32767 * (step - 1536) / 512 - 32767;
		else value = 32767 - 65534 * (step - 512) / 1024;
		break;
	case 4: value = step >= 1024 ? 32767 * (step - 1024) / 1024 - 32767 : 32767 * step / 1023; break;
	case 5:
		if (step >= 1024 && lfo.stateHold2 == 1) { lfo.stateHold2 = 0; lfo.stateHold1 = 2 * ((std::rand() & 0x7fff) - 0x3fff); }
		else if (step < 1024 && lfo.stateHold2 == 0) { lfo.stateHold2 = 1; lfo.stateHold1 = -(std::rand() & 0x7fff) * (std::rand() & 1); }
		value = lfo.stateHold1;
		break;
	default: break;
	}
	return (lfo.setupFlags & 1) != 0 ? -value : value;
}

void TickEffectLfos(AMB& amb)
{
	for (EFFECTLFO& lfo : amb.lfos)
	{
		if (!lfo.active || (++lfo.tick & 1) == 0)
			continue;
		const int value = EffectLfoValue(lfo);
		switch (lfo.target)
		{
		case 1: amb.lfoVolume = (lfo.range * (value - 0x7fff)) >> 16; break;
		case 2: amb.lfoPan = (lfo.range * value) >> 15; break;
		case 3: amb.lfoPitchMod = (lfo.range * value) >> 15; break;
		case 4: amb.lfoPitchBend = (lfo.range * value) >> 15; break;
		default: break;
		}
	}
	const int authoredPitchBend = amb.currentPitchBend;
	ApplyEffectPitchBend(amb, std::clamp(authoredPitchBend + amb.lfoPitchBend, -32768, 32767));
	amb.currentPitchBend = authoredPitchBend;
	const float lfoVolume = static_cast<float>(amb.lfoVolume) / 127.0f;
	for (EFFECTVOICE& active : amb.voices)
		if (active.voice)
		{
			active.voice->SetVolume(EffectVoiceVolume(active, (std::max)(0.0f, amb.volAttenuated + lfoVolume)));
			SetEffectVoicePan(active.voice, active.authoredPan + amb.lfoPan + EffectSpatialPanDegrees(amb.pan));
		}
}

void ConfigureEffectLfo(AMB& amb, const EFFECTBANK::GRAIN& grain)
{
	const uint8_t which = grain.data[0];
	if (which >= amb.lfos.size())
		return;
	EFFECTLFO& lfo = amb.lfos[which];
	lfo = {};
	lfo.target = grain.data[1];
	lfo.targetExtra = grain.data[2];
	lfo.type = grain.data[3];
	uint16_t duty = 0, depth = 0, flags = 0, startOffset = 0;
	std::memcpy(&duty, grain.data.data() + 4, 2);
	std::memcpy(&depth, grain.data.data() + 6, 2);
	std::memcpy(&flags, grain.data.data() + 8, 2);
	std::memcpy(&startOffset, grain.data.data() + 10, 2);
	std::memcpy(&lfo.stepSize, grain.data.data() + 12, 4);
	lfo.depth = static_cast<int16_t>(depth);
	lfo.setupFlags = flags;
	lfo.stateHold1 = lfo.type == 2 ? duty : 0;
	lfo.stateHold2 = lfo.type == 5 ? 1 : 0;
	lfo.nextStep = (flags & 2) != 0 ? (std::rand() & 0x7ff) << 16 : startOffset << 16;
	const EFFECTBANK* bank = static_cast<const EFFECTBANK*>(amb.bnk);
	const int soundVolume = bank && amb.soundIndex >= 0 && amb.soundIndex < static_cast<int>(bank->sounds.size())
		? bank->sounds[amb.soundIndex].volume : 127;
	switch (lfo.target)
	{
	case 1: lfo.range = (soundVolume * lfo.depth) >> 10; break;
	case 2: lfo.range = (180 * lfo.depth) >> 10; break;
	case 3: lfo.range = (6096 * lfo.depth) >> 10; break;
	case 4: lfo.range = (0x7fff * lfo.depth) >> 10; break;
	default: lfo.range = 0; break;
	}
	lfo.active = lfo.target != 0 && lfo.type != 0;
	TickEffectLfos(amb);
}

void StartEffectChild(AMB& parent, const EFFECTBANK::GRAIN& grain)
{
	EFFECTBANK* bank = static_cast<EFFECTBANK*>(parent.bnk);
	if (!bank || bank->ibnk < 0 || bank->ibnk >= static_cast<int>(kEffectBankStarts.size()) - 1)
		return;
	int32_t volume = 0, pan = 0, soundIndex = -1;
	std::memcpy(&volume, grain.data.data(), 4);
	std::memcpy(&pan, grain.data.data() + 4, 4);
	std::memcpy(&soundIndex, grain.data.data() + 12, 4);
	if (soundIndex < 0 || soundIndex >= static_cast<int>(bank->sounds.size()))
		return;
	if (volume < 0)
	{
		if (volume >= -4) volume = parent.registers[-volume - 1];
		else if (volume == -5) volume = std::rand() % 0x7f;
		else if (-volume - 6 < static_cast<int>(s_effectGlobalRegisters.size()))
			volume = s_effectGlobalRegisters[-volume - 6];
	}
	volume = std::clamp(std::abs(volume), 0, 127);
	AMB* child = nullptr;
	const int globalId = static_cast<int>(kEffectBankStarts[bank->ibnk]) + soundIndex;
	StartSound(static_cast<SFXID>(globalId), &child, parent.palo,
		parent.palo ? nullptr : &parent.pos, parent.sStart, parent.sFull,
		parent.uVolAtSource * static_cast<float>(volume) / 127.0f,
		parent.frq, parent.rDoppler, nullptr, nullptr);
	if (!child)
		return;
	child->ppamb = nullptr;
	child->parent = &parent;
	child->pan += static_cast<float>(pan) / 180.0f;
	child->registers = parent.registers;
	parent.children.push_back(child);
	SetPambVol(child, child->uVolAtSource);
}

void CleanEffectVoices(AMB& amb)
{
	for (EFFECTVOICE& active : amb.voices)
	{
		if (!active.voice)
			continue;
		XAUDIO2_VOICE_STATE state{};
		active.voice->GetState(&state);
		if (state.BuffersQueued == 0)
		{
			active.voice->DestroyVoice();
			active.voice = nullptr;
			active.pcm.clear();
		}
	}
	amb.voices.erase(std::remove_if(amb.voices.begin(), amb.voices.end(),
		[](const EFFECTVOICE& voice) { return voice.voice == nullptr; }), amb.voices.end());
	amb.voice = amb.voices.empty() ? nullptr : amb.voices.front().voice;
}

void UpdateEffectEnvelopes(AMB& amb)
{
	for (EFFECTVOICE& active : amb.voices)
	{
		if (!active.voice)
			continue;
		// The effect scheduler runs at 240 Hz and the SPU2 envelope runs at
		// 48 kHz, so each scheduler tick advances 200 envelope samples.
		for (int sample = 0; sample < 200; ++sample)
			StepEffectEnvelope(active);
		if (active.envelopePhase == EffectEnvelope_Stopped)
		{
			active.voice->Stop();
			active.voice->DestroyVoice();
			active.voice = nullptr;
			active.pcm.clear();
		}
		else
		{
			active.voice->SetVolume(EffectVoiceVolume(active,
				(std::max)(0.0f, amb.volAttenuated + static_cast<float>(amb.lfoVolume) / 127.0f)));
		}
	}
}

void ExecuteEffectGrain(AMB& amb)
{
	EFFECTBANK* bank = static_cast<EFFECTBANK*>(amb.bnk);
	if (!bank || amb.soundIndex < 0 || amb.soundIndex >= static_cast<int>(bank->sounds.size()))
	{
		amb.handlerDone = true;
		return;
	}
	EFFECTBANK::SOUND& sound = bank->sounds[amb.soundIndex];
	if (amb.nextGrain < 0 || amb.nextGrain >= static_cast<int>(sound.grains.size()))
	{
		amb.handlerDone = true;
		return;
	}
	EFFECTBANK::GRAIN& grain = sound.grains[amb.nextGrain];
	int extraDelay = 0;
	switch (grain.type)
	{
	case 0: case 20: case 21: // no-op/control-null/loop start
		break;
	case 4: // LFO_SETTINGS
		ConfigureEffectLfo(amb, grain);
		break;
	case 5: // START_CHILD_SOUND
		StartEffectChild(amb, grain);
		break;
	case 1: case 9:
		PlayEffectTone(amb, *bank, sound, grain.tone, amb.volAttenuated, amb.frq);
		ApplyEffectPitchBend(amb, amb.currentPitchBend);
		break;
	case 22: // LOOP_END
		for (int i = amb.nextGrain - 1; i >= 0; --i)
			if (sound.grains[i].type == 21) { amb.nextGrain = i - 1; break; }
		break;
	case 24: // STOP
		for (EFFECTVOICE& active : amb.voices)
			ReleaseEffectEnvelope(active);
		amb.handlerDone = true;
		break;
	case 25: // RAND_PLAY
	{
		const int options = EffectControlParam(grain, 0);
		const int count = EffectControlParam(grain, 1);
		int previous = EffectControlParam(grain, 2);
		if (options > 0 && count >= 0)
		{
			int selected = std::rand() % options;
			if (selected == previous && options > 1) selected = (selected + 1) % options;
			SetEffectControlParam(grain, 2, static_cast<int16_t>(selected));
			amb.nextGrain += selected * count;
			amb.grainsToPlay = count + 1;
			amb.grainsToSkip = (options - 1 - selected) * count;
			amb.skipGrains = true;
		}
		break;
	}
	case 26: // RAND_DELAY
	{
		int32_t amount = 0;
		std::memcpy(&amount, grain.data.data(), sizeof(amount));
		if (amount > 0) extraDelay = std::rand() % amount;
		break;
	}
	case 27: // RAND_PB
	{
		const int amount = EffectControlParam(grain, 0);
		const int randomSigned = (0xffff * (std::rand() % 0x7fff)) / 0x7fff - 0x8000;
		ApplyEffectPitchBend(amb, amount * randomSigned / 100);
		break;
	}
	case 29: // ADD_PB
		ApplyEffectPitchBend(amb, amb.currentPitchBend +
			0x7fff * EffectControlParam(grain, 0) / 127);
		break;
	case 31: // SET_REGISTER_RAND
	{
		const int reg = EffectControlParam(grain, 0);
		const int low = EffectControlParam(grain, 1);
		const int high = EffectControlParam(grain, 2);
		if (high >= low)
		{
			const int8_t value = static_cast<int8_t>(low + std::rand() % (high - low + 1));
			if (reg >= 0 && reg < static_cast<int>(amb.registers.size())) amb.registers[reg] = value;
			else if (reg < 0 && -reg - 1 < static_cast<int>(s_effectGlobalRegisters.size()))
				s_effectGlobalRegisters[-reg - 1] = value;
		}
		break;
	}
	case 38: // WAIT_FOR_ALL_VOICES
		if (!amb.voices.empty()) { --amb.nextGrain; extraDelay = 1; }
		break;
	case 41: // KEY_OFF_VOICES
		for (EFFECTVOICE& active : amb.voices)
			ReleaseEffectEnvelope(active);
		break;
	case 42: // KILL_VOICES
		for (EFFECTVOICE& active : amb.voices)
		{
			if (!active.voice)
				continue;
			active.voice->Stop();
			active.voice->DestroyVoice();
			active.voice = nullptr;
			active.pcm.clear();
			active.envelopePhase = EffectEnvelope_Stopped;
		}
		break;
	default:
		// LFO and child-sound grains are retained by the parser and will be
		// connected after the core handler is verified in game.
		break;
	}

	if (amb.skipGrains && --amb.grainsToPlay == 0)
	{
		amb.nextGrain += amb.grainsToSkip;
		amb.skipGrains = false;
	}
	++amb.nextGrain;
	if (amb.handlerDone || amb.nextGrain >= static_cast<int>(sound.grains.size()))
	{
		amb.handlerDone = true;
		return;
	}
	amb.grainCountdown = sound.grains[amb.nextGrain].delay + extraDelay;
}

void StartSound(SFXID sfxid, AMB** ppamb, ALO* palo, glm::vec3* ppos, float sStart, float sFull, float uVolAtSource, float frq, float uDoppler, LM* plmRepeat, LM* plmRepDist)
{
	const int index = static_cast<int>(sfxid);
	if (index < 0 || index >= static_cast<int>(kSfxCount) || g_psw == nullptr)
	{
		return;
	}
	const SND& mapping = g_mpsfxidsndbnk[index];
	if (mapping.ibnk > 0 && mapping.ibnk != g_psw->ibnk)
	{
		return;
	}
	EFFECTBANK* bank = static_cast<EFFECTBANK*>(SbpEnsureBank(mapping.ibnk));
	if (!bank || mapping.isnd < 0 || mapping.isnd >= static_cast<int>(bank->sounds.size()))
	{
		return;
	}
	EFFECTBANK::SOUND& sound = bank->sounds[mapping.isnd];
	if (sound.grains.empty())
	{
		return;
	}
	if ((sound.flags & 8u) != 0 && sound.instanceLimit > 0)
	{
		std::vector<AMB*> instances;
		for (const std::unique_ptr<AMB>& slot : g_psw->ambOwners)
			if (slot && slot->bnk == bank && slot->soundIndex == mapping.isnd && !slot->fStopped)
				instances.push_back(slot.get());
		if (static_cast<int>(instances.size()) >= sound.instanceLimit)
		{
			AMB* weakest = nullptr;
			if ((sound.flags & 0x10u) != 0)
				for (AMB* instance : instances)
					if (!weakest || instance->uVolAtSource < weakest->uVolAtSource) weakest = instance;
			else if ((sound.flags & 0x20u) != 0)
				for (AMB* instance : instances)
					if (!weakest || instance->tStarted < weakest->tStarted) weakest = instance;
			if (!weakest || ((sound.flags & 0x10u) != 0 && weakest->uVolAtSource >= uVolAtSource))
				return;
			StopSound(weakest, 0);
		}
	}
	const bool continuous = FContinuousSound(sfxid);
	const bool recurring = plmRepeat != nullptr && plmRepeat->gMin >= 0.0f;
	float attenuatedVolume = uVolAtSource;
	glm::vec3 position{};
	bool positioned = false;
	bool outOfRange = false;
	if (palo)
	{
		position = palo->xf.posWorld;
		positioned = true;
	}
	else if (ppos)
	{
		position = *ppos;
		positioned = true;
	}
	if (positioned)
	{
		const glm::vec3* earPosition = PposSoundEar();
		if (earPosition != nullptr)
		{
			const float distance = glm::length(position - *earPosition);
			if (distance > sStart)
			{
				// Retail preserves continuous/recurring AMBs while inaudible so they
				// can start playing when the listener subsequently enters range.
				if (!continuous && !recurring)
				{
					return;
				}
				attenuatedVolume = 0.0f;
				outOfRange = true;
			}
			else if (distance > sFull && sStart > sFull)
				attenuatedVolume *= (sStart - distance) / (sStart - sFull);
		}
	}

	AMB* amb = PambAlloc();
	if (!amb)
	{
		return;
	}
	amb->sfxid = sfxid;
	amb->tStarted = g_clock.t;
	amb->bnk = bank;
	amb->soundIndex = mapping.isnd;
	amb->ppamb = ppamb;
	if (ppamb) *ppamb = amb;
	amb->palo = palo;
	amb->pos = position;
	amb->sStart = sStart;
	amb->sFull = sFull;
	amb->uVolAtSource = uVolAtSource;
	amb->volAttenuated = attenuatedVolume;
	amb->frq = frq;
	amb->rDoppler = uDoppler;
	amb->fContinuous = continuous ? 1 : 0;
	amb->fOutOfRange = outOfRange ? 1 : 0;
	amb->fAttached = positioned || recurring;
	amb->lmRepeat = plmRepeat ? *plmRepeat : LM{ -1.0f, 0.0f };
	amb->lmRepDist = plmRepDist ? *plmRepDist : LM{ -1.0f, 0.0f };
	amb->nextGrain = 0;
	amb->grainCountdown = sound.grains[0].delay;
	amb->handlerDone = recurring || (continuous && outOfRange);
	if (amb->fAttached)
		SetPambVol(amb, uVolAtSource);
	if (recurring)
	{
		ScheduleNextIntermittentSound(amb);
		const float initialDelayMax = (std::max)(0.0f, amb->lmRepeat.gMax);
		amb->tNext = g_clock.t + GRandInRange(0.0f, initialDelayMax);
		return;
	}
	if (continuous && outOfRange)
		return;
	int immediateBudget = 1024;
	while (amb->grainCountdown <= 0 && !amb->handlerDone && immediateBudget-- > 0)
		ExecuteEffectGrain(*amb);
	if (immediateBudget <= 0)
		amb->handlerDone = true;
	if (amb->handlerDone && amb->voices.empty())
	{
		RemoveAmb(amb);
		return;
	}
	SetDoppler(amb);
}

AMB* PambAlloc()
{
	if (g_psw == nullptr)
		return nullptr;

	for (std::unique_ptr<AMB>& slot : g_psw->ambOwners)
	{
		if (slot)
			continue;

		slot = std::make_unique<AMB>();
		AMB* pamb = slot.get();
		AppendDlEntry(&g_psw->dlAmb, pamb);
		return pamb;
	}

	return nullptr;
}

void DropPamb(AMB** ppamb)
{
	if (ppamb == nullptr || *ppamb == nullptr)
		return;

	(*ppamb)->ppamb = nullptr;
	*ppamb = nullptr;
}

void RemoveAmb(AMB* pamb)
{
	if (pamb == nullptr || g_psw == nullptr)
		return;
	if (pamb == s_pambVagAmbient)
	{
		s_pambVagAmbient = nullptr;
		s_hvagAmbient = 0;
	}

	if (pamb->ppamb != nullptr)
	{
		*pamb->ppamb = nullptr;
		pamb->ppamb = nullptr;
	}
	if (pamb->parent != nullptr)
	{
		auto& siblings = pamb->parent->children;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), pamb), siblings.end());
		pamb->parent = nullptr;
	}
	const std::vector<AMB*> children = pamb->children;
	pamb->children.clear();
	for (AMB* child : children)
	{
		if (!child) continue;
		child->parent = nullptr;
		StopSound(child, 0);
	}
	if (!pamb->voices.empty())
	{
		for (EFFECTVOICE& active : pamb->voices)
		{
			if (!active.voice) continue;
			active.voice->Stop();
			active.voice->DestroyVoice();
			active.voice = nullptr;
		}
		pamb->voices.clear();
		pamb->voice = nullptr;
	}
	else if (pamb->voice != nullptr)
	{
		pamb->voice->Stop();
		pamb->voice->DestroyVoice();
		pamb->voice = nullptr;
	}

	pamb->iSerial = -1;
	RemoveDlEntry(&g_psw->dlAmb, pamb);

	for (std::unique_ptr<AMB>& slot : g_psw->ambOwners)
	{
		if (slot.get() == pamb)
		{
			slot.reset();
			return;
		}
	}
}

void StopSound(AMB* pamb, int msRampdown)
{
	if (pamb == nullptr)
		return;
	if (pamb == s_pambVagAmbient || static_cast<int>(pamb->sfxid) == -2)
	{
		s_pambVagAmbient = nullptr;
		CloseVagStream();
		RemoveAmb(pamb);
		return;
	}

	pamb->fStopped = 1;
	pamb->handlerDone = true;
	const std::vector<AMB*> children = pamb->children;
	for (AMB* child : children)
		if (child) StopSound(child, msRampdown);
	if (pamb->ppamb != nullptr)
	{
		*pamb->ppamb = nullptr;
		pamb->ppamb = nullptr;
	}
	for (EFFECTVOICE& active : pamb->voices)
	{
		// XAudio2 owns its infinite loop independently of our emulated SPU
		// envelope. ExitLoop is not sufficient for every currently queued loop,
		// so key off looping effect voices explicitly when their AMB is stopped.
		if (active.voice && active.loop)
		{
			active.voice->Stop();
			active.voice->FlushSourceBuffers();
			active.voice->DestroyVoice();
			active.voice = nullptr;
			active.pcm.clear();
		}
		else
			ReleaseEffectEnvelope(active);
	}
	pamb->voices.erase(std::remove_if(pamb->voices.begin(), pamb->voices.end(),
		[](const EFFECTVOICE& active) { return active.voice == nullptr; }), pamb->voices.end());
	pamb->voice = pamb->voices.empty() ? nullptr : pamb->voices.front().voice;

	// A sound with no live tone has nothing to release. Ramp-down requests use
	// the authored SPU2 release envelope too; 989SND's millisecond automation
	// can be layered on later without bypassing key-off semantics.
	(void)msRampdown;
	if (pamb->voices.empty())
		RemoveAmb(pamb);
}

void PausePamb(AMB* pamb)
{
	if (pamb == nullptr)
		return;

	if (!pamb->voices.empty())
	{
		for (EFFECTVOICE& active : pamb->voices)
			if (active.voice != nullptr)
				active.voice->Stop();
	}
	else if (pamb->voice != nullptr)
	{
		pamb->voice->Stop();
	}
}

void ContinuePamb(AMB* pamb)
{
	if (pamb == nullptr)
		return;

	if (!pamb->voices.empty())
	{
		for (EFFECTVOICE& active : pamb->voices)
			if (active.voice != nullptr)
				active.voice->Start();
	}
	else if (pamb->voice != nullptr)
	{
		pamb->voice->Start();
	}
}

void SetAMRegister(int ireg, byte bVal)
{
	if (ireg < 0 || ireg >= static_cast<int>(s_amRegisters.size()))
		return;
	const uint8_t value = static_cast<uint8_t>(bVal) & 0x7f;
	s_amRegisters[ireg] = value;
	s_music989.registers[ireg] = value;
	ReapplyRegisterAmeGroups(ireg);
}

void RefreshMidiRegister(int ireg)
{
	if (ireg < 0 || ireg >= static_cast<int>(s_amRegisters.size()))
		return;

	// The original queries the live 989SND handle, then refreshes its cached
	// register value with the result. ProjectCane's live value is maintained
	// directly by the in-process sequencer.
	s_amRegisters[ireg] = s_music989.registers[ireg];
}

void CalculateVolPan(float sDist, glm::vec3* pposSound, float* pvol, float* ppan,
	float uvol, float sStart, float sFull)
{
	if (!pvol || !ppan)
		return;
	if (pposSound != nullptr)
	{
		if (sStart < sDist)
		{
			*ppan = 0.0f;
			*pvol = 0.0f;
			return;
		}
		if (sFull < sDist && sStart > sFull)
		{
			const float attenuation = (sStart - sDist) / (sStart - sFull);
			float side = 0.0f;
			if (g_pcm != nullptr)
			{
				const glm::vec3 displacement = *pposSound - g_pcm->pos;
				const float lengthSquared = glm::dot(displacement, displacement);
				if (lengthSquared > 0.00000001f)
					side = glm::dot(displacement / std::sqrt(lengthSquared), g_pcm->mat[1]);
			}
			*ppan = -side * 0.7f * (1.0f - attenuation);
			*pvol = uvol * attenuation;
			return;
		}
	}
	*pvol = uvol;
	*ppan = 0.0f;
}

int FAmbientsPaused()
{
	return s_fPauseAmbients;
}

void SetPambVol(AMB* pamb, float uVolAtSource)
{
	if (pamb == nullptr)
		return;
	pamb->uVolAtSource = uVolAtSource;
	float attenuatedVolume = uVolAtSource;
	if (pamb->palo != nullptr)
		pamb->pos = pamb->palo->xf.posWorld;
	if (pamb->fAttached)
	{
		const glm::vec3* earPosition = PposSoundEar();
		if (earPosition != nullptr)
		{
			const float distance = glm::length(pamb->pos - *earPosition);
			CalculateVolPan(distance, &pamb->pos, &attenuatedVolume, &pamb->pan,
				uVolAtSource, pamb->sStart, pamb->sFull);
			pamb->fOutOfRange = distance > pamb->sStart ? 1 : 0;
		}
	}
	pamb->volAttenuated = attenuatedVolume;
	if (pamb == s_pambVagAmbient || static_cast<int>(pamb->sfxid) == -2)
	{
		if (!s_vagStream.voice)
			return;
		s_vagStream.voice->SetVolume((std::max)(0.0f, attenuatedVolume) * s_channelVolumes[2]);
		if (s_vagStream.format.channels == 1 && s_musicMasterVoice)
		{
			XAUDIO2_VOICE_DETAILS destinationDetails{};
			s_musicMasterVoice->GetVoiceDetails(&destinationDetails);
			if (destinationDetails.InputChannels != 0)
			{
				const int panDegrees = EffectSpatialPanDegrees(pamb->pan);
				const float position = std::sin(static_cast<float>(panDegrees) *
					3.14159265358979323846f / 180.0f);
				std::vector<float> matrix(destinationDetails.InputChannels, 0.0f);
				matrix[0] = s_stereoOutputEnabled ? (1.0f - position) * 0.5f : 0.5f;
				if (destinationDetails.InputChannels >= 2)
					matrix[1] = s_stereoOutputEnabled ? (1.0f + position) * 0.5f : 0.5f;
				s_vagStream.voice->SetOutputMatrix(s_musicMasterVoice, 1,
					destinationDetails.InputChannels, matrix.data());
			}
		}
		return;
	}
	for (EFFECTVOICE& active : pamb->voices)
		if (active.voice != nullptr)
		{
			active.voice->SetVolume(EffectVoiceVolume(active,
				(std::max)(0.0f, attenuatedVolume + static_cast<float>(pamb->lfoVolume) / 127.0f)));
			SetEffectVoicePan(active.voice,
				active.authoredPan + pamb->lfoPan + EffectSpatialPanDegrees(pamb->pan));
		}
}

void SetPambFrq(AMB* pamb, float frq)
{
	if (pamb == nullptr)
		return;

	// 989SND's application pitch modifier is expressed in units of 1524
	// fine steps, with 128 fine steps per semitone. Apply the change from
	// the previous modifier to each tone's native pitch so continuously
	// controlled sounds (engines, tire slip, etc.) actually track it.
	const double fineDelta = static_cast<double>(frq - pamb->frq) * 1524.0;
	const float pitchRatio = static_cast<float>(std::pow(2.0,
		fineDelta / (128.0 * 12.0)));
	for (EFFECTVOICE& active : pamb->voices)
	{
		active.baseFrequencyRatio = std::clamp(active.baseFrequencyRatio * pitchRatio,
			XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio);
	}
	pamb->frq = frq;
	if (!pamb->voices.empty())
		pamb->frequencyRatioBase = pamb->voices.front().baseFrequencyRatio;
	SetDoppler(pamb);
}

void SetDoppler(AMB* pamb)
{
	if (pamb == nullptr)
		return;

	pamb->dnDoppler = 0;
	float dopplerRatio = 1.0f;

	if (pamb->palo != nullptr && pamb->rDoppler > 0.0001f)
	{
		PO* listener = PpoCur();
		if (listener != nullptr)
		{
			glm::vec3 sourceVelocity{};
			glm::vec3 listenerVelocity{};
			CalculateAloMovement(pamb->palo, nullptr, pamb->palo->xf.posWorld,
				&sourceVelocity, nullptr, nullptr, nullptr);
			CalculateAloMovement(reinterpret_cast<ALO*>(listener), nullptr, listener->xf.posWorld,
				&listenerVelocity, nullptr, nullptr, nullptr);

			const glm::vec3 displacement = listener->xf.posWorld - pamb->palo->xf.posWorld;
			const float distanceSquared = glm::dot(displacement, displacement);
			if (distanceSquared > 0.00000001f)
			{
				const glm::vec3 direction = displacement / std::sqrt(distanceSquared);
				const float radialVelocity = glm::dot(sourceVelocity - listenerVelocity, direction);
				const float pitchCents = radialVelocity * pamb->rDoppler * 0.0225f;
				pamb->dnDoppler = static_cast<int>(std::round(pitchCents));
				dopplerRatio = static_cast<float>(std::pow(2.0, pitchCents / 1200.0));
			}
		}
	}

	for (EFFECTVOICE& active : pamb->voices)
	{
		if (!active.voice) continue;
		const float baseRatio = active.baseFrequencyRatio > 0.0f ? active.baseFrequencyRatio : 1.0f;
		const float ratio = std::clamp(baseRatio * dopplerRatio,
			XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio);
		active.voice->SetFrequencyRatio(ratio);
	}
}

void ScheduleNextIntermittentSound(AMB* pamb)
{
	if (pamb == nullptr)
		return;

	const float repeatMin = (std::min)(pamb->lmRepeat.gMin, pamb->lmRepeat.gMax);
	const float repeatMax = (std::max)(pamb->lmRepeat.gMin, pamb->lmRepeat.gMax);
	pamb->tNext = g_clock.t + GRandInRange(repeatMin, repeatMax);

	// Object-attached intermittent sounds retain the object's current position.
	if (pamb->palo != nullptr)
		return;

	const float angle = GRandInRange(0.0f, 6.2831855f);
	const float distanceMin = (std::min)(pamb->lmRepDist.gMin, pamb->lmRepDist.gMax);
	const float distanceMax = (std::max)(pamb->lmRepDist.gMin, pamb->lmRepDist.gMax);
	const float distance = GRandInRange(distanceMin, distanceMax);

	const glm::vec3 offset(std::cos(angle) * distance, std::sin(angle) * distance, 0.0f);
	const glm::vec3* pposEar = PposSoundEar();
	pamb->pos = (pposEar != nullptr ? *pposEar : glm::vec3(0.0f)) + offset;
}

void UnsetExcitementHyst(EXC* pexc)
{
	if (pexc == nullptr)
		return;
	const IEXC removedExcitement = pexc->iexc;
	RemoveExc(pexc);

	if (removedExcitement == s_iexcCurHigh)
	{
		g_iexcHyst = removedExcitement;
		SetIexcCurHigh(nullptr);
	}
}

EXC* PexcAlloc()
{
	if (g_psw == nullptr)
		return nullptr;
	for (std::shared_ptr<EXC>& slot : g_psw->excOwners)
	{
		if (slot)
			continue;
		slot = std::make_shared<EXC>();
		EXC* pexc = slot.get();
		AppendDlEntry(&g_psw->dlExc, pexc);
		return pexc;
	}
	return nullptr;
}

void RemoveExc(EXC* pexc)
{
	if (pexc == nullptr || g_psw == nullptr)
		return;
	RemoveDlEntry(&g_psw->dlExc, pexc);
	for (std::shared_ptr<EXC>& slot : g_psw->excOwners)
	{
		if (slot.get() == pexc)
		{
			slot.reset();
			return;
		}
	}
}

void SetIexcCurHigh(EXC*)
{
	if (g_psw == nullptr || FIsDlEmpty(&g_psw->dlExc) != 0)
	{
		s_iexcCurHigh = 0;
		return;
	}
	s_iexcCurHigh = -20;
	for (EXC* current = g_psw->dlExc.pexcFirst; current != nullptr;
		current = current->dle.pexcNext)
		s_iexcCurHigh = (std::max)(s_iexcCurHigh, current->iexc);
}

EXC* PexcSetExcitement(IEXC iexc)
{
	if (g_psw == nullptr)
		return nullptr;

	const bool wasEmpty = FIsDlEmpty(&g_psw->dlExc) != 0;
	EXC* pexc = PexcAlloc();
	if (pexc == nullptr)
		return nullptr;
	pexc->iexc = std::clamp(iexc, -20, 107);

	if (wasEmpty || pexc->iexc > s_iexcCurHigh)
	{
		g_iexcHyst = s_iexcCurHigh;
		s_iexcCurHigh = pexc->iexc;
	}

	return pexc;
}

void UnsetExcitement(EXC* pexc)
{
	if (pexc == nullptr || g_psw == nullptr)
		return;

	const IEXC removedExcitement = pexc->iexc;
	RemoveExc(pexc);

	if (removedExcitement != s_iexcCurHigh)
		return;

	SetIexcCurHigh(nullptr);
	if (s_iexcCurHigh >= 0)
		g_fSneakyFeet = 0;
	g_iexcHyst = s_iexcCurHigh;
	SetGlobalExciteValue(s_iexcCurHigh + 20);
	SetMvgkRvol(1.0f, 7, 1);
	g_iexcHyst = -100;
}

void StartAloAmbient(float sStart, float sFull, ALO* palo, AMB** pamb)
{
	if (!s_vagStream.loaded || !s_vagStream.voice || palo == nullptr || g_psw == nullptr)
		return;

	// Retail routes the already-loaded VAG through StartSound with SFXID -2.
	// That creates a normal tracked AMB solely for spatial volume/pan updates.
	if (s_pambVagAmbient != nullptr)
		RemoveAmb(s_pambVagAmbient);
	AMB* ambient = PambAlloc();
	if (!ambient)
		return;
	ambient->sfxid = static_cast<SFXID>(-2);
	ambient->tStarted = g_clock.t;
	ambient->ppamb = pamb;
	if (pamb)
		*pamb = ambient;
	ambient->palo = palo;
	ambient->pos = palo->xf.posWorld;
	ambient->sStart = sStart;
	ambient->sFull = sFull;
	ambient->uVolAtSource = 1.0f;
	ambient->fContinuous = 1;
	ambient->fAttached = 1;
	ambient->handlerDone = true;
	s_pambVagAmbient = ambient;
	s_hvagAmbient = 1;
	SetPambVol(ambient, 1.0f);
}

bool FPauseForVag()
{
	if (!s_vagStream.loaded)
		return false;

	if (!s_vagStream.started)
	{
		const HRESULT result = s_vagStream.voice ? s_vagStream.voice->Start() : E_POINTER;
		if (FAILED(result))
		{
			CloseVagStream();
			return false;
		}
		s_vagStream.started = true;
		s_vagStream.paused = false;
		s_dialogMusicDuck = 0.2f;
		for (MusicVoice& active : s_music989.voices)
			if (active.voice) active.voice->SetVolume(
				active.baseVolume * (active.envelope.level / 32767.0f) *
				s_channelVolumes[std::clamp(s_music989.cueVolumeGroup, 0,
					static_cast<int>(s_channelVolumes.size()) - 1)] *
				s_musicVolume * s_dialogMusicDuck);
	}

	return false;
}

int FVagPlaying()
{
	if (!s_vagStream.loaded)
		return 0;

	if (!s_vagStream.started)
		return 1;

	if (s_vagStream.paused)
		return 1;

	XAUDIO2_VOICE_STATE state{};
	s_vagStream.voice->GetState(&state);
	if (state.BuffersQueued != 0)
		return 1;

	CloseVagStream();
	return 0;
}

bool FHasAmbientVag()
{
	return s_hvagAmbient != 0;
}

int HsNextFootFall()
{
	// Retail data at 0x0024D2A8 and 0x0024D350.
	static constexpr int aadhs[7][6] =
	{
		{ -5, -5, -5, -5, -5, -5 },
		{ -5, -5, -5, -5, -5,  1 },
		{ -5, -5, -5, -5,  1,  1 },
		{ -5, -5, -5,  1,  1,  1 },
		{ -5, -5,  1,  1,  1,  1 },
		{ -5,  1,  1,  1,  1,  1 },
		{  1,  1,  1,  1,  1,  1 }
	};
	static constexpr int iadhs[25] =
	{
		6, 6, 6, 6, 6,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		4, 3, 2, 1, 0
	};
	static int hsFootfallCurrent = 0; // retail _DAT_00274830

	const int randomChoice = NRandInRange(0, 5);
	hsFootfallCurrent += aadhs[iadhs[hsFootfallCurrent + 12]][randomChoice];
	return hsFootfallCurrent;
}

void NextSneakyFootstep()
{
	if (g_fSneakyFeet == 0)
		return;

	const int hsFootfall = HsNextFootFall();
	StartSound(static_cast<SFXID>(119), nullptr, nullptr, nullptr, 0.0f, 0.0f, 0.5f,
		static_cast<float>(hsFootfall) * (1.0f / 12.0f), 0.0f, nullptr, nullptr);
}

void HandleWipeVolumes(int fMusicWipes, float u)
{
	constexpr int kWipeVolumeGroup = 6;
	SetMvgkRvol(u, kWipeVolumeGroup, 0); // Effects
	SetMvgkRvol(u, kWipeVolumeGroup, 2); // Dialog
	SetMvgkRvol(u, kWipeVolumeGroup, 3); // User 1

	if (fMusicWipes != 0)
		SetMvgkRvol(u, kWipeVolumeGroup, 1); // Music
}

void ResetCommentaryAudio()
{
	StopVag();
}

void SetAttractVolume(bool fMuted)
{
	SetMvgkRvol(fMuted ? 0.0f : 1.0f, 3, 1);
}

void SetAttractSoundOption(bool fEnabled)
{
	s_stereoOutputEnabled = fEnabled;
}

void StopVag()
{
	if (!s_vagStream.loaded)
		return;

	CloseVagStream();
}

void SetMvgkRvol(float uVol, int mgvk, int iChannel)
{
	if (mgvk < 0 || mgvk >= static_cast<int>(s_relativeVolumes.size()) ||
		iChannel < 0 || iChannel >= static_cast<int>(s_channelVolumes.size()))
		return;

	s_relativeVolumes[mgvk][iChannel] = std::clamp(uVol, 0.0f, 1.0f);
	RecalculateChannelVolume(iChannel);
}

void ResumeVag()
{
	ContinueVag();
}

void PauseVag()
{
	if (s_vagStream.loaded && s_vagStream.started && !s_vagStream.paused)
	{
		if (s_vagStream.voice && SUCCEEDED(s_vagStream.voice->Stop()))
			s_vagStream.paused = true;
	}
}

void ContinueVag()
{
	if (s_vagStream.loaded && !s_vagStream.started)
	{
		FPauseForVag();
	}
	else if (s_vagStream.loaded && s_vagStream.paused)
	{
		if (s_vagStream.voice && SUCCEEDED(s_vagStream.voice->Start()))
			s_vagStream.paused = false;
	}
}

void PauseMusicSequencer()
{
	if (s_musicStream.state == MusicState::Playing && !s_musicStream.paused)
	{
		for (MusicVoice& active : s_music989.voices)
			if (active.voice)
				active.voice->Stop();
		s_musicStream.paused = true;
	}
}

void ContinueMusicSequencer()
{
	if (s_musicStream.state == MusicState::Playing && s_musicStream.paused)
	{
		for (MusicVoice& active : s_music989.voices)
			if (active.voice)
				active.voice->Start();
		s_music989.lastUpdate = std::chrono::steady_clock::now();
		s_musicStream.paused = false;
	}
}

void PreloadVag(const char* descriptorOrPath)
{
	StopVag();

	const std::filesystem::path wavPath = ResolveVagWav(descriptorOrPath);
	if (wavPath.empty())
	{
		return;
	}

	if (!EnsureMusicEngine())
	{
		return;
	}

	VagStreamState replacement{};
	if (!LoadPcmWave(wavPath, replacement.format, replacement.pcm))
	{
		return;
	}
	WAVEFORMATEX format{};
	format.wFormatTag = replacement.format.formatTag;
	format.nChannels = replacement.format.channels;
	format.nSamplesPerSec = replacement.format.samplesPerSec;
	format.nAvgBytesPerSec = replacement.format.avgBytesPerSec;
	format.nBlockAlign = replacement.format.blockAlign;
	format.wBitsPerSample = replacement.format.bitsPerSample;
	const HRESULT createResult = s_xaudio2->CreateSourceVoice(&replacement.voice, &format);
	if (FAILED(createResult))
	{
		return;
	}

	// Do not rely on XAudio2's implicit channel mapping for the mono dialog
	// files. Explicitly send mono speech to both front speakers (and map stereo
	// directly) so it remains audible on stereo and surround output devices.
	XAUDIO2_VOICE_DETAILS masterDetails{};
	s_musicMasterVoice->GetVoiceDetails(&masterDetails);
	const UINT32 sourceChannels = replacement.format.channels;
	const UINT32 destinationChannels = masterDetails.InputChannels;
	std::vector<float> outputMatrix(
		static_cast<std::size_t>(sourceChannels) * destinationChannels, 0.0f);
	if (sourceChannels == 1)
	{
		if (destinationChannels >= 1) outputMatrix[0] = 1.0f;
		if (destinationChannels >= 2) outputMatrix[1] = 1.0f;
	}
	else
	{
		const UINT32 mappedChannels = (std::min)(sourceChannels, destinationChannels);
		for (UINT32 channel = 0; channel < mappedChannels; ++channel)
			outputMatrix[static_cast<std::size_t>(channel) * destinationChannels + channel] = 1.0f;
	}
	const HRESULT matrixResult = replacement.voice->SetOutputMatrix(
		s_musicMasterVoice, sourceChannels, destinationChannels, outputMatrix.data());
	if (FAILED(matrixResult))
	{
		replacement.voice->DestroyVoice();
		return;
	}

	XAUDIO2_BUFFER buffer{};
	buffer.AudioBytes = static_cast<UINT32>(replacement.pcm.size());
	buffer.pAudioData = replacement.pcm.data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	const HRESULT submitResult = replacement.voice->SubmitSourceBuffer(&buffer);
	if (FAILED(submitResult))
	{
		replacement.voice->DestroyVoice();
		return;
	}

	replacement.loaded = true;
	replacement.path = wavPath;
	s_vagStream = std::move(replacement);
	RecalculateChannelVolume(2);
}

void PushSwReverb(SW* psw, REVERBK rvrbk, int depth)
{
	if (!psw || psw->irse < 0 || psw->irse >= 4)
		return;

	psw->arse[psw->irse].rvrbk = rvrbk;
	psw->arse[psw->irse].depth = depth;
	++psw->irse;
	s_effectReverbKind = rvrbk;
	s_effectReverbDepth = depth;
	if (s_xaudio2)
	{
		EnsureEffectReverb();
		ApplyCurrentEffectReverb();
	}
}

void PopSwReverb(SW* psw)
{
	if (!psw || psw->irse <= 1)
		return;

	--psw->irse;
	const RSE& restored = psw->arse[psw->irse - 1];
	s_effectReverbKind = restored.rvrbk;
	s_effectReverbDepth = restored.depth;
	if (s_xaudio2)
	{
		EnsureEffectReverb();
		ApplyCurrentEffectReverb();
	}
}

void PlayCommentaryAudio(char* audioFileLocation)
{
	// Retail does not replace commentary that is already playing.
	if (FPlayingCommentaryAudio() || audioFileLocation == nullptr)
		return;

	// A world/animation VAG may already own the single streaming voice.
	if (FVagPlaying())
		StopVag();

	PreloadVag(audioFileLocation);
	if (!s_vagStream.loaded)
		return;

	s_vagStream.commentary = true;
	FPauseForVag();
	if (!s_vagStream.loaded || !s_vagStream.started)
		return;

	// PlayCommentaryAudio applies mixer modifier 8 after successfully starting
	// the stream. It ducks music, effects, and user channel 1 to one half while
	// leaving the dialog channel untouched.
	s_dialogMusicDuck = 1.0f;
	SetMvgkRvol(0.5f, 8, 1);
	SetMvgkRvol(0.5f, 8, 0);
	SetMvgkRvol(0.5f, 8, 3);
}

bool FPlayingCommentaryAudio()
{
	return s_vagStream.loaded && s_vagStream.commentary;
}

void ContinueMusic()
{
	s_musicVolume = 1.0f;
	s_music989.lastUpdate = std::chrono::steady_clock::now();
	for (MusicVoice& active : s_music989.voices)
		if (active.voice) active.voice->SetVolume(
			active.baseVolume * (active.envelope.level / 32767.0f) *
			s_channelVolumes[std::clamp(s_music989.cueVolumeGroup, 0,
				static_cast<int>(s_channelVolumes.size()) - 1)] * s_dialogMusicDuck);
}

void PauseMusic()
{
	s_musicVolume = 0.0f;
	for (MusicVoice& active : s_music989.voices)
		if (active.voice) active.voice->SetVolume(0.0f);
}

void UnloadMusic()
{
	CloseMusicVoice();
}

void KillMusic()
{
	UnloadMusic();
}

void KillSounds(int msRampdown)
{
	(void)msRampdown;
	// Positional AMB playback is not backed by live PC voices yet. The active
	// streamed VAG/dialog is, so stop and close it during world teardown.
	StopVag();
	if (g_psw != nullptr)
	{
		while (g_psw->dlAmb.pvFirst != nullptr)
			RemoveAmb(static_cast<AMB*>(g_psw->dlAmb.pvFirst));

		// Also release any detached pool entries which are no longer members of
		// dlAmb (DropPamb deliberately does not free them).
		for (std::unique_ptr<AMB>& slot : g_psw->ambOwners)
		{
			if (slot && slot->ppamb != nullptr)
				*slot->ppamb = nullptr;
			slot.reset();
		}
	}
}

void KillExcitement()
{
	if (g_psw != nullptr)
	{
		while (g_psw->dlExc.pexcFirst != nullptr)
			RemoveExc(g_psw->dlExc.pexcFirst);
		for (std::shared_ptr<EXC>& slot : g_psw->excOwners)
			slot.reset();
	}
	s_iexcCurHigh = 0;
	s_globalExcite = 20;
	g_iexcHyst = -100;
	g_fSneakyFeet = 0;
}

void ResetAMRegisters()
{
	for (int ireg = 0; ireg < 8; ++ireg)
		SetAMRegister(ireg, 0);
}

void KillSoundSystem()
{
	// Retail stops the active 989SND handle before it clears ambient sounds,
	// excitement, and the eight cached AM registers.  Closing our in-process
	// sequencer first prevents register reset callbacks from being applied to
	// the outgoing world's still-live music state.
	KillMusic();
	KillSounds(0);
	KillExcitement();
	ResetAMRegisters();

	// Per-world volume modifiers must not leak into the next world.
	for (auto& group : s_relativeVolumes)
		group.fill(1.0f);
	s_channelVolumes = { 1.0f, 0.5f, 1.0f, 1.0f };
	s_musicVolume = 1.0f;
	s_dialogMusicDuck = 1.0f;
	s_effectTickAccumulator = 0.0;
	s_effectLastUpdate = {};
	s_effectGlobalRegisters.fill(0);
	s_fPauseAmbients = 0;
	s_effectReverbKind = REVERBK_None;
	s_effectReverbDepth = 0;
	ApplyCurrentEffectReverb();
	// Bank 0 is permanent in the original. The current world bank is released
	// by SbpEnsureBank when the next world requests a different IBNK.
}

SoundBankPtr SbpEnsureBank(int ibnk)
{
	if (ibnk < 0 || ibnk >= static_cast<int>(kEffectBankCount))
		return nullptr;
	if (ibnk == s_ibnkWorldCur && s_effectBanks[ibnk])
		return s_effectBanks[ibnk].get();
	if (ibnk == 0 && s_effectBanks[0])
		return s_effectBanks[0].get();

	if (ibnk != 0)
	{
		UnloadMusic();
		if (s_ibnkWorldCur > 0 && s_ibnkWorldCur < static_cast<int>(kEffectBankCount))
			s_effectBanks[s_ibnkWorldCur].reset();
	}
	// Bank zero is the permanent common bank and must not replace the tracked
	// world-bank index. This matches the retail bank-zero special case.
	if (ibnk != 0)
		s_ibnkWorldCur = ibnk;

	const std::filesystem::path path = FindEffectBank(ibnk);
	if (path.empty())
	{
		std::printf("[SOUND BANK] Could not find extracted IBNK %d.\n", ibnk);
		return nullptr;
	}
	std::ifstream input(path, std::ios::binary);
	if (!input.is_open())
		return nullptr;
	std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
	if (bytes.empty())
	{
		std::printf("[SOUND BANK] Extracted bank is empty: %ls\n", path.c_str());
		return nullptr;
	}

	auto replacement = std::make_unique<EFFECTBANK>();
	replacement->ibnk = ibnk;
	replacement->bytes = std::move(bytes);
	if (!Parse989SfxBank(*replacement))
	{
		std::printf("[SOUND BANK] Invalid or unsupported 989SND SBlk: %ls\n", path.c_str());
		return nullptr;
	}
	s_effectBanks[ibnk] = std::move(replacement);
	if (!s_mpibnkfEverLoaded[ibnk])
	{
		s_mpibnkfEverLoaded[ibnk] = true;
		for (std::size_t sfxid = 0; sfxid < kSfxCount; ++sfxid)
		{
			const SND& mapping = g_mpsfxidsndbnk[sfxid];
			if (mapping.ibnk != ibnk || mapping.isnd < 0 ||
				mapping.isnd >= static_cast<int>(s_effectBanks[ibnk]->sounds.size()))
				continue;
			s_mpsfxidcont[sfxid] = s_effectBanks[ibnk]->sounds[mapping.isnd].IsLooper();
		}
	}
	return s_effectBanks[ibnk].get();
}

void UnloadEffectBank()
{
	for (std::unique_ptr<EFFECTBANK>& bank : s_effectBanks)
		bank.reset();
	s_ibnkWorldCur = -1;
	s_mpibnkfEverLoaded.fill(false);
	s_mpsfxidcont.fill(false);
}

void PreloadMusidSong(MUSID musid)
{
	if (musid == MUSID_Nil)
	{
		KillMusic();
		return;
	}
	if (s_musicStream.state != MusicState::None && s_musicStream.musid == musid)
		return;

	UnloadMusic();
	const std::filesystem::path bankPath = FindMusicBank(musid);
	if (bankPath.empty())
	{
		return;
	}
	const bool loaded = bankPath.extension() == L".mus"
		? LoadMusic989Mus(bankPath)
		: LoadMusic989Bank(bankPath);
	if (!EnsureMusicEngine() || !loaded)
	{
		CloseMusicVoice();
		return;
	}
	s_music989.registers = s_amRegisters;
	s_musicStream.musid = musid;
	s_musicStream.path = bankPath;
	s_musicStream.state = MusicState::Loaded;
}

void StartMusidSong(MUSID musid)
{
	PreloadMusidSong(musid);
	if (s_musicStream.state == MusicState::None)
		return;
	if (s_musicStream.state == MusicState::Playing)
		return;

	QueueMusicSegment(0);
	ActivateQueuedMusicSegments();
	s_music989.lastUpdate = std::chrono::steady_clock::now();
	s_musicStream.state = MusicState::Playing;
	s_musicStream.paused = false;
}

bool FContinuousSound(SFXID sfxid)
{
	const int index = static_cast<int>(sfxid);
	if (index < 0 || index >= static_cast<int>(kSfxCount))
		return false;

	const int ibnk = g_mpsfxidsndbnk[index].ibnk;
	if (ibnk < 0 || ibnk >= static_cast<int>(kEffectBankCount))
		return false;

	// Retail lazily loads/inspects a bank the first time continuity is queried.
	// SbpEnsureBank takes an IBNK in this port, so translate the global SFXID
	// through g_mpsfxidsndbnk instead of passing the SFXID directly.
	if (!s_mpibnkfEverLoaded[ibnk] && SbpEnsureBank(ibnk) == nullptr)
		return false;

	return s_mpsfxidcont[index];
}

void SetSwDefaultReverb(SW* psw, REVERBK rvrbk, int depth)
{
	if (!psw)
		return;

	psw->arse[0].rvrbk = rvrbk;
	psw->arse[0].depth = depth;
	psw->irse = 1;
	s_effectReverbKind = rvrbk;
	s_effectReverbDepth = depth;
	if (s_xaudio2)
	{
		EnsureEffectReverb();
		ApplyCurrentEffectReverb();
	}
}

void StartupSound()
{
	// The PS2 clears its seven bank handles before starting 989SND. Rebuild the
	// equivalent PC-side state, initialize XAudio2, and keep bank 0 resident as
	// the common effects bank.
	UnloadEffectBank();
	if (!EnsureMusicEngine())
	{
		std::printf("[SOUND] Could not initialize XAudio2.\n");
		return;
	}

	SbpEnsureBank(0);
	s_globalExcite = 20;
	InitializeSoundOptions();

	// Mixer mode, SPU voice ranges, VAG stream buffers, and reverb work-area
	// allocation are owned internally by XAudio2 in this port.
	s_effectReverbKind = REVERBK_None;
	s_effectReverbDepth = 0;
	EnsureEffectReverb();
	ApplyCurrentEffectReverb();
}

void InitializeSoundOptions()
{
	// Retail copies its 11x4 default MVGK matrix into the live matrix here.
	// ProjectCane currently represents those defaults as unity multipliers;
	// channel 1's original 0.5 base gain is applied by RecalculateChannelVolume.
	for (auto& group : s_relativeVolumes)
		group.fill(1.0f);

	SetMasterVolume(1.0f);
	for (int channel = 0; channel < 4; ++channel)
		RecalculateChannelVolume(channel);

	if (g_pgsCur)
	{
		SetAttractVolume((g_pgsCur->grfgs & 0x80) != 0);
		SetAttractSoundOption((g_pgsCur->grfgs & 0x40) != 0);
	}
}

void SetMasterVolume(float uVol)
{
	// Retail sends group 8 a 0..1024 fixed-point gain. XAudio2 uses the
	// equivalent normalized float directly on the mastering voice.
	s_masterVolume = std::clamp(uVol, 0.0f, 1.0f);
	if (EnsureMusicEngine() && s_musicMasterVoice)
		s_musicMasterVoice->SetVolume(s_masterVolume);
}

void SetSwIntermittentSound(SW* psw, SFXID sfxid)
{
	if (psw->cisi >= 16)
		return;

	ISI& isi = psw->aisi[psw->cisi++];

	isi.sfxid = sfxid;
	isi.lmRepeat = s_lmRepeatDefault;
	isi.lmRepDist = s_lmDistDefault;
}

void SetSwIntermittentVolPct(SW* psw, LM plmVolPct)
{
	if (psw->cisi == 0)
		return;

	ISI& isi = psw->aisi[psw->cisi - 1];

	isi.lmRepDist.gMin = 8000.0f - plmVolPct.gMax * 50.0f;
	isi.lmRepDist.gMax = 8000.0f - plmVolPct.gMin * 50.0f;
}

void SetSwIntermittentRepeat(SW* psw, LM plmRepeat)
{
	if (psw->cisi == 0)
		return;

	psw->aisi[psw->cisi - 1].lmRepeat = plmRepeat;
}

void StartSwIntermittentSounds(SW* psw)
{
	for (int i = 0; i < psw->cisi; ++i)
	{
		ISI& isi = psw->aisi[i];

		StartSound(isi.sfxid, nullptr, nullptr, nullptr, 8000.0f, 3000.0f, 1.0f, 0.0f, 0.0f, &isi.lmRepeat, &isi.lmRepDist);
	}
}

void UpdateSounds()
{
	if (FAmbientsPaused() != 0)
		return;

	// Retail polls the active VAG handle here every sound update. Without this,
	// a naturally completed dialog stream can leave music ducking engaged until
	// some unrelated caller happens to query FVagPlaying().
	if (s_vagStream.loaded && s_vagStream.started && !s_vagStream.paused)
		FVagPlaying();

	if (g_iexcHyst != -100)
	{
		// Retail raises positive excitement immediately, but walks downward and
		// through the sneaking range one step every three sound updates.
		if (s_iexcCurHigh > g_iexcHyst && s_iexcCurHigh > 0)
			g_iexcHyst = s_iexcCurHigh;
		else if (--s_cframeExcitementHyst < 0)
		{
			if (g_iexcHyst < s_iexcCurHigh)
				++g_iexcHyst;
			else if (g_iexcHyst > s_iexcCurHigh)
				--g_iexcHyst;
			s_cframeExcitementHyst += 3;
		}

		g_fSneakyFeet = g_iexcHyst < 0 ? 1 : 0;
		SetGlobalExciteValue(g_iexcHyst + 20);

		if (g_iexcHyst < 0)
		{
			const float uSneak = std::clamp(static_cast<float>(g_iexcHyst) * -0.05f, 0.0f, 1.0f);
			SetMvgkRvol((1.0f - uSneak) + uSneak * (2.0f / 3.0f), 7, 1);
		}
		else
			SetMvgkRvol(1.0f, 7, 1);

		if (g_iexcHyst == s_iexcCurHigh)
			g_iexcHyst = -100;
	}

	const auto effectNow = std::chrono::steady_clock::now();
	if (s_effectLastUpdate.time_since_epoch().count() == 0)
		s_effectLastUpdate = effectNow;
	const double effectElapsed = std::chrono::duration<double>(effectNow - s_effectLastUpdate).count();
	s_effectLastUpdate = effectNow;
	s_effectTickAccumulator += (std::min)(effectElapsed, 0.25) * 240.0;
	const int effectTicks = static_cast<int>(s_effectTickAccumulator);
	s_effectTickAccumulator -= effectTicks;

	if (g_psw != nullptr)
	{
		auto restartEffectHandler = [](AMB& amb)
		{
			EFFECTBANK* bank = static_cast<EFFECTBANK*>(amb.bnk);
			if (!bank || amb.soundIndex < 0 ||
				amb.soundIndex >= static_cast<int>(bank->sounds.size()) ||
				bank->sounds[amb.soundIndex].grains.empty())
				return;
			amb.nextGrain = 0;
			amb.grainCountdown = bank->sounds[amb.soundIndex].grains[0].delay;
			amb.handlerDone = false;
			amb.currentPitchBend = 0;
			int immediateBudget = 1024;
			while (amb.grainCountdown <= 0 && !amb.handlerDone && immediateBudget-- > 0)
				ExecuteEffectGrain(amb);
			if (immediateBudget <= 0)
				amb.handlerDone = true;
		};
		std::vector<AMB*> completed;
		for (const std::unique_ptr<AMB>& slot : g_psw->ambOwners)
		{
			if (!slot)
				continue;
			AMB& amb = *slot;
			if (static_cast<int>(amb.sfxid) == -2 && !s_vagStream.loaded)
			{
				completed.push_back(&amb);
				continue;
			}
			// Retail removes an attached AMB as soon as its owning object leaves
			// the active world. This is what keys off the six Common SFX 27 voices
			// shortly after Clockwerk finishes loading; an infinite XAudio loop must
			// not outlive the ALO that created it.
			if (amb.palo != nullptr && !FIsLoInWorld(reinterpret_cast<LO*>(amb.palo)))
			{
				completed.push_back(&amb);
				continue;
			}
			CleanEffectVoices(amb);
			const bool wasOutOfRange = amb.fOutOfRange != 0;
			if (amb.fAttached)
				SetPambVol(&amb, amb.uVolAtSource);

			const bool recurring = amb.lmRepeat.gMin >= 0.0f;
			if (amb.fOutOfRange && !amb.fContinuous && !recurring)
			{
				completed.push_back(&amb);
				continue;
			}
			if (amb.fContinuous && amb.fOutOfRange && !wasOutOfRange)
			{
				// Retail stops an inaudible continuous hardware voice and recreates it
				// on re-entry instead of leaving a muted loop consuming a voice.
				for (EFFECTVOICE& active : amb.voices)
				{
					if (!active.voice)
						continue;
					active.voice->Stop();
					active.voice->DestroyVoice();
					active.voice = nullptr;
					active.pcm.clear();
				}
				amb.voices.clear();
				amb.voice = nullptr;
				amb.handlerDone = true;
			}
			else if (amb.fContinuous && !amb.fOutOfRange && wasOutOfRange &&
				amb.voices.empty())
			{
				restartEffectHandler(amb);
			}

			if (recurring && amb.handlerDone && amb.voices.empty() &&
				g_clock.t >= amb.tNext)
			{
				ScheduleNextIntermittentSound(&amb);
				bool allowStart = true;
				if (wasOutOfRange)
				{
					allowStart = g_clock.t - s_tLastIntermittent >= 1.0f;
					if (allowStart)
						s_tLastIntermittent = g_clock.t;
				}
				// Scheduling may move an unattached intermittent source around the
				// listener, so calculate its new attenuation before deciding to play.
				SetPambVol(&amb, amb.uVolAtSource);
				if (allowStart && !amb.fOutOfRange)
					restartEffectHandler(amb);
			}
			for (int tick = 0; tick < effectTicks; ++tick)
			{
				TickEffectLfos(amb);
				UpdateEffectEnvelopes(amb);
				if (amb.handlerDone)
					continue;
				--amb.grainCountdown;
				int grainBudget = 1024;
				while (amb.grainCountdown <= 0 && !amb.handlerDone && grainBudget-- > 0)
					ExecuteEffectGrain(amb);
				if (grainBudget <= 0) amb.handlerDone = true;
			}
			CleanEffectVoices(amb);
			if (amb.handlerDone && amb.voices.empty() && !recurring && !amb.fContinuous)
				completed.push_back(&amb);
			else
				SetDoppler(&amb);
		}
		for (AMB* pamb : completed)
		{
			const bool stillOwned = std::any_of(g_psw->ambOwners.begin(), g_psw->ambOwners.end(),
				[pamb](const std::unique_ptr<AMB>& slot) { return slot.get() == pamb; });
			if (stillOwned)
				RemoveAmb(pamb);
		}
	}

	if (s_musicStream.state != MusicState::Playing || s_musicStream.paused)
		return;
	const auto now = std::chrono::steady_clock::now();
	if (s_music989.lastUpdate.time_since_epoch().count() == 0)
		s_music989.lastUpdate = now;
	const double elapsed = std::chrono::duration<double>(now - s_music989.lastUpdate).count();
	s_music989.lastUpdate = now;
	s_music989.tickAccumulator += (std::min)(elapsed, 0.25) * 240.0;
	while (s_music989.tickAccumulator >= 1.0)
	{
		for (MusicSegment& segment : s_music989.activeSegments)
			StepMusicSegment(segment);
		// AME branch/start commands can complete a segment directly instead of
		// going through StopMusicSegment(). Retail 989SND keys off every voice
		// owned by that completed handler. Leaving those voices in Sustain caused
		// old battle layers (notably the held trumpet) to survive under the next
		// segment and across dialog ducking.
		for (const MusicSegment& segment : s_music989.activeSegments)
		{
			if (!segment.complete)
				continue;
			for (MusicVoice& voice : s_music989.voices)
				if (voice.voice && voice.segment == segment.id)
					ReleaseMusicEnvelope(voice.envelope);
		}
		UpdateMusicEnvelopes();
		ActivateQueuedMusicSegments();
		s_music989.activeSegments.erase(
			std::remove_if(s_music989.activeSegments.begin(), s_music989.activeSegments.end(),
				[](const MusicSegment& segment) { return segment.complete; }),
			s_music989.activeSegments.end());
		s_music989.tickAccumulator -= 1.0;
	}
	for (MusicVoice& active : s_music989.voices)
	{
		if (!active.voice) continue;
		XAUDIO2_VOICE_STATE state{};
		active.voice->GetState(&state);
		if (state.BuffersQueued == 0)
		{
			active.voice->DestroyVoice();
			active.voice = nullptr;
		}
	}
	s_music989.voices.erase(
		std::remove_if(s_music989.voices.begin(), s_music989.voices.end(),
			[](const MusicVoice& active) { return active.voice == nullptr; }),
		s_music989.voices.end());
}

const std::array<SND, kSfxCount> g_mpsfxidsndbnk = []
{
	std::array<SND, kSfxCount> table{};
	for (int ibnk = 0; ibnk < 7; ++ibnk)
	{
		for (std::size_t sfxid = kEffectBankStarts[ibnk];
			sfxid < kEffectBankStarts[ibnk + 1]; ++sfxid)
		{
			table[sfxid].ibnk = ibnk;
			table[sfxid].isnd = static_cast<int>(sfxid - kEffectBankStarts[ibnk]);
		}
	}
	return table;
}();

static_assert(g_mpsfxidsndbnk.size() == kSfxCount);

IXAudio2* s_xaudio2 = nullptr;
IXAudio2MasteringVoice* s_musicMasterVoice = nullptr;
IXAudio2SubmixVoice* s_effectReverbVoice = nullptr;
REVERBK s_effectReverbKind = REVERBK_None;
int s_effectReverbDepth = 0;
float s_masterVolume = 1.0f;
bool s_stereoOutputEnabled = true;
MusicStreamState s_musicStream;
float s_musicVolume = 1.0f;
float s_dialogMusicDuck = 1.0f;
std::array<uint8_t, 16> s_amRegisters{};
uint8_t s_globalExcite = 20;
IEXC s_iexcCurHigh = 0;
int s_cframeExcitementHyst = 0;
AMB* s_pambVagAmbient = nullptr;
std::array<std::array<float, 4>, 11> s_relativeVolumes = []
{
	std::array<std::array<float, 4>, 11> values{};
	for (auto& group : values)
		group.fill(1.0f);
	return values;
}();
std::array<float, 4> s_channelVolumes{ 1.0f, 0.5f, 1.0f, 1.0f };
Music989State s_music989;
std::array<std::unique_ptr<EFFECTBANK>, kEffectBankCount> s_effectBanks;
int s_ibnkWorldCur = -1;
std::array<bool, kSfxCount> s_mpsfxidcont{};
std::array<bool, kEffectBankCount> s_mpibnkfEverLoaded{};
double s_effectTickAccumulator = 0.0;
std::chrono::steady_clock::time_point s_effectLastUpdate{};
std::array<int8_t, 32> s_effectGlobalRegisters{};
float s_tLastIntermittent = -100000.0f;
int s_fPauseAmbients = 0;
VagStreamState s_vagStream;
const std::array<const wchar_t*, kEffectBankCount> kEffectBankNames =
{
	L"Common.bnk", L"Intro.bnk", L"Snow.bnk", L"Underwater.bnk",
	L"Muggshot.bnk", L"Voodoo.bnk", L"Clockwerk.bnk"
};
LM s_lmRepeatDefault = { 4.0f, 6.0f };
LM s_lmDistDefault = { 3000.0f, 8000.0f };
int s_hvagAmbient = 0;
IEXC g_iexcHyst = 0;
int s_sfxidLast = 0;
int g_fSneakyFeet = 0;
