#include "audio.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>

namespace
{
struct AudioLock
{
    IXAudio2* engine;
    explicit AudioLock(IXAudio2* owner) : engine(owner) { engine->Lock(); }
    ~AudioLock() { engine->Unlock(); }
};
float ReadSample(const IXAudio2SourceVoice& voice, const XAUDIO2_BUFFER& buffer, UINT32 frame, UINT32 channel)
{
    const BYTE* p = buffer.pAudioData + size_t(frame) * voice.format.nBlockAlign + channel * (voice.format.wBitsPerSample / 8);
    if (voice.format.wFormatTag == 3) { float value; std::memcpy(&value, p, 4); return std::isfinite(value) ? value : 0; }
    switch (voice.format.wBitsPerSample)
    {
        case 8: return (int(*p) - 128) / 128.0f;
        case 16: { int16_t value; std::memcpy(&value, p, 2); return value / 32768.0f; }
        case 24: { int32_t value = int32_t(p[0]) | (int32_t(p[1]) << 8) | (int32_t(p[2]) << 16); if (value & 0x800000) value |= int32_t(0xff000000); return value / 8388608.0f; }
        case 32: { int32_t value; std::memcpy(&value, p, 4); return value / 2147483648.0f; }
        default: return 0;
    }
}
}

HRESULT IXAudio2Voice::SetVolume(float value)
{
    if (!std::isfinite(value) || value < 0) return E_INVALIDARG;
    AudioLock lock(engine); volume = value; return S_OK;
}
void IXAudio2Voice::GetVolume(float* value)
{
    if (!value) return;
    AudioLock lock(engine); *value = volume;
}
HRESULT IXAudio2Voice::SetOutputVoices(const XAUDIO2_VOICE_SENDS* outputs)
{
    std::array<Send, 4> replacement{};
    const unsigned count = outputs ? outputs->SendCount : 1;
    if (count > replacement.size() || (outputs && count && !outputs->pSends)) return E_INVALIDARG;
    AudioLock lock(engine);
    for (unsigned i = 0; i < count; ++i)
    {
        auto* destination = outputs ? outputs->pSends[i].pOutputVoice : engine->master;
        if (!destination || destination->engine != engine || destination == this ||
            (destination != engine->master && std::find(engine->submixes.begin(), engine->submixes.end(), destination) == engine->submixes.end())) return E_INVALIDARG;
        replacement[i].destination = destination;
        replacement[i].matrix = channels == 1 ? std::array<float,4>{1,1,0,0} : std::array<float,4>{1,0,0,1};
        for (unsigned j = 0; j < sendCount; ++j)
            if (sends[j].destination == destination) replacement[i].matrix = sends[j].matrix;
    }
    sends = replacement;
    sendCount = count;
    return S_OK;
}
void IXAudio2Voice::Route(float left, float right, unsigned frame, float* output)
{
    for (unsigned i = 0; i < sendCount; ++i)
    {
        const auto& send = sends[i];
        float* buffer = send.destination == engine->master ? output : send.destination->inputBuffer.data();
        buffer[frame * 2] += left * send.matrix[0] + right * send.matrix[2];
        buffer[frame * 2 + 1] += left * send.matrix[1] + right * send.matrix[3];
    }
}
HRESULT IXAudio2::RegisterForCallbacks(IXAudio2EngineCallback* callback)
{
    if (!callback) return E_POINTER;
    AudioLock lock(this);
    if (std::find(callbacks.begin(), callbacks.end(), callback) == callbacks.end()) callbacks.push_back(callback);
    return S_OK;
}
void IXAudio2::UnregisterForCallbacks(IXAudio2EngineCallback* callback)
{
    AudioLock lock(this);
    std::erase(callbacks, callback);
}
void IXAudio2::NotifyCriticalError(HRESULT error)
{
    AudioLock lock(this);
    for (auto* callback : callbacks) callback->OnCriticalError(error);
}
void IXAudio2Voice::GetVoiceDetails(XAUDIO2_VOICE_DETAILS* out)
{ if (out) { *out = {}; out->InputChannels = channels; out->InputSampleRate = sampleRate; } }
void IXAudio2MasteringVoice::DestroyVoice()
{ AudioLock lock(engine); engine->master = nullptr; delete this; }

IXAudio2SubmixVoice::IXAudio2SubmixVoice(IXAudio2* owner, UINT32 count, UINT32 rate) : IXAudio2Voice(owner, count, rate)
{
    constexpr size_t lengths[] = {1557, 1617, 1491, 1422, 1580, 1640, 1514, 1445};
    for (size_t i = 0; i < delays.size(); ++i) delays[i].data.resize(lengths[i]);
}
HRESULT IXAudio2SubmixVoice::SetEffectParameters(UINT32, const void* parameters, UINT32 size)
{
    if (!parameters || size != sizeof(XAUDIO2FX_REVERB_PARAMETERS)) return E_INVALIDARG;
    const auto& settings = *static_cast<const XAUDIO2FX_REVERB_PARAMETERS*>(parameters);
    AudioLock lock(engine);
    damping = std::clamp(1.0f - settings.DecayHFRatio, 0.05f, 0.95f);
    for (auto& delay : delays)
        delay.feedback = std::pow(0.001f, float(delay.data.size()) / (48000 * std::max(0.1f, settings.DecayTime)));
    return S_OK;
}
void IXAudio2SubmixVoice::Mix(float left, float right, float& outLeft, float& outRight)
{
    const float input = (left + right) * 0.125f;
    float sums[2] = {};
    for (size_t i = 0; i < delays.size(); ++i)
    {
        auto& delay = delays[i];
        const float tail = delay.data[delay.position];
        delay.filtered = tail * (1 - damping) + delay.filtered * damping;
        delay.data[delay.position] = input + delay.filtered * delay.feedback;
        if (++delay.position == delay.data.size()) delay.position = 0;
        sums[i / 4] += tail;
    }
    outLeft += sums[0] * volume;
    outRight += sums[1] * volume;
}
void IXAudio2SubmixVoice::DestroyVoice()
{
    AudioLock lock(engine);
    for (auto* source : engine->voices)
        for (unsigned i = source->sendCount; i-- > 0;)
            if (source->sends[i].destination == this) source->sends[i] = source->sends[--source->sendCount];
    for (auto* submix : engine->submixes)
        for (unsigned i = submix->sendCount; i-- > 0;)
            if (submix->sends[i].destination == this) submix->sends[i] = submix->sends[--submix->sendCount];
    std::erase(engine->submixes, this);
    delete this;
}

IXAudio2SourceVoice::IXAudio2SourceVoice(IXAudio2* owner, const WAVEFORMATEX& wave, float maxRatio)
    : IXAudio2Voice(owner, wave.nChannels, wave.nSamplesPerSec), format(wave), maxFrequency(maxRatio)
{

}
HRESULT IXAudio2SourceVoice::Start(UINT32, UINT32) { AudioLock lock(engine); playing = true; return S_OK; }
HRESULT IXAudio2SourceVoice::Stop(UINT32, UINT32) { AudioLock lock(engine); playing = false; return S_OK; }
HRESULT IXAudio2SourceVoice::FlushSourceBuffers()
{ AudioLock lock(engine); queue.clear(); position = samplesPlayed = 0; repeats = 0; bufferStarted = false; return S_OK; }
HRESULT IXAudio2SourceVoice::SubmitSourceBuffer(const XAUDIO2_BUFFER* buffer, const void*)
{
    if (!buffer || !buffer->pAudioData || !buffer->AudioBytes || buffer->AudioBytes % format.nBlockAlign) return E_INVALIDARG;
    const UINT32 total = buffer->AudioBytes / format.nBlockAlign;
    const UINT32 end = buffer->PlayLength ? buffer->PlayBegin + buffer->PlayLength : total;
    if (buffer->PlayBegin >= total || end > total || end <= buffer->PlayBegin) return E_INVALIDARG;
    if (buffer->LoopCount && (buffer->LoopBegin < buffer->PlayBegin || buffer->LoopBegin >= end ||
        (buffer->LoopLength && buffer->LoopLength > end - buffer->LoopBegin))) return E_INVALIDARG;
    AudioLock lock(engine);
    if (queue.empty()) { position = buffer->PlayBegin; repeats = buffer->LoopCount; }
    queue.push_back(*buffer);
    return S_OK;
}
HRESULT IXAudio2SourceVoice::SetFrequencyRatio(float ratio, UINT32)
{
    if (!std::isfinite(ratio) || ratio < XAUDIO2_MIN_FREQ_RATIO || ratio > maxFrequency) return E_INVALIDARG;
    AudioLock lock(engine); frequency = ratio; return S_OK;
}
HRESULT IXAudio2Voice::SetOutputMatrix(IXAudio2Voice* destination, UINT32 sourceCount, UINT32 destinationCount, const float* values, UINT32)
{
    if (!values || sourceCount != channels || destinationCount != 2) return E_INVALIDARG;
    AudioLock lock(engine);
    if (!destination) destination = engine->master;
    for (unsigned i = 0; i < sendCount; ++i)
        if (sends[i].destination == destination)
        {
            for (UINT32 j = 0; j < sourceCount * destinationCount; ++j)
                if (!std::isfinite(values[j])) return E_INVALIDARG;
            std::copy_n(values, sourceCount * destinationCount, sends[i].matrix.begin());
            return S_OK;
        }
    return E_INVALIDARG;
}
void IXAudio2SourceVoice::GetState(XAUDIO2_VOICE_STATE* state, UINT32)
{
    if (!state) return;
    AudioLock lock(engine);
    state->BuffersQueued = UINT32(queue.size());
    state->SamplesPlayed = uint64_t(samplesPlayed);
    state->pCurrentBufferContext = queue.empty() ? nullptr : queue.front().pContext;
}
void IXAudio2SourceVoice::DestroyVoice()
{
    AudioLock lock(engine);
    auto& voices = engine->voices;
    voices.erase(std::remove(voices.begin(), voices.end(), this), voices.end());
    delete this;
}
bool IXAudio2SourceVoice::AdvanceBuffer()
{
    while (!queue.empty())
    {
        const auto& buffer = queue.front();
        const UINT32 end = buffer.PlayLength ? buffer.PlayBegin + buffer.PlayLength : buffer.AudioBytes / format.nBlockAlign;
        const UINT32 loopEnd = buffer.LoopLength ? buffer.LoopBegin + buffer.LoopLength : end;
        if (repeats && position >= loopEnd)
        {
            const UINT32 length = loopEnd - buffer.LoopBegin;
            const double crossed = std::floor((position - buffer.LoopBegin) / length);
            const double wraps = repeats == XAUDIO2_LOOP_INFINITE ? crossed : std::min(crossed, double(repeats));
            position -= wraps * length;
            if (callback && wraps > 0) callback->OnLoopEnd(buffer.pContext);
            if (repeats != XAUDIO2_LOOP_INFINITE) repeats -= UINT32(wraps);
        }
        if (position < end) return true;
        const double overflow = position - end;
        const auto completed = queue.front();
        queue.pop_front();
        bufferStarted = false;
        if (callback)
        {
            callback->OnBufferEnd(completed.pContext);
            if (completed.Flags & XAUDIO2_END_OF_STREAM) callback->OnStreamEnd();
        }
        if (queue.empty()) return false;
        position = queue.front().PlayBegin + overflow;
        repeats = queue.front().LoopCount;
    }
    return false;
}
void IXAudio2SourceVoice::Mix(float* output, int frameCount)
{
    if (!playing || queue.empty()) return;
    const double step = double(sampleRate) * frequency / 48000.0;
    for (int frame = 0; frame < frameCount && AdvanceBuffer(); ++frame)
    {
        const auto* buffer = &queue.front();
        if (!bufferStarted && callback) callback->OnBufferStart(buffer->pContext);
        bufferStarted = true;
        const UINT32 end = buffer->PlayLength ? buffer->PlayBegin + buffer->PlayLength : buffer->AudioBytes / format.nBlockAlign;
        const UINT32 loopEnd = buffer->LoopLength ? buffer->LoopBegin + buffer->LoopLength : end;
        const UINT32 index = UINT32(position);
        UINT32 next = index + 1;
        if (repeats && next >= loopEnd) next = buffer->LoopBegin;
        else next = std::min(next, end - 1);
        const float fraction = float(position - index);
        for (UINT32 channel = 0; channel < channels; ++channel)
        {
            const float a = ReadSample(*this, *buffer, index, channel);
            const float b = ReadSample(*this, *buffer, next, channel);
            const float sample = (a + (b - a) * fraction) * volume;
            Route(channel == 0 ? sample : 0, channel == 1 ? sample : 0, frame, output);
        }
        position += step; samplesPlayed += step;
    }
    AdvanceBuffer();
}

void IXAudio2::Callback(void* userdata, uint8_t* bytes, int length)
{

    auto& engine = *static_cast<IXAudio2*>(userdata);

    auto* output = reinterpret_cast<float*>(bytes);
    const int samples = length / sizeof(float), frames = samples / 2;
    std::memset(bytes, 0, size_t(length));

    for (auto* submix : engine.submixes)
    {
        if (size_t(samples) > submix->inputBuffer.size()) return;
        std::fill_n(submix->inputBuffer.data(), samples, 0.0f);
    }
    for (auto* callback : engine.callbacks) callback->OnProcessingPassStart();

    for (auto* voice : engine.voices)
    {

        if (voice->callback) voice->callback->OnVoiceProcessingPassStart(0);
        voice->Mix(output, frames);
        if (voice->callback) voice->callback->OnVoiceProcessingPassEnd();
    }
    for (auto* submix : engine.submixes)
        for (int frame = 0; frame < frames; ++frame)
        {
            float left = 0, right = 0;
            if (submix->effect) submix->Mix(submix->inputBuffer[frame*2], submix->inputBuffer[frame*2+1], left, right);
            else { left = submix->inputBuffer[frame*2] * submix->volume; right = submix->inputBuffer[frame*2+1] * submix->volume; }
            submix->Route(left, right, frame, output);
        }
    const float gain = engine.master ? engine.master->volume : 0;
    for (int frame = 0; frame < frames; ++frame)
    {

        output[frame * 2] = std::clamp(output[frame * 2] * gain, -1.0f, 1.0f);
        output[frame * 2 + 1] = std::clamp(output[frame * 2 + 1] * gain, -1.0f, 1.0f);
    }

    for (auto* callback : engine.callbacks) callback->OnProcessingPassEnd();
}
HRESULT IXAudio2::CreateMasteringVoice(IXAudio2MasteringVoice** out, UINT32 count, UINT32 rate)
{
    if (!out || master || count != 2 || rate != 48000) return E_INVALIDARG;
    AudioLock lock(this); *out = master = new IXAudio2MasteringVoice(this, 2, 48000); return S_OK;
}
HRESULT IXAudio2::CreateSubmixVoice(IXAudio2SubmixVoice** out, UINT32 count, UINT32 rate, UINT32, UINT32 stage, const XAUDIO2_VOICE_SENDS* outputs, const XAUDIO2_EFFECT_CHAIN* chain)
{
    if (!out || count != 2 || rate != 48000 ||
        (chain && (chain->EffectCount > 1 || (chain->EffectCount && !chain->pEffectDescriptors))) ||
        (outputs && (outputs->SendCount > 4 || (outputs->SendCount && !outputs->pSends)))) return E_INVALIDARG;
    auto* voice = new IXAudio2SubmixVoice(this, count, rate);
    voice->stage = stage;
    voice->effect = chain && chain->EffectCount && chain->pEffectDescriptors && chain->pEffectDescriptors[0].InitialState;
    voice->inputBuffer.resize(2048);
    if (outputs)
        for (unsigned i = 0; i < outputs->SendCount; ++i)
            if (outputs->pSends[i].pOutputVoice != master &&
                (std::find(submixes.begin(), submixes.end(), outputs->pSends[i].pOutputVoice) == submixes.end() ||
                 static_cast<IXAudio2SubmixVoice*>(outputs->pSends[i].pOutputVoice)->stage <= stage))
            { delete voice; return E_INVALIDARG; }
    const auto result = voice->SetOutputVoices(outputs);
    if (FAILED(result)) { delete voice; return result; }
    AudioLock lock(this);
    submixes.push_back(voice);
    std::stable_sort(submixes.begin(), submixes.end(), [](auto* a, auto* b) { return a->stage < b->stage; });
    *out = voice;
    return S_OK;
}
HRESULT IXAudio2::CreateSourceVoice(IXAudio2SourceVoice** out, const WAVEFORMATEX* wave, UINT32, float maxRatio, IXAudio2VoiceCallback* callback, const XAUDIO2_VOICE_SENDS* outputs, const XAUDIO2_EFFECT_CHAIN*)
{
    if (!out || !wave || wave->nChannels < 1 || wave->nChannels > 2 || !wave->nSamplesPerSec || !wave->nBlockAlign ||
        !std::isfinite(maxRatio) || maxRatio < 1.0f || maxRatio > 1024.0f ||
        wave->nBlockAlign != wave->nChannels * (wave->wBitsPerSample / 8) ||
        (wave->wFormatTag != 1 && wave->wFormatTag != 3) ||
        (wave->wBitsPerSample != 8 && wave->wBitsPerSample != 16 && wave->wBitsPerSample != 24 && wave->wBitsPerSample != 32) ||
        (wave->wFormatTag == 3 && wave->wBitsPerSample != 32)) return E_INVALIDARG;
    auto* voice = new IXAudio2SourceVoice(this, *wave, maxRatio);
    voice->callback = callback;
    const auto result = voice->SetOutputVoices(outputs);
    if (FAILED(result)) { delete voice; return result; }
    AudioLock lock(this);
    *out = voice; voices.push_back(voice); return S_OK;
}
