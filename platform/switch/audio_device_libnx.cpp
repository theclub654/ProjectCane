#include "audio.h"
#include "pcm_output.h"
#include "core_policy.h"
#include <switch.h>
#include <array>
#include <atomic>
#include <cstdlib>
#include <cstdio>

namespace
{
constexpr unsigned Frames=1024,Channels=2,BufferCount=3;
struct NativeAudio
{
    IXAudio2* engine{};
    Mutex mutex{};
    Thread thread{};
    bool threadCreated{},threadStarted{},service{};
    std::atomic<bool> stop{false};
    std::atomic<Result> error{0};
    std::array<AudioOutBuffer,BufferCount> buffers{};
    std::array<float,Frames*Channels> mix{};
};
Result FillAndQueue(NativeAudio& state,AudioOutBuffer& buffer)
{
    mutexLock(&state.mutex);
    IXAudio2::Callback(state.engine,reinterpret_cast<uint8_t*>(state.mix.data()),sizeof(state.mix));
    mutexUnlock(&state.mutex);
    auto* output=static_cast<int16_t*>(buffer.buffer);
    for (unsigned i=0;i<state.mix.size();++i) output[i]=CaneFloatToPcm16(state.mix[i]);
    armDCacheFlush(buffer.buffer,buffer.data_size);
    return audoutAppendAudioOutBuffer(&buffer);
}
void Worker(void* userdata)
{
    auto& state=*static_cast<NativeAudio*>(userdata);

    while (!state.stop.load(std::memory_order_acquire))
    {
        AudioOutBuffer* released=nullptr;uint32_t count=0;
        Result rc=audoutWaitPlayFinish(&released,&count,100000000);
        if (state.stop.load(std::memory_order_acquire)) break;

        if (R_FAILED(rc) && rc!=KERNELRESULT(TimedOut)) { state.error=rc;break; }
        if (!count) continue;
        if (count==1 && released)
        {
            rc=FillAndQueue(state,*released);
            if (R_FAILED(rc)) { state.error=rc;break; }
        }
        else
        {

            for (auto& buffer:state.buffers)
            {
                bool queued=false;rc=audoutContainsAudioOutBuffer(&buffer,&queued);
                if (R_SUCCEEDED(rc) && !queued) rc=FillAndQueue(state,buffer);
                if (R_FAILED(rc)) { state.error=rc;break; }
            }
            if (R_FAILED(rc)) break;
        }
    }
    if (R_FAILED(state.error.load())) state.engine->NotifyCriticalError(E_FAIL);
}
}
void IXAudio2::Lock() { if (nativeDevice) mutexLock(&static_cast<NativeAudio*>(nativeDevice)->mutex); }
void IXAudio2::Unlock() { if (nativeDevice) mutexUnlock(&static_cast<NativeAudio*>(nativeDevice)->mutex); }
void IXAudio2::Release()
{
    if (auto* state=static_cast<NativeAudio*>(nativeDevice))
    {
        state->stop.store(true,std::memory_order_release);
        if (state->threadStarted) threadWaitForExit(&state->thread);
        if (state->threadCreated) threadClose(&state->thread);
        if (state->service)
        {
            audoutStopAudioOut();bool flushed=false;audoutFlushAudioOutBuffers(&flushed);audoutExit();
        }
        for (auto& buffer:state->buffers) std::free(buffer.buffer);
        if (R_FAILED(state->error.load())) std::fprintf(stderr,"[AUDIO] Native output error: 0x%x\n",state->error.load());
        delete state;nativeDevice=nullptr;
    }
    for (auto* voice:voices) delete voice;
    for (auto* submix:submixes) delete submix;
    delete master;delete this;
}
HRESULT XAudio2Create(IXAudio2** output,UINT32,UINT32)
{
    if (!output) return E_POINTER;
    *output=nullptr;
    auto* engine=new IXAudio2;
    auto* state=new NativeAudio;state->engine=engine;mutexInit(&state->mutex);engine->nativeDevice=state;
    Result rc=audoutInitialize();state->service=R_SUCCEEDED(rc);
    if (R_FAILED(rc) || audoutGetSampleRate()!=48000 || audoutGetChannelCount()!=Channels || audoutGetPcmFormat()!=PcmFormat_Int16)
    { std::fprintf(stderr,"[AUDIO] Native output initialization failed: 0x%x\n",rc);engine->Release();return E_FAIL; }
    for (auto& buffer:state->buffers)
    {
        buffer.buffer=std::aligned_alloc(4096,4096);buffer.buffer_size=4096;buffer.data_size=Frames*Channels*sizeof(int16_t);
        if (!buffer.buffer) { engine->Release();return E_FAIL; }
        rc=FillAndQueue(*state,buffer);
        if (R_FAILED(rc)) { state->error=rc;engine->Release();return E_FAIL; }
    }
    rc=audoutStartAudioOut();
    if (R_SUCCEEDED(rc))
    {
        rc=threadCreate(&state->thread,Worker,state,nullptr,64*1024,0x2c,CaneThreads::AudioCore());state->threadCreated=R_SUCCEEDED(rc);
        if (R_SUCCEEDED(rc)) { rc=threadStart(&state->thread);state->threadStarted=R_SUCCEEDED(rc); }
    }
    if (R_FAILED(rc)) { state->error=rc;engine->Release();return E_FAIL; }
    *output=engine;

    return S_OK;
}
