#include "spu_reverb_xapo.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>

namespace
{
	std::mutex s_traceMutex;
	std::ofstream s_traceFile;
	bool s_traceActive = false;
	uint64_t s_traceFrame = 0;
	int s_traceReverbKind = -1;
	int s_traceReverbDepth = -1;
	float s_traceDesiredWet = 0.0f;
	float s_traceActualWet = 0.0f;
	float s_traceWetBusVolume = 0.0f;
	bool s_traceFeedStudio = false;
	bool s_traceFeedGeneric = false;
	constexpr uint64_t kTraceFrameLimit = 48000ULL * 20ULL;

	void WriteTraceEventLocked(const char* eventName, int kind, int depth)
	{
		if (!s_traceActive || !s_traceFile)
			return;
		s_traceFile << s_traceFrame << ",event," << eventName << ',' << kind << ',' << depth
			<< ",0,0,0,0,0,0,0,0,0,0," << s_traceDesiredWet << ',' << s_traceActualWet << ','
			<< static_cast<int>(s_traceFeedStudio) << ',' << static_cast<int>(s_traceFeedGeneric) << ','
			<< s_traceWetBusVolume << ",0,0,0,0,0\n";
		s_traceFile.flush();
	}

	constexpr std::array<short, 39> kDownFilter = {
		-1, 0, 2, 0, -10, 0, 35, 0, -103, 0, 266, 0, -616, 0,
		1332, 0, -2960, 0, 10246, 16384, 10246, 0, -2960, 0, 1332, 0,
		-616, 0, 266, 0, -103, 0, 35, 0, -10, 0, 2, 0, -1
	};

	XAPO_REGISTRATION_PROPERTIES MakeRegistration()
	{
		XAPO_REGISTRATION_PROPERTIES properties{};
		properties.clsid = {0xe3508fb4, 0x71cb, 0x4573, {0xa5, 0x91, 0x92, 0xc4, 0x36, 0xdf, 0x72, 0x0d}};
		wcscpy_s(properties.FriendlyName, L"ProjectCane SPU2 Studio C Reverb");
		wcscpy_s(properties.CopyrightInfo, L"ProjectCane");
		properties.MajorVersion = 1;
		properties.MinorVersion = 0;
		properties.Flags = XAPO_FLAG_CHANNELS_MUST_MATCH |
			XAPO_FLAG_FRAMERATE_MUST_MATCH | XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH |
			XAPO_FLAG_BUFFERCOUNT_MUST_MATCH | XAPO_FLAG_INPLACE_SUPPORTED;
		properties.MinInputBufferCount = properties.MaxInputBufferCount = 1;
		properties.MinOutputBufferCount = properties.MaxOutputBufferCount = 1;
		return properties;
	}

	const XAPO_REGISTRATION_PROPERTIES kRegistration = MakeRegistration();
	constexpr GUID kIeeeFloatSubFormat =
		{WAVE_FORMAT_IEEE_FLOAT, 0x0000, 0x0010,
			{0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};

	bool IsStereoFloat48k(const WAVEFORMATEX* format)
	{
		if (!format || format->nChannels != 2 || format->nSamplesPerSec != 48000 ||
			format->wBitsPerSample != 32)
		{
			return false;
		}

		if (format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
			return true;

		if (format->wFormatTag != WAVE_FORMAT_EXTENSIBLE ||
			format->cbSize < sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))
		{
			return false;
		}

		const auto* extensible = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(format);
		return extensible->Samples.wValidBitsPerSample == 32 &&
			IsEqualGUID(extensible->SubFormat, kIeeeFloatSubFormat);
	}
}

void ToggleLevelTransitionAudioTrace()
{
	const std::lock_guard<std::mutex> lock(s_traceMutex);
	if (s_traceActive)
	{
		WriteTraceEventLocked("manual_stop", -1, -1);
		s_traceFile.close();
		s_traceActive = false;
		std::printf("[Audio trace] stopped projectcane_level_transition_audio.csv\n");
		return;
	}

	s_traceFile.open("projectcane_level_transition_audio.csv", std::ios::out | std::ios::trunc);
	if (!s_traceFile)
	{
		std::printf("[ERROR] [Audio trace] could not create projectcane_level_transition_audio.csv\n");
		return;
	}
	s_traceFile << "frame,record,event,reverb_kind,reverb_depth,frames,in_peak_l,in_peak_r,in_rms_l,in_rms_r,out_peak_l,out_peak_r,out_rms_l,out_rms_r,clipped,desired_wet,actual_wet,feed_studio,feed_generic,wet_bus_volume,post_gain_peak_l,post_gain_peak_r,post_gain_rms_l,post_gain_rms_r,sound_details\n";
	s_traceActive = true;
	s_traceFrame = 0;
	WriteTraceEventLocked("manual_start", -1, -1);
	std::printf("[Audio trace] recording projectcane_level_transition_audio.csv (20 seconds max)\n");
}

void StopLevelTransitionAudioTrace()
{
	const std::lock_guard<std::mutex> lock(s_traceMutex);
	if (!s_traceActive)
		return;
	WriteTraceEventLocked("manual_stop", -1, -1);
	s_traceFile.close();
	s_traceActive = false;
}

void MarkLevelTransitionAudioTrace(const char* eventName, const int reverbKind, const int reverbDepth)
{
	const std::lock_guard<std::mutex> lock(s_traceMutex);
	if (reverbKind >= 0)
		s_traceReverbKind = reverbKind;
	if (reverbDepth >= 0)
		s_traceReverbDepth = reverbDepth;
	WriteTraceEventLocked(eventName ? eventName : "marker", reverbKind, reverbDepth);
}

void UpdateLevelTransitionAudioTraceRouting(const float desiredWet, const float actualWet,
	const bool feedStudio, const bool feedGeneric, const float wetBusVolume)
{
	const std::lock_guard<std::mutex> lock(s_traceMutex);
	s_traceDesiredWet = desiredWet;
	s_traceActualWet = actualWet;
	s_traceFeedStudio = feedStudio;
	s_traceFeedGeneric = feedGeneric;
	s_traceWetBusVolume = wetBusVolume;
	WriteTraceEventLocked("routing_update", s_traceReverbKind, s_traceReverbDepth);
}

void MarkLevelTransitionSoundStart(const int sfxid, const float volume, const float frequency,
	const bool attached, const bool continuous)
{
	char event[160]{};
	std::snprintf(event, sizeof(event), "start_sound:id=%d vol=%.6g freq=%.6g attached=%d continuous=%d",
		sfxid, volume, frequency, attached ? 1 : 0, continuous ? 1 : 0);
	MarkLevelTransitionAudioTrace(event);
}

SpuStudioCReverb::SpuStudioCReverb()
	: m_effectBuffer(kEffectBufferSize, 0)
{
}

HRESULT SpuStudioCReverb::QueryInterface(const REFIID interfaceId, void** result)
{
	if (!result)
		return E_POINTER;
	if (interfaceId == __uuidof(IUnknown) || interfaceId == __uuidof(IXAPO))
	{
		*result = static_cast<IXAPO*>(this);
		AddRef();
		return S_OK;
	}
	*result = nullptr;
	return E_NOINTERFACE;
}

ULONG SpuStudioCReverb::AddRef()
{
	return ++m_references;
}

ULONG SpuStudioCReverb::Release()
{
	const ULONG references = --m_references;
	if (references == 0)
		delete this;
	return references;
}

HRESULT SpuStudioCReverb::GetRegistrationProperties(XAPO_REGISTRATION_PROPERTIES** properties)
{
	if (!properties)
		return E_POINTER;
	auto* copy = static_cast<XAPO_REGISTRATION_PROPERTIES*>(XAPOAlloc(sizeof(kRegistration)));
	if (!copy)
		return E_OUTOFMEMORY;
	*copy = kRegistration;
	*properties = copy;
	return S_OK;
}

HRESULT SpuStudioCReverb::IsInputFormatSupported(const WAVEFORMATEX* outputFormat,
	const WAVEFORMATEX* requestedInputFormat, WAVEFORMATEX** supportedInputFormat)
{
	if (supportedInputFormat)
		*supportedInputFormat = nullptr;
	return IsStereoFloat48k(outputFormat) && IsStereoFloat48k(requestedInputFormat)
		? S_OK : XAPO_E_FORMAT_UNSUPPORTED;
}

HRESULT SpuStudioCReverb::IsOutputFormatSupported(const WAVEFORMATEX* inputFormat,
	const WAVEFORMATEX* requestedOutputFormat, WAVEFORMATEX** supportedOutputFormat)
{
	if (supportedOutputFormat)
		*supportedOutputFormat = nullptr;
	return IsStereoFloat48k(inputFormat) && IsStereoFloat48k(requestedOutputFormat)
		? S_OK : XAPO_E_FORMAT_UNSUPPORTED;
}

HRESULT SpuStudioCReverb::Initialize(const void*, const UINT32)
{
	return S_OK;
}

HRESULT SpuStudioCReverb::LockForProcess(const UINT32 inputCount,
	const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* inputs, const UINT32 outputCount,
	const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* outputs)
{
	if (m_locked || inputCount != 1 || outputCount != 1 || !inputs || !outputs)
		return E_INVALIDARG;
	if (!IsStereoFloat48k(inputs[0].pFormat) || !IsStereoFloat48k(outputs[0].pFormat))
		return XAPO_E_FORMAT_UNSUPPORTED;
	m_locked = true;
	Reset();
	return S_OK;
}

void SpuStudioCReverb::UnlockForProcess()
{
	m_locked = false;
}

UINT32 SpuStudioCReverb::CalcInputFrames(const UINT32 outputFrames)
{
	return outputFrames;
}

UINT32 SpuStudioCReverb::CalcOutputFrames(const UINT32 inputFrames)
{
	return inputFrames;
}

void SpuStudioCReverb::Reset()
{
	MarkLevelTransitionAudioTrace("studio_c_reset");
	const std::lock_guard<std::mutex> lock(m_processMutex);
	std::fill(m_effectBuffer.begin(), m_effectBuffer.end(), 0);
	m_down = {};
	m_up = {};
	m_ringPosition = 0;
	m_cycle = 0;
}

int SpuStudioCReverb::Clamp16(const int64_t value)
{
	return static_cast<int>(std::clamp<int64_t>(value, -32768, 32767));
}

int64_t SpuStudioCReverb::MulQ15(const int64_t left, const int64_t right)
{
	return (left * right) >> 15;
}

int SpuStudioCReverb::EffectIndex(const int offset) const
{
	const long long value = static_cast<long long>(m_cycle >> 1) + offset;
	const long long wrapped = value % kEffectBufferSize;
	return static_cast<int>(wrapped < 0 ? wrapped + kEffectBufferSize : wrapped);
}

int SpuStudioCReverb::Downsample(const bool right) const
{
	const int index = (static_cast<int>(m_ringPosition) - kTapCount) & 63;
	int64_t value = 0;
	for (int tap = 0; tap < kTapCount; ++tap)
		value += m_down[right ? 1 : 0][index + tap] * kDownFilter[tap];
	return Clamp16(value >> 15);
}

std::array<int, 2> SpuStudioCReverb::Upsample() const
{
	const int index = (static_cast<int>(m_ringPosition) - kTapCount) & 63;
	std::array<int, 2> result{};
	for (int channel = 0; channel < 2; ++channel)
	{
		int64_t value = 0;
		for (int tap = 0; tap < kTapCount; ++tap)
		{
			const int coefficient = Clamp16(kDownFilter[tap] * 2);
			value += m_up[channel][index + tap] * coefficient;
		}
		result[channel] = Clamp16(value >> 15);
	}
	return result;
}

std::array<int, 2> SpuStudioCReverb::ProcessFrame(const int left, const int right)
{
	const short inputLeft = static_cast<short>(Clamp16(left));
	const short inputRight = static_cast<short>(Clamp16(right));
	m_down[0][m_ringPosition] = m_down[0][m_ringPosition | 64] = inputLeft;
	m_down[1][m_ringPosition] = m_down[1][m_ringPosition | 64] = inputRight;

	const bool processRight = (m_cycle & 1) != 0;
	const int sameSource = EffectIndex(processRight ? 9064 : 11620);
	const int sameDestination = EffectIndex(processRight ? 11616 : 14316);
	const int samePrevious = EffectIndex((processRight ? 11616 : 14316) - 1);
	const int diffSource = EffectIndex(processRight ? 3188 : 6056);
	const int diffDestination = EffectIndex(processRight ? 6052 : 9060);
	const int diffPrevious = EffectIndex((processRight ? 6052 : 9060) - 1);
	const int comb1Source = EffectIndex(processRight ? 10480 : 13348);
	const int comb2Source = EffectIndex(processRight ? 9676 : 12132);
	const int comb3Source = EffectIndex(processRight ? 4800 : 8112);
	const int comb4Source = EffectIndex(processRight ? 3912 : 7100);
	const int apf1DestinationOffset = processRight ? 2272 : 3184;
	const int apf2DestinationOffset = processRight ? 680 : 1360;
	const int apf1Source = EffectIndex(apf1DestinationOffset - 908);
	const int apf1Destination = EffectIndex(apf1DestinationOffset);
	const int apf2Source = EffectIndex(apf2DestinationOffset - 676);
	const int apf2Destination = EffectIndex(apf2DestinationOffset);

	const int64_t filteredInput = MulQ15(-32768, Downsample(processRight));
	const int64_t same = MulQ15(28512, filteredInput + MulQ15(-22912, m_effectBuffer[sameSource]) -
		m_effectBuffer[samePrevious]) + m_effectBuffer[samePrevious];
	const int64_t diff = MulQ15(28512, filteredInput + MulQ15(-22912, m_effectBuffer[diffSource]) -
		m_effectBuffer[diffPrevious]) + m_effectBuffer[diffPrevious];
	int64_t output = MulQ15(20392, m_effectBuffer[comb1Source]) +
		MulQ15(-17184, m_effectBuffer[comb2Source]) +
		MulQ15(17680, m_effectBuffer[comb3Source]) +
		MulQ15(-16656, m_effectBuffer[comb4Source]);
	const int64_t apf1 = output - MulQ15(22144, m_effectBuffer[apf1Source]);
	output = m_effectBuffer[apf1Source] + MulQ15(22144, apf1);
	const int64_t apf2 = output - MulQ15(21184, m_effectBuffer[apf2Source]);
	output = m_effectBuffer[apf2Source] + MulQ15(21184, apf2);

	m_effectBuffer[sameDestination] = static_cast<short>(Clamp16(same));
	m_effectBuffer[diffDestination] = static_cast<short>(Clamp16(diff));
	m_effectBuffer[apf1Destination] = static_cast<short>(Clamp16(apf1));
	m_effectBuffer[apf2Destination] = static_cast<short>(Clamp16(apf2));
	output = Clamp16(output);
	m_up[processRight ? 1 : 0][m_ringPosition] = static_cast<short>(output);
	m_up[processRight ? 0 : 1][m_ringPosition] = 0;
	m_up[processRight ? 1 : 0][m_ringPosition | 64] = static_cast<short>(output);
	m_up[processRight ? 0 : 1][m_ringPosition | 64] = 0;
	m_ringPosition = (m_ringPosition + 1) & 63;
	++m_cycle;
	return Upsample();
}

void SpuStudioCReverb::Process(UINT32 inputCount,
	const XAPO_PROCESS_BUFFER_PARAMETERS* inputs, UINT32 outputCount,
	XAPO_PROCESS_BUFFER_PARAMETERS* outputs, const BOOL enabled)
{
	if (inputCount != 1 || outputCount != 1)
		return;
	const std::lock_guard<std::mutex> lock(m_processMutex);
	const UINT32 frames = inputs[0].ValidFrameCount;
	auto* input = static_cast<const float*>(inputs[0].pBuffer);
	auto* output = static_cast<float*>(outputs[0].pBuffer);
	outputs[0].ValidFrameCount = frames;
	outputs[0].BufferFlags = XAPO_BUFFER_VALID;
	double inputSquares[2]{};
	double outputSquares[2]{};
	float inputPeaks[2]{};
	float outputPeaks[2]{};
	uint32_t clipped = 0;
	for (UINT32 frame = 0; frame < frames; ++frame)
	{
		float left = inputs[0].BufferFlags == XAPO_BUFFER_SILENT ? 0.0f : input[frame * 2];
		float right = inputs[0].BufferFlags == XAPO_BUFFER_SILENT ? 0.0f : input[frame * 2 + 1];
		// Reject malformed first-buffer samples before converting them to the
		// SPU's fixed-point domain. lround(NaN/Inf) is not a safe conversion and
		// could otherwise turn a rare device-start transient into full-scale noise.
		left = std::isfinite(left) ? std::clamp(left, -1.0f, 1.0f) : 0.0f;
		right = std::isfinite(right) ? std::clamp(right, -1.0f, 1.0f) : 0.0f;
		inputPeaks[0] = (std::max)(inputPeaks[0], std::abs(left));
		inputPeaks[1] = (std::max)(inputPeaks[1], std::abs(right));
		inputSquares[0] += static_cast<double>(left) * left;
		inputSquares[1] += static_cast<double>(right) * right;
		if (!enabled)
		{
			output[frame * 2] = left;
			output[frame * 2 + 1] = right;
		}
		else
		{
			const auto wet = ProcessFrame(static_cast<int>(std::lround(left * 32768.0f)),
				static_cast<int>(std::lround(right * 32768.0f)));
			output[frame * 2] = std::clamp(wet[0] / 32768.0f, -1.0f, 1.0f);
			output[frame * 2 + 1] = std::clamp(wet[1] / 32768.0f, -1.0f, 1.0f);
		}
		const float outLeft = output[frame * 2];
		const float outRight = output[frame * 2 + 1];
		outputPeaks[0] = (std::max)(outputPeaks[0], std::abs(outLeft));
		outputPeaks[1] = (std::max)(outputPeaks[1], std::abs(outRight));
		outputSquares[0] += static_cast<double>(outLeft) * outLeft;
		outputSquares[1] += static_cast<double>(outRight) * outRight;
		clipped += static_cast<uint32_t>(std::abs(outLeft) >= 0.999f);
		clipped += static_cast<uint32_t>(std::abs(outRight) >= 0.999f);
	}

	{
		const std::lock_guard<std::mutex> traceLock(s_traceMutex);
		if (s_traceActive && s_traceFile)
		{
			const double divisor = frames ? static_cast<double>(frames) : 1.0;
			const float postPeakLeft = outputPeaks[0] * s_traceActualWet;
			const float postPeakRight = outputPeaks[1] * s_traceActualWet;
			s_traceFile << s_traceFrame << ",block,studio_c," << s_traceReverbKind << ','
				<< s_traceReverbDepth << ',' << frames << ','
				<< inputPeaks[0] << ',' << inputPeaks[1] << ','
				<< std::sqrt(inputSquares[0] / divisor) << ',' << std::sqrt(inputSquares[1] / divisor) << ','
				<< outputPeaks[0] << ',' << outputPeaks[1] << ','
				<< std::sqrt(outputSquares[0] / divisor) << ',' << std::sqrt(outputSquares[1] / divisor) << ','
				<< clipped << ',' << s_traceDesiredWet << ',' << s_traceActualWet << ','
				<< static_cast<int>(s_traceFeedStudio) << ',' << static_cast<int>(s_traceFeedGeneric) << ','
				<< s_traceWetBusVolume << ',' << postPeakLeft << ',' << postPeakRight << ','
				<< std::sqrt(outputSquares[0] / divisor) * s_traceActualWet << ','
				<< std::sqrt(outputSquares[1] / divisor) * s_traceActualWet << ",\n";
			s_traceFrame += frames;
			if (s_traceFrame >= kTraceFrameLimit)
			{
				WriteTraceEventLocked("automatic_stop", -1, -1);
				s_traceFile.close();
				s_traceActive = false;
			}
		}
	}
}
