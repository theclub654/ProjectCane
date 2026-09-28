#pragma once
#include "shdanim.h"

struct HOLOGRAM : public SAA
{
    float dradAdjust;
    float dradSymmetry;
    float dradFrame;
};

std::shared_ptr<HOLOGRAM> NewHologram();
void  LoadHologramFromBrx(HOLOGRAM* phologram, CBinaryInputStream* pbis);
void  InitHologram(HOLOGRAM* phologram, SAAF* psaaf);
void  PostHologramLoad(HOLOGRAM* phologram);
void  NotifyHologramRender(HOLOGRAM* phologram, ALO* palo, RPL* prpl);
void  DeleteHologram(HOLOGRAM* phologram);
