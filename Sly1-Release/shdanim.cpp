#include "shdanim.h"

void* NewSaa(SAAK saak)
{
    switch (saak)
    {
        case SAAK_Loop:
        return new LOOP{};

        case SAAK_Scroller:
        return new SCROLLER{};

        case SAAK_Circler:
        return new CIRCLER{};

        default:
        return nullptr;
    }
}

SAA* PsaaLoadFromBrx(CBinaryInputStream *pbis)
{
    SAAK saakType = (SAAK)pbis->U16Read();
    //std::cout << saakType << " ";
    SAA *psaa{};
    psaa = (SAA*)NewSaa(saakType);

    if (saakType != 0)
    {
        if (psaa != nullptr)
        {
            g_apsaaSw.push_back(psaa);
            psaa->saak = saakType;
            psaa->pvtsaa = PvtsaaFromSaak(saakType);
            SAAF saaf{};
            saaf.oid = pbis->S16Read();
            saaf.fInstanced = pbis->U16Read();
            psaa->pvtscroller->pfnLoadScrollerFromBrx((SCROLLER*)psaa, pbis);
            psaa->pvtsaa->pfnInitSaa(psaa, &saaf);
        }
        else
        {
            pbis->file.seekg(0x1C, SEEK_CUR);
        }
    }

    return psaa;
}

VTSAA* PvtsaaFromSaak(SAAK saak)
{
    switch (saak)
    {
        case SAAK_Loop:
        return (VTSAA*)&g_vtloop;

        case SAAK_Scroller:
        return (VTSAA*)&g_vtscroller;

        case SAAK_Circler:
        return (VTSAA*)&g_vtcircler;

        default:
        return nullptr;
    }
}

void InitSaa(SAA* psaa, SAAF* psaaf)
{
    psaa->oid = (OID)psaaf->oid;
    psaa->sai.grfsai |= 0x01;

    if (psaaf->fInstanced != 0)
        psaa->sai.grfsai |= 0x04;
}

void PostSaaLoad(SAA* psaa)
{
    if (psaa->sai.pshd == nullptr) 
        psaa->sai.pshd = PshdFindShader(psaa->oid);
}

float UCompleteSaa(SAA* psaa)
{
    return 0.0f;
}

SAI* PsaiFromSaaShd(SAA* psaa, SHD* pshd)
{
    if (pshd->oid == psaa->oid) 
        return &psaa->sai;

    return nullptr;
}

int FUpdatableSaa(SAA* psaa)
{
    if (psaa->tUpdated != g_clock.t) 
    {
        psaa->tUpdated = g_clock.t;
        return 1;
    }

    return 0;
}

void DeleteSaa(SAA* psaa)
{
    delete psaa;
}

void LoadLoopFromBrx(LOOP* ploop, CBinaryInputStream* pbis)
{
    ploop->dtLoopMin  = pbis->F32Read();
    ploop->dtLoopMax  = pbis->F32Read();
    ploop->dtPauseMin = pbis->F32Read();
    ploop->dtPauseMax = pbis->F32Read();
    pbis->F32Read();
    pbis->F32Read();
}

void InitLoop(LOOP* ploop, SAAF* psaaf)
{
    InitSaa(ploop, psaaf);

    ploop->gframe = 0.0;
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

void LoadScrollerFromBrx(SCROLLER* pscroller, CBinaryInputStream* pbis)
{
    pscroller->svu   = pbis->F32Read();
    pscroller->svv   = pbis->F32Read();
    pscroller->duMod = pbis->F32Read();
    pscroller->dvMod = pbis->F32Read();
    pbis->F32Read();
    pbis->F32Read();
}

void InitScroller(SCROLLER *pscroller, SAAF *psaaf)
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

void LoadCirclerFromBrx(CIRCLER* pcircler, CBinaryInputStream* pbis)
{
    pcircler->sw = pbis->F32Read();
    pcircler->sRadius = pbis->F32Read();
    pcircler->du = pbis->F32Read();
    pcircler->dv = pbis->F32Read();
    pbis->F32Read();
    pbis->F32Read();
}

void InitCircler(CIRCLER* pcircler, SAAF* psaaf)
{
    InitSaa(pcircler, psaaf);

    const uint32_t oldFlags = pcircler->sai.grfsai;
    pcircler->sai.grfsai = (oldFlags & ~1u) | 2u;
}

void UpdateCircler(CIRCLER* pcircler, float dt)
{
    if (!pcircler) return;

    if (pcircler->sai.pshd == nullptr)
        return;

    const float rad = RadNormalize(g_clock.t * pcircler->sw);

    float s = 0.0f;
    float c = 0.0f;
    CalculateSinCos(rad, &s, &c);

    const float duNew = s * pcircler->sRadius + pcircler->du;
    const float dvNew = c * pcircler->sRadius + pcircler->dv;

    SetSaiDuDv(&pcircler->sai, duNew, dvNew);
}

float UCompleteCircler(CIRCLER* pcircler)
{
    constexpr float TWO_PI = 6.283185f;
    constexpr float INV_TWO_PI = 0.1591549f; 

    float angle = GModPositive(g_clock.t * pcircler->sw, TWO_PI);
    return angle * INV_TWO_PI;
}

void DeleteCircler(CIRCLER* pcircler)
{
    delete pcircler;
}

void SetSaiDuDv(SAI* psai, float du, float dv)
{
    // If nothing changed, do nothing.
    if (psai->tcx.du == du && psai->tcx.dv == dv)
        return;
    
    psai->tcx.du = du;
    psai->tcx.dv = dv;

    // If it's not already queued (psaiNext == nullptr) and it's not the tail,
    // push it onto the global update list.
    if (psai->psaiNext == nullptr && psai != g_psaiUpdateTail) {
        if (!g_psaiUpdateTail)
            g_psaiUpdateTail = psai;

        psai->psaiNext = g_psaiUpdate;
        g_psaiUpdate = psai;
    }
}