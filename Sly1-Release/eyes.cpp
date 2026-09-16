#include "eyes.h"

EYES* NewEyes()
{
    return new EYES{};
}

void LoadEyesFromBrx(EYES* peyes, CBinaryInputStream* pbis)
{
    peyes->dtBlink = pbis->F32Read();
    peyes->dtOpenMin = pbis->F32Read();
    peyes->dtOpenMax = pbis->F32Read();
    peyes->uDoubleBlink = pbis->F32Read();
    peyes->oidOther = (OID)pbis->S16Read();

    pbis->U16Read();
    pbis->U32Read();
}

void InitEyes(EYES* peyes, SAAF* psaaf)
{
    InitSaa(peyes, psaaf);

    peyes->saiOther.grfsai = peyes->sai.grfsai;
}

void PostEyesLoad(EYES* peyes)
{
    PostSaaLoad(peyes);

    SHD* pshdOther = PshdFindShader(peyes->oidOther);
    peyes->saiOther.pshd = pshdOther;

    SHD* pshd = peyes->sai.pshd;

    if (pshdOther != NULL)
    {
        if (pshdOther->psaa != NULL || pshd->psaa == (SAA*)peyes)
            pshdOther->psaa = peyes;
    }

    int cframe = 0;

    if (pshd != nullptr)
        cframe = pshd->cframe;

    if (pshdOther != nullptr && cframe < pshdOther->cframe)
        cframe = pshdOther->cframe;

    peyes->eyess = EYESS_Nil;
    peyes->cframe = cframe;

    SetEyesEyess(peyes, EYESS_Open);
}

void UpdateEyes(EYES* peyes, float dt)
{
    int cframe = peyes->cframe;

    if (cframe <= 1)
        return;

    EYESS eyess = peyes->eyess;

    if (eyess == EYESS_Closing)
    {
        float frameStep = peyes->sviframe * dt;
        float nextFrame = peyes->gframe + frameStep;

        peyes->gframe = nextFrame;

        if (nextFrame >= (float)cframe)
        {
            peyes->gframe -= frameStep;

            eyess = EYESS_Opening;

            if ((int)peyes->gframe == cframe - 1)
                eyess = EYESS_Closed;
        }
    }
    else if (eyess == EYESS_Open)
    {
        if (g_clock.t - peyes->tEyess >= peyes->dtOpen)
            eyess = EYESS_Closing;
    }
    else if (eyess == EYESS_Closed)
    {
        if (peyes->uClosed < 1.0f)
            eyess = EYESS_Opening;
    }
    else if (eyess == EYESS_Opening)
    {
        peyes->gframe -= peyes->sviframe * dt;

        if (peyes->gframe <= (float)cframe * peyes->uClosed)
            eyess = EYESS_Open;
    }

    SetEyesEyess(peyes, eyess);

    SetSaiIframe(&peyes->sai, (int)peyes->gframe);
    SetSaiIframe(&peyes->saiOther, (int)peyes->gframe);
}

void SetEyesClosed(EYES* peyes, float uClosed)
{
    peyes->uClosed = uClosed;

    if (uClosed >= 1.0f)
        SetEyesEyess(peyes, EYESS_Closed);
    else
    {
        // Original explicitly clears state first.
        peyes->eyess = EYESS_Nil;
        SetEyesEyess(peyes, EYESS_Open);
    }

    SetSaiIframe(&peyes->sai, static_cast<int>(peyes->gframe));
    SetSaiIframe(&peyes->saiOther, static_cast<int>(peyes->gframe));
}

SAI* PsaiFromEyesShd(EYES* peyes, SHD* pshd)
{
    SAI* psai = PsaiFromSaaShd(peyes, pshd);

    if (psai == nullptr && (short)pshd->oid == peyes->oidOther)
        psai = &peyes->saiOther;

    return psai;
}

void SetEyesEyess(EYES* peyes, EYESS eyess)
{
    if (peyes->eyess == eyess)
        return;

    if (eyess == EYESS_Open)
    {
        peyes->gframe = (float)(peyes->cframe - 1) * peyes->uClosed;

        peyes->sviframe = ((float)(peyes->cframe << 1) * (1.0f - peyes->uClosed)) / peyes->dtBlink;

        if (GRandInRange(0.0f, 1.0f) < peyes->uDoubleBlink)
            peyes->dtOpen = 0.0f;
        else
            peyes->dtOpen = GRandInRange(peyes->dtOpenMin, peyes->dtOpenMax);
    }
    else if (eyess == EYESS_Closed)
    {
        int lastFrame = peyes->cframe - 1;

        if ((int)peyes->gframe != lastFrame)
            peyes->gframe = (float)lastFrame;
    }

    peyes->eyess = eyess;
    peyes->tEyess = g_clock.t;
}

void DeleteEyes(EYES* peyes)
{
    delete peyes;
}