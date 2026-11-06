#include "audio_prefetch.h"
#include "core_policy.h"
#include <switch.h>

namespace CaneAudio
{
namespace
{
PrefetchQueue queue;
Thread thread;
bool running = false;
void Worker(void*)
{
    while (auto entry = queue.Take())
    {
        WaveAsset wave;
        try { wave = entry->load(); }
        catch (...) {}
        queue.Complete(entry, std::move(wave));
    }
}
}
void InitializePrefetch()
{
    if (running) return;
    queue.Open();
    Result rc = threadCreate(&thread, Worker, nullptr, nullptr, 128 * 1024, 0x3b, CaneThreads::IoCore());
    if (R_SUCCEEDED(rc))
    {
        rc = threadStart(&thread);
        if (R_FAILED(rc)) threadClose(&thread);
    }
    running = R_SUCCEEDED(rc);
}
void ShutdownPrefetch()
{
    queue.Close();
    if (!running) return;
    threadWaitForExit(&thread);
    threadClose(&thread);
    running = false;
}
bool PrefetchEnabled() { return running; }
bool RequestWave(const std::string& key, PrefetchQueue::Loader load, bool urgent)
{ return queue.Request(key, std::move(load), urgent); }
std::optional<WaveAsset> ConsumeWave(const std::string& key) { return queue.Consume(key); }
void ClearPrefetch() { queue.Clear(); }
}
