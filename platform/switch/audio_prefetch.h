#pragma once
#include "pcm_wave.h"
#include <algorithm>
#include <condition_variable>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

namespace CaneAudio
{
struct WaveAsset
{
    std::filesystem::path path;
    SoundWaveFormat format;
    std::vector<uint8_t> pcm;
};

class PrefetchQueue
{
public:
    using Loader = std::function<WaveAsset()>;
    struct Entry
    {
        enum State { Queued, Loading, Ready } state = Queued;
        std::string key;
        bool required = false;
        Loader load;
        WaveAsset wave;
    };

    bool Request(const std::string& key, Loader load, bool urgent = false)
    {
        std::lock_guard lock(mutex);
        if (closed) return false;
        auto found = Find(key);
        if (found != entries.end())
        {
            auto entry = *found;
            entry->required |= urgent;
            entries.splice(entries.begin(), entries, found);
            if (urgent && entry->state == Entry::Queued)
            {
                std::erase(pending, entry);
                pending.push_front(entry);
            }
            return entry->state == Entry::Ready;
        }
        if (entries.size() == 4)
        {
            auto discard = std::find_if(entries.rbegin(), entries.rend(), [](const auto& e) { return !e->required; });
            if (discard == entries.rend()) return false;
            Erase(std::prev(discard.base()));
        }
        auto entry = std::make_shared<Entry>();
        entry->key = key;
        entry->required = urgent;
        entry->load = std::move(load);
        entries.push_front(entry);
        if (urgent) pending.push_front(entry);
        else pending.push_back(entry);
        wake.notify_one();
        return false;
    }

    std::optional<WaveAsset> Consume(const std::string& key)
    {
        std::lock_guard lock(mutex);
        auto found = Find(key);
        if (found == entries.end() || (*found)->state != Entry::Ready) return {};
        auto wave = std::move((*found)->wave);
        entries.erase(found);
        return wave;
    }

    std::shared_ptr<Entry> Take()
    {
        std::unique_lock lock(mutex);
        wake.wait(lock, [&] { return closed || !pending.empty(); });
        if (closed) return {};
        auto entry = pending.front();
        pending.pop_front();
        entry->state = Entry::Loading;
        return entry;
    }

    void Complete(const std::shared_ptr<Entry>& entry, WaveAsset wave)
    {
        std::lock_guard lock(mutex);
        if (std::find(entries.begin(), entries.end(), entry) == entries.end()) return;
        entry->wave = std::move(wave);
        entry->load = {};
        entry->state = Entry::Ready;
        std::size_t bytes = 0;
        for (const auto& cached : entries) bytes += cached->wave.pcm.size();
        for (auto it = entries.end(); bytes > 32 * 1024 * 1024 && it != entries.begin();)
        {
            --it;
            if (*it == entry || (*it)->required || (*it)->state != Entry::Ready) continue;
            bytes -= (*it)->wave.pcm.size();
            it = entries.erase(it);
        }
    }

    void Clear()
    {
        std::lock_guard lock(mutex);
        entries.clear();
        pending.clear();
    }
    void Open()
    {
        std::lock_guard lock(mutex);
        closed = false;
    }
    void Close()
    {
        std::lock_guard lock(mutex);
        closed = true;
        entries.clear();
        pending.clear();
        wake.notify_all();
    }

private:
    using Entries = std::list<std::shared_ptr<Entry>>;
    Entries::iterator Find(const std::string& key)
    { return std::find_if(entries.begin(), entries.end(), [&](const auto& entry) { return entry->key == key; }); }
    void Erase(Entries::iterator it)
    { std::erase(pending, *it); entries.erase(it); }
    std::mutex mutex;
    std::condition_variable wake;
    Entries entries;
    std::deque<std::shared_ptr<Entry>> pending;
    bool closed = false;
};

void InitializePrefetch();
void ShutdownPrefetch();
bool PrefetchEnabled();
bool RequestWave(const std::string& key, PrefetchQueue::Loader load, bool urgent = false);
std::optional<WaveAsset> ConsumeWave(const std::string& key);
void ClearPrefetch();
}
