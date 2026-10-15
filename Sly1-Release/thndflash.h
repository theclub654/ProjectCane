#pragma once

#include "lo.h"
#include "shd.h"

class THNDFLASH : public LO
{
public:
    glm::vec4 rgbaStart;
    glm::vec4 rgbaPeak;
    glm::vec4 rgbaEnd;
    float dtFadeIn;
    float dtHold;
    float dtFadeOut;
    float tStart;
    int thndflashk;
    int fActive;

    // Host-side runtime state. The PS2 object reuses an LO list entry at
    // offset 0x34; the port keeps a dedicated entry to avoid aliasing lists.
    DLE dleThndFlash;
    glm::vec4 rgbaCurrent;
};

THNDFLASH* NewThndFlash();
int GetThndFlashSize();
void CloneThndFlash(THNDFLASH* pthndflash, THNDFLASH* pthndflashBase);
void SetThndFlashRgbaStartFloat(THNDFLASH* pthndflash, const glm::vec4* rgba);
void SetThndFlashRgbaPeakFloat(THNDFLASH* pthndflash, const glm::vec4* rgba);
void SetThndFlashRgbaEndFloat(THNDFLASH* pthndflash, const glm::vec4* rgba);
void StartThndFlash(THNDFLASH* pthndflash);
void RenderThndFlash(THNDFLASH* pthndflash);
void DrawThndFlash(RPL* prpl);
void DeleteThndFlash(THNDFLASH* pthndflash);
