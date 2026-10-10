#include "spu_reverb_xapo.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace
{
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
	for (UINT32 frame = 0; frame < frames; ++frame)
	{
		float left = inputs[0].BufferFlags == XAPO_BUFFER_SILENT ? 0.0f : input[frame * 2];
		float right = inputs[0].BufferFlags == XAPO_BUFFER_SILENT ? 0.0f : input[frame * 2 + 1];
		// Reject malformed first-buffer samples before converting them to the
		// SPU's fixed-point domain. lround(NaN/Inf) is not a safe conversion and
		// could otherwise turn a rare device-start transient into full-scale noise.
		left = std::isfinite(left) ? std::clamp(left, -1.0f, 1.0f) : 0.0f;
		right = std::isfinite(right) ? std::clamp(right, -1.0f, 1.0f) : 0.0f;
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
	}
}
