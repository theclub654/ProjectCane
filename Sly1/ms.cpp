#include "ms.h"

void* NewMs()
{
    return new MS{};
}

int GetMsSize()
{
    return sizeof(MS);
}

void DeleteMs(LO* plo)
{
    delete (MS*)plo;
}
