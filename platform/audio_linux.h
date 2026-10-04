#pragma once
// Only the XAudio2 surface used by ProjectCane is adapted. Game sequencing,
// sample ownership, envelopes, routing, and reverb remain in sound.cpp.
#include <FAPO.h>
#include <FAudio.h>
#include <FAudioFX.h>

#include <cstdint>
#include <memory>
#include <vector>
using HRESULT = int32_t;
using UINT32 = uint32_t;
using BYTE = uint8_t;
constexpr HRESULT E_FAIL = static_cast<int32_t>(0x80004005u);
constexpr HRESULT E_POINTER = static_cast<int32_t>(0x80004003u);
constexpr int TRUE = 1, FALSE = 0;
inline bool FAILED(HRESULT r) { return r < 0; }
inline bool SUCCEEDED(HRESULT r) { return r >= 0; }
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
#define XAUDIO2FX_I3DL2_PRESET_DEFAULT FAUDIOFX_I3DL2_PRESET_DEFAULT
#define XAUDIO2FX_I3DL2_PRESET_SMALLROOM FAUDIOFX_I3DL2_PRESET_SMALLROOM
#define XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM FAUDIOFX_I3DL2_PRESET_MEDIUMROOM
#define XAUDIO2FX_I3DL2_PRESET_LARGEROOM FAUDIOFX_I3DL2_PRESET_LARGEROOM
#define XAUDIO2FX_I3DL2_PRESET_SEWERPIPE FAUDIOFX_I3DL2_PRESET_SEWERPIPE
#define XAUDIO2FX_I3DL2_PRESET_ROOM FAUDIOFX_I3DL2_PRESET_ROOM
inline void ReverbConvertI3DL2ToNative(const FAudioFXReverbI3DL2Parameters* p,
                                       FAudioFXReverbParameters9* native, int32_t seven) {
    ReverbConvertI3DL2ToNative9(p, native, seven);
}
struct IUnknown {
    FAPO* effect = nullptr;
    void Release() {
        effect->Release(effect);
        delete this;
    }
};
inline HRESULT XAudio2CreateReverb(IUnknown** result) {
    auto wrapper = std::make_unique<IUnknown>();
    auto r = FAudioCreateReverb9(&wrapper->effect, 0);
    if (!r) *result = wrapper.release();
    return static_cast<HRESULT>(r);
}
struct CaneAudioVoice {
    FAudioVoice* handle = nullptr;
    void GetVoiceDetails(FAudioVoiceDetails* d) { FAudioVoice_GetVoiceDetails(handle, d); }
    HRESULT SetVolume(float v) { return static_cast<HRESULT>(FAudioVoice_SetVolume(handle, v, 0)); }
    HRESULT SetOutputMatrix(CaneAudioVoice* destination, uint32_t source, uint32_t channels,
                            const float* matrix) {
        return static_cast<HRESULT>(FAudioVoice_SetOutputMatrix(
            handle, destination ? destination->handle : nullptr, source, channels, matrix, 0));
    }
    HRESULT SetEffectParameters(uint32_t index, const void* data, uint32_t size) {
        return static_cast<HRESULT>(FAudioVoice_SetEffectParameters(handle, index, data, size, 0));
    }
    void DestroyVoice() {
        FAudioVoice_DestroyVoice(handle);
        delete this;
    }
    virtual ~CaneAudioVoice() = default;
};
struct IXAudio2MasteringVoice : CaneAudioVoice {};
struct IXAudio2SubmixVoice : CaneAudioVoice {};
struct IXAudio2SourceVoice : CaneAudioVoice {
    HRESULT Start() { return static_cast<HRESULT>(FAudioSourceVoice_Start(handle, 0, 0)); }
    HRESULT Stop() { return static_cast<HRESULT>(FAudioSourceVoice_Stop(handle, 0, 0)); }
    HRESULT FlushSourceBuffers() {
        return static_cast<HRESULT>(FAudioSourceVoice_FlushSourceBuffers(handle));
    }
    HRESULT SubmitSourceBuffer(const FAudioBuffer* buffer) {
        return static_cast<HRESULT>(FAudioSourceVoice_SubmitSourceBuffer(handle, buffer, nullptr));
    }
    HRESULT SetFrequencyRatio(float ratio) {
        return static_cast<HRESULT>(FAudioSourceVoice_SetFrequencyRatio(handle, ratio, 0));
    }
    void GetState(FAudioVoiceState* state) { FAudioSourceVoice_GetState(handle, state, 0); }
};
struct XAUDIO2_SEND_DESCRIPTOR {
    uint32_t Flags = 0;
    CaneAudioVoice* pOutputVoice = nullptr;
};
struct XAUDIO2_VOICE_SENDS {
    uint32_t SendCount = 0;
    XAUDIO2_SEND_DESCRIPTOR* pSends = nullptr;
};
struct XAUDIO2_EFFECT_DESCRIPTOR {
    IUnknown* pEffect = nullptr;
    int32_t InitialState = 0;
    uint32_t OutputChannels = 0;
};
struct XAUDIO2_EFFECT_CHAIN {
    uint32_t EffectCount = 0;
    XAUDIO2_EFFECT_DESCRIPTOR* pEffectDescriptors = nullptr;
};
struct IXAudio2 {
    FAudio* handle = nullptr;
    HRESULT CreateMasteringVoice(IXAudio2MasteringVoice** result) {
        auto voice = std::make_unique<IXAudio2MasteringVoice>();
        auto r = FAudio_CreateMasteringVoice(handle, &voice->handle, 2, 48000, 0, 0, nullptr);
        if (!r) *result = voice.release();
        return static_cast<HRESULT>(r);
    }
    HRESULT CreateSubmixVoice(IXAudio2SubmixVoice** result, uint32_t channels, uint32_t rate,
                              uint32_t flags, uint32_t stage, const XAUDIO2_VOICE_SENDS*,
                              const XAUDIO2_EFFECT_CHAIN* chain) {
        std::vector<FAudioEffectDescriptor> descriptors;
        if (chain)
            for (uint32_t i = 0; i < chain->EffectCount; ++i) {
                auto& e = chain->pEffectDescriptors[i];
                descriptors.push_back({e.pEffect->effect, e.InitialState, e.OutputChannels});
            }
        FAudioEffectChain effects{static_cast<uint32_t>(descriptors.size()), descriptors.data()};
        auto voice = std::make_unique<IXAudio2SubmixVoice>();
        auto r = FAudio_CreateSubmixVoice(handle, &voice->handle, channels, rate, flags, stage,
                                          nullptr, chain ? &effects : nullptr);
        if (!r) *result = voice.release();
        return static_cast<HRESULT>(r);
    }
    HRESULT CreateSourceVoice(IXAudio2SourceVoice** result, const WAVEFORMATEX* format,
                              uint32_t flags = 0, float maximum = 2.0f, void* = nullptr,
                              const XAUDIO2_VOICE_SENDS* sends = nullptr, void* = nullptr) {
        std::vector<FAudioSendDescriptor> descriptors;
        if (sends)
            for (uint32_t i = 0; i < sends->SendCount; ++i)
                descriptors.push_back(
                    {sends->pSends[i].Flags, sends->pSends[i].pOutputVoice->handle});
        FAudioVoiceSends sendList{static_cast<uint32_t>(descriptors.size()), descriptors.data()};
        auto voice = std::make_unique<IXAudio2SourceVoice>();
        auto r = FAudio_CreateSourceVoice(handle, &voice->handle, format, flags, maximum, nullptr,
                                          sends ? &sendList : nullptr, nullptr);
        if (!r) *result = voice.release();
        return static_cast<HRESULT>(r);
    }
    void Release() {
        FAudio_Release(handle);
        delete this;
    }
};
inline HRESULT XAudio2Create(IXAudio2** result, uint32_t flags, uint32_t processor) {
    auto engine = std::make_unique<IXAudio2>();
    auto r = FAudioCreate(&engine->handle, flags, processor);
    if (!r) *result = engine.release();
    return static_cast<HRESULT>(r);
}
