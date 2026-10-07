#pragma once

#include <xapo.h>
#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <vector>

// Bounded diagnostic capture used to compare ProjectCane's Studio C path with
// PCSX2. The capture is opt-in and has no effect on normal audio processing.
void ToggleLevelTransitionAudioTrace();
void StopLevelTransitionAudioTrace();
void MarkLevelTransitionAudioTrace(const char* eventName, int reverbKind = -1, int reverbDepth = -1);
void UpdateLevelTransitionAudioTraceRouting(float desiredWet, float actualWet,
	bool feedStudio, bool feedGeneric, float wetBusVolume);
void MarkLevelTransitionSoundStart(int sfxid, float volume, float frequency,
	bool attached, bool continuous);

class SpuStudioCReverb final : public IXAPO
{
public:
	SpuStudioCReverb();

	STDMETHOD(QueryInterface)(REFIID interfaceId, void** result) override;
	STDMETHOD_(ULONG, AddRef)() override;
	STDMETHOD_(ULONG, Release)() override;
	STDMETHOD(GetRegistrationProperties)(XAPO_REGISTRATION_PROPERTIES** properties) override;
	STDMETHOD(IsInputFormatSupported)(const WAVEFORMATEX* outputFormat,
		const WAVEFORMATEX* requestedInputFormat, WAVEFORMATEX** supportedInputFormat) override;
	STDMETHOD(IsOutputFormatSupported)(const WAVEFORMATEX* inputFormat,
		const WAVEFORMATEX* requestedOutputFormat, WAVEFORMATEX** supportedOutputFormat) override;
	STDMETHOD(Initialize)(const void* data, UINT32 dataSize) override;
	STDMETHOD_(void, Reset)() override;
	STDMETHOD(LockForProcess)(UINT32 inputCount,
		const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* inputs, UINT32 outputCount,
		const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* outputs) override;
	STDMETHOD_(void, UnlockForProcess)() override;
	STDMETHOD_(void, Process)(UINT32 inputCount,
		const XAPO_PROCESS_BUFFER_PARAMETERS* inputs, UINT32 outputCount,
		XAPO_PROCESS_BUFFER_PARAMETERS* outputs, BOOL enabled) override;
	STDMETHOD_(UINT32, CalcInputFrames)(UINT32 outputFrames) override;
	STDMETHOD_(UINT32, CalcOutputFrames)(UINT32 inputFrames) override;

private:
	static constexpr int kTapCount = 39;
	static constexpr int kRingSize = 64;
	static constexpr int kEffectBufferSize = 0x100000 - 0xFC810;
	static int Clamp16(int64_t value);
	static int64_t MulQ15(int64_t left, int64_t right);
	int EffectIndex(int offset) const;
	int Downsample(bool right) const;
	std::array<int, 2> Upsample() const;
	std::array<int, 2> ProcessFrame(int left, int right);

	std::vector<short> m_effectBuffer;
	std::array<std::array<short, kRingSize * 2>, 2> m_down{};
	std::array<std::array<short, kRingSize * 2>, 2> m_up{};
	std::atomic<ULONG> m_references{1};
	std::mutex m_processMutex;
	bool m_locked = false;
	unsigned m_ringPosition = 0;
	unsigned long long m_cycle = 0;
};
