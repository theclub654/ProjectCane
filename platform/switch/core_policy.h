#pragma once
#include <bit>
#include <cstdint>

namespace CaneThreads
{
struct CorePolicy { int main{}, io=-2, audio=-2; uint64_t allowed{}; };
inline CorePolicy ChooseCores(uint64_t allowed, unsigned current)
{

    allowed &= 7;
    CorePolicy result;
    result.allowed = allowed;
    result.main = current < 3 && (allowed & (1ull<<current)) ? int(current) :
        allowed ? int(std::countr_zero(allowed)) : int(current);
    const auto other = allowed & ~(1ull<<result.main);
    if (other)
    {
        result.io = int(std::countr_zero(other));
        const auto audio = other & ~(1ull<<result.io);
        result.audio = audio ? int(std::countr_zero(audio)) : result.io;
    }
    return result;
}
void Initialize();
int IoCore();
int AudioCore();
}
