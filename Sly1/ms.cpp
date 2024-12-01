#include "ms.h"

MS* NewMs()
{
    return new MS{};
}

int GetMsSize()
{
    return sizeof(MS);
}

void RenderMsGlobset(MS* pms, CM* pcm, RO* pro)
{
    
}

void DeleteMs(LO* plo)
{
    delete (MS*)plo;
}
