#include "shuffle.h"

std::shared_ptr<SHUFFLE> NewShuffle()
{
    return std::make_shared<SHUFFLE>();
}

void LoadShuffleFromBrx(SHUFFLE* pshuffle, CBinaryInputStream* pbis)
{
    pshuffle->dtPauseMin = pbis->F32Read();
    pshuffle->dtPauseMax = pbis->F32Read();

    pbis->F32Read();
    pbis->F32Read();
    pbis->F32Read();
    pbis->F32Read();
}

void InitShuffle(SHUFFLE* pshuffle, SAAF* psaaf)
{
    InitSaa(pshuffle, psaaf);
}

void UpdateShuffle(SHUFFLE* pshuffle, float dt)
{
    SHD* pshd = pshuffle->sai.pshd;

    if (pshd == nullptr || pshd->cframe <= 1)
        return;

    if (pshuffle->dtPause > 0.0f)
        pshuffle->dtPause -= dt;
    else
    {
        int frameOffset = NRandInRange(1, pshd->cframe - 1);

        SetSaiIframe(&pshuffle->sai, (pshuffle->sai.iframe + frameOffset) % pshd->cframe);
        pshuffle->dtPause = GRandInRange(pshuffle->dtPauseMin, pshuffle->dtPauseMax);
    }
}

void DeleteShuffle(SHUFFLE* pshuffle)
{
    DeleteSaa(reinterpret_cast<SAA*>(pshuffle));
}
