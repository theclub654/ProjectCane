#include "loop.h"

LOOP* NewLoop()
{
    return new LOOP{};
}

void LoadLoopFromBrx(LOOP* ploop, CBinaryInputStream* pbis)
{
    ploop->dtLoopMin  = pbis->F32Read();
    ploop->dtLoopMax  = pbis->F32Read();
    ploop->dtPauseMin = pbis->F32Read();
    ploop->dtPauseMax = pbis->F32Read();
    ploop->gframe     = pbis->F32Read();
    pbis->F32Read();
}

void InitLoop(LOOP* ploop, SAAF* psaaf)
{
    InitSaa(ploop, psaaf);
}

void PostLoopLoad(LOOP* ploop)
{
    PostSaaLoad(ploop);

    // Only proceed if the loop has a shadow/animation descriptor
    if (ploop->sai.pshd != nullptr)
    {
        // Randomized loop playback speed
        const float loopDuration = GRandInRange(ploop->dtLoopMin, ploop->dtLoopMax);

        ploop->sviframe = static_cast<float>(ploop->sai.pshd->cframe) / loopDuration;

        // Randomized pause duration
        const float pauseDuration = GRandInRange(ploop->dtPauseMin, ploop->dtPauseMax);

        ploop->dtPause = pauseDuration;
        ploop->dtPauseRequested = pauseDuration;
    }
}

void UpdateLoop(LOOP* ploop, float dt)
{
    SHD* pshd = ploop->sai.pshd;

    if (!pshd || pshd->cframe <= 1)
        return;

    // If we're currently pausing, count it down and do nothing else.
    if (ploop->dtPause > 0.0f)
    {
        ploop->dtPause -= dt;
        return;
    }

    // Advance fractional frame time.
    ploop->gframe += ploop->sviframe * dt;

    // If we reached/passed the end of the loop, pick new loop speed + pause time.
    if (ploop->gframe >= static_cast<float>(pshd->cframe))
    {
        const float loopDuration = GRandInRange(ploop->dtLoopMin, ploop->dtLoopMax);
        ploop->sviframe = static_cast<float>(pshd->cframe) / loopDuration;

        const float pauseDuration = GRandInRange(ploop->dtPauseMin, ploop->dtPauseMax);
        ploop->dtPause = pauseDuration;
        ploop->dtPauseRequested = pauseDuration;
    }

    // Wrap into [0, cframe) and apply to the SAI iframe.
    ploop->gframe = GModPositive(ploop->gframe, static_cast<float>(pshd->cframe));
    SetSaiIframe(&ploop->sai, static_cast<int>(ploop->gframe));
}

float UCompleteLoop(LOOP* ploop)
{
    return (ploop->gframe / ploop->sviframe) / ((float)((ploop->sai).pshd)->cframe / ploop->sviframe + ploop->dtPauseRequested);
}

void DeleteLoop(LOOP* ploop)
{
    delete ploop;
}