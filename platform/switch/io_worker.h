#pragma once
#include <functional>

namespace CaneIo
{
void Initialize();
bool QueueSettings(std::function<bool()> job);
void Drain();
void Shutdown();
}
