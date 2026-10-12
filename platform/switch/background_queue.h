#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <utility>

namespace CaneIo
{
class BackgroundQueue
{
    std::mutex mutex;
    std::condition_variable changed;
    std::optional<std::function<bool()>> pending;
    bool busy = false;
    bool closing = false;
public:
    bool Push(std::function<bool()> job)
    {
        if (!job) return false;
        std::lock_guard lock(mutex);
        if (closing) return false;
        pending = std::move(job);
        changed.notify_all();
        return true;
    }
    std::optional<std::function<bool()>> Take()
    {
        std::unique_lock lock(mutex);
        changed.wait(lock, [&] { return closing || pending.has_value(); });
        if (!pending) return {};
        auto job = std::move(pending);
        pending.reset();
        busy = true;
        return job;
    }
    void Complete()
    {
        std::lock_guard lock(mutex);
        busy = false;
        changed.notify_all();
    }
    void Drain()
    {
        std::unique_lock lock(mutex);
        changed.wait(lock, [&] { return !pending && !busy; });
    }
    void Close()
    {
        std::lock_guard lock(mutex);
        closing = true;
        changed.notify_all();
    }
};
}
