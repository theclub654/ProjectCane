#include "core_policy.h"
#include <switch.h>

namespace CaneThreads
{
namespace { CorePolicy policy; }
void Initialize()
{
    u64 allowed = 0;
    const auto core = svcGetCurrentProcessorNumber();
    const Result query = svcGetInfo(&allowed, InfoType_CoreMask, CUR_PROCESS_HANDLE, 0);
    policy = ChooseCores(R_SUCCEEDED(query) ? allowed : 0, core);
    if (policy.allowed)
        svcSetThreadCoreMask(CUR_THREAD_HANDLE, policy.main, 1u << policy.main);
}
int IoCore() { return policy.io; }
int AudioCore() { return policy.audio; }
}
