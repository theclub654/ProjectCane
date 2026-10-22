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
#include "spu_reverb_xapo.h"
#include "runtime_error.h"
#include "content.h"
#include "../platform/platform.h"
#ifdef __SWITCH__
#include "audio.h"
#include "audio_prefetch.h"
#include "vag_file.h"
#elif defined(_WIN32)
#include <Windows.h>
#include <mmsystem.h>
#include <xaudio2.h>
#include <xaudio2fx.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <propvarutil.h>
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Uuid.lib")
#pragma comment(lib, "Propsys.lib")
#else
#include "../platform/audio_linux.h"
#endif
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
#include <cfloat>
#include <memory>
#include <algorithm>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <string>
#include <limits>
#include <unordered_set>
#include <cctype>
#include <iomanip>
#include <deque>
#include <sstream>
#include <nlohmann/json.hpp>

extern std::string file;

static std::recursive_mutex s_musicMutex;
static std::atomic<bool> s_musicWorkerRunning{ false };
static std::atomic<bool> s_musicUsesGameplayClock{ false };
static std::thread s_musicWorker;
static int s_musicGameplayClockFrame = -1;
static constexpr float kVagDriverGain = 0x3ffc / 32767.0f;
// 989's cue volume-group byte is an internal sound-bank routing value.  It is
// not one of ProjectCane's four MVGK output channels; retail music is channel 1.
static constexpr int kMusicMixerChannel = 1;
static bool s_fFmvAudioMode = false;
static bool s_dialogAudioDuckHeld = false;
static bool s_commentaryStartPending = false;
static bool s_vagStartWhenReady = false;
static float s_userMusicVolume = 1.0f;
static float s_userSfxVolume = 1.0f;
static float s_userDialogueVolume = 1.0f;
static std::string s_audioOutputDeviceId;
static std::ofstream s_musicDropoutTrace;
static bool s_musicDropoutTraceEnabled = false;
static bool s_musicTraceHadVoices = false;
static std::deque<std::string> s_musicTraceHistory;
static constexpr std::size_t kMusicTraceHistoryLimit = 512;

static void TraceMusicEvent(const char* event, int value0 = -1, int value1 = -1)
{
	std::ostringstream line;
	line << "EVENT " << event
		<< " tick=" << g_clock.tickFrame
		<< " t=" << std::fixed << std::setprecision(6) << g_clock.t
		<< " value0=" << value0 << " value1=" << value1
		<< " state=" << static_cast<int>(s_musicStream.state)
		<< " musid=" << static_cast<int>(s_musicStream.musid)
		<< " paused=" << s_musicStream.paused
		<< " volume=" << s_musicVolume
		<< " duck=" << s_dialogMusicDuck
		<< " excite=" << static_cast<unsigned>(s_globalExcite)
		<< " segments=" << s_music989.activeSegments.size()
		<< " pending=" << s_music989.pendingStarts.size()
		<< " voices=" << s_music989.voices.size()
		<< " regs=";
	for (std::size_t i = 0; i < s_music989.registers.size(); ++i)
	{
		if (i != 0) line << ',';
		line << static_cast<unsigned>(s_music989.registers[i]);
	}
	for (const MusicSegment& segment : s_music989.activeSegments)
	{
		unsigned mutedMask = 0;
		for (std::size_t channel = 0; channel < segment.muted.size(); ++channel)
			if (segment.muted[channel]) mutedMask |= 1u << channel;
		line << " segment[" << segment.id
			<< ":complete=" << segment.complete
			<< ":ticks=" << segment.ticksLeft
			<< ":muted=0x" << std::hex << mutedMask << std::dec << ']';
	}
	line << '\n';
	const std::string entry = line.str();
	s_musicTraceHistory.push_back(entry);
	if (s_musicTraceHistory.size() > kMusicTraceHistoryLimit)
		s_musicTraceHistory.pop_front();
	if (s_musicDropoutTraceEnabled && s_musicDropoutTrace.is_open())
	{
		s_musicDropoutTrace << entry;
		s_musicDropoutTrace.flush();
	}
}

struct AudioOutputDevice
{
	std::string id;
	std::string name;
};

#ifdef _WIN32
static std::string Utf8FromWide(const wchar_t* text)
{
	if (text == nullptr || *text == L'\0') return {};
	const int bytes = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
	std::string result(static_cast<size_t>((std::max)(bytes, 1)), '\0');
	if (bytes > 1) WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), bytes, nullptr, nullptr);
	if (!result.empty() && result.back() == '\0') result.pop_back();
	return result;
}

static std::wstring WideFromUtf8(const std::string& text)
{
	if (text.empty()) return {};
	const int chars = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
	std::wstring result(static_cast<size_t>((std::max)(chars, 1)), L'\0');
	if (chars > 1) MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, result.data(), chars);
	if (!result.empty() && result.back() == L'\0') result.pop_back();
	return result;
}
#endif

static std::vector<AudioOutputDevice> AudioOutputDevices()
{
	std::vector<AudioOutputDevice> devices{ { {}, "System Default" } };
#ifdef _WIN32
	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	IMMDeviceEnumerator* enumerator = nullptr;
	IMMDeviceCollection* collection = nullptr;
	if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
		IID_PPV_ARGS(&enumerator))) &&
		SUCCEEDED(enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection)))
	{
		UINT count = 0;
		collection->GetCount(&count);
		for (UINT i = 0; i < count; ++i)
		{
			IMMDevice* device = nullptr;
			LPWSTR id = nullptr;
			IPropertyStore* properties = nullptr;
			PROPVARIANT name;
			PropVariantInit(&name);
			if (SUCCEEDED(collection->Item(i, &device)) && SUCCEEDED(device->GetId(&id)) &&
				SUCCEEDED(device->OpenPropertyStore(STGM_READ, &properties)) &&
				SUCCEEDED(properties->GetValue(PKEY_Device_FriendlyName, &name)))
			{
				devices.push_back({ Utf8FromWide(id), Utf8FromWide(name.pwszVal) });
			}
			PropVariantClear(&name);
			if (properties) properties->Release();
			if (id) CoTaskMemFree(id);
			if (device) device->Release();
		}
	}
	if (collection) collection->Release();
	if (enumerator) enumerator->Release();
	if (SUCCEEDED(comResult)) CoUninitialize();
#endif
	return devices;
}
static IXAudio2SubmixVoice* s_effectDryVoice = nullptr;
static IXAudio2SubmixVoice* s_effectWetInputVoice = nullptr;
static IXAudio2SubmixVoice* s_effectStudioCVoice = nullptr;
// The SPU2 clamps each core's accumulated voices before that core is routed to
// the final mixer.  Keep sequenced music on its own equivalent core bus so a
// dense arrangement cannot overdrive the shared XAudio mastering voice first.
static IXAudio2SubmixVoice* s_musicCoreVoice = nullptr;
static SpuStudioCReverb* s_effectStudioCProcessor = nullptr;
#ifdef _WIN32
static SpuMixSaturator* s_finalOutputProcessor = nullptr;
#endif
static bool s_effectReverbCreationAttempted = false;
static bool s_effectStudioCCreationAttempted = false;
static std::atomic<bool> s_audioCriticalError{ false };
static std::atomic<HRESULT> s_audioCriticalResult{ S_OK };
static bool s_audioRecoveryPending = false;
static bool s_soundStartupComplete = false;
static std::chrono::steady_clock::time_point s_audioRecoveryRetry{};
static int s_effectAmbSerial = 0;
static std::atomic<uint64_t> s_audioVoiceSerial{0};
static constexpr std::size_t kSpuVoicesPerCore = 24;
static constexpr auto kVoiceRetirementFade = std::chrono::milliseconds(5);
std::filesystem::path ExecutableDirectory();

void ReportXAudioFailureOnce(const char* operation, HRESULT result);

// XAudio2 does not ramp SetVolume calls. Destroying a source voice at an
// arbitrary sample therefore creates a discontinuity which is heard as a pop
// (and, when many voices are stolen together, as a short static burst). Keep
// the submitted PCM alive while a small worker-side fade reaches silence.
struct RetiredSourceVoice
{
	IXAudio2SourceVoice* voice = nullptr;
	float initialVolume = 0.0f;
	std::chrono::steady_clock::time_point started{};
	std::vector<int16_t> pcm16;
	std::vector<uint8_t> pcm8;
	// A submitted XAudio buffer retains the exact pointer supplied by its owner.
	// Keep a retired music bank alive instead of copying its bytes elsewhere.
	std::shared_ptr<Music989State> musicOwner;
};

static std::mutex s_retiredVoiceMutex;
static std::vector<RetiredSourceVoice> s_retiredVoices;

static void UpdateRetiredSourceVoices()
{
	std::lock_guard<std::mutex> lock(s_retiredVoiceMutex);
	const auto now = std::chrono::steady_clock::now();
	for (RetiredSourceVoice& retired : s_retiredVoices)
	{
		if (!retired.voice)
			continue;
		const float elapsed = std::chrono::duration<float>(now - retired.started).count();
		const float fadeSeconds = std::chrono::duration<float>(kVoiceRetirementFade).count();
		const float gain = std::clamp(1.0f - elapsed / fadeSeconds, 0.0f, 1.0f);
		ReportXAudioFailureOnce("Fade retiring source voice",
			retired.voice->SetVolume(retired.initialVolume * gain));
		if (gain > 0.0f)
			continue;
		ReportXAudioFailureOnce("Stop retired source voice", retired.voice->Stop());
		ReportXAudioFailureOnce("Flush retired source voice", retired.voice->FlushSourceBuffers());
		retired.voice->DestroyVoice();
		retired.voice = nullptr;
	}
	s_retiredVoices.erase(std::remove_if(s_retiredVoices.begin(), s_retiredVoices.end(),
		[](const RetiredSourceVoice& retired) { return retired.voice == nullptr; }),
		s_retiredVoices.end());
}

static void DestroyRetiredSourceVoicesImmediately()
{
	std::lock_guard<std::mutex> lock(s_retiredVoiceMutex);
	for (RetiredSourceVoice& retired : s_retiredVoices)
	{
		if (!retired.voice)
			continue;
		retired.voice->Stop();
		retired.voice->FlushSourceBuffers();
		retired.voice->DestroyVoice();
	}
	s_retiredVoices.clear();
}

static void RetireSourceVoice(IXAudio2SourceVoice*& voice,
	std::vector<int16_t>& pcm)
{
	if (!voice)
		return;
	RetiredSourceVoice retired{};
	retired.voice = voice;
	retired.started = std::chrono::steady_clock::now();
	voice->GetVolume(&retired.initialVolume);
	retired.pcm16 = std::move(pcm);
	{
		std::lock_guard<std::mutex> lock(s_retiredVoiceMutex);
		s_retiredVoices.emplace_back(std::move(retired));
	}
	voice = nullptr;
}

static void RetireSourceVoice(IXAudio2SourceVoice*& voice,
	std::vector<uint8_t>& pcm)
{
	if (!voice)
		return;
	RetiredSourceVoice retired{};
	retired.voice = voice;
	retired.started = std::chrono::steady_clock::now();
	voice->GetVolume(&retired.initialVolume);
	retired.pcm8 = std::move(pcm);
	{
		std::lock_guard<std::mutex> lock(s_retiredVoiceMutex);
		s_retiredVoices.emplace_back(std::move(retired));
	}
	voice = nullptr;
}

static void RetireMusicVoice(MusicVoice& active,
	const std::shared_ptr<Music989State>& musicOwner = {})
{
	if (!active.voice)
		return;
	RetiredSourceVoice retired{};
	retired.voice = active.voice;
	retired.started = std::chrono::steady_clock::now();
	active.voice->GetVolume(&retired.initialVolume);
	// Copying tone PCM would allocate a different address while XAudio2 keeps
	// reading the pointer submitted earlier. Ownership of the original bank is
	// therefore carried through the fade whenever the bank is being replaced.
	retired.musicOwner = musicOwner;
	{
		std::lock_guard<std::mutex> lock(s_retiredVoiceMutex);
		s_retiredVoices.emplace_back(std::move(retired));
	}
	active.voice = nullptr;
	active.tone = nullptr;
}

class AudioEngineCallback final : public IXAudio2EngineCallback
{
public:
	void STDMETHODCALLTYPE OnProcessingPassStart() override {}
	void STDMETHODCALLTYPE OnProcessingPassEnd() override {}
	void STDMETHODCALLTYPE OnCriticalError(HRESULT error) override
	{
		s_audioCriticalResult.store(error, std::memory_order_release);
		s_audioCriticalError.store(true, std::memory_order_release);
	}
};

static AudioEngineCallback s_audioEngineCallback;
class VagVoiceCallback final : public IXAudio2VoiceCallback
{
public:
	void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32) override {}
	void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
	void STDMETHODCALLTYPE OnStreamEnd() override {}
	void STDMETHODCALLTYPE OnBufferStart(void*) override {}
	void STDMETHODCALLTYPE OnBufferEnd(void*) override {}
	void STDMETHODCALLTYPE OnLoopEnd(void*) override {}
	void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT error) override
	{
		ReportXAudioFailureOnce("VAG/dialog source voice callback", error);
	}
};

static VagVoiceCallback s_vagVoiceCallback;

void ReportXAudioFailureOnce(const char* operation, HRESULT result)
{
	if (SUCCEEDED(result))
		return;
	static std::mutex reportedMutex;
	static std::unordered_set<std::string> reported;
	char key[192]{};
	std::snprintf(key, sizeof(key), "%s:%08X", operation,
		static_cast<unsigned>(result));
	{
		std::lock_guard<std::mutex> lock(reportedMutex);
		if (!reported.emplace(key).second)
			return;
	}
	ReportRuntimeError("Audio", "%s failed (HRESULT 0x%08X)", operation,
		static_cast<unsigned>(result));
}

void SetVoiceVolumeChecked(IXAudio2Voice* voice, float volume, const char* operation)
{
	if (!voice)
		return;
	if (!std::isfinite(volume))
	{
		ReportRuntimeError("Audio", "%s produced a non-finite volume; muting the voice", operation);
		volume = 0.0f;
	}
	ReportXAudioFailureOnce(operation, voice->SetVolume((std::max)(0.0f, volume)));
}

HRESULT SetVoiceFrequencyRatioChecked(IXAudio2SourceVoice* voice, float ratio,
	const char* operation)
{
	if (!voice)
		return E_POINTER;
	if (!std::isfinite(ratio))
	{
		ReportRuntimeError("Audio", "%s produced a non-finite pitch ratio; using unity pitch",
			operation);
		ratio = 1.0f;
	}
	ratio = std::clamp(ratio, XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio);
	const HRESULT result = voice->SetFrequencyRatio(ratio);
	ReportXAudioFailureOnce(operation, result);
	return result;
}

struct PendingVagLoad
{
	uint64_t generation = 0;
	std::filesystem::path readablePath;
	std::filesystem::path path;
	SoundWaveFormat format{};
	std::vector<uint8_t> pcm;
	bool requested = false;
	bool loading = false;
	bool ready = false;
};

static std::mutex s_vagLoadMutex;
static std::condition_variable s_vagLoadCondition;
static std::thread s_vagLoadWorker;
static bool s_vagLoadWorkerRunning = false;
static uint64_t s_vagLoadGeneration = 0;
static PendingVagLoad s_vagLoad;

static void UpdateMusicSequencer(double logicalElapsed = -1.0);
static void PrimeMusicSequencerStartup();
static bool FinishPendingVagLoad();
static bool RecoverAudioEngine();

std::string GetAudioOutputDeviceId()
{
	return s_audioOutputDeviceId;
}

std::string GetAudioOutputDeviceName()
{
	for (const AudioOutputDevice& device : AudioOutputDevices())
		if (device.id == s_audioOutputDeviceId)
			return device.name;
	return "System Default";
}

void SetAudioOutputDeviceId(const std::string& deviceId)
{
	std::string validatedId;
	for (const AudioOutputDevice& device : AudioOutputDevices())
	{
		if (device.id == deviceId)
		{
			validatedId = deviceId;
			break;
		}
	}
	if (s_audioOutputDeviceId == validatedId) return;
	s_audioOutputDeviceId = validatedId;
	if (s_xaudio2 != nullptr)
		RecoverAudioEngine();
}

void SelectAudioOutputDevice(int direction)
{
	const std::vector<AudioOutputDevice> devices = AudioOutputDevices();
	if (devices.empty()) return;
	auto current = std::find_if(devices.begin(), devices.end(), [](const AudioOutputDevice& device)
	{
		return device.id == s_audioOutputDeviceId;
	});
	int index = current == devices.end() ? 0 : static_cast<int>(current - devices.begin());
	index = (index + (direction < 0 ? -1 : 1) + static_cast<int>(devices.size())) %
		static_cast<int>(devices.size());
	const std::string previous = s_audioOutputDeviceId;
	s_audioOutputDeviceId = devices[index].id;
	if (s_xaudio2 != nullptr && !RecoverAudioEngine())
	{
		s_audioOutputDeviceId = previous;
		RecoverAudioEngine();
	}
}
static bool CreateVagStreamVoice(VagStreamState& stream, uint64_t playBegin);
static bool SetVagStreamOutputMatrix(IXAudio2SourceVoice* voice, UINT32 sourceChannels);
static void ResetMusicSegmentForLoop(MusicSegment& segment);
static void MusicWorkerMain()
{
	using namespace std::chrono_literals;
	while (s_musicWorkerRunning.load(std::memory_order_acquire))
	{
		// Voice retirement is independent of the music transport.  It must keep
		// running while gameplay audio is paused and while the sequencer is idle;
		// otherwise stopped music, dialogue, and FMV voices remain alive forever.
		// Besides leaking XAudio voices and their submitted buffers, that lets old
		// audio continue underneath the next scene.  The retirement list is
		// internally synchronized, so producers may safely enqueue from either the
		// game thread or this worker.
		UpdateRetiredSourceVoices();
		if (!s_musicUsesGameplayClock.load(std::memory_order_acquire))
		{
			std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
			UpdateMusicSequencer();
		}
		std::this_thread::sleep_for(1ms);
	}
}

const glm::vec3* PposSoundEar()
{
	PO* ppo = PpoCur();
	if (ppo != nullptr && ppo->pvtpo->pfnFIsPoSoundBase(ppo))
		return &ppo->xf.posWorld;
	return g_pcm != nullptr ? &g_pcm->pos : nullptr;
}

float SDistSoundEar(const glm::vec3& posEar, const glm::vec3& posSound)
{
	const glm::vec3 displacement = posSound - posEar;
	const LOOKK lookk = g_pcm != nullptr ? LookkCurCplook(&g_pcm->cplook) : LOOKK_Nil;
	if (g_pcm == nullptr || FActiveCplcy(static_cast<CPLCY*>(&g_pcm->cplook)) == 0 ||
		lookk < LOOKK_Sniper || lookk > LOOKK_SniperTrack)
	{
		return glm::length(displacement);
	}

	// Retail SDistEarZoom compresses only the component along the camera's
	// forward axis. At full sniper zoom, distant objects in view are treated as
	// one tenth as far away for sound attenuation.
	const glm::vec3 forward = g_pcm->mat[0];
	const float forwardDistance = glm::dot(displacement, forward);
	const glm::vec3 perpendicular = displacement - forward * forwardDistance;
	const float zoomScale = 1.0f /
		((1.0f - g_pcm->cplook.uZoom) + g_pcm->cplook.uZoom * 10.0f);
	return glm::length(perpendicular + forward * (forwardDistance * zoomScale));
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

	// The effect-bank tone record stores only the start of each VAG stream.
	// Resolve its end from the next distinct stream start, just as the 989SND
	// loader does when it builds its sample table.  Leaving sampleEndOffset at
	// its default zero makes DecodeEffectAdpcm reject every ordinary SFX tone,
	// while music and dialogue continue working because they use other loaders.
	std::vector<uint32_t> sampleOffsets;
	for (const EFFECTBANK::SOUND& sound : bank.sounds)
		for (const EFFECTBANK::GRAIN& grain : sound.grains)
			if (grain.hasTone && grain.tone.sampleOffset < bank.sampleSize)
				sampleOffsets.push_back(grain.tone.sampleOffset);
	std::sort(sampleOffsets.begin(), sampleOffsets.end());
	sampleOffsets.erase(std::unique(sampleOffsets.begin(), sampleOffsets.end()),
		sampleOffsets.end());
	for (EFFECTBANK::SOUND& sound : bank.sounds)
		for (EFFECTBANK::GRAIN& grain : sound.grains)
		{
			if (!grain.hasTone)
				continue;
			const auto next = std::upper_bound(sampleOffsets.begin(), sampleOffsets.end(),
				grain.tone.sampleOffset);
			grain.tone.sampleEndOffset = next != sampleOffsets.end()
				? *next : bank.sampleSize;
		}
	return true;
}


std::filesystem::path ExecutableDirectory()
{
    return cane::DataDirectory();
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
    // In the organized extractor layout, a level's BRX and Dialog directory
    // are siblings. Prefer that directory so identical VAG names in different
    // levels cannot resolve to the wrong voice line.
    if (!file.empty())
    {
        const std::filesystem::path levelDirectory =
            std::filesystem::path(file).parent_path();
        if (FLocalizedContent())
        {
            // v19 keeps the unified BRX in the level root, while legacy PAL
            // v17 keeps each BRX inside its language directory. Accept both
            // without accidentally constructing French/French/Dialogs.
            std::filesystem::path languageDirectory = levelDirectory;
            if (_stricmp(levelDirectory.filename().string().c_str(),
                    ContentAudioLanguageDirectory()) != 0)
                languageDirectory /= ContentAudioLanguageDirectory();
            const std::filesystem::path localizedFile = languageDirectory /
                L"Dialogs" / fileName;
            std::error_code localizedError;
            if (std::filesystem::is_regular_file(localizedFile, localizedError))
                return localizedFile;
            const std::filesystem::path localizedCommentary = languageDirectory /
                L"Commentary" / fileName;
            if (std::filesystem::is_regular_file(localizedCommentary, localizedError))
                return localizedCommentary;

            // PAL dialogue and commentary are language-specific. Never fall
            // through into another language, level, or extraction tree.
            return {};
        }
        std::filesystem::path levelDialogDirectory = levelDirectory / L"Dialogs";
        std::filesystem::path levelDialogFile = levelDialogDirectory / fileName;
        std::error_code error;
        if (std::filesystem::is_regular_file(levelDialogFile, error))
            return levelDialogFile;

		const std::filesystem::path commentaryFile =
			levelDirectory / L"Commentary" / fileName;
		if (std::filesystem::is_regular_file(commentaryFile, error))
			return commentaryFile;

		// Backward compatibility with older NTSC extractor layouts.
		levelDialogDirectory = std::filesystem::path(file).parent_path() / L"Dialog";
		levelDialogFile = levelDialogDirectory / fileName;
		if (std::filesystem::is_regular_file(levelDialogFile, error))
			return levelDialogFile;
    }

    const std::filesystem::path executableDirectory = ExecutableDirectory();
    const std::filesystem::path repositoryDirectory = std::filesystem::current_path();
    const std::filesystem::path executableReposDirectory =
        executableDirectory.parent_path().parent_path().parent_path();
    const std::filesystem::path roots[] =
    {
        executableDirectory / L"Worlds",
        std::filesystem::current_path() / L"Worlds",
        std::filesystem::current_path() / L"Sly1-Release" / L"Worlds",
        std::filesystem::current_path() / L"x64" / L"Debug" / L"Worlds",
        std::filesystem::current_path() / L"x64" / L"Release" / L"Worlds",
        executableDirectory / L"x64" / L"Debug" / L"Worlds",
        executableDirectory / L"x64" / L"Release" / L"Worlds",
        executableDirectory.parent_path() / L"x64" / L"Debug" / L"Worlds",
        executableDirectory.parent_path() / L"x64" / L"Release" / L"Worlds",
        executableDirectory.parent_path().parent_path() / L"x64" / L"Debug" / L"Worlds",
        executableDirectory.parent_path().parent_path() / L"x64" / L"Release" / L"Worlds",
        executableDirectory.parent_path().parent_path().parent_path() / L"x64" / L"Debug" / L"Worlds",
        executableDirectory.parent_path().parent_path().parent_path() / L"x64" / L"Release" / L"Worlds",
        // Development fallback: allow ProjectCane to use a freshly generated
        // extractor asset tree without duplicating roughly 500 MB of dialog.
        repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" /
            L"Debug" / L"Worlds",
        repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" /
            L"Release" / L"Worlds",
        executableReposDirectory / L"Sly1 File Extractor" / L"x64" /
            L"Debug" / L"Worlds",
        executableReposDirectory / L"Sly1 File Extractor" / L"x64" /
            L"Release" / L"Worlds"
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

std::filesystem::path FindCommentaryWavByLevelId(uint32_t levelId)
{
    const std::filesystem::path mapPath = FindExtractedWav(L"CommentaryMap.txt");
    if (mapPath.empty())
        return {};

    std::ifstream map(mapPath);
    std::string line;
    while (std::getline(map, line))
    {
        char fileName[260]{};
        unsigned int mapLevelId = 0;
        if (std::sscanf(line.c_str(), "%259s | levelId 0x%x", fileName, &mapLevelId) != 2)
            continue;
        if (mapLevelId != levelId)
            continue;

        const std::filesystem::path commentaryPath = mapPath.parent_path() / fileName;
        std::error_code error;
        if (std::filesystem::is_regular_file(commentaryPath, error))
            return commentaryPath;

        return FindExtractedWav(commentaryPath.filename().wstring());
    }

    return {};
}


std::filesystem::path ResolveNamedVag(const char* value)
{
    if (!IsReadablePath(value))
        return {};

    const std::filesystem::path supplied(value);
    std::filesystem::path direct = supplied;
    direct.replace_extension(".wav");
    std::error_code error;
    if (!FLocalizedContent() && std::filesystem::is_regular_file(direct, error))
        return direct;

    // The commentary UI uses descriptive names such as Commentary_0202.vag,
    // while the current extractor stores descriptor-stable VAG names. Resolve
    // the descriptive level ID through CommentaryMap.txt.
    const std::string filename = direct.filename().string();
    unsigned int commentaryLevelId = 0;
    int commentaryParsedLength = 0;
    if (std::sscanf(filename.c_str(), "Commentary_%x.wav%n", &commentaryLevelId,
            &commentaryParsedLength) == 1 &&
        commentaryParsedLength == static_cast<int>(filename.size()))
    {
        return FindCommentaryWavByLevelId(commentaryLevelId);
    }

    // Older extractor releases omitted the sector when an identity was unique
    // inside a level. Keep those asset trees usable with PCBRX v17 files.
    unsigned int identity = 0;
    unsigned int sector = 0;
    int parsedLength = 0;
    if (std::sscanf(filename.c_str(), "VAG_%x_%x.wav%n", &identity, &sector,
            &parsedLength) == 2 && parsedLength == static_cast<int>(filename.size()))
    {
        if (std::filesystem::path found = FindExtractedWav(direct.filename().wstring());
            !found.empty())
            return found;

        // Current PAL extractions include both identity and sector. Do not
        // weaken the match and risk playing another localized dialogue line.
        if (FLocalizedContent())
            return {};

        wchar_t identityName[64]{};
        std::swprintf(identityName, std::size(identityName), L"VAG_%04X.wav", identity);
        if (std::filesystem::path found = FindExtractedWav(identityName); !found.empty())
            return found;

        // Commentary produced by older extractors used descriptive filenames.
        return FindCommentaryWav(identity, sector);
    }

    if (std::filesystem::path found = FindExtractedWav(direct.filename().wstring());
        !found.empty())
        return found;

    return {};
}

std::filesystem::path FindEffectBank(int ibnk, const char* fileName)
{
	if (ibnk < 0 || ibnk >= static_cast<int>(kEffectBankNames.size()))
		return {};
	const std::filesystem::path requestedFile = fileName != nullptr && fileName[0] != '\0'
		? std::filesystem::path(fileName)
		: std::filesystem::path(kEffectBankNames[ibnk]);
	const std::filesystem::path executableDirectory = ExecutableDirectory();
	const std::filesystem::path repositoryDirectory = std::filesystem::current_path();
	const std::filesystem::path executableReposDirectory =
		executableDirectory.parent_path().parent_path().parent_path();
	std::filesystem::path loadedWorldsDirectory;
	if (!file.empty())
	{
		std::filesystem::path ancestor = std::filesystem::path(file).parent_path();
		while (!ancestor.empty())
		{
			if (MatchesInsensitive(ancestor.filename(), L"Worlds"))
			{
				loadedWorldsDirectory = ancestor;
				break;
			}
			const std::filesystem::path parent = ancestor.parent_path();
			if (parent == ancestor)
				break;
			ancestor = parent;
		}
	}
	const std::filesystem::path roots[] =
	{
		loadedWorldsDirectory / L"Sound Banks",
		executableDirectory / L"Worlds" / L"Sound Banks",
		repositoryDirectory / L"Worlds" / L"Sound Banks",
		repositoryDirectory / L"Sly1-Release" / L"Worlds" / L"Sound Banks",
		repositoryDirectory / L"x64" / L"Debug" / L"Worlds" / L"Sound Banks",
		repositoryDirectory / L"x64" / L"Release" / L"Worlds" / L"Sound Banks",
		executableDirectory / L"x64" / L"Debug" / L"Worlds" / L"Sound Banks",
		executableDirectory / L"x64" / L"Release" / L"Worlds" / L"Sound Banks",
		executableDirectory.parent_path() / L"x64" / L"Debug" / L"Worlds" / L"Sound Banks",
		executableDirectory.parent_path() / L"x64" / L"Release" / L"Worlds" / L"Sound Banks",
		executableDirectory.parent_path().parent_path() / L"x64" / L"Debug" / L"Worlds" / L"Sound Banks",
		executableDirectory.parent_path().parent_path() / L"x64" / L"Release" / L"Worlds" / L"Sound Banks",
		executableDirectory.parent_path().parent_path().parent_path() / L"x64" / L"Debug" / L"Worlds" / L"Sound Banks",
		executableDirectory.parent_path().parent_path().parent_path() / L"x64" / L"Release" / L"Worlds" / L"Sound Banks",
		repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" / L"Debug" / L"Worlds" / L"Sound Banks",
		repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" / L"Release" / L"Worlds" / L"Sound Banks",
		executableReposDirectory / L"Sly1 File Extractor" / L"x64" / L"Debug" / L"Worlds" / L"Sound Banks",
		executableReposDirectory / L"Sly1 File Extractor" / L"x64" / L"Release" / L"Worlds" / L"Sound Banks"
	};
	for (const std::filesystem::path& root : roots)
	{
		const std::filesystem::path candidate = root / requestedFile.filename();
		std::error_code error;
		if (std::filesystem::is_regular_file(candidate, error))
			return candidate;
	}
	return {};
}

std::filesystem::path FindMusicBank(MUSID musid, const char* fileName)
{
	wchar_t bankName[32]{};
	std::swprintf(bankName, std::size(bankName), L"MUSID_%02X", static_cast<unsigned>(musid));
	const std::filesystem::path requestedFile = fileName != nullptr && fileName[0] != '\0'
		? std::filesystem::path(fileName)
		: std::filesystem::path(std::wstring(bankName) + L".mus");
	const std::filesystem::path executableDirectory = ExecutableDirectory();
	const std::filesystem::path repositoryDirectory = std::filesystem::current_path();
	const std::filesystem::path executableReposDirectory =
		executableDirectory.parent_path().parent_path().parent_path();
	std::filesystem::path loadedWorldsDirectory;
	if (!file.empty())
	{
		std::filesystem::path ancestor = std::filesystem::path(file).parent_path();
		while (!ancestor.empty())
		{
			if (MatchesInsensitive(ancestor.filename(), L"Worlds"))
			{
				loadedWorldsDirectory = ancestor;
				break;
			}
			const std::filesystem::path parent = ancestor.parent_path();
			if (parent == ancestor)
				break;
			ancestor = parent;
		}
	}
	const std::filesystem::path roots[] =
	{
		loadedWorldsDirectory / L"Music",
		executableDirectory / L"Worlds" / L"Music",
		repositoryDirectory / L"Worlds" / L"Music",
		repositoryDirectory / L"Sly1-Release" / L"Worlds" / L"Music",
		repositoryDirectory / L"x64" / L"Debug" / L"Worlds" / L"Music",
		repositoryDirectory / L"x64" / L"Release" / L"Worlds" / L"Music",
		executableDirectory / L"x64" / L"Debug" / L"Worlds" / L"Music",
		executableDirectory / L"x64" / L"Release" / L"Worlds" / L"Music",
		executableDirectory.parent_path() / L"x64" / L"Debug" / L"Worlds" / L"Music",
		executableDirectory.parent_path() / L"x64" / L"Release" / L"Worlds" / L"Music",
		executableDirectory.parent_path().parent_path() / L"x64" / L"Debug" / L"Worlds" / L"Music",
		executableDirectory.parent_path().parent_path() / L"x64" / L"Release" / L"Worlds" / L"Music",
		executableDirectory.parent_path().parent_path().parent_path() / L"x64" / L"Debug" / L"Worlds" / L"Music",
		executableDirectory.parent_path().parent_path().parent_path() / L"x64" / L"Release" / L"Worlds" / L"Music",
		repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" / L"Debug" / L"Worlds" / L"Music",
		repositoryDirectory.parent_path() / L"Sly1 File Extractor" / L"x64" / L"Release" / L"Worlds" / L"Music",
		executableReposDirectory / L"Sly1 File Extractor" / L"x64" / L"Debug" / L"Worlds" / L"Music",
		executableReposDirectory / L"Sly1 File Extractor" / L"x64" / L"Release" / L"Worlds" / L"Music"
	};

	for (const std::filesystem::path& root : roots)
	{
		const std::filesystem::path raw = root / requestedFile.filename();
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
#ifdef __SWITCH__
    return CaneWave::Load(path, format, pcm);
#else
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
		if (payload > file.size() || size > file.size() - payload)
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
			// Accept both classic PCM and WAVE_FORMAT_EXTENSIBLE carrying the PCM
			// subtype. Modern editors commonly emit the extensible header even for
			// ordinary stereo files. The sample payload is identical, so normalize
			// it to WAVE_FORMAT_PCM before creating the source voice.
			if (format.formatTag == 1)
			{
				foundFormat = true;
			}
			else if (format.formatTag == 0xfffe && size >= 40)
			{
				uint16_t extensionSize = 0;
				uint16_t validBits = 0;
				std::memcpy(&extensionSize, file.data() + payload + 16, sizeof(extensionSize));
				std::memcpy(&validBits, file.data() + payload + 18, sizeof(validBits));
				static constexpr uint8_t pcmSubtype[16] =
				{
					0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
					0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
				};
				foundFormat = extensionSize >= 22 &&
					std::memcmp(file.data() + payload + 24, pcmSubtype, sizeof(pcmSubtype)) == 0 &&
					(validBits == 0 || validBits <= format.bitsPerSample);
				if (foundFormat)
					format.formatTag = 1;
			}
		}
		else if (std::memcmp(file.data() + offset, "data", 4) == 0)
		{
			pcm.assign(file.begin() + payload, file.begin() + payload + size);
			foundData = !pcm.empty();
		}
		offset = payload + size + (size & 1);
	}
	if (!foundFormat || !foundData)
		return false;
	const uint32_t bytesPerSample = format.bitsPerSample / 8;
	const uint32_t expectedBlockAlign = format.channels * bytesPerSample;
	const bool valid = format.channels >= 1 && format.channels <= XAUDIO2_MAX_AUDIO_CHANNELS &&
		format.samplesPerSec >= XAUDIO2_MIN_SAMPLE_RATE &&
		format.samplesPerSec <= XAUDIO2_MAX_SAMPLE_RATE &&
		(format.bitsPerSample == 8 || format.bitsPerSample == 16 ||
			format.bitsPerSample == 24 || format.bitsPerSample == 32) &&
		format.blockAlign == expectedBlockAlign && format.blockAlign != 0 &&
		format.avgBytesPerSec == format.samplesPerSec * format.blockAlign &&
		pcm.size() % format.blockAlign == 0;
	if (!valid)
	{
		ReportRuntimeError("Audio",
			"Invalid PCM WAV '%s' (%u Hz, %u channels, %u-bit, block %u, average %u, data %zu bytes)",
			path.string().c_str(), format.samplesPerSec, format.channels, format.bitsPerSample,
			format.blockAlign, format.avgBytesPerSec, pcm.size());
		pcm.clear();
	}
	return valid;
#endif
}

#ifdef __SWITCH__
static std::filesystem::path FindExtractedWav(const std::wstring& fileName, const CaneVag::Request& request)
{
    const auto& executableDirectory = request.executableDirectory;
    const auto& repositoryDirectory = request.workingDirectory;
    const std::filesystem::path roots[] =
    {
        executableDirectory / "Worlds", repositoryDirectory / "Worlds"
    };

    std::vector<std::filesystem::path> visited;
    for (const std::filesystem::path& root : roots)
    {
        const auto normalized = root.lexically_normal();
        if (std::find(visited.begin(), visited.end(), normalized) != visited.end()) continue;
        visited.push_back(normalized);
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
            if (MatchesInsensitive(iter->path(), fileName) && iter->is_regular_file(error))
                return iter->path();
        }
    }

    return {};
}

static std::filesystem::path FindCommentaryWav(uint32_t identity, uint32_t sector, const CaneVag::Request& request, uint32_t level)
{
    const std::filesystem::path mapPath = FindExtractedWav(L"CommentaryMap.txt", request);
    if (mapPath.empty())
        return {};

    std::ifstream map(mapPath);
    std::string line;
    while (std::getline(map, line))
    {
        char fileName[260]{};
        unsigned int mapLevel = 0;
        unsigned int mapIdentity = 0;
        unsigned int mapSector = 0;
        if (std::sscanf(line.c_str(),
                "%259s | levelId 0x%x | duration %*f | identity 0x%x | sector 0x%x",
                fileName, &mapLevel, &mapIdentity, &mapSector) != 4)
        {
            continue;
        }

        if (level != UINT32_MAX ? mapLevel != level : (mapIdentity != identity || mapSector != sector))
            continue;

        const std::filesystem::path commentaryPath = mapPath.parent_path() / fileName;
        std::error_code error;
        if (std::filesystem::is_regular_file(commentaryPath, error))
            return commentaryPath;

        return FindExtractedWav(commentaryPath.filename().wstring(), request);
    }

    return {};
}

static CaneVag::Request MakeVagRequest(const char* filename)
{
    CaneVag::Request request;
    if (!filename || !*filename) return request;
    request.valid = true;
    request.levelDirectory = std::filesystem::path(file).parent_path();
    request.executableDirectory = ExecutableDirectory();
    request.workingDirectory = std::filesystem::current_path();
    request.localized = FLocalizedContent();
    if (request.localized) request.language = ContentAudioLanguageDirectory();
    request.direct = filename;
    request.direct.replace_extension(".wav");
    return request;
}

static std::filesystem::path ResolveVagRequest(const CaneVag::Request& request)
{
    return CaneVag::Resolve(request,
        [&request](const std::wstring& name) { return FindExtractedWav(name, request); },
        [&request](uint32_t identity, uint32_t sector, uint32_t level) { return FindCommentaryWav(identity, sector, request, level); });
}

static bool RequestVag(const char* descriptorOrPath, bool urgent)
{
    if (!CaneAudio::PrefetchEnabled() || !descriptorOrPath) return true;
    const auto request = MakeVagRequest(descriptorOrPath);
    return CaneAudio::RequestWave(request.Key(), [request]
    {
        CaneAudio::WaveAsset wave;
        wave.path = ResolveVagRequest(request);
        if (wave.path.empty() || !LoadPcmWave(wave.path, wave.format, wave.pcm)) return CaneAudio::WaveAsset{};
        return wave;
    }, urgent);
}
void PrefetchVag(const char* descriptorOrPath) { RequestVag(descriptorOrPath, false); }
bool FReadyVag(const char* descriptorOrPath) { return RequestVag(descriptorOrPath, true); }
void ClearVagPrefetch() { CaneAudio::ClearPrefetch(); }
static std::string s_nativeVagKey;
static void QueueNativeVag(const CaneVag::Request& request)
{
    s_nativeVagKey = request.Key();
    s_vagLoad.loading = true;
    s_vagLoad.ready = false;
    if (!CaneAudio::PrefetchEnabled())
    {
        s_vagLoad.path = ResolveVagRequest(request);
        LoadPcmWave(s_vagLoad.path, s_vagLoad.format, s_vagLoad.pcm);
        s_vagLoad.loading = false;
        s_vagLoad.ready = true;
    }
    else
        CaneAudio::RequestWave(s_nativeVagKey, [request]
        {
            CaneAudio::WaveAsset wave;
            wave.path = ResolveVagRequest(request);
            if (!wave.path.empty()) LoadPcmWave(wave.path, wave.format, wave.pcm);
            return wave;
        }, true);
    FinishPendingVagLoad();
}

#endif

bool EnsureEffectReverb()
{
	if (!s_xaudio2 || !s_musicMasterVoice)
		return false;
	if (!s_effectDryVoice)
	{
		const HRESULT dryResult = s_xaudio2->CreateSubmixVoice(
			&s_effectDryVoice, 2, 48000, 0, 0, nullptr, nullptr);
		ReportXAudioFailureOnce("Create 48 kHz dry-effects submix", dryResult);
	}
	HRESULT studioResult = S_OK;
	#ifdef _WIN32
	if (!s_effectStudioCVoice && !s_effectStudioCCreationAttempted)
	{
		s_effectStudioCCreationAttempted = true;
		SpuStudioCReverb* studio = new SpuStudioCReverb();
		XAUDIO2_EFFECT_DESCRIPTOR effect{studio, TRUE, 2};
		XAUDIO2_EFFECT_CHAIN chain{1, &effect};
		studioResult = s_xaudio2->CreateSubmixVoice(&s_effectStudioCVoice, 2, 48000,
			0, 1, nullptr, &chain);
		studio->Release();
		ReportXAudioFailureOnce("Create Studio C reverb submix", studioResult);
		if (SUCCEEDED(studioResult) && s_effectStudioCVoice)
			s_effectStudioCProcessor = studio;
	}
	#endif

	HRESULT genericResult = S_OK;
	if (!s_effectReverbVoice && !s_effectReverbCreationAttempted)
	{
		s_effectReverbCreationAttempted = true;
		IUnknown* reverb = nullptr;
		genericResult = XAudio2CreateReverb(&reverb);
		if (SUCCEEDED(genericResult) && !reverb)
			genericResult = E_POINTER;
		if (SUCCEEDED(genericResult) && reverb)
		{
			XAUDIO2_EFFECT_DESCRIPTOR effect{reverb, TRUE, 2};
			XAUDIO2_EFFECT_CHAIN chain{1, &effect};
			genericResult = s_xaudio2->CreateSubmixVoice(&s_effectReverbVoice, 2, 48000,
				0, 1, nullptr, &chain);
			reverb->Release();
		}
		ReportXAudioFailureOnce("Create generic reverb submix", genericResult);
	}
	// Keep one permanent wet-input bus between gameplay voices and the effect
	// processors. This mirrors SPU2's stable wet gate: changing environments
	// changes the return level/work area, never the routing of active voices.
	if (!s_effectWetInputVoice && (s_effectStudioCVoice || s_effectReverbVoice))
	{
		XAUDIO2_SEND_DESCRIPTOR sends[2]{};
		UINT32 sendCount = 0;
		if (s_effectStudioCVoice)
			sends[sendCount++].pOutputVoice = s_effectStudioCVoice;
		if (s_effectReverbVoice)
			sends[sendCount++].pOutputVoice = s_effectReverbVoice;
		XAUDIO2_VOICE_SENDS sendList{sendCount, sends};
		const HRESULT wetInputResult = s_xaudio2->CreateSubmixVoice(
			&s_effectWetInputVoice, 2, 48000, 0, 0, &sendList, nullptr);
		ReportXAudioFailureOnce("Create stable 48 kHz wet-input submix", wetInputResult);
	}
	if (s_effectDryVoice && (s_effectReverbVoice || s_effectStudioCVoice))
		ApplyCurrentEffectReverb();
	return s_effectDryVoice != nullptr && s_effectWetInputVoice != nullptr;
}

IXAudio2Voice* ActiveEffectWetVoice()
{
	return s_effectWetInputVoice;
}

void SetEffectVoiceDestinations(IXAudio2SourceVoice* voice)
{
	if (!voice || !s_musicMasterVoice)
		return;
	XAUDIO2_SEND_DESCRIPTOR sends[2]{};
	UINT32 sendCount = 0;
	if (s_effectDryVoice)
	{
		sends[sendCount++].pOutputVoice = s_effectDryVoice;
		if (IXAudio2Voice* wet = ActiveEffectWetVoice())
			sends[sendCount++].pOutputVoice = wet;
	}
	else
	{
		sends[sendCount++].pOutputVoice = s_musicMasterVoice;
	}
	XAUDIO2_VOICE_SENDS sendList{sendCount, sends};
	ReportXAudioFailureOnce("Update gameplay-effect reverb routing",
		voice->SetOutputVoices(&sendList));
}

void ResetEffectReverbProcessors()
{
	if (s_effectWetInputVoice)
	{
		s_effectWetInputVoice->DestroyVoice();
		s_effectWetInputVoice = nullptr;
	}
	if (s_effectReverbVoice)
	{
		s_effectReverbVoice->DestroyVoice();
		s_effectReverbVoice = nullptr;
	}
	if (s_effectStudioCVoice)
	{
		s_effectStudioCProcessor = nullptr;
		s_effectStudioCVoice->DestroyVoice();
		s_effectStudioCVoice = nullptr;
	}
	s_effectReverbCreationAttempted = false;
	s_effectStudioCCreationAttempted = false;
}

void ApplyCurrentEffectReverb()
{
	if (!s_effectReverbVoice && !s_effectStudioCVoice)
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
	// Retail passes the authored depth directly to the SPU2 effect-volume
	// registers, which are signed Q15 values. Keep this auxiliary path fully wet
	// and scale only the return; mixing dry signal inside the effect would
	// duplicate the source voice's direct path.
	const float studioWetLevel = s_effectReverbKind == REVERBK_StudioC
		? std::clamp(static_cast<float>(s_effectReverbDepth) / 32767.0f, -1.0f, 1.0f)
		: 0.0f;
	const float genericWetLevel = s_effectReverbKind == REVERBK_None
		? 0.0f
		: s_effectReverbKind == REVERBK_StudioC
			? (s_effectStudioCVoice ? 0.0f : studioWetLevel)
			: std::clamp(static_cast<float>(s_effectReverbDepth) / 32767.0f, -1.0f, 1.0f);
	i3dl2.WetDryMix = 100.0f;
	XAUDIO2FX_REVERB_PARAMETERS native{};
	ReverbConvertI3DL2ToNative(&i3dl2, &native, FALSE);
	const HRESULT parametersResult = s_effectReverbVoice
		? s_effectReverbVoice->SetEffectParameters(0, &native, sizeof(native)) : S_OK;
	const HRESULT volumeResult = s_effectReverbVoice
		? s_effectReverbVoice->SetVolume(genericWetLevel) : S_OK;
	const HRESULT studioVolumeResult = s_effectStudioCVoice
		? s_effectStudioCVoice->SetVolume(studioWetLevel) : S_OK;
	const bool feedStudio = s_effectReverbKind == REVERBK_StudioC &&
		s_effectStudioCVoice != nullptr;
	const bool feedGeneric = s_effectReverbKind != REVERBK_None &&
		(s_effectReverbKind != REVERBK_StudioC || s_effectStudioCVoice == nullptr);
	#ifdef _WIN32
	if (s_effectStudioCProcessor)
		s_effectStudioCProcessor->SetActive(feedStudio);
	#endif
	const float studioInputMatrix[4] =
	{
		feedStudio ? 1.0f : 0.0f, 0.0f,
		0.0f, feedStudio ? 1.0f : 0.0f
	};
	const float genericInputMatrix[4] =
	{
		feedGeneric ? 1.0f : 0.0f, 0.0f,
		0.0f, feedGeneric ? 1.0f : 0.0f
	};
	const HRESULT studioInputResult = s_effectWetInputVoice && s_effectStudioCVoice
		? s_effectWetInputVoice->SetOutputMatrix(
			s_effectStudioCVoice, 2, 2, studioInputMatrix) : S_OK;
	const HRESULT genericInputResult = s_effectWetInputVoice && s_effectReverbVoice
		? s_effectWetInputVoice->SetOutputMatrix(
			s_effectReverbVoice, 2, 2, genericInputMatrix) : S_OK;
	if (FAILED(parametersResult) || FAILED(volumeResult) || FAILED(studioVolumeResult) ||
		FAILED(studioInputResult) || FAILED(genericInputResult))
	{
		ReportRuntimeError("Audio",
			"Reverb update failed (parameters=0x%08X volume=0x%08X studio=0x%08X studio_input=0x%08X generic_input=0x%08X kind=%d depth=%d wet=%.3f)",
			static_cast<unsigned>(parametersResult), static_cast<unsigned>(volumeResult),
			static_cast<unsigned>(studioVolumeResult), static_cast<unsigned>(studioInputResult),
			static_cast<unsigned>(genericInputResult), static_cast<int>(s_effectReverbKind),
			s_effectReverbDepth, studioWetLevel);
	}
}

bool EnsureMusicEngine()
{
	if (s_xaudio2)
		return true;
	const HRESULT engineResult = XAudio2Create(&s_xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (FAILED(engineResult))
	{
		ReportRuntimeError("Audio", "XAudio2Create failed (HRESULT 0x%08X)",
			static_cast<unsigned>(engineResult));
		return false;
	}
	const HRESULT callbackResult = s_xaudio2->RegisterForCallbacks(&s_audioEngineCallback);
	if (FAILED(callbackResult))
		ReportXAudioFailureOnce("Register audio engine callback", callbackResult);
	// Keep ProjectCane's internal mix deterministic. The PS2 mix is stereo at
	// 48 kHz; asking XAudio/FAudio for the endpoint's native layout made source
	// channels 0 and 1 mean different things on stereo, surround and virtual
	// devices. The operating system can perform the final endpoint conversion.
	#ifdef _WIN32
	SpuMixSaturator* finalOutput = new SpuMixSaturator(true);
	finalOutput->SetTargetGain(0.0f);
	XAUDIO2_EFFECT_DESCRIPTOR finalEffect{finalOutput, TRUE, 2};
	XAUDIO2_EFFECT_CHAIN finalChain{1, &finalEffect};
	const std::wstring outputDeviceId = WideFromUtf8(s_audioOutputDeviceId);
	HRESULT masteringResult = s_xaudio2->CreateMasteringVoice(
		&s_musicMasterVoice, 2, 48000, 0,
		outputDeviceId.empty() ? nullptr : outputDeviceId.c_str(), &finalChain,
		AudioCategory_GameEffects);
	if (FAILED(masteringResult) && !outputDeviceId.empty())
	{
		// A saved endpoint can disappear or reject the requested fixed 48 kHz
		// graph after settings validation. Fall back to the Windows default so
		// one stale device ID cannot disable all game audio.
		s_audioOutputDeviceId.clear();
		s_musicMasterVoice = nullptr;
		masteringResult = s_xaudio2->CreateMasteringVoice(
			&s_musicMasterVoice, 2, 48000, 0, nullptr, &finalChain,
			AudioCategory_GameEffects);
	}
	if (SUCCEEDED(masteringResult))
		s_finalOutputProcessor = finalOutput;
	finalOutput->Release();
	#else
	const HRESULT masteringResult = s_xaudio2->CreateMasteringVoice(
		&s_musicMasterVoice, 2, 48000);
	#endif
	if (FAILED(masteringResult))
	{
		ReportRuntimeError("Audio",
			"CreateMasteringVoice failed (HRESULT 0x%08X). Check the active Windows output device and audio format.",
			static_cast<unsigned>(masteringResult));
		s_xaudio2->UnregisterForCallbacks(&s_audioEngineCallback);
		s_xaudio2->Release();
		s_xaudio2 = nullptr;
		return false;
	}
	// Keep the output silent while the fixed-rate dry/reverb graph is created
	// and reset. Some drivers begin processing the mastering voice immediately;
	// exposing a partially constructed graph can produce a one-buffer startup
	// burst even though no authored sound has started yet.
	ReportXAudioFailureOnce("Mute mastering voice during initialization",
		s_musicMasterVoice->SetVolume(0.0f));
	#ifdef _WIN32
	SpuMixSaturator* musicCore = new SpuMixSaturator(false);
	XAUDIO2_EFFECT_DESCRIPTOR musicCoreEffect{musicCore, TRUE, 2};
	XAUDIO2_EFFECT_CHAIN musicCoreChain{1, &musicCoreEffect};
	const HRESULT musicCoreResult = s_xaudio2->CreateSubmixVoice(
		&s_musicCoreVoice, 2, 48000, 0, 0, nullptr, &musicCoreChain);
	musicCore->Release();
	#else
	const HRESULT musicCoreResult = s_xaudio2->CreateSubmixVoice(
		&s_musicCoreVoice, 2, 48000, 0, 0, nullptr, nullptr);
	#endif
	if (FAILED(musicCoreResult) || !s_musicCoreVoice)
	{
		ReportRuntimeError("Audio",
			"Create music core submix failed (HRESULT 0x%08X)",
			static_cast<unsigned>(musicCoreResult));
		#ifdef _WIN32
		s_finalOutputProcessor = nullptr;
		#endif
		s_musicMasterVoice->DestroyVoice();
		s_musicMasterVoice = nullptr;
		s_xaudio2->UnregisterForCallbacks(&s_audioEngineCallback);
		s_xaudio2->Release();
		s_xaudio2 = nullptr;
		return false;
	}
	s_audioCriticalResult.store(S_OK, std::memory_order_release);
	s_audioCriticalError.store(false, std::memory_order_release);
	EnsureEffectReverb();
	return true;
}

void RecalculateChannelVolume(int channel)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	if (channel < 0 || channel >= static_cast<int>(s_channelVolumes.size()))
		return;
	// Retail initializes all 11 mixer modifiers for all four channels to 1.0.
	// Channel differences come only from the active modifier groups.
	float volume = 1.0f;
	for (const auto& group : s_relativeVolumes)
		volume *= group[channel];
	s_channelVolumes[channel] = (std::max)(0.0f, (std::min)(1.0f, volume));

	const int musicChannel = kMusicMixerChannel;
	if (channel == musicChannel)
	{
		for (MusicVoice& active : s_music989.voices)
			if (active.voice) SetVoiceVolumeChecked(active.voice,
				active.baseVolume * (active.envelope.level / 32767.0f) *
				s_channelVolumes[musicChannel] * s_musicVolume * s_dialogMusicDuck *
				s_userMusicVolume, "Update music channel volume");
	}
	// Retail VAG/dialog streams use mixer channel 2. Channel 1 is music and is
	// independently controlled by the prompt's Music option.
	if (channel == 2 && s_vagStream.loaded)
	{
		if (s_pambVagAmbient)
			SetPambVol(s_pambVagAmbient, s_pambVagAmbient->uVolAtSource);
		else if (s_vagStream.voice)
			SetVoiceVolumeChecked(s_vagStream.voice, kVagDriverGain *
				(s_fFmvAudioMode ? 1.0f : s_channelVolumes[2]) * s_userDialogueVolume,
				"Update VAG/dialog channel volume");
	}
}

void CloseMusicVoice()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	// Source buffers point directly into MusicTone::pcm. Move the complete bank
	// into shared retirement storage before clearing the live sequencer so those
	// addresses remain valid until every fading XAudio voice has been destroyed.
	auto retiredMusic = std::make_shared<Music989State>(std::move(s_music989));
	for (MusicVoice& active : retiredMusic->voices)
	{
		if (active.voice)
			RetireMusicVoice(active, retiredMusic);
	}
	s_music989 = {};
	s_musicStream = {};
}

std::filesystem::path ResolveVagWav(const char* fileName)
{
    if (!fileName || fileName[0] == '\0')
        return {};

    return ResolveNamedVag(fileName);
}

static void ApplyDialogMusicDuck(float duck)
{
	s_dialogMusicDuck = duck;
	const int musicChannel = kMusicMixerChannel;
	for (MusicVoice& active : s_music989.voices)
		if (active.voice) SetVoiceVolumeChecked(active.voice,
			active.baseVolume * (active.envelope.level / 32767.0f) *
			s_channelVolumes[musicChannel] * s_musicVolume * s_dialogMusicDuck *
			s_userMusicVolume, "Update dialogue music duck");
}

void CloseVagStream()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	const bool wasCommentary = s_vagStream.commentary;
	if (s_vagStream.voice)
		RetireSourceVoice(s_vagStream.voice, s_vagStream.pcm);
    s_vagStream = {};
	s_hvagAmbient = 0;
	// A dialogue can contain several separate VAG clips with camera and speaker
	// events between them. Keep music ducked while that dialogue owns the audio;
	// otherwise every completed line briefly restores full music volume.
	ApplyDialogMusicDuck(s_dialogAudioDuckHeld ? 0.2f : 1.0f);

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
	bool sawEnd = false;
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
			// Multiplication preserves the SPU sign-extension math without relying
			// on undefined C++ behavior from left-shifting a negative nibble.
			int value = (nibble * 4096) >> shift;
			value += (coefficients[filter][0] * history1 +
				coefficients[filter][1] * history2 + 32) >> 6;
			value = std::clamp(value, -32768, 32767);
			history2 = history1;
			history1 = value;
			pcm.push_back(static_cast<int16_t>(value));
		}
		if ((flags & 1) != 0)
		{
			sawEnd = true;
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
	// Keep the transient predictor passes and move the fixed PCM loop point
	// forward once the ending history repeats. Replacing the intro's first loop
	// pass with a later pass creates a potentially full-scale discontinuity.
	if (tone.loop && loopBlockStart < loopBlockEnd)
	{
		const UINT32 encodedLoopLength = tone.loopLength;
		std::vector<int16_t> decodedLoop;
		int previousEndHistory1 = history1;
		int previousEndHistory2 = history2;
		for (int iteration = 0; iteration < 32; ++iteration)
		{
			decodedLoop.clear();
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
					int value = (nibble * 4096) >> shift;
					value += (coefficients[filter][0] * history1 +
						coefficients[filter][1] * history2 + 32) >> 6;
					value = std::clamp(value, -32768, 32767);
					history2 = history1;
					history1 = value;
					decodedLoop.push_back(static_cast<int16_t>(value));
				}
			}
			if (decodedLoop.size() != encodedLoopLength)
				break;
			const UINT32 passBegin = static_cast<UINT32>(pcm.size());
			pcm.insert(pcm.end(), decodedLoop.begin(), decodedLoop.end());
			tone.loopBegin = passBegin;
			tone.loopLength = encodedLoopLength;
			if (history1 == previousEndHistory1 && history2 == previousEndHistory2)
				break;
			previousEndHistory1 = history1;
			previousEndHistory2 = history2;
		}
		// SPU2 always honors LOOP/END and carries decoder history across the jump.
		// XAudio loops fixed PCM, so use the latest predictor pass even when the
		// history has not reached an exact fixed point. Dropping the loop entirely
		// makes authored sustained instruments terminate and breaks the arrangement.
	}
	if (pcm.empty() || !sawEnd)
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
			tone.flags = flags;
			tone.sampleOffset = streamOffset;
			tone.program = programIndex;
			tone.priority = data[toneEntry];
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
			s_music989.tones.push_back(std::move(tone));
		}
	}
	std::vector<uint32_t> musicSampleOffsets;
	musicSampleOffsets.reserve(s_music989.tones.size());
	for (const MusicTone& tone : s_music989.tones)
		musicSampleOffsets.push_back(tone.sampleOffset);
	std::sort(musicSampleOffsets.begin(), musicSampleOffsets.end());
	musicSampleOffsets.erase(std::unique(musicSampleOffsets.begin(), musicSampleOffsets.end()),
		musicSampleOffsets.end());
	for (MusicTone& tone : s_music989.tones)
	{
		const auto next = std::upper_bound(musicSampleOffsets.begin(), musicSampleOffsets.end(),
			tone.sampleOffset);
		tone.sampleEndOffset = next != musicSampleOffsets.end() ? *next : audioSize;
		if (tone.sampleEndOffset <= tone.sampleOffset ||
			!DecodeMusicAdpcm(data, audioOffset + tone.sampleOffset,
				static_cast<std::size_t>(audioOffset) + tone.sampleEndOffset, tone))
			return false;
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

	std::vector<uint32_t> sampleOffsets;
	for (uint16_t program = 0; program < programCount; ++program)
	{
		const std::size_t programEntry = programTable + static_cast<std::size_t>(program) * 8;
		if (programEntry + 8 > data.size())
			return;
		const uint8_t toneCount = data[programEntry];
		const uint32_t toneTable = sblkOffset + MusicLe32(data.data() + programEntry + 4);
		for (uint8_t toneIndex = 0; toneIndex < toneCount; ++toneIndex)
		{
			const std::size_t toneEntry = toneTable + static_cast<std::size_t>(toneIndex) * 0x18;
			if (toneEntry + 0x18 > data.size())
				return;
			const uint16_t toneFlags = MusicLe16(data.data() + toneEntry + 0x0e);
			const uint32_t streamOffset = MusicLe32(data.data() + toneEntry + 0x10);
			if ((toneFlags & 0x80) == 0 && streamOffset < audioSize)
				sampleOffsets.push_back(streamOffset);
		}
	}
	std::sort(sampleOffsets.begin(), sampleOffsets.end());
	sampleOffsets.erase(std::unique(sampleOffsets.begin(), sampleOffsets.end()),
		sampleOffsets.end());

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
			tone.sampleOffset = streamOffset;
			tone.loop = false;
			tone.loopBegin = 0;
			tone.loopLength = 0;
			UINT32 loopStart = 0;
			bool foundStart = false;
			const std::size_t stream = audioOffset + streamOffset;
			const auto next = std::upper_bound(sampleOffsets.begin(), sampleOffsets.end(), streamOffset);
			const uint32_t streamEndOffset = next != sampleOffsets.end() ? *next : audioSize;
			const std::size_t streamEnd = static_cast<std::size_t>(audioOffset) + streamEndOffset;
			for (std::size_t frame = 0; stream + frame * 16 + 16 <= streamEnd; ++frame)
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
		tone.priority = toneJson.value("priority", 0);
		tone.mapLow = toneJson.value("mapLow", 0);
		tone.mapHigh = toneJson.value("mapHigh", 127);
		tone.volume = toneJson.value("volume", 127);
		tone.pan = toneJson.value("pan", 0);
		tone.centerNote = toneJson.value("centerNote", 60);
		tone.centerFine = toneJson.value("centerFine", 0);
		tone.pitchBendLow = toneJson.value("pitchBendLow", 0);
		tone.pitchBendHigh = toneJson.value("pitchBendHigh", 0);
		tone.flags = toneJson.value("flags", 0);
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
		tone.applyPs1SampleRateCorrection = false;
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
	if (id >= s_music989.segmentOffsets.size())
	{
		TraceMusicEvent("queue_segment_rejected", static_cast<int>(id));
		return;
	}
	if (std::find(s_music989.pendingStarts.begin(), s_music989.pendingStarts.end(), id) ==
		s_music989.pendingStarts.end())
	{
		s_music989.pendingStarts.push_back(id);
		TraceMusicEvent("queue_segment", static_cast<int>(id));
	}
}

void StopMusicSegment(uint32_t id)
{
	TraceMusicEvent("stop_segment", static_cast<int>(id));
	for (MusicSegment& segment : s_music989.activeSegments)
		if (segment.id == id)
			segment.complete = true;
	for (MusicVoice& active : s_music989.voices)
	{
		if (active.voice && active.segment == id)
			ReleaseMusicEnvelope(active.envelope);
	}
}

static void ReapplyMusicAmeGroups(int basis);

void SetGlobalExciteValue(int excite)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	const int previous = s_globalExcite;
	s_globalExcite = static_cast<uint8_t>(std::clamp(excite, 0, 127));
	if (previous != s_globalExcite)
		TraceMusicEvent("set_excite", previous, s_globalExcite);
	ReapplyMusicAmeGroups(0);
}

static void StopMusicVoicesOnChannel(uint32_t segment, uint8_t channel)
{
	for (MusicVoice& voice : s_music989.voices)
	{
		if (!voice.voice || voice.segment != segment || voice.channel != channel)
			continue;
		RetireMusicVoice(voice);
		voice.envelope.phase = MusicEnvelopePhase::Stopped;
	}
}

static void ApplyMusicAmeGroup(MusicAmeGroup& group, MusicSegment& segment)
{
	if (!group.active || segment.complete)
		return;
	if (group.basis > s_music989.registers.size())
	{
		group.active = false;
		return;
	}

	const uint8_t value = group.basis == 0
		? s_globalExcite
		: s_music989.registers[group.basis - 1];
	for (uint8_t i = 0; i < group.channelCount; ++i)
	{
		const uint8_t channel = group.channels[i];
		if (channel >= segment.muted.size())
			continue;
		const bool mute = value < group.exciteMin[i] || value > group.exciteMax[i];
		const bool wasMuted = segment.muted[channel];
		if (mute && !segment.muted[channel])
			StopMusicVoicesOnChannel(segment.id, channel);
		segment.muted[channel] = mute;
		if (mute != wasMuted)
			TraceMusicEvent(mute ? "mute_channel" : "unmute_channel",
				static_cast<int>(segment.id), static_cast<int>(channel));
	}
}

static void ReapplyMusicAmeGroups(int basis)
{
	for (MusicAmeGroup& group : s_music989.groups)
		if (group.active && group.basis == basis)
			for (MusicSegment& segment : s_music989.activeSegments)
				ApplyMusicAmeGroup(group, segment);
}

bool RunMusicAme(MusicSegment& midi, const uint8_t*& stream,
	bool* transferredSegment)
{
	static thread_local int recursionDepth = 0;
	if (recursionDepth >= 16)
		return false;
	struct RecursionGuard
	{
		int& depth;
		explicit RecursionGuard(int& value) : depth(value) { ++depth; }
		~RecursionGuard() { --depth; }
	} recursionGuard(recursionDepth);
	int skip = 0;
	for (int safety = 0; safety < 1024 && stream < midi.end; ++safety)
	{
		const uint8_t op = *stream++;
		if (op == 0xf7)
			return true;
		auto require = [&](std::size_t count)
		{
			return count <= static_cast<std::size_t>(midi.end - stream);
		};
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
		case 0x03: if (!require(1)) return false; if (beginCommand()) StopMusicSegment(*stream); ++stream; break;
		case 0x04: if (skip == 1) skip = 2; break;
		case 0x05: if (skip == 2) skip = 0; break;
		case 0x06: if (!require(2) || stream[0] >= s_music989.registers.size()) return false; if (beginCommand() && s_music989.registers[stream[0]] > stream[1] - 1) skip = 1; stream += 2; break;
		case 0x07: if (!require(2) || stream[0] >= s_music989.registers.size()) return false; if (beginCommand() && s_music989.registers[stream[0]] < stream[1] + 1) skip = 1; stream += 2; break;
		case 0x0b:
		{
			if (!require(1)) return false;
			const uint8_t macroIndex = *stream++;
			if (macroIndex >= s_music989.macros.size())
				return false;
			s_music989.macros[macroIndex] = stream;
			while (stream < midi.end && *stream++ != 0xf7) {}
			if (stream == midi.end && midi.end[-1] != 0xf7) return false;
			break;
		}
		case 0x0c:
			if (!require(1) || *stream >= s_music989.macros.size()) return false;
			if (beginCommand() && s_music989.macros[*stream])
			{
				const uint8_t* macro = s_music989.macros[*stream];
				bool macroTransferredSegment = false;
				if (!RunMusicAme(midi, macro, &macroTransferredSegment))
				{
					midi.complete = true;
					return false;
				}
				// A branch/start inside a called macro transfers control and must
				// unwind immediately. A macro is also allowed to stop its caller,
				// then return so the caller can queue the replacement segment. Using
				// midi.complete for both cases swallowed that replacement and left
				// the level music permanently exhausted.
				if (macroTransferredSegment)
				{
					if (transferredSegment)
						*transferredSegment = true;
					++stream;
					return true;
				}
			}
			++stream; break;
		case 0x0d: if (!require(1) || stream[0] >= s_music989.registers.size()) return false; if (beginCommand()) { QueueMusicSegment(s_music989.registers[stream[0]] - 1); midi.complete = true; if (transferredSegment) *transferredSegment = true; } ++stream; break;
		case 0x0e: if (!require(1) || stream[0] >= s_music989.registers.size()) return false; if (beginCommand()) QueueMusicSegment(s_music989.registers[stream[0]] - 1); ++stream; break;
		case 0x11: if (!require(1)) return false; if (beginCommand()) { QueueMusicSegment(*stream); midi.complete = true; if (transferredSegment) *transferredSegment = true; } ++stream; break;
		case 0x12: if (!require(1)) return false; if (beginCommand()) QueueMusicSegment(*stream); ++stream; break;
		case 0x13:
			if (!require(2) || stream[0] >= s_music989.registers.size()) return false;
			if (beginCommand())
			{
				const int reg = stream[0];
				s_music989.registers[reg] = stream[1];
				ReapplyMusicAmeGroups(reg + 1);
			}
			stream += 2;
			break;
		case 0x14:
			if (!require(1) || *stream >= s_music989.registers.size()) return false;
			if (beginCommand())
			{
				const int reg = *stream;
				if (s_music989.registers[reg] < 0x7f) ++s_music989.registers[reg];
				ReapplyMusicAmeGroups(reg + 1);
			}
			++stream;
			break;
		case 0x15:
			if (!require(1) || *stream >= s_music989.registers.size()) return false;
			if (beginCommand())
			{
				const int reg = *stream;
				if (s_music989.registers[reg] > 0) --s_music989.registers[reg];
				ReapplyMusicAmeGroups(reg + 1);
			}
			++stream;
			break;
		case 0x16: if (!require(2) || stream[0] >= s_music989.registers.size()) return false; if (beginCommand() && s_music989.registers[stream[0]] != stream[1]) skip = 1; stream += 2; break;
		// Retail performs these comparisons after adjusting the encoded threshold:
		// opcode 00 tests excite < operand + 1 and opcode 02 tests
		// operand - 1 < excite, equivalent to the boundary checks below.
		case 0x00: if (!require(1)) return false; if (beginCommand() && s_globalExcite <= static_cast<int>(stream[0])) skip = 1; ++stream; break;
		case 0x01: if (!require(1)) return false; if (beginCommand() && s_globalExcite != static_cast<int>(stream[0]) + 1) skip = 1; ++stream; break;
		case 0x02: if (!require(1)) return false; if (beginCommand() && s_globalExcite >= static_cast<int>(stream[0])) skip = 1; ++stream; break;
		case 0x0f:
			if (skip)
			{
				while (stream < midi.end && *stream != 0xf7) ++stream;
				if (stream >= midi.end) return false;
				++stream;
				if (skip == 1) skip = 0;
			}
			else
			{
				if (!require(2)) return false;
				const uint8_t groupIndex = *stream++;
				if (groupIndex >= s_music989.groups.size()) return false;
				MusicAmeGroup& group = s_music989.groups[groupIndex];
				group = {};
				group.basis = *stream++;
				if (group.basis > s_music989.registers.size()) return false;
				while (stream < midi.end && *stream != 0xf7)
				{
					if (group.channelCount >= 16 || stream + 3 > midi.end) return false;
					const uint8_t index = group.channelCount++;
					group.channels[index] = *stream++;
					group.exciteMin[index] = *stream++;
					group.exciteMax[index] = *stream++;
				}
				if (stream >= midi.end) return false;
				++stream;
			}
			break;
		case 0x10:
			if (!require(1)) return false;
			if (beginCommand())
			{
				const uint8_t groupIndex = *stream;
				if (groupIndex >= s_music989.groups.size()) return false;
				MusicAmeGroup& group = s_music989.groups[groupIndex];
				group.active = true;
				ApplyMusicAmeGroup(group, midi);
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
	counterStep = (std::max)(1u, counterStep);
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
			// The envelope is logically silent, but XAudio may still be holding the
			// previous 240 Hz gain. Retire through the short host-side fade instead
			// of destroying a potentially nonzero waveform at an arbitrary sample.
			RetireMusicVoice(active);
		}
		else
		{
			const float envelopeVolume = active.envelope.level / 32767.0f;
			const int musicChannel = kMusicMixerChannel;
			const float finalVolume = active.baseVolume * envelopeVolume *
				s_channelVolumes[musicChannel] * s_musicVolume * s_dialogMusicDuck *
				s_userMusicVolume;
			SetVoiceVolumeChecked(active.voice, finalVolume, "Update music envelope volume");
		}
	}
}

void SetMusicVoicePan(IXAudio2SourceVoice* voice, int pan)
{
	if (!voice || !s_musicCoreVoice)
		return;
	XAUDIO2_VOICE_DETAILS sourceDetails{};
	XAUDIO2_VOICE_DETAILS destinationDetails{};
	voice->GetVoiceDetails(&sourceDetails);
	s_musicCoreVoice->GetVoiceDetails(&destinationDetails);
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
	ReportXAudioFailureOnce("Set music output matrix",
		voice->SetOutputMatrix(s_musicCoreVoice, 1,
			destinationDetails.InputChannels, matrix.data()));
}

static void RefreshMusicVoiceControllerState(MusicVoice& active,
	const MusicSegment& segment)
{
	if (!active.voice || !active.tone || active.channel >= segment.volumes.size())
		return;
	const int programIndex = active.tone->program;
	const float programVolume = programIndex >= 0 &&
		programIndex < static_cast<int>(s_music989.programs.size())
		? s_music989.programs[programIndex].volume / 127.0f : 1.0f;
	const int programPan = programIndex >= 0 &&
		programIndex < static_cast<int>(s_music989.programs.size())
		? s_music989.programs[programIndex].pan : 0;
	const float linearVolume = (active.velocity / 127.0f) *
		(segment.volumes[active.channel] / 127.0f) *
		(active.tone->volume / 127.0f) * programVolume *
		(s_music989.cueVolume / 127.0f);
	active.baseVolume = linearVolume * linearVolume;
	active.pan = s_music989.cuePan + segment.pans[active.channel] +
		programPan + active.tone->pan;
	SetMusicVoicePan(active.voice, active.pan);
	const int musicChannel = kMusicMixerChannel;
	SetVoiceVolumeChecked(active.voice,
		active.baseVolume * (active.envelope.level / 32767.0f) *
		s_channelVolumes[musicChannel] * s_musicVolume * s_dialogMusicDuck *
		s_userMusicVolume, "Update music controller state");
}

static bool ReserveMusicHardwareVoice(int incomingPriority)
{
	std::size_t activeCount = 0;
	auto victim = s_music989.voices.end();
	for (auto it = s_music989.voices.begin(); it != s_music989.voices.end(); ++it)
	{
		if (!it->voice)
			continue;
		++activeCount;
		if (victim == s_music989.voices.end() || it->priority < victim->priority ||
			(it->priority == victim->priority &&
				it->allocationSerial < victim->allocationSerial))
		{
			victim = it;
		}
	}
	if (activeCount < kSpuVoicesPerCore)
		return true;
	if (victim == s_music989.voices.end() || incomingPriority < victim->priority)
		return false;

	// XAudio2 cannot switch a hardware-style voice slot at a sample boundary.
	// Retire the old waveform over the short anti-pop window so stealing a busy
	// music voice cannot inject a full-band discontinuity into the output.
	RetireMusicVoice(*victim);
	s_music989.voices.erase(victim);
	return true;
}

void StartMusicTone(MusicSegment& segment, uint8_t channel, uint8_t note, uint8_t velocity,
	const MusicTone* onlyTone, uint32_t playBegin)
{
	if (segment.muted[channel])
		return;
	const int programIndex = segment.programs[channel];
	for (MusicTone& tone : s_music989.tones)
	{
		if (onlyTone && &tone != onlyTone)
			continue;
		if (tone.program != programIndex || note < tone.mapLow || note > tone.mapHigh)
			continue;
		if (!ReserveMusicHardwareVoice(tone.priority))
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
		XAUDIO2_SEND_DESCRIPTOR musicSend{};
		musicSend.Flags = 0;
		musicSend.pOutputVoice = s_musicCoreVoice;
		XAUDIO2_VOICE_SENDS musicSends{1, &musicSend};
		const HRESULT createResult = s_xaudio2->CreateSourceVoice(
			&voice, &format, 0, kSpuMaximumFrequencyRatio, nullptr, &musicSends);
		if (FAILED(createResult) || !voice)
		{
			ReportXAudioFailureOnce("Create music source voice",
				FAILED(createResult) ? createResult : E_POINTER);
			continue;
		}
		XAUDIO2_BUFFER buffer{};
		buffer.AudioBytes = static_cast<UINT32>(tone.pcm.size());
		buffer.pAudioData = tone.pcm.data();
		buffer.Flags = XAUDIO2_END_OF_STREAM;
		const uint32_t sampleCount = tone.format.blockAlign != 0
			? static_cast<uint32_t>(tone.pcm.size() / tone.format.blockAlign) : 0;
		if (sampleCount != 0)
		{
			if (tone.loop && tone.loopLength != 0 && playBegin >= tone.loopBegin)
				playBegin = tone.loopBegin + (playBegin - tone.loopBegin) % tone.loopLength;
			buffer.PlayBegin = (std::min)(playBegin, sampleCount - 1);
		}
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
		const bool applyPs1RateCorrection = ps1Pitch && tone.applyPs1SampleRateCorrection;
		const float baseFrequencyRatio = static_cast<float>(std::pow(2.0,
			baseSemitones / 12.0) * (applyPs1RateCorrection ? 44100.0 / 48000.0 : 1.0));
		const int currentPitchBend = segment.pitchBends[channel];
		const double bendSemitones = currentPitchBend >= 0
			? tone.pitchBendHigh * static_cast<double>(currentPitchBend) / 32767.0
			: tone.pitchBendLow * static_cast<double>(currentPitchBend) / 32768.0;
		const float safeBaseFrequencyRatio = std::isfinite(baseFrequencyRatio)
			? baseFrequencyRatio : 1.0f;
		const float frequencyRatio = std::clamp(
			safeBaseFrequencyRatio * static_cast<float>(std::pow(2.0, bendSemitones / 12.0)),
			XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio);
		const int finalPan = s_music989.cuePan + segment.pans[channel] + programPan + tone.pan;
		SetVoiceVolumeChecked(voice, 0.0f, "Mute new music source voice");
		SetMusicVoicePan(voice, finalPan);
		const HRESULT pitchResult = SetVoiceFrequencyRatioChecked(voice, frequencyRatio,
			"Set music source pitch");
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
			active.tone = &tone;
			active.segment = segment.id;
			active.channel = channel;
			active.note = note;
			active.velocity = velocity;
			active.baseVolume = baseVolume;
			active.baseFrequencyRatio = safeBaseFrequencyRatio;
			active.pitchBendLow = tone.pitchBendLow;
			active.pitchBendHigh = tone.pitchBendHigh;
			active.pan = finalPan;
			active.priority = tone.priority;
			active.allocationSerial = ++s_audioVoiceSerial;
			StartMusicEnvelope(active.envelope, tone.adsr1, tone.adsr2);
			s_music989.voices.push_back(std::move(active));
		}
		else
		{
			ReportXAudioFailureOnce("Submit music source buffer", submitResult);
			ReportXAudioFailureOnce("Start music source voice", startResult);
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
		segment.initialTempo = segment.tempo;
		segment.ppq = static_cast<int32_t>(MusicLe32(base + 36));
		segment.repeatsRemaining = s_music989.cueRepeats;
		segment.volumes.fill(127);
		s_music989.activeSegments.push_back(segment);
		TraceMusicEvent("activate_segment", static_cast<int>(id));
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
		if (segment.status == 0)
		{
			segment.complete = true;
			break;
		}
		const uint8_t channel = segment.status & 0x0f;
		auto require = [&](std::size_t count)
		{
			if (count <= static_cast<std::size_t>(segment.end - segment.cursor))
				return true;
			segment.complete = true;
			return false;
		};
		switch (segment.status >> 4)
		{
		case 0x8: if (!require(2)) break; StopMusicTone(segment.id, channel, segment.cursor[0]); segment.cursor += 2; break;
		case 0x9:
			if (!require(2)) break;
			if (segment.cursor[1] == 0) StopMusicTone(segment.id, channel, segment.cursor[0]);
			else StartMusicTone(segment, channel, segment.cursor[0], segment.cursor[1]);
			segment.cursor += 2; break;
		case 0xa:
			// Polyphonic key pressure is a valid two-byte MIDI channel event. 989SND
			// does not use it to alter SPU voices, but it must still be consumed so an
			// otherwise valid track is not terminated or parsed out of alignment.
			if (!require(2)) break;
			segment.cursor += 2;
			break;
		case 0xb:
			if (!require(2)) break;
			if (segment.cursor[0] == 7)
			{
				segment.volumes[channel] = segment.cursor[1];
				for (MusicVoice& active : s_music989.voices)
					if (active.segment == segment.id && active.channel == channel)
						RefreshMusicVoiceControllerState(active, segment);
			}
			else if (segment.cursor[0] == 10)
			{
				const uint8_t pan = segment.cursor[1];
				segment.pans[channel] = pan >= 64
					? static_cast<int16_t>(90 * (pan - 64) / 63)
					: static_cast<int16_t>(90 * pan / 64 + 270);
				for (MusicVoice& active : s_music989.voices)
					if (active.segment == segment.id && active.channel == channel)
						RefreshMusicVoiceControllerState(active, segment);
			}
			segment.cursor += 2; break;
		case 0xc: if (!require(1)) break; segment.programs[channel] = *segment.cursor++; break;
		case 0xd:
			if (!require(1)) break;
			// 989SND uses channel pressure's byte as a note and keys off matching voices.
			StopMusicTone(segment.id, channel, *segment.cursor++);
			break;
		case 0xe:
		{
			if (!require(2)) break;
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
				SetVoiceFrequencyRatioChecked(active.voice, std::clamp(
					active.baseFrequencyRatio *
					static_cast<float>(std::pow(2.0, semitones / 12.0)),
					XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio),
					"Update music pitch bend");
			}
			segment.cursor += 2;
			break;
		}
		case 0xf:
			if (segment.status == 0xff)
			{
				if (!require(1)) break;
				const uint8_t type = *segment.cursor++;
				uint32_t length = 0; if (!ReadMusicVlq(segment, length)) { segment.complete = true; break; }
				if (!require(length)) break;
				if (type == 0x2f)
				{
					// A repeat count of zero is the 989SND convention for an infinite
					// music cue.  Positive values count complete track plays.
					if (segment.repeatsRemaining > 0 && --segment.repeatsRemaining == 0)
						segment.complete = true;
					else
					{
						ResetMusicSegmentForLoop(segment);
					}
				}
				else
				{
					if (type == 0x51 && length >= 3)
					{
						segment.tempo = (segment.cursor[0] << 16) |
							(segment.cursor[1] << 8) | segment.cursor[2];
					}
					segment.cursor += length;
				}
			}
			else if (segment.status == 0xf0 && require(1) && *segment.cursor++ == 0x75)
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

bool DecodeEffectAdpcm(const EFFECTBANK& bank, uint32_t sampleOffset,
	uint32_t sampleEndOffset, DecodedEffectSample& decoded)
{
	if (sampleOffset >= bank.sampleSize || sampleEndOffset <= sampleOffset ||
		sampleEndOffset > bank.sampleSize)
	{
		return false;
	}
	// Match SPU2's complete ADPCM header handling. Reserved predictor IDs have
	// zero coefficients instead of aliasing to IDs 0..7 or rejecting the tone.
	static constexpr int coefficients[16][2] =
	{
		{ 0, 0 }, { 60, 0 }, { 115, -52 }, { 98, -55 }, { 122, -60 }
	};
	const std::size_t begin = static_cast<std::size_t>(bank.sampleOffset) + sampleOffset;
	const std::size_t end = static_cast<std::size_t>(bank.sampleOffset) + sampleEndOffset;
	int history1 = 0;
	int history2 = 0;
	UINT32 loopStart = 0;
	std::size_t loopBlockStart = 0;
	std::size_t loopBlockEnd = 0;
	bool sawLoopStart = false;
	bool sawEnd = false;
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
			// Multiplication preserves the SPU sign-extension math without relying
			// on undefined C++ behavior from left-shifting a negative nibble.
			int value = (nibble * 4096) >> shift;
			value += (coefficients[filter][0] * history1 +
				coefficients[filter][1] * history2 + 32) >> 6;
			value = std::clamp(value, -32768, 32767);
			history2 = history1;
			history1 = value;
			decoded.pcm.push_back(static_cast<int16_t>(value));
		}
		if ((flags & 1) != 0)
		{
			sawEnd = true;
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
	if (!sawEnd)
	{
		return false;
	}

	// The SPU2 carries the ADPCM predictor history across every loop jump.
	// XAudio2 loops fixed PCM, so preserve the predictor's transient loop passes
	// in the submitted buffer and begin the fixed PCM loop only when the ending
	// predictor state repeats. Replacing the first pass with the final pass made
	// the intro jump directly to a different predictor state; some ambient loops
	// produced a nearly full-scale discontinuity during world startup.
	if (decoded.loop && loopBlockStart < loopBlockEnd)
	{
		const std::size_t originalPcmSize = decoded.pcm.size();
		const UINT32 encodedLoopLength = decoded.loopLength;
		std::vector<int16_t> decodedLoop;
		int previousEndHistory1 = history1;
		int previousEndHistory2 = history2;
		bool loopConverged = false;
		for (int iteration = 0; iteration < 32; ++iteration)
		{
			decodedLoop.clear();
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
					int value = (nibble * 4096) >> shift;
					value += (coefficients[filter][0] * history1 +
						coefficients[filter][1] * history2 + 32) >> 6;
					value = std::clamp(value, -32768, 32767);
					history2 = history1;
					history1 = value;
					decodedLoop.push_back(static_cast<int16_t>(value));
				}
			}
			if (decodedLoop.size() != encodedLoopLength)
				break;

			const UINT32 passBegin = static_cast<UINT32>(decoded.pcm.size());
			decoded.pcm.insert(decoded.pcm.end(), decodedLoop.begin(), decodedLoop.end());
			decoded.loopBegin = passBegin;
			decoded.loopLength = encodedLoopLength;

			// An identical end state means this pass starts and ends with the same
			// predictor history as the preceding pass and can repeat seamlessly.
			if (history1 == previousEndHistory1 && history2 == previousEndHistory2)
			{
				loopConverged = true;
				break;
			}
			previousEndHistory1 = history1;
			previousEndHistory2 = history2;
		}
		if (!loopConverged)
		{
			decoded.pcm.resize(originalPcmSize);
			decoded.loop = false;
			decoded.loopBegin = 0;
			decoded.loopLength = 0;
		}
	}
	return !decoded.pcm.empty();
}

void SetEffectVoicePan(IXAudio2SourceVoice* voice, int pan)
{
	if (!voice || !s_musicMasterVoice)
		return;
	IXAudio2Voice* dryDestination = s_effectDryVoice
		? static_cast<IXAudio2Voice*>(s_effectDryVoice)
		: static_cast<IXAudio2Voice*>(s_musicMasterVoice);
	XAUDIO2_VOICE_DETAILS destinationDetails{};
	dryDestination->GetVoiceDetails(&destinationDetails);
	if (destinationDetails.InputChannels == 0)
		return;
	while (pan >= 360) pan -= 360;
	while (pan < 0) pan += 360;
	if (pan > 180) pan -= 360;
	const float position = std::sin(static_cast<float>(pan) * 3.14159265358979323846f / 180.0f);
	std::vector<float> matrix(destinationDetails.InputChannels, 0.0f);
	// Match the SPU2 voice-volume pan law used by 989SND. A centered voice is
	// programmed to approximately 0x3fff on each side (half of the 0x7fff
	// full-scale voice volume), so it must be 0.5/0.5 rather than an
	// equal-power 0.707/0.707 split. The latter made layered centered effects
	// roughly 3 dB too hot before they reached the dry and reverb buses.
	matrix[0] = s_stereoOutputEnabled ? (1.0f - position) * 0.5f : 0.5f;
	if (destinationDetails.InputChannels >= 2)
		matrix[1] = s_stereoOutputEnabled ? (1.0f + position) * 0.5f : 0.5f;
	ReportXAudioFailureOnce("Set dry-effects output matrix",
		voice->SetOutputMatrix(dryDestination, 1,
			destinationDetails.InputChannels, matrix.data()));
	IXAudio2Voice* wetDestination = ActiveEffectWetVoice();
	if (s_effectDryVoice && wetDestination)
	{
		const float reverbMatrix[2] =
		{
			matrix[0], matrix[1]
		};
		ReportXAudioFailureOnce("Set active-reverb output matrix",
			voice->SetOutputMatrix(wetDestination, 1, 2, reverbMatrix));
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
	const float programmedVolume = adjustedSoundVolume * active.toneVolume;
	// AdjustVolToGroup curves the programmed voice volume. Mixer-channel and
	// user controls are downstream gains and therefore remain linear, matching
	// the music voice path and retail's grouping order.
	return programmedVolume * programmedVolume * s_channelVolumes[group] *
		s_userSfxVolume * (active.envelopeLevel / 32767.0f);
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
		}
	}
	counterStep = (std::max)(1u, counterStep);
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

static bool ReserveEffectHardwareVoice(int incomingPriority)
{
	if (!g_psw)
		return false;
	std::size_t activeCount = 0;
	AMB* victimOwner = nullptr;
	std::size_t victimIndex = 0;
	for (const std::unique_ptr<AMB>& ownerSlot : g_psw->ambOwners)
	{
		if (!ownerSlot)
			continue;
		AMB& owner = *ownerSlot;
		for (std::size_t index = 0; index < owner.voices.size(); ++index)
		{
			const EFFECTVOICE& candidate = owner.voices[index];
			if (!candidate.voice)
				continue;
			++activeCount;
			if (!victimOwner)
			{
				victimOwner = &owner;
				victimIndex = index;
				continue;
			}
			const EFFECTVOICE& victim = victimOwner->voices[victimIndex];
			if (candidate.priority < victim.priority ||
				(candidate.priority == victim.priority &&
					candidate.allocationSerial < victim.allocationSerial))
			{
				victimOwner = &owner;
				victimIndex = index;
			}
		}
	}
	if (activeCount < kSpuVoicesPerCore)
		return true;
	if (!victimOwner || incomingPriority < victimOwner->voices[victimIndex].priority)
		return false;

	EFFECTVOICE& victim = victimOwner->voices[victimIndex];
	RetireSourceVoice(victim.voice, victim.pcm);
	victimOwner->voices.erase(victimOwner->voices.begin() + victimIndex);
	victimOwner->voice = victimOwner->voices.empty() ? nullptr : victimOwner->voices.front().voice;
	return true;
}

bool PlayEffectTone(AMB& amb, const EFFECTBANK& bank, const EFFECTBANK::SOUND& sound,
	const EFFECTBANK::TONE& tone, float applicationVolume, float pitchMod)
{
	if (!EnsureMusicEngine())
		return false;
	const int hardwarePriority = static_cast<uint8_t>(tone.priority);
	if (!ReserveEffectHardwareVoice(hardwarePriority))
	{
		return false;
	}

	DecodedEffectSample decoded;
	if ((tone.flags & 8) != 0) // SPU noise tone
	{
		// Noise voices do not read ADPCM data on SPU2. Generate one complete
		// deterministic LFSR period and loop it; ADSR is handled by the normal
		// voice path while pitch controls are ignored like an SPU noise voice.
		// This uses the same feedback table as
		// PCSX2's SPU2 noise generator at the default (fastest) noise clock.
		static constexpr uint8_t noiseAdd[64] =
		{
			1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0,
			1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0,
			0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1,
			0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1
		};
		static const std::vector<int16_t> noiseCycle = []
		{
			std::vector<int16_t> cycle(32767);
			uint16_t noise = 1;
			for (int16_t& sample : cycle)
			{
				noise = static_cast<uint16_t>((noise << 1) |
					noiseAdd[(noise >> 10) & 63]);
				sample = static_cast<int16_t>(noise);
			}
			return cycle;
		}();
		// SPU2 cores expose one continuously-running noise source to every noise
		// voice. Rotate the common cycle from a shared 48 kHz clock so newly keyed
		// voices no longer restart from the same seed and create a coherent spike.
		const auto noiseNow = std::chrono::steady_clock::now().time_since_epoch();
		const uint64_t noiseSamples = static_cast<uint64_t>(
			std::chrono::duration<double>(noiseNow).count() * 48000.0);
		const std::size_t noisePhase = static_cast<std::size_t>(noiseSamples % noiseCycle.size());
		decoded.pcm.resize(noiseCycle.size());
		for (std::size_t sample = 0; sample < noiseCycle.size(); ++sample)
			decoded.pcm[sample] = noiseCycle[(noisePhase + sample) % noiseCycle.size()];
		decoded.loop = true;
		decoded.loopBegin = 0;
		decoded.loopLength = static_cast<uint32_t>(decoded.pcm.size());
	}
	else if (!DecodeEffectAdpcm(bank, tone.sampleOffset, tone.sampleEndOffset, decoded))
	{
		return false;
	}
	EFFECTVOICE active;
	active.pcm = std::move(decoded.pcm);
	active.loop = decoded.loop;
	active.noise = (tone.flags & 8) != 0;
	active.priority = hardwarePriority;
	active.allocationSerial = ++s_audioVoiceSerial;
	WAVEFORMATEX format{};
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 1;
	format.nSamplesPerSec = 48000;
	format.wBitsPerSample = 16;
	format.nBlockAlign = 2;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	// Retail mixes gameplay effects on SPU2 core 0, then feeds that core's
	// complete output through both the dry and external-wet gates on core 1.
	// Parallel sends reproduce that bus routing without reverberating music,
	// dialogue, or FMV voices which use their own source paths.
	EnsureEffectReverb();
	XAUDIO2_SEND_DESCRIPTOR sends[2]{};
	UINT32 sendCount = 0;
	if (s_effectDryVoice)
	{
		sends[sendCount++].pOutputVoice = s_effectDryVoice;
		if (IXAudio2Voice* wet = ActiveEffectWetVoice())
			sends[sendCount++].pOutputVoice = wet;
	}
	else
	{
		// Never combine the device-native mastering voice with 48 kHz wet
		// buses: every destination of one XAudio2 source must share a rate.
		sends[sendCount++].pOutputVoice = s_musicMasterVoice;
	}
	XAUDIO2_VOICE_SENDS sendList{};
	sendList.SendCount = sendCount;
	sendList.pSends = sends;
	const HRESULT createResult = s_xaudio2->CreateSourceVoice(&active.voice, &format, 0,
		kSpuMaximumFrequencyRatio, nullptr, &sendList, nullptr);
	if (FAILED(createResult) ||
		active.voice == nullptr)
	{
		ReportXAudioFailureOnce("Create gameplay-effect source voice",
			FAILED(createResult) ? createResult : E_POINTER);
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
	SetVoiceVolumeChecked(active.voice, EffectVoiceVolume(active, applicationVolume),
		"Set gameplay-effect initial volume");
	SetEffectVoicePan(active.voice, active.authoredPan + EffectSpatialPanDegrees(amb.pan));

	const int centerNote = tone.centerNote >= 0 ? tone.centerNote : -tone.centerNote;
	// 989SND stores centerFine as an unsigned 0..127 fraction above the
	// encoded center note.  Treating it as signed and subtracting it shifted
	// most fine-tuned gameplay effects below their authored PS2 pitch.
	const double semitones = 60.0 - centerNote + tone.centerFine / 128.0 +
		(static_cast<double>(pitchMod) * 1524.0 / 128.0);
	// SPU2 pitch 0x1000 advances one source sample per 48 kHz output sample.
	// The decoded effect voice is also declared as 48 kHz, so applying the
	// PS1-era 44.1/48 kHz correction here lowered ordinary PS2 effects by about
	// 8 percent.  Tone center-note/fine tuning is the only base adjustment.
	active.baseFrequencyRatio = active.noise ? 1.0f :
		static_cast<float>(std::pow(2.0, semitones / 12.0));
	if (!std::isfinite(active.baseFrequencyRatio))
		active.baseFrequencyRatio = 1.0f;
	active.baseFrequencyRatio = std::clamp(active.baseFrequencyRatio,
		XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio);
	// 989SND treats an unspecified tone bend range as the standard two
	// semitones.  Several release-bank effects (including the striking owls)
	// execute RAND_PB while leaving these fields at zero; treating zero as a
	// disabled range made those authored bends inaudible.  Preserve every
	// explicitly authored asymmetric range.
	active.pitchBendLow = tone.pitchBendLow != 0 ? tone.pitchBendLow : 2;
	active.pitchBendHigh = tone.pitchBendHigh != 0 ? tone.pitchBendHigh : 2;
	StartEffectEnvelope(active, tone.adsr1, tone.adsr2);
	SetVoiceVolumeChecked(active.voice, EffectVoiceVolume(active, applicationVolume),
		"Set gameplay-effect envelope volume");
	const HRESULT pitchResult = SetVoiceFrequencyRatioChecked(active.voice,
		active.baseFrequencyRatio, "Set gameplay-effect pitch");
	const HRESULT submitResult = active.voice->SubmitSourceBuffer(&buffer);
	const HRESULT startResult = SUCCEEDED(submitResult) ? active.voice->Start() : E_FAIL;
	const int16_t sample0 = active.pcm.size() > 0 ? active.pcm[0] : 0;
	const int16_t sample1 = active.pcm.size() > 1 ? active.pcm[1] : 0;
	const int16_t sample2 = active.pcm.size() > 2 ? active.pcm[2] : 0;
	const int16_t sample3 = active.pcm.size() > 3 ? active.pcm[3] : 0;
	if (FAILED(pitchResult) || FAILED(submitResult) || FAILED(startResult))
	{
		ReportXAudioFailureOnce("Submit gameplay-effect buffer", submitResult);
		ReportXAudioFailureOnce("Start gameplay-effect voice", startResult);
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
		if (!active.voice || active.noise)
			continue;
		// 989SND uses the conventional signed bend axis. The SPU2 trace for the
		// owl strike starts at one octave below its base pitch and rises to base:
		// a -32768 bend through the tone's 12-semitone low range, ending at zero.
		const double bendSemitones = amb.currentPitchBend >= 0
			? active.pitchBendHigh * static_cast<double>(amb.currentPitchBend) / 32767.0
			: active.pitchBendLow * static_cast<double>(amb.currentPitchBend) / 32768.0;
		const double pitchModSemitones = static_cast<double>(amb.lfoPitchMod) / 128.0;
		// snd_GetDopplerPitchMod returns the same 128-steps-per-semitone pitch
		// modifier used by the rest of 989SND's effect pitch path.
		const double dopplerSemitones = static_cast<double>(amb.dnDoppler) / 128.0;
		const double rawRatio = static_cast<double>(active.baseFrequencyRatio) *
			std::pow(2.0,
				(bendSemitones + pitchModSemitones + dopplerSemitones) / 12.0);
		if (!std::isfinite(rawRatio) || rawRatio < XAUDIO2_MIN_FREQ_RATIO ||
			rawRatio > kSpuMaximumFrequencyRatio)
		{
			static std::mutex extremePitchMutex;
			static std::unordered_set<int> reportedSfx;
			const int sfxid = static_cast<int>(amb.sfxid);
			std::lock_guard<std::mutex> reportLock(extremePitchMutex);
			if (reportedSfx.emplace(sfxid).second)
			{
				ReportRuntimeError("Audio",
					"SFXID %d requested extreme pitch %.4f (base %.4f bend %.3f LFO %.3f doppler %.3f); clamping",
					sfxid, rawRatio, active.baseFrequencyRatio, bendSemitones,
					pitchModSemitones, dopplerSemitones);
			}
		}
		const float ratio = std::isfinite(rawRatio)
			? std::clamp(static_cast<float>(rawRatio),
				XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio)
			: 1.0f;
		SetVoiceFrequencyRatioChecked(active.voice, ratio,
			"Update gameplay-effect pitch");
		active.appliedPitchBend = amb.currentPitchBend;
		active.hasAppliedPitchBend = true;
	}
}

int EffectLfoValue(EFFECTLFO& lfo)
{
	constexpr uint64_t kPhasePeriod = 0x08000000ull;
	const uint64_t phase = static_cast<uint32_t>(lfo.nextStep) % kPhasePeriod;
	const int step = static_cast<int>(phase >> 16);
	const uint64_t advance = static_cast<uint64_t>(lfo.stepSize) * 2ull;
	lfo.nextStep = static_cast<int32_t>((phase + advance) % kPhasePeriod);
	int value = 0;
	switch (lfo.type)
	{
	// 989SND's sine LFO begins at its positive peak (cosine phase), not at the
	// zero crossing. With the invert flag this gives the retail -32768-to-zero
	// opening sweep instead of ProjectCane's former zero-to--32768 sweep.
	case 1: value = static_cast<int>(std::cos(step * 3.14159265358979323846 / 1024.0) * 32767.0); break;
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
		case 3:
			amb.lfoPitchMod = static_cast<int>(std::clamp<int64_t>(
				(static_cast<int64_t>(lfo.range) * value) >> 15,
				std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));
			break;
		case 4:
			amb.lfoPitchBend = static_cast<int>(std::clamp<int64_t>(
				(static_cast<int64_t>(lfo.range) * value) >> 15,
				-32768, 32767));
			break;
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
			SetVoiceVolumeChecked(active.voice, EffectVoiceVolume(active,
				(std::max)(0.0f, amb.volAttenuated + lfoVolume) * amb.autoVolumeScale),
				"Update gameplay-effect LFO volume");
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
	child->parentVolumeScale = static_cast<float>(volume) / 127.0f;
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
			RetireSourceVoice(active.voice, active.pcm);
		}
		else
		{
			float applicationVolume =
				amb.volAttenuated + static_cast<float>(amb.lfoVolume) / 127.0f;
			if (applicationVolume < 0.0f)
				applicationVolume = 0.0f;
			applicationVolume *= amb.autoVolumeScale;
			const float outputVolume = EffectVoiceVolume(active, applicationVolume);
			SetVoiceVolumeChecked(active.voice, outputVolume,
				"Update gameplay-effect envelope volume");
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
	{
		PlayEffectTone(amb, *bank, sound, grain.tone, amb.volAttenuated, amb.frq);
		const int authoredPitchBend = amb.currentPitchBend;
		ApplyEffectPitchBend(amb,
			std::clamp(authoredPitchBend + amb.lfoPitchBend, -32768, 32767));
		amb.currentPitchBend = authoredPitchBend;
		break;
	}
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
		const int64_t randomBend = static_cast<int64_t>(amount) * randomSigned / 100;
		ApplyEffectPitchBend(amb, static_cast<int>(std::clamp<int64_t>(
			randomBend, -32768, 32767)));
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
			RetireSourceVoice(active.voice, active.pcm);
			active.envelopePhase = EffectEnvelope_Stopped;
		}
		break;
	default:
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
	// A caller-provided handle is exclusive ownership of one logical sound.
	// Replacing it without first stopping the old AMB loses the only handle to
	// a looping voice and lets that voice stack indefinitely beneath the new one.
	if (ppamb != nullptr && *ppamb != nullptr)
		StopSound(*ppamb, 0);
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
			{
				return;
			}
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
			const float distance = SDistSoundEar(*earPosition, position);
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
	amb->iSerial = s_effectAmbSerial++;
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
		// Retail staggers the initial playback from zero through the minimum
		// repeat interval. Subsequent plays use the full min/max range.
		const float initialDelayMax = (std::max)(0.0f, amb->lmRepeat.gMin);
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
			RetireSourceVoice(active.voice, active.pcm);
		}
		pamb->voices.clear();
		pamb->voice = nullptr;
	}
	else if (pamb->voice != nullptr)
	{
		RetireSourceVoice(pamb->voice, pamb->pcm);
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

	const std::vector<AMB*> children = pamb->children;
	for (AMB* child : children)
		if (child) StopSound(child, msRampdown);
	if (pamb->ppamb != nullptr)
	{
		*pamb->ppamb = nullptr;
		pamb->ppamb = nullptr;
	}
	pamb->fStopped = 1;
	pamb->handlerDone = true;
	if (msRampdown <= 0 || pamb->voices.empty())
	{
		RemoveAmb(pamb);
		return;
	}

	// Retail calls snd_AutoVol(handle, -4, ms * 0.24, 2). 989SND treats -4 as
	// fade to silence and stop, rather than as an authored SPU release envelope.
	pamb->autoVolumeTicks = (std::max)(1,
		static_cast<int>(static_cast<float>(msRampdown) * 0.24000001f));
	pamb->autoVolumeTicksRemaining = pamb->autoVolumeTicks;
	pamb->autoVolumeScale = 1.0f;
}

void PausePamb(AMB* pamb)
{
	if (pamb == nullptr || pamb->paused)
		return;
	bool paused = true;

	if (!pamb->voices.empty())
	{
		for (EFFECTVOICE& active : pamb->voices)
			if (active.voice != nullptr)
			{
				const HRESULT result = active.voice->Stop();
				ReportXAudioFailureOnce("Pause ambient effect voice", result);
				paused = paused && SUCCEEDED(result);
			}
	}
	else if (pamb->voice != nullptr)
	{
		const HRESULT result = pamb->voice->Stop();
		ReportXAudioFailureOnce("Pause ambient voice", result);
		paused = SUCCEEDED(result);
	}
	pamb->paused = paused;
}

static void StopAllMusicVoicesForSegment(uint32_t segment)
{
	for (MusicVoice& voice : s_music989.voices)
	{
		if (!voice.voice || voice.segment != segment)
			continue;
		RetireMusicVoice(voice);
		voice.envelope.phase = MusicEnvelopePhase::Stopped;
	}
}

void SetEffectReverbTypeDepth(REVERBK kind, int depth)
{
	if (!s_xaudio2)
	{
		s_effectReverbKind = kind;
		s_effectReverbDepth = depth;
		return;
	}

	// Retail initializes the selected SPU2 work area without reconnecting any
	// voices. Silence the returns first, clear Studio C's history in place, then
	// apply the new type/depth while the permanent wet-input bus stays connected.
	s_effectReverbKind = REVERBK_None;
	s_effectReverbDepth = 0;
	ApplyCurrentEffectReverb();
	EnsureEffectReverb();
	if (s_effectStudioCProcessor)
		s_effectStudioCProcessor->Reset();

	s_effectReverbKind = kind;
	s_effectReverbDepth = depth;
	EnsureEffectReverb();
	ApplyCurrentEffectReverb();
}

static void ResetMusicSegmentForLoop(MusicSegment& segment)
{
	// A repeated MIDI handler starts as a fresh handler. Its looping sample
	// voices and channel state must not accumulate across song passes.
	StopAllMusicVoicesForSegment(segment.id);
	segment.cursor = segment.begin;
	segment.tempo = segment.initialTempo;
	segment.status = 0;
	segment.ticksLeft = 0;
	segment.tickError = 0.0;
	segment.needDelta = true;
	segment.programs.fill(0);
	segment.volumes.fill(127);
	segment.pans.fill(0);
	segment.pitchBends.fill(0);
	segment.muted.fill(false);
}

void ContinuePamb(AMB* pamb)
{
	if (pamb == nullptr || !pamb->paused)
		return;
	bool resumed = true;

	if (!pamb->voices.empty())
	{
		for (EFFECTVOICE& active : pamb->voices)
			if (active.voice != nullptr)
			{
				const HRESULT result = active.voice->Start();
				ReportXAudioFailureOnce("Resume ambient effect voice", result);
				resumed = resumed && SUCCEEDED(result);
			}
	}
	else if (pamb->voice != nullptr)
	{
		const HRESULT result = pamb->voice->Start();
		ReportXAudioFailureOnce("Resume ambient voice", result);
		resumed = SUCCEEDED(result);
	}
	pamb->paused = !resumed;
}

void SetAMRegister(int ireg, byte bVal)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	if (ireg < 0 || ireg >= static_cast<int>(s_amRegisters.size()))
		return;
	const uint8_t value = static_cast<uint8_t>(bVal) & 0x7f;
	if (s_amRegisters[ireg] == value)
		return;
	const int previous = s_amRegisters[ireg];
	s_amRegisters[ireg] = value;
	s_music989.registers[ireg] = value;
	TraceMusicEvent("set_register", ireg, (previous << 8) | value);
	ReapplyMusicAmeGroups(ireg + 1);
}

static void PrimeMusicSequencerStartup()
{
	// 989SND executes a cue's zero-time MIDI/AME setup as part of starting the
	// sound handle.  Deferring it to the worker lets gameplay write an AM
	// register first, only for the cue initializer to overwrite that request.
	// Prime each segment in the startup chain once, activating any segment that
	// an immediate AME command queues, before returning to the game thread.
	std::vector<uint32_t> primed;
	for (int pass = 0; pass < 64; ++pass)
	{
		bool stepped = false;
		for (MusicSegment& segment : s_music989.activeSegments)
		{
			if (segment.complete ||
				std::find(primed.begin(), primed.end(), segment.id) != primed.end())
				continue;

			primed.push_back(segment.id);
			StepMusicSegment(segment);
			stepped = true;
		}

		ActivateQueuedMusicSegments();
		s_music989.activeSegments.erase(
			std::remove_if(s_music989.activeSegments.begin(), s_music989.activeSegments.end(),
				[](const MusicSegment& segment) { return segment.complete; }),
			s_music989.activeSegments.end());

		if (!stepped && s_music989.pendingStarts.empty())
			break;
	}
}

void RefreshMidiRegister(int ireg)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	if (ireg < 0 || ireg >= static_cast<int>(s_amRegisters.size()))
		return;

	// The original queries the live 989SND handle, then refreshes its cached
	// register value with the result. ProjectCane's live value is maintained
	// directly by the in-process sequencer.
	s_amRegisters[ireg] = s_music989.registers[ireg];
}

void SetMusicSequencerGameplayClock(bool enabled)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	s_musicUsesGameplayClock.store(enabled, std::memory_order_release);
	s_musicGameplayClockFrame = -1;
	// Discard wall time accumulated while changing clock domains. Otherwise the
	// worker can batch the switch interval into the next sequencer update.
	s_music989.lastUpdate = std::chrono::steady_clock::now();
}

void CalculateVolPan(float sDist, glm::vec3* pposSound, float* pvol, float* ppan, float uvol, float sStart, float sFull)
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

static void SetPambVolTree(AMB* pamb, float uVolAtSource)
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
			const float distance = SDistSoundEar(*earPosition, pamb->pos);
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
		SetVoiceVolumeChecked(s_vagStream.voice, (std::max)(0.0f, attenuatedVolume) *
			kVagDriverGain * (s_fFmvAudioMode ? 1.0f : s_channelVolumes[2]) *
			s_userDialogueVolume, "Update positioned dialogue volume");
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
				ReportXAudioFailureOnce("Set positioned dialogue output matrix",
					s_vagStream.voice->SetOutputMatrix(s_musicMasterVoice, 1,
						destinationDetails.InputChannels, matrix.data()));
			}
		}
		return;
	}
	for (EFFECTVOICE& active : pamb->voices)
		if (active.voice != nullptr)
		{
			SetVoiceVolumeChecked(active.voice, EffectVoiceVolume(active,
				(std::max)(0.0f, attenuatedVolume + static_cast<float>(pamb->lfoVolume) / 127.0f)) *
				pamb->autoVolumeScale, "Update positioned gameplay-effect volume");
			SetEffectVoicePan(active.voice,
				active.authoredPan + pamb->lfoPan + EffectSpatialPanDegrees(pamb->pan));
		}
	const std::vector<AMB*> children = pamb->children;
	for (AMB* child : children)
		if (child != nullptr)
			SetPambVolTree(child, uVolAtSource * child->parentVolumeScale);
}

void SetPambVol(AMB* pamb, float uVolAtSource)
{
	SetPambVolTree(pamb, uVolAtSource);
}

static void SetPambFrqTree(AMB* pamb, float frq, int depth)
{
	if (pamb == nullptr || !std::isfinite(frq))
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
		if (active.noise)
			continue;
		active.baseFrequencyRatio = std::clamp(active.baseFrequencyRatio * pitchRatio,
			XAUDIO2_MIN_FREQ_RATIO, kSpuMaximumFrequencyRatio);
	}
	pamb->frq = frq;
	if (!pamb->voices.empty())
		pamb->frequencyRatioBase = pamb->voices.front().baseFrequencyRatio;
	SetDoppler(pamb);
	// START_CHILD_SOUND handlers are part of the parent's 989SND voice tree.
	// Runtime pitch controls target the root handle, so every child voice must
	// track the same application pitch change.  Without this, compound effects
	// (including the owl strikes and Raleigh's tongue) remain at their starting
	// pitch because the audible tone lives on a child AMB.
	const std::vector<AMB*> children = pamb->children;
	for (AMB* child : children)
		if (child != nullptr)
			SetPambFrqTree(child, frq, depth + 1);
}

void SetPambFrq(AMB* pamb, float frq)
{
	SetPambFrqTree(pamb, frq, 0);
}

void SetDoppler(AMB* pamb)
{
	if (pamb == nullptr)
		return;

	pamb->dnDoppler = 0;

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
				// Retail passes the truncated approaching-speed value to
				// snd_GetDopplerPitchMod: (approaching_mph * 1524) / 741.
				const int approachingMph = static_cast<int>(
					radialVelocity * pamb->rDoppler * 0.0225f);
				pamb->dnDoppler = (approachingMph * 1524) / 741;
			}
		}
	}

	// Doppler is one component of the final 989SND pitch. Reapply the complete
	// handler bend so this spatial refresh cannot reset an active pitch-bend LFO
	// back to the authored/base value between sound ticks.
	const int authoredPitchBend = pamb->currentPitchBend;
	ApplyEffectPitchBend(*pamb, std::clamp(authoredPitchBend + pamb->lfoPitchBend, -32768, 32767));
	pamb->currentPitchBend = authoredPitchBend;
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
	FinishPendingVagLoad();
	{
		std::lock_guard<std::mutex> lock(s_vagLoadMutex);
		if (s_vagLoad.requested || s_vagLoad.loading || s_vagLoad.ready)
			return true;
	}
	if (!s_vagStream.loaded)
		return false;
	// Retail reports true when the VAG was already paused; callers use this to
	// decide whether queued sound commands need to be flushed before continuing.
	if (s_vagStream.paused)
		return true;

	if (!s_vagStream.started)
	{
		const HRESULT result = s_vagStream.voice ? s_vagStream.voice->Start() : E_POINTER;
		if (FAILED(result))
		{
			ReportXAudioFailureOnce("Start VAG/dialog voice", result);
			CloseVagStream();
			return false;
		}
		s_vagStream.started = true;
		s_vagStartWhenReady = false;
		s_vagStream.paused = false;
		ApplyDialogMusicDuck(0.2f);
	}

	return false;
}

int FVagPlaying()
{
	FinishPendingVagLoad();
	{
		std::lock_guard<std::mutex> lock(s_vagLoadMutex);
		if (s_vagLoad.requested || s_vagLoad.loading || s_vagLoad.ready)
			return 1;
	}
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

double VagPlaybackSeconds()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	if (!s_vagStream.loaded || !s_vagStream.started || s_vagStream.voice == nullptr ||
		s_vagStream.format.samplesPerSec == 0)
		return -1.0;

	XAUDIO2_VOICE_STATE state{};
	s_vagStream.voice->GetState(&state);
	if (state.BuffersQueued == 0)
		return -1.0;
	return static_cast<double>(s_vagStream.playBegin + state.SamplesPlayed) /
		static_cast<double>(s_vagStream.format.samplesPerSec);
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

void SetAttractSoundOption(bool fMono)
{
	// GRFGS 0x40 is the retail "mono" option bit: clear selects stereo,
	// set selects mono.  Keep the renderer flag in the opposite sense.
	s_stereoOutputEnabled = !fMono;
}

void SetUserStereoEnabled(bool enabled)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	s_stereoOutputEnabled = enabled;
	for (MusicVoice& active : s_music989.voices)
		if (active.voice)
			SetMusicVoicePan(active.voice, active.pan);
	if (g_psw)
		for (const std::unique_ptr<AMB>& slot : g_psw->ambOwners)
			if (slot) SetPambVol(slot.get(), slot->uVolAtSource);
	if (s_vagStream.voice)
	{
		if (s_pambVagAmbient)
			SetPambVol(s_pambVagAmbient, s_pambVagAmbient->uVolAtSource);
		else
			SetVagStreamOutputMatrix(s_vagStream.voice, s_vagStream.format.channels);
	}
}

bool FUserStereoEnabled() { return s_stereoOutputEnabled; }

void SetUserMusicVolume(float volume)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	if (!std::isfinite(volume))
		volume = 0.0f;
	s_userMusicVolume = std::clamp(volume, 0.0f, 1.0f);
	RecalculateChannelVolume(kMusicMixerChannel);
}

float GetUserMusicVolume() { return s_userMusicVolume; }

void SetUserSfxVolume(float volume)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	if (!std::isfinite(volume))
		volume = 0.0f;
	s_userSfxVolume = std::clamp(volume, 0.0f, 1.0f);
	if (g_psw)
		for (const std::unique_ptr<AMB>& slot : g_psw->ambOwners)
			if (slot) SetPambVol(slot.get(), slot->uVolAtSource);
}

float GetUserSfxVolume() { return s_userSfxVolume; }

void SetUserDialogueVolume(float volume)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	if (!std::isfinite(volume))
		volume = 0.0f;
	s_userDialogueVolume = std::clamp(volume, 0.0f, 1.0f);
	RecalculateChannelVolume(2);
}

float GetUserDialogueVolume() { return s_userDialogueVolume; }

void StopVag()
{
	s_commentaryStartPending = false;
	s_vagStartWhenReady = false;
	{
		std::lock_guard<std::mutex> lock(s_vagLoadMutex);
		++s_vagLoadGeneration;
		s_vagLoad.requested = false;
		s_vagLoad.loading = false;
		s_vagLoad.ready = false;
		s_vagLoad.readablePath.clear();
		s_vagLoad.path.clear();
		s_vagLoad.pcm.clear();
	}
	if (!s_vagStream.loaded)
		return;

	CloseVagStream();
}

void StopVagAndWaitForFade()
{
	StopVag();

	// FMV playback must finish its short anti-pop fade before gameplay voices
	// and reverb returns are restored. Other retired source voices are unrelated
	// to this handoff; an owned byte buffer identifies the retiring VAG voice.
	const auto deadline = std::chrono::steady_clock::now() +
		kVoiceRetirementFade + std::chrono::milliseconds(5);
	for (;;)
	{
		UpdateRetiredSourceVoices();
		bool vagStillRetiring = false;
		{
			std::lock_guard<std::mutex> lock(s_retiredVoiceMutex);
			vagStillRetiring = std::any_of(s_retiredVoices.begin(), s_retiredVoices.end(),
				[](const RetiredSourceVoice& retired)
				{
					return retired.voice != nullptr && !retired.pcm8.empty();
				});
		}
		if (!vagStillRetiring || std::chrono::steady_clock::now() >= deadline)
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	UpdateRetiredSourceVoices();
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
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	if (s_vagStream.loaded && s_vagStream.started && !s_vagStream.paused)
	{
		if (s_vagStream.voice)
		{
			ReportXAudioFailureOnce("Mute VAG/dialog voice for pause",
				s_vagStream.voice->SetVolume(0.0f));
			const HRESULT result = s_vagStream.voice->Stop();
			if (SUCCEEDED(result))
				s_vagStream.paused = true;
			else
				ReportXAudioFailureOnce("Pause VAG/dialog voice", result);
		}
	}
}

void ContinueVag()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	{
		std::lock_guard<std::mutex> loadLock(s_vagLoadMutex);
		if (s_vagLoad.requested || s_vagLoad.loading || s_vagLoad.ready)
		{
			s_vagStartWhenReady = true;
			return;
		}
	}
	if (s_vagStream.loaded && !s_vagStream.started)
	{
		FPauseForVag();
	}
	else if (s_vagStream.loaded && s_vagStream.paused)
	{
		if (s_vagStream.voice)
		{
			if (s_pambVagAmbient)
				SetPambVol(s_pambVagAmbient, s_pambVagAmbient->uVolAtSource);
			else
				SetVoiceVolumeChecked(s_vagStream.voice, kVagDriverGain *
					(s_fFmvAudioMode ? 1.0f : s_channelVolumes[2]) * s_userDialogueVolume,
					"Restore VAG/dialog volume");
			const HRESULT result = s_vagStream.voice->Start();
			if (SUCCEEDED(result))
				s_vagStream.paused = false;
			else
				ReportXAudioFailureOnce("Resume VAG/dialog voice", result);
		}
	}
}

void BeginDialogAudioDuck()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	s_dialogAudioDuckHeld = true;
	ApplyDialogMusicDuck(0.2f);
}

void EndDialogAudioDuck()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	s_dialogAudioDuckHeld = false;
	ApplyDialogMusicDuck(1.0f);
}

void PauseAmbientSounds()
{
	if (s_fPauseAmbients != 0)
		return;

	s_fPauseAmbients = 1;
	if (g_psw == nullptr)
		return;

	for (std::unique_ptr<AMB>& slot : g_psw->ambOwners)
	{
		if (!slot)
			continue;
		for (EFFECTVOICE& active : slot->voices)
			if (active.voice)
				ReportXAudioFailureOnce("Pause gameplay effects", active.voice->Stop());
	}
}

void ContinueAmbientSounds()
{
	if (s_fPauseAmbients == 0)
		return;

	if (g_psw != nullptr)
	{
		for (std::unique_ptr<AMB>& slot : g_psw->ambOwners)
		{
			if (!slot)
				continue;
			for (EFFECTVOICE& active : slot->voices)
				if (active.voice)
					ReportXAudioFailureOnce("Resume gameplay effects", active.voice->Start());
		}
	}

	// UpdateSounds uses wall time to emulate the independent 240 Hz sound
	// clock. Reset its anchor so a pause does not fast-forward SFX handlers.
	s_effectLastUpdate = std::chrono::steady_clock::now();
	s_fPauseAmbients = 0;
}

void PauseMusicSequencer()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	TraceMusicEvent("pause_sequencer_request");
	if (s_musicStream.state == MusicState::Playing && !s_musicStream.paused)
	{
		// Mark the sequencer paused before touching its voices so no worker tick
		// can create or refresh a voice during the hard-pause transition.
		s_musicStream.paused = true;
		for (MusicVoice& active : s_music989.voices)
			if (active.voice)
			{
				SetVoiceVolumeChecked(active.voice, 0.0f, "Mute music for pause");
				ReportXAudioFailureOnce("Pause music voice", active.voice->Stop());
			}
	}
}

void ContinueMusicSequencer(bool continueAmbientSounds)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	TraceMusicEvent("continue_sequencer_request", continueAmbientSounds ? 1 : 0);
	if (continueAmbientSounds && s_fPauseAmbients != 0)
		ContinueAmbientSounds();
	if (s_musicStream.state == MusicState::Playing && s_musicStream.paused)
	{
		bool resumed = true;
		const int musicChannel = kMusicMixerChannel;
		for (MusicVoice& active : s_music989.voices)
			if (active.voice)
			{
				const float resumedVolume = active.baseVolume *
					(active.envelope.level / 32767.0f) * s_channelVolumes[musicChannel] *
					s_musicVolume * s_dialogMusicDuck * s_userMusicVolume;
				SetVoiceVolumeChecked(active.voice, resumedVolume,
					"Restore music volume after pause");
				const HRESULT startResult = active.voice->Start();
				ReportXAudioFailureOnce("Resume music voice", startResult);
				resumed = resumed && SUCCEEDED(startResult);
			}
		if (resumed)
		{
			s_music989.lastUpdate = std::chrono::steady_clock::now();
			s_musicStream.paused = false;
		}
	}
}

#ifndef __SWITCH__
static void VagLoadWorkerMain()
{
	for (;;)
	{
		PendingVagLoad request{};
		{
			std::unique_lock<std::mutex> lock(s_vagLoadMutex);
			s_vagLoadCondition.wait(lock, []
			{
				return !s_vagLoadWorkerRunning || s_vagLoad.requested;
			});
			if (!s_vagLoadWorkerRunning)
				return;
			request.generation = s_vagLoad.generation;
			request.readablePath = s_vagLoad.readablePath;
			s_vagLoad.requested = false;
		}

		if (!request.readablePath.empty())
		{
			request.path = request.readablePath;
			request.path.replace_extension(L".wav");

			// Named VAG streams are normally supplied as a bare extracted filename.
			// Resolve those through the same Worlds search used by descriptor-backed
			// dialogue instead of assuming the process working directory owns them.
			std::error_code pathError;
			// Explicit paths (FMV audio, prompts, ambient streams, and other
			// package media) are already fully resolved.  Only bare/missing VAG
			// names should enter the per-level dialogue resolver; forcing every
			// path through it for multilingual content discards valid files such
			// as Cutscenes/Cutscene_16.wav.
			if (!std::filesystem::is_regular_file(request.path, pathError))
			{
				const std::string namedVag = request.readablePath.filename().string();
				if (std::filesystem::path found = ResolveNamedVag(namedVag.c_str());
					!found.empty())
				{
					request.path = std::move(found);
				}
				else if (FLocalizedContent())
				{
					request.path.clear();
				}
			}
		}
		if (!request.path.empty() &&
			!LoadPcmWave(request.path, request.format, request.pcm))
		{
			ReportRuntimeError("Audio", "Could not load PCM WAV '%s'",
				request.path.string().c_str());
		}

		std::lock_guard<std::mutex> lock(s_vagLoadMutex);
		if (s_vagLoadWorkerRunning && request.generation == s_vagLoadGeneration)
		{
			s_vagLoad.path = std::move(request.path);
			s_vagLoad.format = request.format;
			s_vagLoad.pcm = std::move(request.pcm);
			s_vagLoad.loading = false;
			s_vagLoad.ready = true;
		}
	}
}

#endif

static bool SetVagStreamOutputMatrix(IXAudio2SourceVoice* voice, UINT32 sourceChannels)
{
	if (!voice || !s_musicMasterVoice || sourceChannels == 0)
		return false;
	XAUDIO2_VOICE_DETAILS masterDetails{};
	s_musicMasterVoice->GetVoiceDetails(&masterDetails);
	const UINT32 destinationChannels = masterDetails.InputChannels;
	if (destinationChannels == 0)
		return false;
	std::vector<float> matrix(
		static_cast<std::size_t>(sourceChannels) * destinationChannels, 0.0f);
	if (sourceChannels == 1)
	{
		matrix[0] = 1.0f;
		if (destinationChannels >= 2)
			matrix[1] = 1.0f;
	}
	else if (destinationChannels == 1)
	{
		const float gain = 1.0f / static_cast<float>(sourceChannels);
		for (UINT32 source = 0; source < sourceChannels; ++source)
			matrix[source] = gain;
	}
	else if (!s_stereoOutputEnabled)
	{
		// Fold every source channel into a balanced front-center image while
		// retaining the source-major XAudio2 matrix layout.
		const float gain = 1.0f / static_cast<float>(sourceChannels);
		for (UINT32 source = 0; source < sourceChannels; ++source)
		{
			matrix[static_cast<std::size_t>(source) * destinationChannels] = gain;
			matrix[static_cast<std::size_t>(source) * destinationChannels + 1] = gain;
		}
	}
	else
	{
		// The mastering graph is explicitly stereo. Preserve the authored front
		// left/right pair and ignore unsupported auxiliary channels instead of
		// relying on the endpoint's vendor-specific native channel order.
		matrix[0] = 1.0f;
		matrix[static_cast<std::size_t>(1) * destinationChannels + 1] = 1.0f;
	}
	const HRESULT result = voice->SetOutputMatrix(
		s_musicMasterVoice, sourceChannels, destinationChannels, matrix.data());
	ReportXAudioFailureOnce("Set VAG/dialog output matrix", result);
	return SUCCEEDED(result);
}

static void BalanceFmvStereoPcm(VagStreamState& stream)
{
	if (!s_fFmvAudioMode || stream.format.channels != 2 ||
		stream.format.bitsPerSample != 16 || stream.format.blockAlign != 4 ||
		stream.pcm.size() < 4)
		return;

	double sumSquares[2]{};
	const std::size_t frameCount = stream.pcm.size() / 4;
	for (std::size_t frame = 0; frame < frameCount; ++frame)
	{
		int16_t samples[2]{};
		std::memcpy(samples, stream.pcm.data() + frame * 4, sizeof(samples));
		for (int channel = 0; channel < 2; ++channel)
		{
			const double sample = static_cast<double>(samples[channel]);
			sumSquares[channel] += sample * sample;
		}
	}
	if (sumSquares[0] <= 0.0 || sumSquares[1] <= 0.0)
		return;

	const double imbalanceDb = 10.0 * std::log10(sumSquares[0] / sumSquares[1]);
	// Small whole-file differences are normal stereo authorship. Correct only
	// a persistent imbalance large enough to pull centered dialogue off-screen.
	if (std::abs(imbalanceDb) < 1.5)
		return;

	const int loudChannel = imbalanceDb > 0.0 ? 0 : 1;
	const double quieterEnergy = sumSquares[1 - loudChannel];
	const double gain = std::sqrt(quieterEnergy / sumSquares[loudChannel]);
	for (std::size_t frame = 0; frame < frameCount; ++frame)
	{
		int16_t sample = 0;
		const std::size_t offset = frame * 4 + loudChannel * sizeof(sample);
		std::memcpy(&sample, stream.pcm.data() + offset, sizeof(sample));
		const int adjusted = static_cast<int>(std::lround(static_cast<double>(sample) * gain));
		sample = static_cast<int16_t>(std::clamp(adjusted, -32768, 32767));
		std::memcpy(stream.pcm.data() + offset, &sample, sizeof(sample));
	}
}

static bool CreateVagStreamVoice(VagStreamState& stream, uint64_t playBegin)
{
	if (!EnsureMusicEngine() || stream.pcm.empty() || stream.format.blockAlign == 0)
	{
		return false;
	}
	WAVEFORMATEX format{};
	format.wFormatTag = stream.format.formatTag;
	format.nChannels = stream.format.channels;
	format.nSamplesPerSec = stream.format.samplesPerSec;
	format.nAvgBytesPerSec = stream.format.avgBytesPerSec;
	format.nBlockAlign = stream.format.blockAlign;
	format.wBitsPerSample = stream.format.bitsPerSample;
	const HRESULT createResult = s_xaudio2->CreateSourceVoice(
		&stream.voice, &format, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &s_vagVoiceCallback);
	if (FAILED(createResult) || !stream.voice)
	{
		ReportRuntimeError("Audio",
			"CreateSourceVoice failed for '%s' (HRESULT 0x%08X, %u Hz, %u channels, %u-bit)",
			stream.path.string().c_str(), static_cast<unsigned>(createResult),
			stream.format.samplesPerSec, stream.format.channels,
			stream.format.bitsPerSample);
		stream.voice = nullptr;
		return false;
	}
	if (!SetVagStreamOutputMatrix(stream.voice, stream.format.channels))
	{
		stream.voice->DestroyVoice();
		stream.voice = nullptr;
		return false;
	}
	const uint64_t totalSamples = stream.pcm.size() / stream.format.blockAlign;
	const UINT32 firstSample = static_cast<UINT32>((std::min)(playBegin,
		(std::min)(totalSamples - 1, static_cast<uint64_t>(UINT32_MAX))));
	XAUDIO2_BUFFER buffer{};
	buffer.AudioBytes = static_cast<UINT32>(stream.pcm.size());
	buffer.pAudioData = stream.pcm.data();
	buffer.PlayBegin = firstSample;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	const HRESULT submitResult = stream.voice->SubmitSourceBuffer(&buffer);
	if (FAILED(submitResult))
	{
		ReportRuntimeError("Audio",
			"SubmitSourceBuffer failed for '%s' (HRESULT 0x%08X, %u bytes)",
			stream.path.string().c_str(), static_cast<unsigned>(submitResult), buffer.AudioBytes);
		stream.voice->DestroyVoice();
		stream.voice = nullptr;
		return false;
	}
	stream.loaded = true;
	stream.playBegin = firstSample;
	return true;
}

static bool FinishPendingVagLoad()
{
#ifdef __SWITCH__
    if (s_vagLoad.loading)
        if (auto wave = CaneAudio::ConsumeWave(s_nativeVagKey))
        {
            s_vagLoad.path = std::move(wave->path);
            s_vagLoad.format = wave->format;
            s_vagLoad.pcm = std::move(wave->pcm);
            s_vagLoad.loading = false;
            s_vagLoad.ready = true;
        }
#endif
	PendingVagLoad loaded{};
	{
		std::lock_guard<std::mutex> lock(s_vagLoadMutex);
		if (!s_vagLoad.ready)
			return false;
		loaded.path = std::move(s_vagLoad.path);
		loaded.format = s_vagLoad.format;
		loaded.pcm = std::move(s_vagLoad.pcm);
		s_vagLoad.ready = false;
	}
	if (loaded.path.empty() || loaded.pcm.empty() || !EnsureMusicEngine())
	{
		return true;
	}

	VagStreamState replacement{};
	replacement.format = loaded.format;
	replacement.pcm = std::move(loaded.pcm);
	replacement.path = std::move(loaded.path);
	BalanceFmvStereoPcm(replacement);
	if (!CreateVagStreamVoice(replacement, 0))
		return true;
	s_vagStream = std::move(replacement);
	RecalculateChannelVolume(2);
	return true;
}

void PreloadVag(const char* descriptorOrPath)
{
#ifdef __SWITCH__
    StopVag();
    if (descriptorOrPath) QueueNativeVag(MakeVagRequest(descriptorOrPath));
#else
	StopVag();
	if (!descriptorOrPath)
		return;

	PendingVagLoad request{};
	request.readablePath = std::filesystem::path(descriptorOrPath);
	{
		std::lock_guard<std::mutex> lock(s_vagLoadMutex);
		request.generation = ++s_vagLoadGeneration;
		s_vagLoad.generation = request.generation;
		s_vagLoad.readablePath = std::move(request.readablePath);
		s_vagLoad.path.clear();
		s_vagLoad.pcm.clear();
		s_vagLoad.ready = false;
		s_vagLoad.loading = true;
		s_vagLoad.requested = true;
	}
	s_vagLoadCondition.notify_one();
#endif
}

void PreloadVagFileName(const char* fileName)
{
	PreloadVag(fileName);
}

void PreloadVag(const std::filesystem::path& path)
{
#ifdef __SWITCH__
    StopVag();
    if (!path.empty()) QueueNativeVag(MakeVagRequest(path.string().c_str()));
#else
	StopVag();
	if (path.empty())
		return;

	PendingVagLoad request{};
	request.readablePath = path;
	{
		std::lock_guard<std::mutex> lock(s_vagLoadMutex);
		request.generation = ++s_vagLoadGeneration;
		s_vagLoad.generation = request.generation;
		s_vagLoad.readablePath = std::move(request.readablePath);
		s_vagLoad.path.clear();
		s_vagLoad.pcm.clear();
		s_vagLoad.ready = false;
		s_vagLoad.loading = true;
		s_vagLoad.requested = true;
	}
	s_vagLoadCondition.notify_one();
#endif
}

void PushSwReverb(SW* psw, REVERBK rvrbk, int depth)
{
	if (!psw || psw->irse < 0 || psw->irse >= 4)
		return;

	psw->arse[psw->irse].rvrbk = rvrbk;
	psw->arse[psw->irse].depth = depth;
	++psw->irse;
	SetEffectReverbTypeDepth(rvrbk, depth);
}

void PopSwReverb(SW* psw)
{
	if (!psw || psw->irse <= 1)
		return;

	--psw->irse;
	const RSE& restored = psw->arse[psw->irse - 1];
	SetEffectReverbTypeDepth(restored.rvrbk, restored.depth);
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
	s_commentaryStartPending = true;
}

bool FPlayingCommentaryAudio()
{
	return s_commentaryStartPending || (s_vagStream.loaded && s_vagStream.commentary);
}

void ContinueMusic()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	TraceMusicEvent("continue_music_volume");
	s_musicVolume = 1.0f;
	s_music989.lastUpdate = std::chrono::steady_clock::now();
	for (MusicVoice& active : s_music989.voices)
		if (active.voice) SetVoiceVolumeChecked(active.voice,
			active.baseVolume * (active.envelope.level / 32767.0f) *
			s_channelVolumes[kMusicMixerChannel] * s_dialogMusicDuck *
			s_userMusicVolume, "Continue music volume");
}

void PauseMusic()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	TraceMusicEvent("pause_music_volume");
	s_musicVolume = 0.0f;
	for (MusicVoice& active : s_music989.voices)
		if (active.voice) SetVoiceVolumeChecked(active.voice, 0.0f, "Pause music volume");
}

void UnloadMusic()
{
	TraceMusicEvent("unload_music");
	if (s_musicStream.state != MusicState::None)
	{
	#ifndef __SWITCH__
	std::printf("[MUSIC LOAD] unload musid=0x%X state=%d\n",
			static_cast<unsigned>(s_musicStream.musid),
			static_cast<int>(s_musicStream.state));
#endif
	}
	CloseMusicVoice();
}

void KillMusic()
{
	UnloadMusic();
}

void KillSounds(int msRampdown)
{
#ifdef __SWITCH__
    ClearVagPrefetch();
#endif
	(void)msRampdown;
	// Positional AMB playback is not backed by live PC voices yet. The active
	// streamed VAG/dialog is, so stop and close it during world teardown.
	StopVag();
	if (g_psw != nullptr)
	{
		// The world is being torn down, so AMB back-pointers no longer need to
		// clear their owning fields. Some owners (notably the active world-map
		// controller) can already have been released by shutdown callbacks.
		// Detach every back-pointer before RemoveAmb destroys the pool.
		for (std::unique_ptr<AMB>& slot : g_psw->ambOwners)
		{
			if (slot)
				slot->ppamb = nullptr;
		}

		while (g_psw->dlAmb.pvFirst != nullptr)
			RemoveAmb(static_cast<AMB*>(g_psw->dlAmb.pvFirst));

		// Also release any detached pool entries which are no longer members of
		// dlAmb (DropPamb deliberately does not free them).
		for (std::unique_ptr<AMB>& slot : g_psw->ambOwners)
			slot.reset();
	}
}

bool FMusicSequencerPaused()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	return s_musicStream.paused;
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
	// Disable the wet return before touching any source or processor. SPU2's
	// effect-enable gate makes the return silent immediately while its work-area
	// pointers remain valid; destroying a live XAudio effect first can expose one
	// final buffer of its feedback tail during a world transition.
	s_effectReverbKind = REVERBK_None;
	s_effectReverbDepth = 0;
	ApplyCurrentEffectReverb();
	if (s_effectWetInputVoice)
		ReportXAudioFailureOnce("Mute wet-input bus during world teardown",
			s_effectWetInputVoice->SetVolume(0.0f));
	KillMusic();
	KillSounds(0);
	// KillSounds normally retires voices through a short anti-pop fade. A full
	// world teardown cannot leave those voices connected while their destination
	// wet submixes are destroyed, so finish their destruction synchronously.
	DestroyRetiredSourceVoicesImmediately();
	// All source voices are gone and every wet return is silent, so rebuilding
	// the downstream graph cannot create a routing discontinuity. This clears
	// both Studio C and the otherwise opaque XAudio generic-reverb history.
	ResetEffectReverbProcessors();
	KillExcitement();
	ResetAMRegisters();

	// Per-world volume modifiers must not leak into the next world.
	for (auto& group : s_relativeVolumes)
		group.fill(1.0f);
	s_relativeVolumes[0][1] = 0.5f;
	s_channelVolumes = { 1.0f, 0.5f, 1.0f, 1.0f };
	s_musicVolume = 1.0f;
	s_dialogMusicDuck = 1.0f;
	s_dialogAudioDuckHeld = false;
	s_effectTickAccumulator = 0.0;
	s_effectLastUpdate = {};
	s_effectGlobalRegisters.fill(0);
	s_fPauseAmbients = 0;
	// Bank 0 is permanent in the original. Other successfully loaded banks stay
	// resident as well because live AMBs retain pointers into their parsed data.
	// ShutdownSound/StartupSound remains the explicit lifetime boundary.
}

SoundBankPtr SbpEnsureBank(int ibnk, const char* fileName)
{
	const auto bankLoadBegin = std::chrono::steady_clock::now();
	if (ibnk < 0 || ibnk >= static_cast<int>(kEffectBankCount))
		return nullptr;
	// AMBs retain a direct pointer to the parsed bank for their entire lifetime.
	// Keep every successfully loaded bank resident instead of replacing the
	// previous world bank underneath active handlers. This also allows sounds
	// which legitimately reference another world's bank without disturbing the
	// currently playing music.
	if (s_effectBanks[ibnk])
	{
		if (ibnk != 0)
			s_ibnkWorldCur = ibnk;
		return s_effectBanks[ibnk].get();
	}

	const std::filesystem::path path = FindEffectBank(ibnk, fileName);
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
		std::printf("[SOUND BANK] Extracted bank is empty: %s\n", path.string().c_str());
		return nullptr;
	}

	auto replacement = std::make_unique<EFFECTBANK>();
	replacement->ibnk = ibnk;
	replacement->bytes = std::move(bytes);
	if (!Parse989SfxBank(*replacement))
	{
		std::printf("[SOUND BANK] Invalid or unsupported 989SND SBlk: %s\n", path.string().c_str());
		return nullptr;
	}
	s_effectBanks[ibnk] = std::move(replacement);
	// Bank zero is the permanent common bank and must not replace the tracked
	// world-bank index. Only publish a new current world bank after loading and
	// parsing it successfully.
	if (ibnk != 0)
		s_ibnkWorldCur = ibnk;
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

void PreloadMusidSong(MUSID musid, const char* fileName)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	const auto musicLoadBegin = std::chrono::steady_clock::now();
	if (musid == MUSID_Nil)
	{
		KillMusic();
		return;
	}
	if (s_musicStream.state != MusicState::None && s_musicStream.musid == musid)
		return;

	UnloadMusic();
	const std::filesystem::path bankPath = FindMusicBank(musid, fileName);
	if (bankPath.empty())
	{
		ReportRuntimeError("Music", "Bank was not found for MUSID 0x%X",
			static_cast<unsigned>(musid));
		return;
	}
	const bool loaded = bankPath.extension() == L".mus"
		? LoadMusic989Mus(bankPath)
		: LoadMusic989Bank(bankPath);
	const bool engineReady = EnsureMusicEngine();
	if (!engineReady || !loaded)
	{
		ReportRuntimeError("Music", "Load failed for MUSID 0x%X (%s), path '%s'",
			static_cast<unsigned>(musid), loaded ? "audio_engine" : "bank_parse",
			bankPath.string().c_str());
		CloseMusicVoice();
		return;
	}
#ifndef __SWITCH__
	std::printf("[MUSIC LOAD] ready musid=0x%X path=%s segments=%zu tones=%zu\n",
		static_cast<unsigned>(musid), bankPath.string().c_str(),
		s_music989.segmentOffsets.size(), s_music989.tones.size());
#endif
	s_music989.registers = s_amRegisters;
	s_musicStream.musid = musid;
	s_musicStream.path = bankPath;
	s_musicStream.state = MusicState::Loaded;
}

void StartMusidSong(MUSID musid, const char* fileName)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	TraceMusicEvent("start_song_request", static_cast<int>(musid));
	PreloadMusidSong(musid, fileName);
	if (s_musicStream.state == MusicState::None)
		return;
	if (s_musicStream.state == MusicState::Playing)
		return;
	QueueMusicSegment(0);
	ActivateQueuedMusicSegments();
	PrimeMusicSequencerStartup();
	std::printf("[MUSIC START] musid=0x%X active=%zu pending=%zu voices=%zu excite=%u",
		static_cast<unsigned>(musid), s_music989.activeSegments.size(),
		s_music989.pendingStarts.size(), s_music989.voices.size(),
		static_cast<unsigned>(s_globalExcite));
	for (const MusicSegment& segment : s_music989.activeSegments)
	{
		unsigned mutedMask = 0;
		for (std::size_t channel = 0; channel < segment.muted.size(); ++channel)
			if (segment.muted[channel]) mutedMask |= 1u << channel;
		std::printf(" segment=%u:complete%d:muted%04X",
			segment.id, segment.complete ? 1 : 0, mutedMask);
	}
	std::printf("\n");
	s_music989.lastUpdate = std::chrono::steady_clock::now();
	s_musicStream.state = MusicState::Playing;
	s_musicStream.paused = false;
	s_musicTraceHadVoices = !s_music989.voices.empty();
	TraceMusicEvent("start_song_complete", static_cast<int>(musid));
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
	SetEffectReverbTypeDepth(rvrbk, depth);
}

static void DiscardAmbientEngineVoices(bool restartPersistentSounds)
{
	if (!g_psw)
		return;
	for (const std::unique_ptr<AMB>& slot : g_psw->ambOwners)
	{
		if (!slot)
			continue;
		AMB& amb = *slot;
		if (amb.voices.empty() && amb.voice)
			amb.voice->DestroyVoice();
		for (EFFECTVOICE& active : amb.voices)
		{
			if (active.voice)
				active.voice->DestroyVoice();
			active.voice = nullptr;
			active.pcm.clear();
		}
		amb.voices.clear();
		amb.voice = nullptr;
		if (!restartPersistentSounds || amb.fStopped || static_cast<int>(amb.sfxid) == -2)
			continue;
		if (amb.fContinuous && !amb.fOutOfRange)
		{
			amb.nextGrain = 0;
			amb.grainCountdown = 0;
			amb.handlerDone = false;
			amb.currentPitchBend = 0;
		}
		else if (amb.lmRepeat.gMin >= 0.0f)
		{
			amb.handlerDone = true;
			amb.tNext = g_clock.t;
		}
		else
		{
			// A one-shot cannot be resumed at an exact sample after device loss.
			// Let the normal update remove it rather than replaying it from frame zero.
			amb.fStopped = 1;
			amb.handlerDone = true;
		}
	}
}

static void ReleaseAudioEngineGraph(bool restartPersistentSounds)
{
	DiscardAmbientEngineVoices(restartPersistentSounds);
	CloseMusicVoice();
	CloseVagStream();
	// Recovery/shutdown is about to invalidate the entire graph, so deferred
	// voices cannot safely outlive their destination submixes here.
	DestroyRetiredSourceVoicesImmediately();
	if (s_musicCoreVoice)
	{
		s_musicCoreVoice->DestroyVoice();
		s_musicCoreVoice = nullptr;
	}
	if (s_effectWetInputVoice)
	{
		s_effectWetInputVoice->DestroyVoice();
		s_effectWetInputVoice = nullptr;
	}
	if (s_effectReverbVoice)
	{
		s_effectReverbVoice->DestroyVoice();
		s_effectReverbVoice = nullptr;
	}
	if (s_effectStudioCVoice)
	{
		s_effectStudioCProcessor = nullptr;
		s_effectStudioCVoice->DestroyVoice();
		s_effectStudioCVoice = nullptr;
	}
	if (s_effectDryVoice)
	{
		s_effectDryVoice->DestroyVoice();
		s_effectDryVoice = nullptr;
	}
	s_effectReverbCreationAttempted = false;
	s_effectStudioCCreationAttempted = false;
	if (s_musicMasterVoice)
	{
		#ifdef _WIN32
		s_finalOutputProcessor = nullptr;
		#endif
		s_musicMasterVoice->DestroyVoice();
		s_musicMasterVoice = nullptr;
	}
	if (s_xaudio2)
	{
		s_xaudio2->UnregisterForCallbacks(&s_audioEngineCallback);
		s_xaudio2->Release();
		s_xaudio2 = nullptr;
	}
}

static bool CompleteSoundStartup()
{
	if (!EnsureMusicEngine())
	{
		return false;
	}
	if (!s_soundStartupComplete)
	{
		SbpEnsureBank(0);
		s_globalExcite = 20;
		InitializeSoundOptions();
		// StartupSound runs before the initial world transition.  Do not expose
		// the newly-created XAudio graph while its banks, submixes, and first
		// source buffers are still being populated; doing so can put a short
		// uninitialized/reverb transient on the device.  CTransition::Execute
		// calls InitializeSoundOptions again after the world is completely loaded,
		// which is the first safe point to ramp the final output up to unity.
		SetMasterVolume(0.0f);
		s_effectReverbKind = REVERBK_None;
		s_effectReverbDepth = 0;
		s_soundStartupComplete = true;
	}
	EnsureEffectReverb();
	ApplyCurrentEffectReverb();
	// Every destination and effect is valid now, so it is safe to expose the
	// graph at the requested volume.
	if (s_musicMasterVoice)
	{
		#ifdef _WIN32
		ReportXAudioFailureOnce("Enable initialized mastering voice",
			s_musicMasterVoice->SetVolume(1.0f));
		if (s_finalOutputProcessor)
			s_finalOutputProcessor->SetTargetGain(s_masterVolume);
		#else
		ReportXAudioFailureOnce("Enable initialized mastering voice",
			s_musicMasterVoice->SetVolume(s_masterVolume));
		#endif
	}
	if (!s_musicWorkerRunning.exchange(true, std::memory_order_acq_rel))
		s_musicWorker = std::thread(MusicWorkerMain);
	s_audioRecoveryPending = false;
	return true;
}

static bool RecoverAudioEngine()
{
	struct SavedMusicSegment
	{
		MusicSegment segment{};
		std::size_t beginOffset = 0;
		std::size_t cursorOffset = 0;
		std::size_t endOffset = 0;
	};
	struct SavedMusicVoice
	{
		uint32_t segment = 0;
		uint8_t channel = 0;
		uint8_t note = 0;
		uint8_t velocity = 127;
		std::size_t toneIndex = 0;
		uint64_t samplePosition = 0;
		float baseVolume = 1.0f;
		MusicEnvelope envelope{};
	};
	MUSID previousMusic = MUSID_Nil;
	bool musicWasLoaded = false;
	bool musicWasPlaying = false;
	bool musicWasPaused = false;
	std::vector<SavedMusicSegment> previousSegments;
	std::vector<SavedMusicVoice> previousMusicVoices;
	std::vector<uint32_t> previousPendingStarts;
	std::array<uint8_t, 16> previousRegisters{};
	std::array<MusicAmeGroup, 16> previousGroups{};
	std::array<std::size_t, 16> previousMacroOffsets{};
	double previousTickAccumulator = 0.0;
	VagStreamState previousVag{};
	uint64_t previousVagSample = 0;
	if (s_musicWorkerRunning.exchange(false, std::memory_order_acq_rel) &&
		s_musicWorker.joinable())
		s_musicWorker.join();
	{
		std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
		previousMusic = s_musicStream.musid;
		musicWasLoaded = s_musicStream.state != MusicState::None;
		musicWasPlaying = s_musicStream.state == MusicState::Playing;
		musicWasPaused = s_musicStream.paused;
		if (musicWasPlaying && !s_music989.sequence.empty())
		{
			const uint8_t* sequenceBegin = s_music989.sequence.data();
			const uint8_t* sequenceEnd = sequenceBegin + s_music989.sequence.size();
			for (const MusicSegment& segment : s_music989.activeSegments)
			{
				if (!segment.begin || !segment.cursor || !segment.end ||
					segment.begin < sequenceBegin || segment.begin > sequenceEnd ||
					segment.cursor < sequenceBegin || segment.cursor > sequenceEnd ||
					segment.end < sequenceBegin || segment.end > sequenceEnd)
					continue;
				SavedMusicSegment saved{};
				saved.segment = segment;
				saved.beginOffset = static_cast<std::size_t>(segment.begin - sequenceBegin);
				saved.cursorOffset = static_cast<std::size_t>(segment.cursor - sequenceBegin);
				saved.endOffset = static_cast<std::size_t>(segment.end - sequenceBegin);
				previousSegments.push_back(saved);
			}
			for (const MusicVoice& voice : s_music989.voices)
			{
				if (!voice.voice || !voice.tone || voice.tone < s_music989.tones.data() ||
					voice.tone >= s_music989.tones.data() + s_music989.tones.size())
					continue;
				XAUDIO2_VOICE_STATE state{};
				voice.voice->GetState(&state);
				previousMusicVoices.push_back({ voice.segment, voice.channel, voice.note,
					voice.velocity,
					static_cast<std::size_t>(voice.tone - s_music989.tones.data()),
					state.SamplesPlayed, voice.baseVolume, voice.envelope });
			}
			previousPendingStarts = s_music989.pendingStarts;
			previousRegisters = s_music989.registers;
			previousGroups = s_music989.groups;
			previousTickAccumulator = s_music989.tickAccumulator;
			previousMacroOffsets.fill(static_cast<std::size_t>(-1));
			for (std::size_t i = 0; i < s_music989.macros.size(); ++i)
				if (s_music989.macros[i] && s_music989.macros[i] >= sequenceBegin &&
					s_music989.macros[i] <= sequenceEnd)
					previousMacroOffsets[i] = static_cast<std::size_t>(s_music989.macros[i] - sequenceBegin);
		}
		if (s_vagStream.loaded && s_vagStream.voice && !s_vagStream.pcm.empty())
		{
			previousVag.loaded = true;
			previousVag.started = s_vagStream.started;
			previousVag.paused = s_vagStream.paused;
			previousVag.commentary = s_vagStream.commentary;
			previousVag.path = s_vagStream.path;
			previousVag.format = s_vagStream.format;
			previousVag.pcm = s_vagStream.pcm;
			previousVagSample = s_vagStream.playBegin;
			if (s_vagStream.started)
			{
				XAUDIO2_VOICE_STATE state{};
				s_vagStream.voice->GetState(&state);
				previousVagSample += state.SamplesPlayed;
			}
		}
		ReleaseAudioEngineGraph(true);
	}
	if (!CompleteSoundStartup())
		return false;
	if (musicWasLoaded && previousMusic != MUSID_Nil)
	{
		PreloadMusidSong(previousMusic);
		if (musicWasPlaying)
		{
			bool restored = !previousSegments.empty() &&
				s_musicStream.state == MusicState::Loaded && !s_music989.sequence.empty();
			if (restored)
			{
				const std::size_t sequenceSize = s_music989.sequence.size();
				const uint8_t* sequenceBegin = s_music989.sequence.data();
				s_music989.activeSegments.clear();
				for (const SavedMusicSegment& saved : previousSegments)
				{
					if (saved.beginOffset > saved.cursorOffset || saved.cursorOffset > saved.endOffset ||
						saved.endOffset > sequenceSize)
					{
						restored = false;
						break;
					}
					MusicSegment segment = saved.segment;
					segment.begin = sequenceBegin + saved.beginOffset;
					segment.cursor = sequenceBegin + saved.cursorOffset;
					segment.end = sequenceBegin + saved.endOffset;
					s_music989.activeSegments.push_back(segment);
				}
				if (restored)
				{
					s_music989.pendingStarts = previousPendingStarts;
					s_music989.registers = previousRegisters;
					s_music989.groups = previousGroups;
					s_music989.tickAccumulator = previousTickAccumulator;
					for (std::size_t i = 0; i < previousMacroOffsets.size(); ++i)
						s_music989.macros[i] = previousMacroOffsets[i] <= sequenceSize
							? sequenceBegin + previousMacroOffsets[i] : nullptr;

					// Restore every held layer at its current decoded-sample position.
					// The sequencer cursor and envelope continue from the same instant.
					for (const SavedMusicVoice& savedVoice : previousMusicVoices)
					{
						if (savedVoice.toneIndex >= s_music989.tones.size())
							continue;
						auto segment = std::find_if(s_music989.activeSegments.begin(),
							s_music989.activeSegments.end(), [&](const MusicSegment& active)
							{ return active.id == savedVoice.segment; });
						if (segment == s_music989.activeSegments.end())
							continue;
						StartMusicTone(*segment, savedVoice.channel, savedVoice.note,
							savedVoice.velocity, &s_music989.tones[savedVoice.toneIndex],
							static_cast<uint32_t>((std::min<uint64_t>)(savedVoice.samplePosition,
								std::numeric_limits<uint32_t>::max())));
						for (MusicVoice& voice : s_music989.voices)
							if (voice.segment == savedVoice.segment && voice.channel == savedVoice.channel &&
								voice.note == savedVoice.note && voice.tone == &s_music989.tones[savedVoice.toneIndex])
							{
								voice.baseVolume = savedVoice.baseVolume;
								voice.envelope = savedVoice.envelope;
							}
					}
					s_music989.lastUpdate = std::chrono::steady_clock::now();
					s_musicStream.state = MusicState::Playing;
					s_musicStream.paused = false;
				}
			}
			if (!restored)
			{
				s_music989.activeSegments.clear();
				s_music989.pendingStarts.clear();
				StartMusidSong(previousMusic);
			}
			if (musicWasPaused)
				PauseMusicSequencer();
		}
	}
	if (previousVag.loaded)
	{
		const uint64_t totalSamples = previousVag.format.blockAlign == 0 ? 0 :
			previousVag.pcm.size() / previousVag.format.blockAlign;
		if (previousVagSample < totalSamples &&
			CreateVagStreamVoice(previousVag, previousVagSample))
		{
			const bool shouldStart = previousVag.started && !previousVag.paused;
			const bool wasCommentary = previousVag.commentary;
			s_vagStream = std::move(previousVag);
			if (s_pambVagAmbient)
				s_hvagAmbient = 1;
			RecalculateChannelVolume(2);
			if (shouldStart)
			{
				const HRESULT startResult = s_vagStream.voice->Start();
				if (FAILED(startResult))
				{
					ReportXAudioFailureOnce("Resume VAG/dialog after device recovery", startResult);
					CloseVagStream();
				}
			}
			if (s_vagStream.loaded && s_vagStream.started)
				ApplyDialogMusicDuck(0.2f);
			if (s_vagStream.loaded && wasCommentary)
			{
				SetMvgkRvol(0.5f, 8, 1);
				SetMvgkRvol(0.5f, 8, 0);
				SetMvgkRvol(0.5f, 8, 3);
			}
		}
	}
	s_effectLastUpdate = std::chrono::steady_clock::now();
	s_effectTickAccumulator = 0.0;
#ifndef __SWITCH__
	std::printf("[AUDIO] Windows audio engine rebuilt successfully.\n");
#endif
	return true;
}

void StartupSound()
{
	// The PS2 clears its seven bank handles before starting 989SND. Rebuild the
	// equivalent PC-side state, initialize XAudio2, and keep bank 0 resident as
	// the common effects bank.
	UnloadEffectBank();
#ifdef __SWITCH__
    CaneAudio::InitializePrefetch();
#else
	// Keep file loading independent from XAudio2 initialization.  Movie playback
	// can then detect an unavailable output device and continue silently instead
	// of waiting forever for a loader thread that was never started.
	{
		std::lock_guard<std::mutex> lock(s_vagLoadMutex);
		s_vagLoadWorkerRunning = true;
	}
	if (!s_vagLoadWorker.joinable())
		s_vagLoadWorker = std::thread(VagLoadWorkerMain);
#endif

	if (!CompleteSoundStartup())
	{
		ReportRuntimeError("Audio",
			"Sound initialization failed; playback will retry when an output device becomes available");
		s_audioRecoveryPending = true;
		s_audioRecoveryRetry = std::chrono::steady_clock::now() + std::chrono::seconds(1);
		return;
	}
}

void ShutdownSound()
{
#ifdef __SWITCH__
    CaneAudio::ShutdownPrefetch();
#endif
	{
		std::lock_guard<std::mutex> lock(s_vagLoadMutex);
		s_vagLoadWorkerRunning = false;
		s_vagLoad.requested = false;
		++s_vagLoadGeneration;
	}
	s_vagLoadCondition.notify_one();
	if (s_vagLoadWorker.joinable())
		s_vagLoadWorker.join();
	{
		std::lock_guard<std::mutex> lock(s_vagLoadMutex);
		s_vagLoad = {};
	}

	if (s_musicWorkerRunning.exchange(false, std::memory_order_acq_rel) &&
		s_musicWorker.joinable())
		s_musicWorker.join();

	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	// Explicitly discard every source voice before its destination submixes.
	ReleaseAudioEngineGraph(false);
	UnloadEffectBank();
	s_soundStartupComplete = false;
	s_audioRecoveryPending = false;
	s_audioCriticalResult.store(S_OK, std::memory_order_release);
	s_audioCriticalError.store(false, std::memory_order_release);
}

void InitializeSoundOptions()
{
	// Retail's first MVGK row is { effects=1, music=0.5, dialog=1, user=1 }.
	// Every remaining row starts at unity.
	for (auto& group : s_relativeVolumes)
		group.fill(1.0f);
	s_relativeVolumes[0][1] = 0.5f;

	SetMasterVolume(1.0f);
	for (int channel = 0; channel < 4; ++channel)
		RecalculateChannelVolume(channel);

	SetUserStereoEnabled(s_stereoOutputEnabled);
}

void SetMasterVolume(float uVol)
{
	// Retail sends group 8 a 0..1024 fixed-point gain. XAudio2 uses the
	// equivalent normalized float directly on the mastering voice.
	if (!std::isfinite(uVol))
		uVol = 0.0f;
	s_masterVolume = std::clamp(uVol, 0.0f, 1.0f);
	if (!EnsureMusicEngine())
		return;
	#ifdef _WIN32
	if (s_finalOutputProcessor)
		s_finalOutputProcessor->SetTargetGain(s_masterVolume);
	else if (s_musicMasterVoice)
		ReportXAudioFailureOnce("Set mastering volume fallback",
			s_musicMasterVoice->SetVolume(s_masterVolume));
	#else
	if (s_musicMasterVoice)
		ReportXAudioFailureOnce("Set mastering volume",
			s_musicMasterVoice->SetVolume(s_masterVolume));
	#endif
}

float GetMasterVolume()
{
	return s_masterVolume;
}

void SetFmvAudioMode(bool enabled)
{
	// A movie is never positional. A dialogue VAG interrupted by a transition
	// may still own the shared stream through an AMB; remove that ownership so
	// it cannot attenuate or pan the subsequently loaded movie.
	if (enabled && s_pambVagAmbient != nullptr)
		RemoveAmb(s_pambVagAmbient);
	s_fFmvAudioMode = enabled;

	// Retail pauses every gameplay sound group before initializing movie audio.
	// Muting source voices alone is insufficient in XAudio2: the reverb submixes
	// retain their delay history and can keep producing (or feeding back) after
	// all of their inputs have stopped.  Isolate FMV audio from those wet returns
	// and clear Studio C's history before the movie voice is exposed.
	if (enabled)
	{
		if (s_effectWetInputVoice)
			ReportXAudioFailureOnce("Mute gameplay wet-input bus for FMV",
				s_effectWetInputVoice->SetVolume(0.0f));
		if (s_effectStudioCVoice)
			ReportXAudioFailureOnce("Mute Studio C return for FMV",
				s_effectStudioCVoice->SetVolume(0.0f));
		if (s_effectReverbVoice)
			ReportXAudioFailureOnce("Mute generic reverb return for FMV",
				s_effectReverbVoice->SetVolume(0.0f));
		#ifdef _WIN32
		if (s_effectStudioCProcessor)
		{
			s_effectStudioCProcessor->SetActive(false);
			s_effectStudioCProcessor->Reset();
		}
		#endif
	}
	else
	{
		// Restore the existing authored kind/depth; FMV mode never changes the
		// world's reverb selection itself.
		EnsureEffectReverb();
		ApplyCurrentEffectReverb();
		if (s_effectWetInputVoice)
			ReportXAudioFailureOnce("Restore gameplay wet-input bus after FMV",
				s_effectWetInputVoice->SetVolume(1.0f));
	}
	if (s_vagStream.loaded)
		RecalculateChannelVolume(2);
}

void SetSwIntermittentSound(SW* psw, SFXID sfxid)
{
	if (psw == nullptr)
		return;

	if (psw->cisi >= 16)
		return;

	ISI& isi = psw->aisi[psw->cisi++];

	isi.sfxid = sfxid;
	isi.lmRepeat = s_lmRepeatDefault;
	isi.lmRepDist = s_lmDistDefault;
}

void SetSwIntermittentVolPct(SW* psw, const LM* plmVolPct)
{
	if (psw == nullptr)
		return;

	if (psw->cisi == 0 || plmVolPct == nullptr)
		return;

	ISI& isi = psw->aisi[psw->cisi - 1];

	isi.lmRepDist.gMin = 8000.0f - plmVolPct->gMax * 50.0f;
	isi.lmRepDist.gMax = 8000.0f - plmVolPct->gMin * 50.0f;
}

void SetSwIntermittentRepeat(SW* psw, const LM* plmRepeat)
{
	if (psw == nullptr)
		return;

	if (psw->cisi == 0 || plmRepeat == nullptr)
		return;

	psw->aisi[psw->cisi - 1].lmRepeat = *plmRepeat;
}

void StartSwIntermittentSounds(SW* psw)
{
	for (int i = 0; i < psw->cisi; ++i)
	{
		ISI& isi = psw->aisi[i];
		StartSound(isi.sfxid, nullptr, nullptr, nullptr, 8000.0f, 3000.0f, 1.0f, 0.0f, 0.0f, &isi.lmRepeat, &isi.lmRepDist);
	}
}

void ServiceAudioDevice()
{
	if (s_audioCriticalError.exchange(false, std::memory_order_acq_rel))
	{
		const HRESULT result = s_audioCriticalResult.load(std::memory_order_acquire);
		ReportRuntimeError("Audio",
			"The audio device stopped, changed format, or was removed (HRESULT 0x%08X). Rebuilding the audio engine.",
			static_cast<unsigned>(result));
		s_audioRecoveryPending = true;
		s_audioRecoveryRetry = {};
	}
	if (!s_audioRecoveryPending)
		return;

	const auto now = std::chrono::steady_clock::now();
	if (s_audioRecoveryRetry.time_since_epoch().count() != 0 && now < s_audioRecoveryRetry)
		return;
	if (!RecoverAudioEngine())
	{
		s_audioRecoveryPending = true;
		s_audioRecoveryRetry = now + std::chrono::seconds(1);
	}
}

namespace
{
	std::ofstream s_eerieAudioTrace;
	bool s_eerieAudioTraceEnabled = false;
#ifdef _WIN32
	SpuMixSaturator* s_eerieAudioCaptureProcessor = nullptr;
	SpuMixSaturator::DiagnosticStats s_eerieAudioPreviousStats{};

	void WriteDiagnosticWave(const std::filesystem::path& path, const std::vector<float>& samples)
	{
		std::ofstream output(path, std::ios::binary | std::ios::trunc);
		if (!output)
			return;

		const std::uint32_t sampleRate = 48000;
		const std::uint16_t channels = 2;
		const std::uint16_t bitsPerSample = 16;
		const std::uint32_t dataBytes = static_cast<std::uint32_t>(
			(std::min)(samples.size(), static_cast<std::size_t>(UINT32_MAX / 2))) * 2u;
		const std::uint32_t riffBytes = 36u + dataBytes;
		const std::uint32_t byteRate = sampleRate * channels * bitsPerSample / 8u;
		const std::uint16_t blockAlign = channels * bitsPerSample / 8u;
		const std::uint32_t fmtSize = 16;
		const std::uint16_t pcmFormat = 1;
		output.write("RIFF", 4);
		output.write(reinterpret_cast<const char*>(&riffBytes), sizeof(riffBytes));
		output.write("WAVEfmt ", 8);
		output.write(reinterpret_cast<const char*>(&fmtSize), sizeof(fmtSize));
		output.write(reinterpret_cast<const char*>(&pcmFormat), sizeof(pcmFormat));
		output.write(reinterpret_cast<const char*>(&channels), sizeof(channels));
		output.write(reinterpret_cast<const char*>(&sampleRate), sizeof(sampleRate));
		output.write(reinterpret_cast<const char*>(&byteRate), sizeof(byteRate));
		output.write(reinterpret_cast<const char*>(&blockAlign), sizeof(blockAlign));
		output.write(reinterpret_cast<const char*>(&bitsPerSample), sizeof(bitsPerSample));
		output.write("data", 4);
		output.write(reinterpret_cast<const char*>(&dataBytes), sizeof(dataBytes));
		for (std::size_t i = 0; i < dataBytes / 2u; ++i)
		{
			const float value = std::isfinite(samples[i])
				? std::clamp(samples[i], -1.0f, 32767.0f / 32768.0f) : 0.0f;
			const std::int16_t pcm = static_cast<std::int16_t>(std::lrint(value * 32768.0f));
			output.write(reinterpret_cast<const char*>(&pcm), sizeof(pcm));
		}
	}
#endif

	void UpdateEerieAudioTrace()
	{
		if (!s_eerieAudioTraceEnabled || !s_eerieAudioTrace.is_open())
			return;

#ifdef _WIN32
		if (s_eerieAudioCaptureProcessor != s_finalOutputProcessor)
		{
			s_eerieAudioCaptureProcessor = s_finalOutputProcessor;
			s_eerieAudioPreviousStats = {};
			if (s_eerieAudioCaptureProcessor != nullptr)
			{
				s_eerieAudioCaptureProcessor->BeginDiagnosticCapture();
				s_eerieAudioTrace << "EVENT final_output_capture_started tick=" << g_clock.tickFrame << '\n';
			}
			else
				s_eerieAudioTrace << "EVENT final_output_capture_lost tick=" << g_clock.tickFrame << '\n';
		}

		SpuMixSaturator::DiagnosticStats stats{};
		if (s_eerieAudioCaptureProcessor != nullptr)
			stats = s_eerieAudioCaptureProcessor->GetDiagnosticStats();
		XAUDIO2_VOICE_STATE vagState{};
		if (s_vagStream.voice != nullptr)
			s_vagStream.voice->GetState(&vagState, XAUDIO2_VOICE_NOSAMPLESPLAYED);
#endif

		std::size_t retiredVoices = 0;
		{
			std::lock_guard<std::mutex> lock(s_retiredVoiceMutex);
			retiredVoices = s_retiredVoices.size();
		}

		std::lock_guard<std::recursive_mutex> musicLock(s_musicMutex);
		s_eerieAudioTrace << "FRAME tick=" << g_clock.tickFrame
			<< " t=" << std::fixed << std::setprecision(6) << g_clock.t
			<< " treal=" << g_clock.tReal
			<< " fmv=" << s_fFmvAudioMode
			<< " recovery=" << s_audioRecoveryPending
			<< " master=" << s_masterVolume
			<< " userMusic=" << s_userMusicVolume
			<< " userSfx=" << s_userSfxVolume
			<< " userDialog=" << s_userDialogueVolume
			<< " musicState=" << static_cast<int>(s_musicStream.state)
			<< " musid=" << static_cast<int>(s_musicStream.musid)
			<< " musicPaused=" << s_musicStream.paused
			<< " musicVoices=" << s_music989.voices.size()
			<< " musicSegments=" << s_music989.activeSegments.size()
			<< " musicPending=" << s_music989.pendingStarts.size()
			<< " vagLoaded=" << s_vagStream.loaded
			<< " vagStarted=" << s_vagStream.started
			<< " vagPaused=" << s_vagStream.paused
			<< " vagCommentary=" << s_vagStream.commentary
			<< " vagChannels=" << s_vagStream.format.channels
			<< " vagRate=" << s_vagStream.format.samplesPerSec
			<< " vagBits=" << s_vagStream.format.bitsPerSample
			<< " vagBytes=" << s_vagStream.pcm.size()
			<< " vagPath=\"" << s_vagStream.path.string() << '"'
			<< " retiredVoices=" << retiredVoices
			<< " reverbKind=" << static_cast<int>(s_effectReverbKind)
			<< " reverbDepth=" << s_effectReverbDepth
			<< " duck=" << s_dialogMusicDuck
			<< " ambientsPaused=" << s_fPauseAmbients;
#ifdef _WIN32
		s_eerieAudioTrace << " vagQueued=" << vagState.BuffersQueued
			<< " samples=" << stats.samples
			<< " inPeak=" << stats.inputPeak
			<< " outPeak=" << stats.outputPeak
			<< " inJump=" << stats.inputMaxJump
			<< " outJump=" << stats.outputMaxJump
			<< " invalid=" << stats.invalidInputSamples
			<< " inClip=" << stats.clippedInputSamples
			<< " outClip=" << stats.clippedOutputSamples;

		if (stats.invalidInputSamples != s_eerieAudioPreviousStats.invalidInputSamples ||
			stats.clippedInputSamples != s_eerieAudioPreviousStats.clippedInputSamples ||
			stats.clippedOutputSamples != s_eerieAudioPreviousStats.clippedOutputSamples)
		{
			s_eerieAudioTrace << " EVENT_AUDIO_ANOMALY"
				<< " invalidDelta=" << stats.invalidInputSamples - s_eerieAudioPreviousStats.invalidInputSamples
				<< " inClipDelta=" << stats.clippedInputSamples - s_eerieAudioPreviousStats.clippedInputSamples
				<< " outClipDelta=" << stats.clippedOutputSamples - s_eerieAudioPreviousStats.clippedOutputSamples;
		}
		s_eerieAudioPreviousStats = stats;
#endif
		s_eerieAudioTrace << '\n';
		s_eerieAudioTrace.flush();
	}
}

bool FEerieAudioTraceEnabled()
{
	return s_eerieAudioTraceEnabled;
}

bool FMusicDropoutTraceEnabled()
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	return s_musicDropoutTraceEnabled;
}

void SetMusicDropoutTraceEnabled(bool enabled)
{
	std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
	if (enabled == s_musicDropoutTraceEnabled)
		return;

	if (s_musicDropoutTrace.is_open())
	{
		TraceMusicEvent("trace_disabled");
		s_musicDropoutTrace.close();
	}
	s_musicDropoutTraceEnabled = false;
	s_musicTraceHadVoices = false;
	if (!enabled)
		return;

	const std::filesystem::path directory =
		ExecutableDirectory() / "MusicDiagnostics";
	std::error_code error;
	std::filesystem::create_directories(directory, error);
	s_musicDropoutTrace.open(directory / "music_dropout_trace.log", std::ios::trunc);
	s_musicDropoutTraceEnabled = s_musicDropoutTrace.is_open();
	s_musicTraceHadVoices = !s_music989.voices.empty();
	if (s_musicDropoutTraceEnabled)
	{
		s_musicDropoutTrace << "ProjectCane music dropout trace\n";
		s_musicDropoutTrace << "BEGIN rolling history (oldest first)\n";
		for (const std::string& entry : s_musicTraceHistory)
			s_musicDropoutTrace << entry;
		s_musicDropoutTrace << "END rolling history\n";
		TraceMusicEvent("trace_enabled");
	}
}

void SetEerieAudioTraceEnabled(const bool enabled)
{
	if (enabled == s_eerieAudioTraceEnabled)
		return;

	if (!enabled)
	{
		if (s_eerieAudioTrace.is_open())
			s_eerieAudioTrace << "EVENT trace_disabled tick=" << g_clock.tickFrame << '\n';
#ifdef _WIN32
		if (s_eerieAudioCaptureProcessor != nullptr)
		{
			auto capture = s_eerieAudioCaptureProcessor->EndDiagnosticCapture();
			const std::filesystem::path directory = ExecutableDirectory() / "CameraDiagnostics";
			WriteDiagnosticWave(directory / "eerie_audio_before_saturator.wav", capture.input);
			WriteDiagnosticWave(directory / "eerie_audio_after_saturator.wav", capture.output);
		}
		s_eerieAudioCaptureProcessor = nullptr;
		s_eerieAudioPreviousStats = {};
#endif
		if (s_eerieAudioTrace.is_open())
			s_eerieAudioTrace.close();
		s_eerieAudioTraceEnabled = false;
		return;
	}

	const std::filesystem::path directory = ExecutableDirectory() / "CameraDiagnostics";
	std::error_code error;
	std::filesystem::create_directories(directory, error);
	s_eerieAudioTrace.open(directory / "eerie_audio_trace.log", std::ios::trunc);
	s_eerieAudioTraceEnabled = s_eerieAudioTrace.is_open();
	if (s_eerieAudioTraceEnabled)
	{
		s_eerieAudioTrace << "ProjectCane eerie audio trace\n"
			<< "EVENT trace_enabled tick=" << g_clock.tickFrame << '\n';
		s_eerieAudioTrace.flush();
	}
}

void UpdateSounds()
{
	ServiceAudioDevice();
	UpdateEerieAudioTrace();
	if (s_audioRecoveryPending)
		return;

	// Ruby's retail rhythm logic and 989SND were both driven by fixed PS2 time.
	// During that encounter, advance adaptive music from the same capped game
	// delta exactly once per rendered game frame. This prevents a host stall from
	// moving the music ahead while g_clock intentionally discards excess time.
	if (s_musicUsesGameplayClock.load(std::memory_order_acquire) &&
		s_musicGameplayClockFrame != g_cframe)
	{
		s_musicGameplayClockFrame = g_cframe;
		std::lock_guard<std::recursive_mutex> lock(s_musicMutex);
		UpdateMusicSequencer(static_cast<double>(g_clock.dt));
	}

	if (FAmbientsPaused() != 0)
		return;

	if (s_commentaryStartPending && !FPauseForVag())
	{
		s_commentaryStartPending = false;
		if (s_vagStream.loaded && s_vagStream.started)
		{
			s_vagStream.commentary = true;
			// Commentary uses mixer modifier 8 to duck music and effects while
			// leaving the dialogue channel at full volume.
			s_dialogMusicDuck = 1.0f;
			SetMvgkRvol(0.5f, 8, 1);
			SetMvgkRvol(0.5f, 8, 0);
			SetMvgkRvol(0.5f, 8, 3);
		}
	}
	else if (s_vagStartWhenReady)
		FPauseForVag();

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
	// Match the game-clock cap and discard excess wall time after a loading or
	// debugger stall. Catching up a quarter-second used to execute as many as 60
	// authored grains in one host frame, stacking overdue voices into a burst.
	s_effectTickAccumulator += (std::min)(effectElapsed, 1.0 / 30.0) * 240.0;
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
			{
				return;
			}
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
			if (amb.paused)
				continue;
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
			if (amb.fAttached && !amb.fStopped)
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
					RetireSourceVoice(active.voice, active.pcm);
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
				// Retail applies the shared one-second gate only when this AMB was
				// previously out of range. In-range intermittent sounds are allowed to
				// overlap even when two independent timers expire close together.
				bool allowStart = true;
				if (amb.fOutOfRange)
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
				if (amb.autoVolumeTicksRemaining > 0)
				{
					--amb.autoVolumeTicksRemaining;
					amb.autoVolumeScale = static_cast<float>(amb.autoVolumeTicksRemaining) /
						static_cast<float>(amb.autoVolumeTicks);
				}
				if (!amb.fStopped)
					TickEffectLfos(amb);
				UpdateEffectEnvelopes(amb);
				if (amb.fStopped && amb.autoVolumeTicksRemaining == 0)
				{
					for (EFFECTVOICE& active : amb.voices)
					{
						if (!active.voice)
							continue;
						RetireSourceVoice(active.voice, active.pcm);
					}
					break;
				}
				if (amb.handlerDone)
					continue;
				--amb.grainCountdown;
				int grainBudget = 1024;
				while (amb.grainCountdown <= 0 && !amb.handlerDone && grainBudget-- > 0)
					ExecuteEffectGrain(amb);
				if (grainBudget <= 0) amb.handlerDone = true;
			}
			CleanEffectVoices(amb);
			if (amb.fStopped && amb.voices.empty())
				completed.push_back(&amb);
			else if (amb.handlerDone && amb.voices.empty() && !recurring && !amb.fContinuous)
				completed.push_back(&amb);
			else if (!amb.fStopped)
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
}

static void UpdateMusicSequencer(double logicalElapsed)
{
	if (s_musicStream.state != MusicState::Playing || s_musicStream.paused)
		return;
	const auto now = std::chrono::steady_clock::now();
	double elapsed = logicalElapsed;
	if (elapsed < 0.0)
	{
		if (s_music989.lastUpdate.time_since_epoch().count() == 0)
			s_music989.lastUpdate = now;
		elapsed = std::chrono::duration<double>(now - s_music989.lastUpdate).count();
	}
	s_music989.lastUpdate = now;
	// MarkClockTick already applies the retail maximum step. Keep this guard for
	// callers outside the game loop and reject invalid clock values.
	if (!std::isfinite(elapsed) || elapsed <= 0.0)
		elapsed = 0.0;
	else if (logicalElapsed >= 0.0)
		elapsed = (std::min)(elapsed, 1.0 / 30.0);
	s_music989.tickAccumulator += elapsed * 240.0;
	while (s_music989.tickAccumulator >= 1.0)
	{
		for (MusicSegment& segment : s_music989.activeSegments)
		{
			const bool wasComplete = segment.complete;
			StepMusicSegment(segment);
			if (!wasComplete && segment.complete)
				TraceMusicEvent("segment_completed", static_cast<int>(segment.id));
		}
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
	const bool hasMusicVoices = !s_music989.voices.empty();
	if (s_musicTraceHadVoices && !hasMusicVoices &&
		s_musicStream.state == MusicState::Playing)
	{
		TraceMusicEvent("voices_dropped_to_zero");
	}
	s_musicTraceHadVoices = hasMusicVoices;
	// Do not leave a completed or rejected sequence advertised as Playing.
	// StartMusidSong deliberately ignores an already-playing MUSID, so stale
	// state here otherwise turns a recoverable sequence end into permanent
	// silence for every later request for the same song.
	if (s_music989.activeSegments.empty() && s_music989.pendingStarts.empty() &&
		s_music989.voices.empty())
	{
		TraceMusicEvent("sequence_exhausted");
		s_musicStream.state = MusicState::Loaded;
		s_musicStream.paused = false;
		s_music989.tickAccumulator = 0.0;
	}
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
	values[0][1] = 0.5f;
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
	L"Common.bnk", L"Intro.bnk", L"Underwater.bnk", L"Muggshot.bnk",
	L"Voodoo.bnk", L"Snow.bnk", L"Clockwerk.bnk"
};
LM s_lmRepeatDefault = { 4.0f, 6.0f };
LM s_lmDistDefault = { 3000.0f, 8000.0f };
int s_hvagAmbient = 0;
IEXC g_iexcHyst = 0;
int s_sfxidLast = 0;
int g_fSneakyFeet = 0;
