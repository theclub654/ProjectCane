#pragma once
#include <cstdint>
#include <cstddef>
#include <array>
#include <deque>
#include <vector>

using BYTE = uint8_t;
using UINT32 = uint32_t;
using HRESULT = int32_t;
constexpr HRESULT S_OK = 0, E_FAIL = -1, E_POINTER = -2, E_INVALIDARG = -3;
constexpr bool TRUE = true, FALSE = false;
inline bool FAILED(HRESULT result) { return result < 0; }
inline bool SUCCEEDED(HRESULT result) { return result >= 0; }
constexpr UINT32 WAVE_FORMAT_PCM = 1, XAUDIO2_DEFAULT_PROCESSOR = 0;
constexpr UINT32 XAUDIO2_END_OF_STREAM = 0x40, XAUDIO2_LOOP_INFINITE = 255;
constexpr float XAUDIO2_DEFAULT_FREQ_RATIO = 2.0f;
constexpr float XAUDIO2_MIN_FREQ_RATIO = 1.0f / 1024.0f;
#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif
struct IXAudio2EngineCallback
{
    virtual ~IXAudio2EngineCallback() = default;
    virtual void OnProcessingPassStart() = 0;
    virtual void OnProcessingPassEnd() = 0;
    virtual void OnCriticalError(HRESULT error) = 0;
};
struct IXAudio2VoiceCallback
{
    virtual ~IXAudio2VoiceCallback() = default;
    virtual void OnVoiceProcessingPassStart(UINT32) = 0;
    virtual void OnVoiceProcessingPassEnd() = 0;
    virtual void OnStreamEnd() = 0;
    virtual void OnBufferStart(void*) = 0;
    virtual void OnBufferEnd(void*) = 0;
    virtual void OnLoopEnd(void*) = 0;
    virtual void OnVoiceError(void*, HRESULT) = 0;
};
struct WAVEFORMATEX
{
    uint16_t wFormatTag{}, nChannels{};
    uint32_t nSamplesPerSec{}, nAvgBytesPerSec{};
    uint16_t nBlockAlign{}, wBitsPerSample{}, cbSize{};
};
struct XAUDIO2_BUFFER
{
    UINT32 Flags{}, AudioBytes{};
    const BYTE* pAudioData{};
    UINT32 PlayBegin{}, PlayLength{}, LoopBegin{}, LoopLength{}, LoopCount{};
    void* pContext{};
};
struct XAUDIO2_VOICE_STATE { void* pCurrentBufferContext{}; UINT32 BuffersQueued{}; uint64_t SamplesPlayed{}; };
struct XAUDIO2_VOICE_DETAILS { UINT32 CreationFlags{}, ActiveFlags{}, InputChannels{}, InputSampleRate{}; };
struct IUnknown { void Release() { delete this; } };
struct XAUDIO2_EFFECT_DESCRIPTOR { IUnknown* pEffect{}; bool InitialState{}; UINT32 OutputChannels{}; };
struct XAUDIO2_EFFECT_CHAIN { UINT32 EffectCount{}; XAUDIO2_EFFECT_DESCRIPTOR* pEffectDescriptors{}; };
struct IXAudio2;
struct IXAudio2SourceVoice;
struct XAUDIO2_VOICE_SENDS;
struct IXAudio2Voice
{
    struct Send { IXAudio2Voice* destination{}; std::array<float, 4> matrix{}; };
    IXAudio2* engine;
    UINT32 channels, sampleRate;
    float volume = 1;
    std::array<Send, 4> sends{};
    unsigned sendCount = 0;
    std::vector<float> inputBuffer;
    IXAudio2Voice(IXAudio2* owner, UINT32 count, UINT32 rate) : engine(owner), channels(count), sampleRate(rate) {}
    HRESULT SetVolume(float value);
    void GetVolume(float* value);
    HRESULT SetOutputMatrix(IXAudio2Voice* destination, UINT32 sourceChannels, UINT32 destinationChannels, const float* values, UINT32 = 0);
    HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS* outputs);
    void Route(float left, float right, unsigned frame, float* masterOutput);
    void GetVoiceDetails(XAUDIO2_VOICE_DETAILS* details);
};
struct IXAudio2MasteringVoice : IXAudio2Voice
{
    using IXAudio2Voice::IXAudio2Voice;
    void DestroyVoice();
};
struct XAUDIO2FX_REVERB_I3DL2_PARAMETERS { float WetDryMix = 100, DecayTime = 1.0f, DecayHFRatio = 0.5f; };
struct XAUDIO2FX_REVERB_PARAMETERS { float DecayTime = 1.0f, DecayHFRatio = 0.5f; };
inline constexpr XAUDIO2FX_REVERB_I3DL2_PARAMETERS XAUDIO2FX_I3DL2_PRESET_DEFAULT{100, 1.49f, 0.83f};
inline constexpr XAUDIO2FX_REVERB_I3DL2_PARAMETERS XAUDIO2FX_I3DL2_PRESET_SMALLROOM{100, 0.7f, 0.7f};
inline constexpr XAUDIO2FX_REVERB_I3DL2_PARAMETERS XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM{100, 1.3f, 0.7f};
inline constexpr XAUDIO2FX_REVERB_I3DL2_PARAMETERS XAUDIO2FX_I3DL2_PRESET_LARGEROOM{100, 2.3f, 0.7f};
inline constexpr XAUDIO2FX_REVERB_I3DL2_PARAMETERS XAUDIO2FX_I3DL2_PRESET_SEWERPIPE{100, 2.81f, 0.14f};
inline constexpr XAUDIO2FX_REVERB_I3DL2_PARAMETERS XAUDIO2FX_I3DL2_PRESET_ROOM{100, 0.4f, 0.83f};
inline void ReverbConvertI3DL2ToNative(const XAUDIO2FX_REVERB_I3DL2_PARAMETERS* in, XAUDIO2FX_REVERB_PARAMETERS* out, bool)
{ out->DecayTime = in->DecayTime; out->DecayHFRatio = in->DecayHFRatio; }
struct IXAudio2SubmixVoice : IXAudio2Voice
{
    struct Delay { std::vector<float> data; size_t position = 0; float filtered = 0, feedback = 0.7f; };
    std::array<Delay, 8> delays;
    float damping = 0.4f;
    UINT32 stage = 0;
    bool effect = false;
    IXAudio2SubmixVoice(IXAudio2* owner, UINT32 count, UINT32 rate);
    HRESULT SetEffectParameters(UINT32, const void* parameters, UINT32 size);
    void Mix(float left, float right, float& outLeft, float& outRight);
    void DestroyVoice();
};
struct XAUDIO2_SEND_DESCRIPTOR { UINT32 Flags{}; IXAudio2Voice* pOutputVoice{}; };
struct XAUDIO2_VOICE_SENDS { UINT32 SendCount{}; XAUDIO2_SEND_DESCRIPTOR* pSends{}; };
struct IXAudio2SourceVoice : IXAudio2Voice
{
    WAVEFORMATEX format;
    float frequency = 1, maxFrequency = 2;
    bool playing = false, bufferStarted = false;
    IXAudio2VoiceCallback* callback = nullptr;
    std::deque<XAUDIO2_BUFFER> queue;
    double position = 0, samplesPlayed = 0;
    UINT32 repeats = 0;
    IXAudio2SourceVoice(IXAudio2* owner, const WAVEFORMATEX& wave, float maxRatio);
    HRESULT Start(UINT32 = 0, UINT32 = 0);
    HRESULT Stop(UINT32 = 0, UINT32 = 0);
    HRESULT FlushSourceBuffers();
    HRESULT SubmitSourceBuffer(const XAUDIO2_BUFFER* buffer, const void* = nullptr);
    HRESULT SetFrequencyRatio(float ratio, UINT32 = 0);
    void GetState(XAUDIO2_VOICE_STATE* state, UINT32 = 0);
    void DestroyVoice();
    void Mix(float* output, int frameCount);
    bool AdvanceBuffer();
};
struct IXAudio2
{
    void* nativeDevice = nullptr;
    std::vector<IXAudio2SourceVoice*> voices;
    std::vector<IXAudio2SubmixVoice*> submixes;
    std::vector<IXAudio2EngineCallback*> callbacks;
    IXAudio2MasteringVoice* master = nullptr;
    HRESULT CreateMasteringVoice(IXAudio2MasteringVoice** out, UINT32 channels = 2, UINT32 rate = 48000);
    HRESULT CreateSubmixVoice(IXAudio2SubmixVoice** out, UINT32 channels, UINT32 rate, UINT32 = 0, UINT32 = 0, const XAUDIO2_VOICE_SENDS* = nullptr, const XAUDIO2_EFFECT_CHAIN* = nullptr);
    HRESULT CreateSourceVoice(IXAudio2SourceVoice** out, const WAVEFORMATEX* format, UINT32 = 0, float maxRatio = 2.0f, IXAudio2VoiceCallback* = nullptr, const XAUDIO2_VOICE_SENDS* = nullptr, const XAUDIO2_EFFECT_CHAIN* = nullptr);
    void Release();
    void Lock();
    void Unlock();
    HRESULT RegisterForCallbacks(IXAudio2EngineCallback* callback);
    void UnregisterForCallbacks(IXAudio2EngineCallback* callback);
    void NotifyCriticalError(HRESULT error);
    static void Callback(void* userdata, uint8_t* output, int length);
};
HRESULT XAudio2Create(IXAudio2** out, UINT32 = 0, UINT32 = 0);
inline HRESULT XAudio2CreateReverb(IUnknown** out) { if (!out) return E_POINTER; *out = new IUnknown; return S_OK; }
