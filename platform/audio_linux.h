#pragma once

// Small C++ compatibility layer over FAudio. It intentionally exposes only
// the XAudio2 surface used by ProjectCane so game-side sequencing stays shared.
#include <FAPO.h>
#include <FAudio.h>
#include <FAudioFX.h>

#include <cstdint>
#include <memory>
#include <vector>

using HRESULT = int32_t;
using UINT32 = uint32_t;
using BYTE = uint8_t;
using ULONG = uint32_t;
constexpr HRESULT S_OK = 0;
constexpr HRESULT E_FAIL = static_cast<int32_t>(0x80004005u);
constexpr HRESULT E_POINTER = static_cast<int32_t>(0x80004003u);
constexpr int TRUE = 1;
constexpr int FALSE = 0;
inline bool FAILED(HRESULT result) { return result < 0; }
inline bool SUCCEEDED(HRESULT result) { return result >= 0; }
#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif

using WAVEFORMATEX = FAudioWaveFormatEx;
using XAUDIO2_BUFFER = FAudioBuffer;
using XAUDIO2_VOICE_DETAILS = FAudioVoiceDetails;
using XAUDIO2_VOICE_STATE = FAudioVoiceState;
using XAUDIO2FX_REVERB_I3DL2_PARAMETERS = FAudioFXReverbI3DL2Parameters;
using XAUDIO2FX_REVERB_PARAMETERS = FAudioFXReverbParameters9;
constexpr auto WAVE_FORMAT_PCM = FAUDIO_FORMAT_PCM;
constexpr auto XAUDIO2_DEFAULT_PROCESSOR = FAUDIO_DEFAULT_PROCESSOR;
constexpr auto XAUDIO2_END_OF_STREAM = FAUDIO_END_OF_STREAM;
constexpr auto XAUDIO2_LOOP_INFINITE = FAUDIO_LOOP_INFINITE;
constexpr auto XAUDIO2_MIN_FREQ_RATIO = FAUDIO_MIN_FREQ_RATIO;
constexpr auto XAUDIO2_DEFAULT_FREQ_RATIO = FAUDIO_DEFAULT_FREQ_RATIO;
constexpr uint32_t XAUDIO2_MAX_AUDIO_CHANNELS = 64;
constexpr uint32_t XAUDIO2_MIN_SAMPLE_RATE = 1000;
constexpr uint32_t XAUDIO2_MAX_SAMPLE_RATE = 200000;

#define XAUDIO2FX_I3DL2_PRESET_DEFAULT FAUDIOFX_I3DL2_PRESET_DEFAULT
#define XAUDIO2FX_I3DL2_PRESET_SMALLROOM FAUDIOFX_I3DL2_PRESET_SMALLROOM
#define XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM FAUDIOFX_I3DL2_PRESET_MEDIUMROOM
#define XAUDIO2FX_I3DL2_PRESET_LARGEROOM FAUDIOFX_I3DL2_PRESET_LARGEROOM
#define XAUDIO2FX_I3DL2_PRESET_SEWERPIPE FAUDIOFX_I3DL2_PRESET_SEWERPIPE
#define XAUDIO2FX_I3DL2_PRESET_ROOM FAUDIOFX_I3DL2_PRESET_ROOM

inline void ReverbConvertI3DL2ToNative(
    const FAudioFXReverbI3DL2Parameters* source,
    FAudioFXReverbParameters9* destination, int32_t sevenDotOne)
{
    ReverbConvertI3DL2ToNative9(source, destination, sevenDotOne);
}

struct IUnknown
{
    FAPO* effect = nullptr;
    void Release()
    {
        effect->Release(effect);
        delete this;
    }
};

inline HRESULT XAudio2CreateReverb(IUnknown** result)
{
    auto wrapper = std::make_unique<IUnknown>();
    const uint32_t status = FAudioCreateReverb9(&wrapper->effect, 0);
    if (status == 0)
        *result = wrapper.release();
    return static_cast<HRESULT>(status);
}

struct IXAudio2EngineCallback
{
    virtual ~IXAudio2EngineCallback() = default;
    virtual void STDMETHODCALLTYPE OnProcessingPassStart() = 0;
    virtual void STDMETHODCALLTYPE OnProcessingPassEnd() = 0;
    virtual void STDMETHODCALLTYPE OnCriticalError(HRESULT error) = 0;
};

struct IXAudio2VoiceCallback
{
    virtual ~IXAudio2VoiceCallback() = default;
    virtual void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 bytesRequired) = 0;
    virtual void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() = 0;
    virtual void STDMETHODCALLTYPE OnStreamEnd() = 0;
    virtual void STDMETHODCALLTYPE OnBufferStart(void* bufferContext) = 0;
    virtual void STDMETHODCALLTYPE OnBufferEnd(void* bufferContext) = 0;
    virtual void STDMETHODCALLTYPE OnLoopEnd(void* bufferContext) = 0;
    virtual void STDMETHODCALLTYPE OnVoiceError(void* bufferContext, HRESULT error) = 0;
};

struct XAUDIO2_SEND_DESCRIPTOR;
struct XAUDIO2_VOICE_SENDS;

struct IXAudio2Voice
{
    FAudioVoice* handle = nullptr;
    void GetVoiceDetails(FAudioVoiceDetails* details)
    {
        FAudioVoice_GetVoiceDetails(handle, details);
    }
    HRESULT SetVolume(float volume)
    {
        return static_cast<HRESULT>(FAudioVoice_SetVolume(handle, volume, 0));
    }
    void GetVolume(float* volume)
    {
        FAudioVoice_GetVolume(handle, volume);
    }
    HRESULT SetOutputMatrix(IXAudio2Voice* destination, uint32_t sourceChannels,
        uint32_t destinationChannels, const float* matrix)
    {
        return static_cast<HRESULT>(FAudioVoice_SetOutputMatrix(handle,
            destination ? destination->handle : nullptr, sourceChannels,
            destinationChannels, matrix, 0));
    }
    HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS* sends);
    HRESULT SetEffectParameters(uint32_t index, const void* data, uint32_t size)
    {
        return static_cast<HRESULT>(
            FAudioVoice_SetEffectParameters(handle, index, data, size, 0));
    }
    void DestroyVoice()
    {
        FAudioVoice_DestroyVoice(handle);
        delete this;
    }
    virtual ~IXAudio2Voice() = default;
};

struct IXAudio2MasteringVoice : IXAudio2Voice {};
struct IXAudio2SubmixVoice : IXAudio2Voice {};
struct IXAudio2SourceVoice : IXAudio2Voice
{
    HRESULT Start() { return static_cast<HRESULT>(FAudioSourceVoice_Start(handle, 0, 0)); }
    HRESULT Stop() { return static_cast<HRESULT>(FAudioSourceVoice_Stop(handle, 0, 0)); }
    HRESULT FlushSourceBuffers()
    {
        return static_cast<HRESULT>(FAudioSourceVoice_FlushSourceBuffers(handle));
    }
    HRESULT SubmitSourceBuffer(const FAudioBuffer* buffer)
    {
        return static_cast<HRESULT>(
            FAudioSourceVoice_SubmitSourceBuffer(handle, buffer, nullptr));
    }
    HRESULT SetFrequencyRatio(float ratio)
    {
        return static_cast<HRESULT>(FAudioSourceVoice_SetFrequencyRatio(handle, ratio, 0));
    }
    void GetState(FAudioVoiceState* state)
    {
        FAudioSourceVoice_GetState(handle, state, 0);
    }
};

struct XAUDIO2_SEND_DESCRIPTOR
{
    uint32_t Flags = 0;
    IXAudio2Voice* pOutputVoice = nullptr;
};
struct XAUDIO2_VOICE_SENDS
{
    uint32_t SendCount = 0;
    XAUDIO2_SEND_DESCRIPTOR* pSends = nullptr;
};
struct XAUDIO2_EFFECT_DESCRIPTOR
{
    IUnknown* pEffect = nullptr;
    int32_t InitialState = 0;
    uint32_t OutputChannels = 0;
};
struct XAUDIO2_EFFECT_CHAIN
{
    uint32_t EffectCount = 0;
    XAUDIO2_EFFECT_DESCRIPTOR* pEffectDescriptors = nullptr;
};

inline std::vector<FAudioSendDescriptor> CaneAudioSends(const XAUDIO2_VOICE_SENDS* sends)
{
    std::vector<FAudioSendDescriptor> result;
    if (sends != nullptr)
        for (uint32_t i = 0; i < sends->SendCount; ++i)
            result.push_back({ sends->pSends[i].Flags,
                sends->pSends[i].pOutputVoice->handle });
    return result;
}

inline HRESULT IXAudio2Voice::SetOutputVoices(const XAUDIO2_VOICE_SENDS* sends)
{
    auto descriptors = CaneAudioSends(sends);
    FAudioVoiceSends list{ static_cast<uint32_t>(descriptors.size()), descriptors.data() };
    return static_cast<HRESULT>(FAudioVoice_SetOutputVoices(handle,
        sends != nullptr ? &list : nullptr));
}

struct IXAudio2
{
    FAudio* handle = nullptr;
    HRESULT RegisterForCallbacks(IXAudio2EngineCallback*) { return S_OK; }
    void UnregisterForCallbacks(IXAudio2EngineCallback*) {}
	HRESULT CreateMasteringVoice(IXAudio2MasteringVoice** result,
		uint32_t channels = 0, uint32_t sampleRate = 0)
	{
		auto voice = std::make_unique<IXAudio2MasteringVoice>();
		const uint32_t status = FAudio_CreateMasteringVoice(
			handle, &voice->handle, channels, sampleRate, 0, 0, nullptr);
        if (status == 0)
            *result = voice.release();
        return static_cast<HRESULT>(status);
    }
    HRESULT CreateSubmixVoice(IXAudio2SubmixVoice** result, uint32_t channels,
        uint32_t rate, uint32_t flags, uint32_t stage,
        const XAUDIO2_VOICE_SENDS* sends, const XAUDIO2_EFFECT_CHAIN* chain)
    {
        auto sendDescriptors = CaneAudioSends(sends);
        FAudioVoiceSends sendList{ static_cast<uint32_t>(sendDescriptors.size()),
            sendDescriptors.data() };
        std::vector<FAudioEffectDescriptor> effectDescriptors;
        if (chain != nullptr)
            for (uint32_t i = 0; i < chain->EffectCount; ++i)
            {
                const auto& effect = chain->pEffectDescriptors[i];
                effectDescriptors.push_back({ effect.pEffect->effect,
                    effect.InitialState, effect.OutputChannels });
            }
        FAudioEffectChain effectChain{ static_cast<uint32_t>(effectDescriptors.size()),
            effectDescriptors.data() };
        auto voice = std::make_unique<IXAudio2SubmixVoice>();
        const uint32_t status = FAudio_CreateSubmixVoice(handle, &voice->handle,
            channels, rate, flags, stage, sends ? &sendList : nullptr,
            chain ? &effectChain : nullptr);
        if (status == 0)
            *result = voice.release();
        return static_cast<HRESULT>(status);
    }
    HRESULT CreateSourceVoice(IXAudio2SourceVoice** result,
        const WAVEFORMATEX* format, uint32_t flags = 0, float maximum = 2.0f,
        IXAudio2VoiceCallback* = nullptr,
        const XAUDIO2_VOICE_SENDS* sends = nullptr, void* = nullptr)
    {
        auto descriptors = CaneAudioSends(sends);
        FAudioVoiceSends sendList{ static_cast<uint32_t>(descriptors.size()),
            descriptors.data() };
        auto voice = std::make_unique<IXAudio2SourceVoice>();
        const uint32_t status = FAudio_CreateSourceVoice(handle, &voice->handle,
            format, flags, maximum, nullptr, sends ? &sendList : nullptr, nullptr);
        if (status == 0)
            *result = voice.release();
        return static_cast<HRESULT>(status);
    }
    void Release()
    {
        FAudio_Release(handle);
        delete this;
    }
};

inline HRESULT XAudio2Create(IXAudio2** result, uint32_t flags, uint32_t processor)
{
    auto engine = std::make_unique<IXAudio2>();
    const uint32_t status = FAudioCreate(&engine->handle, flags, processor);
    if (status == 0)
        *result = engine.release();
    return static_cast<HRESULT>(status);
}
