#pragma once

#include "lo.h"
#include "shd.h"

class THNDFLASH : public LO
{
public:
    RGBA rgbaStart;
    RGBA rgbaPeak;
    RGBA rgbaEnd;
    float dtFadeIn;
    float dtHold;
    float dtFadeOut;
    float tStart;
    int thndflashk;
    int fActive;

    // Host-side runtime state. The PS2 object reuses an LO list entry at
    // offset 0x34; the port keeps a dedicated entry to avoid aliasing lists.
    DLE dleThndFlash;
    RGBA rgbaCurrent;
};

THNDFLASH* NewThndFlash();
int GetThndFlashSize();
void StartThndFlash(THNDFLASH* pthndflash);
void RenderThndFlash(THNDFLASH* pthndflash);
void DrawThndFlash(RPL* prpl);
void DeleteThndFlash(THNDFLASH* pthndflash);
