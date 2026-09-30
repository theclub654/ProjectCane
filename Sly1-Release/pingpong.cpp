#include "pingpong.h"

std::shared_ptr<PINGPONG> NewPingPong()
{
    return std::make_shared<PINGPONG>();
}

void LoadPingPongFromBrx(PINGPONG* ppingpong, CBinaryInputStream* pbis)
{
    ppingpong->dtPingpongMin = pbis->F32Read();
    ppingpong->dtPingpongMax = pbis->F32Read();
    ppingpong->dtPauseMin = pbis->F32Read();
    ppingpong->dtPauseMax = pbis->F32Read();
    ppingpong->gframe = pbis->U16Read();

    pbis->F32Read();
    pbis->U16Read();
}

void InitPingpong(PINGPONG* ppingpong, SAAF* psaaf)
{
    InitSaa(ppingpong, psaaf);
}

void PostPingpongLoad(PINGPONG* ppingpong)
{
    PostSaaLoad(ppingpong);

    SHD* pshd = ppingpong->sai.pshd;

    if (pshd == nullptr)
        return;

    float pingpongTime = GRandInRange(ppingpong->dtPingpongMin, ppingpong->dtPingpongMax);
    ppingpong->sviframe = (float)(pshd->cframe << 1) / pingpongTime;
    float pauseTime = GRandInRange(ppingpong->dtPauseMin, ppingpong->dtPauseMax);

    ppingpong->dtPause = pauseTime;
    ppingpong->dtPauseRequested = pauseTime;
}

void UpdatePingpong(PINGPONG* ppingpong, float dt)
{
    SHD* pshd = ppingpong->sai.pshd;

    if (pshd == nullptr || pshd->cframe <= 1)
        return;

    if (ppingpong->dtPause > 0.0f)
    {
        ppingpong->dtPause -= dt;
        return;
    }

    float frameStep = ppingpong->sviframe * dt;
    float nextFrame = ppingpong->gframe + frameStep;

    ppingpong->gframe = nextFrame;

    if (nextFrame >= (float)pshd->cframe)
    {
        ppingpong->gframe -= frameStep;
        ppingpong->sviframe = -ppingpong->sviframe;
    }

    if (ppingpong->gframe < 0.0f)
    {
        ppingpong->gframe = 0.0f;

        float pingpongTime = GRandInRange(ppingpong->dtPingpongMin, ppingpong->dtPingpongMax);

        ppingpong->sviframe = (float)(pshd->cframe << 1) / pingpongTime;

        float pauseTime = GRandInRange(ppingpong->dtPauseMin, ppingpong->dtPauseMax);

        ppingpong->dtPause = pauseTime;
        ppingpong->dtPauseRequested = pauseTime;
    }

    SetSaiIframe(&ppingpong->sai, (int)ppingpong->gframe);
}

float UCompletePingpong(PINGPONG* ppingpong)
{
    SHD* pshd = ppingpong->sai.pshd;

    float speed = ppingpong->sviframe;
    float frame = ppingpong->gframe;

    if (speed < 0.0f)
    {
        speed = -speed;
        frame = (float)(pshd->cframe << 1) - frame;
    }

    float elapsedTime = frame / speed;
    float totalTime = ((float)(pshd->cframe << 1) / speed) + ppingpong->dtPauseRequested;

    return elapsedTime / totalTime;
}

void DeletePingpong(PINGPONG* ppingpong)
{
    DeleteSaa(reinterpret_cast<SAA*>(ppingpong));
}
