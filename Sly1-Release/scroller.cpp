#include "scroller.h"

SCROLLER* NewScroller()
{
    return new SCROLLER{};
}

void LoadScrollerFromBrx(SCROLLER* pscroller, CBinaryInputStream* pbis)
{
    pscroller->svu   = pbis->F32Read();
    pscroller->svv   = pbis->F32Read();
    pscroller->duMod = pbis->F32Read();
    pscroller->dvMod = pbis->F32Read();
    pbis->F32Read();
    pbis->F32Read();
}

void SetScrollerMasterSpeeds(SCROLLER* pscroller, float svu, float svv)
{
    pscroller->svvMaster = svv;
    pscroller->svuMaster = svu;
}

void InitScroller(SCROLLER* pscroller, SAAF* psaaf)
{
    InitSaa(pscroller, psaaf);

    pscroller->sai.grfsai = (pscroller->sai.grfsai & ~0x01u) | 0x02u;

    pscroller->svuMaster = 1.0f;
    pscroller->svvMaster = 1.0f;
}

void UpdateScroller(SCROLLER* pscroller, float dt)
{
    if (pscroller->sai.pshd != nullptr)
    {
        float du = fmodf(pscroller->sai.tcx.du + pscroller->svu * pscroller->svuMaster * dt, pscroller->duMod);
        float dv = fmodf(pscroller->sai.tcx.dv + pscroller->svv * pscroller->svvMaster * dt, pscroller->dvMod);
        SetSaiDuDv(&pscroller->sai, du, dv);
    }
}

float UCompleteScroller(SCROLLER* pscroller)
{
    const float uTerm = (pscroller->svu == 0.0f)
        ? 0.5f
        : (pscroller->sai.tcx.du / pscroller->duMod) * 0.5f;

    const float vTerm = (pscroller->svv == 0.0f)
        ? 0.5f
        : (pscroller->sai.tcx.dv / pscroller->dvMod) * 0.5f;

    return uTerm + vTerm;
}

void DeleteScroller(SCROLLER* pscroller)
{
    delete pscroller;
}