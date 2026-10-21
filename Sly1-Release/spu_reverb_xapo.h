#pragma once

#ifdef _WIN32
#include <xapo.h>
#endif
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

#ifdef _WIN32
class SpuMixSaturator final : public IXAPO
{
public:
	struct DiagnosticCapture
	{
		std::vector<float> input;
		std::vector<float> output;
	};
	struct DiagnosticStats
	{
		std::uint64_t samples = 0;
		std::uint64_t invalidInputSamples = 0;
		std::uint64_t clippedInputSamples = 0;
		std::uint64_t clippedOutputSamples = 0;
		float inputPeak = 0.0f;
		float outputPeak = 0.0f;
		float inputMaxJump = 0.0f;
		float outputMaxJump = 0.0f;
	};
	explicit SpuMixSaturator(bool finalOutput = false)
		: m_targetGain(finalOutput ? 0.0f : 1.0f),
		  m_currentGain(finalOutput ? 0.0f : 1.0f),
		  m_finalOutput(finalOutput),
		  m_diagnosticInput(finalOutput ? kDiagnosticSampleCapacity : 0),
		  m_diagnosticOutput(finalOutput ? kDiagnosticSampleCapacity : 0)
	{
	}
	void SetTargetGain(float gain);
	void BeginDiagnosticCapture();
	DiagnosticCapture EndDiagnosticCapture();
	DiagnosticStats GetDiagnosticStats() const;
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
	static constexpr std::size_t kDiagnosticSampleCapacity = 48000u * 2u * 30u;
	std::atomic<ULONG> m_references{1};
	std::atomic<float> m_targetGain{1.0f};
	float m_currentGain = 1.0f;
	std::array<float, 2> m_previousInput{};
	std::array<float, 2> m_previousOutput{};
	bool m_finalOutput = false;
	bool m_locked = false;
	std::vector<float> m_diagnosticInput;
	std::vector<float> m_diagnosticOutput;
	std::atomic<std::size_t> m_diagnosticSampleCount{0};
	std::atomic<bool> m_diagnosticCaptureEnabled{false};
	std::atomic<std::uint64_t> m_diagnosticInvalidInputSamples{0};
	std::atomic<std::uint64_t> m_diagnosticClippedInputSamples{0};
	std::atomic<std::uint64_t> m_diagnosticClippedOutputSamples{0};
	std::atomic<float> m_diagnosticInputPeak{0.0f};
	std::atomic<float> m_diagnosticOutputPeak{0.0f};
	std::atomic<float> m_diagnosticInputMaxJump{0.0f};
	std::atomic<float> m_diagnosticOutputMaxJump{0.0f};
	std::array<float, 2> m_diagnosticPreviousInput{};
	std::array<float, 2> m_diagnosticPreviousOutput{};
};

class SpuStudioCReverb final : public IXAPO
{
public:
	SpuStudioCReverb();
	void SetActive(bool active);

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
	void ClearStateUnlocked();
	bool HasStateUnlocked() const;
	int EffectIndex(int offset) const;
	int Downsample(bool right) const;
	std::array<int, 2> Upsample() const;
	std::array<int, 2> ProcessFrame(int left, int right);

	std::vector<short> m_effectBuffer;
	std::array<std::array<short, kRingSize * 2>, 2> m_down{};
	std::array<std::array<short, kRingSize * 2>, 2> m_up{};
	std::atomic<ULONG> m_references{1};
	std::atomic<bool> m_active{false};
	std::mutex m_processMutex;
	bool m_locked = false;
	unsigned m_ringPosition = 0;
	unsigned long long m_cycle = 0;
};
#else
class SpuStudioCReverb
{
public:
	void Reset() {}
};
#endif
