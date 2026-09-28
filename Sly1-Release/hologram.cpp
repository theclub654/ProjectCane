#include "hologram.h"

std::shared_ptr<HOLOGRAM> NewHologram()
{
    return std::make_shared<HOLOGRAM>();
}

void LoadHologramFromBrx(HOLOGRAM* phologram, CBinaryInputStream* pbis)
{
    phologram->dradAdjust = pbis->F32Read();
    int cSymmetry = pbis->U32Read();

    phologram->dradSymmetry = 6.2831855f / (float)cSymmetry;

    if (phologram->dradAdjust == 3.4028235e+38f)
        phologram->dradAdjust = GRandInRange(0.0f, phologram->dradSymmetry);

    pbis->F32Read();
    pbis->F32Read();
    pbis->F32Read();
    pbis->F32Read();
}

void InitHologram(HOLOGRAM* phologram, SAAF* psaaf)
{
    InitSaa(phologram, psaaf);
}

void PostHologramLoad(HOLOGRAM* phologram)
{
    PostSaaLoad((SAA*)phologram);

    SHD* pshd = phologram->sai.pshd;

    if (pshd == nullptr || pshd->cframe <= 1)
        return;

    phologram->dradFrame = phologram->dradSymmetry / (float)pshd->cframe;
}

void NotifyHologramRender(HOLOGRAM* phologram, ALO* palo, RPL* prpl)
{

}

void DeleteHologram(HOLOGRAM* phologram)
{
    delete phologram;
}
