#include "io_worker.h"
#include "background_queue.h"
#include "core_policy.h"
#include <switch.h>
#include <cstdio>
#include <exception>

namespace CaneIo
{
namespace
{
BackgroundQueue queue;
Thread thread;
bool running = false;
void Worker(void*)
{
    while (auto job = queue.Take())
    {
        try
        {
            if (!(*job)()) std::fprintf(stderr, "Settings write failed\n");
        }
        catch (const std::exception& error)
        {
            std::fprintf(stderr, "Settings write failed: %s\n", error.what());
        }
        catch (...) { std::fprintf(stderr, "Settings write failed\n"); }
        queue.Complete();
    }
}
}
void Initialize()
{
    Result rc = threadCreate(&thread, Worker, nullptr, nullptr, 128 * 1024, 0x3b, CaneThreads::IoCore());
    if (R_SUCCEEDED(rc))
    {
        rc = threadStart(&thread);
        if (R_FAILED(rc)) threadClose(&thread);
    }
    running = R_SUCCEEDED(rc);
}
bool QueueSettings(std::function<bool()> job)
{
    if (!job) return false;
    if (!running) return job();
    return queue.Push(std::move(job));
}
void Drain() { if (running) queue.Drain(); }
void Shutdown()
{
    if (!running) return;
    queue.Close();
    threadWaitForExit(&thread);
    threadClose(&thread);
    running = false;
}
}
