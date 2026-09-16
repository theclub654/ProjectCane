#include "rat.h"
#include "step.h"
#include "emitter.h"
#include "target.h"
#include "expl.h"

RAT* NewRat()
{
	return new RAT{};
}

void InitSwRatDl(SW* psw)
{
	InitDl(&psw->dlRat, offsetof(RAT, dleRat));
}

void InitRat(RAT* prat)
{
    InitSo(prat);

    prat->lmDtStop.gMax = 1.0f;
    prat->lmDtStop.gMin = 0.5f;
    prat->tRatSqueak = GRandGaussian(g_clock.t + 3.0f, 1.0f, 2.5f);
    prat->ccoin = -1;
}

void SetRatSScurry(RAT* prat, float sScurry)
{
    prat->sScurry = sScurry;
}

void* GetRatSScurry(RAT* prat)
{
    return &prat->sScurry;
}

void SetRatSvScurry(RAT* prat, float svScurry)
{
    prat->svScurry = svScurry;
}

void* GetRatSvScurry(RAT* prat)
{
    return &prat->svScurry;
}

void SetRatDsFrameScurry(RAT* prat, float dsFrameScurry)
{
    prat->dsFrameScurry = dsFrameScurry;
}

void* GetRatDsFrameScurry(RAT* prat)
{
    return &prat->dsFrameScurry;
}

void SetRatSvFlee(RAT* prat, float svFlee)
{
    prat->svFlee = svFlee;
}

void* GetRatSvFlee(RAT* prat)
{
    return &prat->svFlee;
}

void SetRatSGroupRadius(RAT* prat, float sGroupRadius)
{
    prat->sGroupRadius = sGroupRadius;
}

void* GetRatSGroupRadius(RAT* prat)
{
    return &prat->sGroupRadius;
}

void SetRatLmDtStopGMin(RAT* prat, float gMin)
{
    prat->lmDtStop.gMin = gMin;
}

void* GetRatLmDtStopGMin(RAT* prat)
{
    return &prat->lmDtStop.gMin;
}

void SetRatSvLeap(RAT* prat, float svLeap)
{
    prat->svLeap = svLeap;
}

void* GetRatSvLeap(RAT* prat)
{
    return &prat->svLeap;
}

void* GetRatRats(RAT* prat)
{
    return &prat->rats;
}

void SetRatCcoin(RAT* prat, int ccoin)
{
    prat->ccoin = ccoin;
}

void* GetRatCcoin(RAT* prat)
{
    return &prat->ccoin;
}

void SetRatFSuppressDifficultyUpdate(RAT* prat, int fSuppressDifficultyUpdate)
{
    prat->fSuppressDifficultyUpdate = fSuppressDifficultyUpdate;
}

void* GetRatFSuppressDifficultyUpdate(RAT* prat)
{
    return &prat->fSuppressDifficultyUpdate;
}

void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis)
{
    LoadSoFromBrx(prat, pbis);
    InferExpl(&prat->pexpl, prat);
    SnipAloObjects(prat, 1, &s_asnipRat);

    for (int oid = 617; oid <= 647; ++oid)
    {
        ALO* paloRender = static_cast<ALO*>(PloFindSwChild(prat->psw, static_cast<OID>(oid), prat));

        if (paloRender == nullptr)
            continue;

        SnipLo(paloRender);

        if (oid < 633)
        {
            assert(prat->cpaloRenderScurry < 16);
            prat->apaloRenderScurry[prat->cpaloRenderScurry++] = paloRender;
        }
        else
        {
            assert(prat->cpaloRenderFlee < 16);
            prat->apaloRenderFlee[prat->cpaloRenderFlee++] = paloRender;
        }
    }
}

void CloneRat(RAT* prat, RAT* pratBase)
{
    DLE dleRat = prat->dleRat;

    CloneSo(prat, pratBase);

    // Shallow copy of the value members
    prat->tRats = pratBase->tRats;
    prat->tRatsNext = pratBase->tRatsNext;
    prat->tRatSqueak = pratBase->tRatSqueak;
    prat->ipaloRender = pratBase->ipaloRender;
    prat->cpaloRenderScurry = pratBase->cpaloRenderScurry;
    prat->cpaloRenderFlee = pratBase->cpaloRenderFlee;
    prat->dsFrameScurry = pratBase->dsFrameScurry;
    prat->dsFrameFlee = pratBase->dsFrameFlee;
    prat->sScurry = pratBase->sScurry;
    prat->svScurry = pratBase->svScurry;
    prat->svFlee = pratBase->svFlee;
    prat->svCur = pratBase->svCur;
    prat->svLeap = pratBase->svLeap;
    prat->sGroupRadius = pratBase->sGroupRadius;
    prat->dposTarget = pratBase->dposTarget;
    prat->posWanderNext = pratBase->posWanderNext;
    prat->posFrameLast = pratBase->posFrameLast;
    prat->ccoin = pratBase->ccoin;

    // Shallow copy of the array members (assuming they're pointers)
    for (int i = 0; i < 16; ++i) {
        prat->apaloRenderScurry[i] = pratBase->apaloRenderScurry[i];
        prat->apaloRenderFlee[i] = pratBase->apaloRenderFlee[i];
    }

    // Shallow copy of the pointer members (e.g., pointers to ALO, EXPL, TARGET, etc.)
    prat->paloRenderStopped = pratBase->paloRenderStopped;
    prat->pexpl = pratBase->pexpl;
    prat->ptarget = pratBase->ptarget;
    prat->prathole = pratBase->prathole;

    // Shallow copy of the DLE structure
    prat->dleRat = pratBase->dleRat;

    // Shallow copy of the LM structure
    prat->lmDtStop = pratBase->lmDtStop;

    prat->dleRat = dleRat;
}

void PostRatLoad(RAT* prat)
{
    SnipAloObjects(prat, 1, &s_asnipPostRatLoad);
    PostAloLoad(prat);

    prat->rats = RATS_Nil;
    SetRatRats(prat, RATS_Scurry);

    prat->posFrameLast = prat->xf.posWorld;
}

int GetRatSize()
{
	return sizeof(RAT);
}

void OnRatAdd(RAT* prat)
{
	OnSoAdd(prat);
	AppendDlEntry(&prat->psw->dlRat, prat);
}

void OnRatRemove(RAT* prat)
{
	OnSoRemove(prat);
	RemoveDlEntry(&prat->psw->dlRat, prat);
}

void PresetRatAccel(RAT* prat, float dt)
{
    PresetSoAccel(prat, dt);

    glm::vec3 posTarget;

    if (prat->rats == RATS_Scurry)
        posTarget = prat->posWanderNext;
    else if (prat->rats == RATS_Flee)
        ConvertAloPos(prat->prathole->paloParent, nullptr, &prat->prathole->posLocal, &posTarget);
    else
    {
        AccelSoTowardMatSpring(prat, nullptr, nullptr, &g_vecZero, prat->pclqRotSpring.get(), dt);
        return;
    }

    glm::vec3 dpos = posTarget - prat->xf.posWorld;
    dpos.z = 0.0f;

    glm::mat3 mat;
    LoadLookAtMatrix(&dpos, &mat);

    AccelSoTowardMatSmooth(prat, dt, &mat, &s_smpRatSpin);
}

void UpdateRat(RAT* prat, float dt)
{
    PO* ppo = PpoCur();

    UpdateSo(prat, dt);

    RATS rats = prat->rats;
    glm::vec3 posPoSoon(0.0f);

    if (ppo != nullptr)
        posPoSoon = ppo->xf.posWorld + ppo->xf.v * 0.5f;

    if (RATS_Nil < rats && rats < RATS_Dead && prat->tRatSqueak < g_clock.t)
    {
        StartSound((SFXID)59, nullptr, prat, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        prat->tRatSqueak = GRandGaussian(g_clock.t + 3.0f, 1.0f, 2.5f);
    }

    bool fPlayerTriggeredFlee = false;

    if (ppo != nullptr && (rats == RATS_Stopped || rats == RATS_Scurry))
    {
        const glm::vec3 dposPo = posPoSoon - prat->xf.posWorld;
        fPlayerTriggeredFlee = glm::dot(dposPo, dposPo) < 90000.0f;
    }

    if (fPlayerTriggeredFlee)
    {
        for (RAT* pratOther = prat->psw->dlRat.pratFirst; pratOther != nullptr; pratOther = pratOther->dleRat.pratNext)
        {
            if (pratOther == prat)
                continue;

            const glm::vec3 dposRat = pratOther->xf.posWorld - prat->xf.posWorld;

            if (glm::dot(dposRat, dposRat) < prat->sGroupRadius * prat->sGroupRadius)
                SetRatRats(pratOther, RATS_Flee);
        }

        rats = RATS_Flee;
    }
    else
    {
        switch (rats)
        {
            case RATS_Stopped:
            if (prat->tRatsNext < g_clock.t)
                rats = RATS_Scurry;
            break;

            case RATS_Scurry:
            {
                bool fLeap = false;

                if (prat->ipaloRender == 0 && prat->tRats + 0.5f < g_clock.t && glm::length(prat->xf.v) < prat->svScurry * 0.1f)
                {
                    int ixpd;
                    fLeap = PxpFindSoGround(prat, nullptr, &ixpd) != nullptr;
                }

                if (fLeap)
                {
                    glm::vec3 velocity = prat->xf.matWorld[0] * (prat->svScurry * 0.7f) + g_normalZ * prat->svLeap;
                    prat->pvtalo->pfnSetAloVelocityVec(prat, &velocity);
                    break;
                }

                const glm::vec3 dposTarget = prat->posWanderNext - prat->xf.posWorld;

                if (prat->tRatsNext < g_clock.t || glm::dot(dposTarget, dposTarget) < 400.0f)
                    rats = RATS_Stopped;

                break;
            }

            case RATS_Flee:
            {
                bool fLeap = false;

                if (prat->ipaloRender == 0 && prat->tRats + 0.7f < g_clock.t && glm::length(prat->xf.v) < prat->svFlee * 0.3f)
                {
                    int ixpd;
                    fLeap = PxpFindSoGround(prat, nullptr, &ixpd) != nullptr;
                }

                if (fLeap)
                {
                    glm::vec3 velocity = prat->xf.matWorld[0] * (prat->svFlee * 0.5f) + g_normalZ * prat->svLeap;
                    prat->pvtalo->pfnSetAloVelocityVec(prat, &velocity);
                    break;
                }

                if (prat->tRatsNext < g_clock.t)
                    rats = RATS_Scurry;
                else if (prat->prathole != nullptr && FCheckVolPoint(prat->prathole, &prat->xf.posWorld))
                    rats = RATS_Hidden;

                break;
            }

            case RATS_Hidden:
            {
                bool fKeepStill = true;

                if (prat->tRatsNext < g_clock.t && ppo != nullptr)
                {
                    const glm::vec3 dposPo = prat->xf.posWorld - ppo->xf.posWorld;

                    if (glm::dot(dposPo, dposPo) > 360000.0f)
                    {
                        bool fRatNearPlayer = false;
                        fKeepStill = false;

                        for (RAT* pratOther = prat->psw->dlRat.pratFirst; pratOther != nullptr; pratOther = pratOther->dleRat.pratNext)
                        {
                            const glm::vec3 dposOtherPo = pratOther->xf.posWorld - ppo->xf.posWorld;

                            if (glm::dot(dposOtherPo, dposOtherPo) < 90000.0f)
                            {
                                fRatNearPlayer = true;
                                break;
                            }
                        }

                        if (fRatNearPlayer)
                            prat->tRatsNext = g_clock.t + GRandInRange(0.2f, 3.0f);
                        else
                            rats = RATS_Scurry;
                    }
                }

                if (fKeepStill)
                {
                    prat->pvtalo->pfnSetAloVelocityVec(prat, &g_vecZero);
                    prat->pvtalo->pfnSetAloAngularVelocityVec(prat, &g_vecZero);
                }

                break;
            }

            default:
            break;
        }
    }

    SetRatRats(prat, rats);

    switch (rats)
    {
        case RATS_Stopped:
        case RATS_Dead:
        case RATS_Hidden:
        prat->svCur = 0.0f;
        break;

        case RATS_Scurry:
        case RATS_Flee:
        {
            float svTarget;

            if (rats == RATS_Scurry)
            {
                prat->dposTarget = prat->posWanderNext - prat->xf.posWorld;
                svTarget = prat->svScurry;
            }
            else
            {
                glm::vec3 posHoleWorld;
                ConvertAloPos(prat->prathole->paloParent, nullptr, &prat->prathole->posLocal, &posHoleWorld);

                prat->dposTarget = posHoleWorld - prat->xf.posWorld;
                svTarget = prat->svFlee;

                const float distanceToHole = glm::length(prat->dposTarget);

                if (distanceToHole < 500.0f)
                {
                    const float svNearHole = s_clqRatSvNearHole.g0 + distanceToHole * (s_clqRatSvNearHole.g1 + distanceToHole * s_clqRatSvNearHole.g2);
                    svTarget = std::min(svTarget, svNearHole);
                }
            }

            const glm::vec3 directionTarget = glm::normalize(prat->dposTarget);
            const float alignment = std::clamp(glm::dot(prat->xf.matWorld[0], directionTarget), 0.0f, 1.0f);

            prat->svCur = alignment * svTarget;

            ResolveAlo(prat);
            break;
        }

        default:
        break;
    }
}

void AdjustRatNewXp(RAT* prat, XP* pxp, int ixpd)
{
    if (glm::dot(g_normalZ, pxp->normal) > 0.7f) {
        if (prat->rats == RATS_Scurry) {
            pxp->mu = 3.0f;
        }
        else {
            pxp->mu = 5.0f;
        }

        pxp->elas = 0.1f;
    }
}

void AdjustRatXpVelocity(RAT* prat, XP* pxp, int ixpd)
{
    if (pxp->normal.z * g_mpixpdr[ixpd] < 0.7f || prat->svCur <= 0.0001f) {
        return;
    }

    glm::vec3 v = -prat->xf.matWorld[0] * prat->svCur;
    v -= prat->xf.matWorld[2] * glm::dot(v, prat->xf.matWorld[2]);

    pxp->axpd[ixpd].v += v;
}

void RenderRatAll(RAT* prat, CM* pcm, RO* pro)
{
    if (prat->rats == RATS_Hidden || prat->fHidden)
        return;

    RO ro;
    DupAloRo(prat, pro, &ro);

    ALO* paloRender = nullptr;

    switch (prat->rats)
    {
        case RATS_Scurry:
        {
            const glm::vec3 dpos = prat->xf.posWorld - prat->posFrameLast;
            int ipaloRender = prat->ipaloRender;

            if (prat->dsFrameScurry * prat->dsFrameScurry < glm::dot(dpos, dpos))
            {
                assert(prat->cpaloRenderScurry > 0);

                prat->posFrameLast = prat->xf.posWorld;
                prat->ipaloRender = (ipaloRender + 1) % prat->cpaloRenderScurry;
                ipaloRender = prat->ipaloRender;
            }

            paloRender = prat->apaloRenderScurry[ipaloRender];
            break;
        }

        case RATS_Flee:
        {
            const glm::vec3 dpos = prat->xf.posWorld - prat->posFrameLast;
            int ipaloRender = prat->ipaloRender;

            if (prat->dsFrameFlee * prat->dsFrameFlee < glm::dot(dpos, dpos))
            {
                assert(prat->cpaloRenderFlee > 0);

                prat->posFrameLast = prat->xf.posWorld;
                prat->ipaloRender = (ipaloRender + 1) % prat->cpaloRenderFlee;
                ipaloRender = prat->ipaloRender;
            }

            paloRender = prat->apaloRenderFlee[ipaloRender];
            break;
        }

        case RATS_Stopped:
        paloRender = prat->paloRenderStopped;
        break;

        default:
        return;
    }

    paloRender->pvtalo->pfnRenderAloAll(paloRender, pcm, &ro);
}

int FAbsorbRatWkr(RAT* prat, WKR* pwkr)
{
    EXPL* pexpl;
    // Only position and impact velocity are supplied for a rat death.  Keep
    // every optional explosion field deterministic; otherwise stale stack
    // data can be interpreted as scale, touch, transform, or particle-count
    // overrides by nested explosion styles.
    EXPLSO explso{};

    if (prat->rats == RATS_Dead)
        return 1;

    SetRatRats(prat, RATS_Dead);

    pexpl = prat->pexpl;
    if (pexpl != nullptr) 
    {
        explso.posOrigin = pwkr->pos;
        explso.vec = pwkr->v;
        explso.grfexplso = 6;

        pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, &explso);
    }

    return 1;
}

void ChooseRatFleeTarget(RAT* prat)
{
    PO* player = PpoCur();
    if (player == nullptr)
        return;

    RATHOLE* bestHole = nullptr;
    float bestLineDistance = 0.0f;
    float bestU = 0.0f;
    float bestRatDistance = std::numeric_limits<float>::max();

    glm::vec3 ratPosition = glm::vec3(prat->xf.posWorld);
    glm::vec3 playerPosition = glm::vec3(player->xf.posWorld);
    const float groupRadiusSquared = prat->sGroupRadius * prat->sGroupRadius;

    DL& ratholeList = prat->psw->dlRathole;

    for (RATHOLE* hole = ratholeList.pratholeFirst; hole != nullptr; hole = hole->dleRathole.pratholeNext) 
    {
        glm::vec3 holePosition;
        ConvertAloPos(hole->paloParent, nullptr, &hole->posLocal, &holePosition);

        const glm::vec3 ratToHole = holePosition - ratPosition;
        const float ratDistanceSquared = glm::dot(ratToHole, ratToHole);

        if (ratDistanceSquared > groupRadiusSquared) {
            continue;
        }

        float u;
        float lineDistance;
        FindClosestPointOnLineSegment(&playerPosition, &ratPosition, &holePosition, &u, &lineDistance);

        const float ratDistance = glm::sqrt(ratDistanceSquared);

        const bool hasBetterLineDistance = lineDistance > bestLineDistance;
        const bool bothBehindSegmentStart = u < 0.0001f && bestU < 0.0001f;
        const bool isCloserToRat = ratDistance < bestRatDistance;

        if (bestHole == nullptr || hasBetterLineDistance || (bothBehindSegmentStart && isCloserToRat)) {
            bestHole = hole;
            bestLineDistance = lineDistance;
            bestU = u;
            bestRatDistance = ratDistance;
        }
    }

    prat->prathole = bestHole;
}

void HideRat(RAT* prat, int fHide)
{
    const bool hidden = fHide != 0;
    const bool enablePhysics = !hidden;

    if (hidden) 
    {
        RemoveDlEntry(&prat->psw->dlRat, prat);
        prat->ptarget->pvtlo->pfnRemoveLo(prat->ptarget);
    }
    else 
    {
        AppendDlEntry(&prat->psw->dlRat, prat);
        prat->ptarget->pvtlo->pfnAddLo(prat->ptarget);

        glm::vec3 worldPosition;
        ConvertAloPos(prat->prathole->paloParent, nullptr, &prat->prathole->posLocal, &worldPosition);
        TranslateSoToPosSafe(prat, &worldPosition, &g_normalZ);
    }

    EnableSoPhys(prat, enablePhysics);

    DL& physicsList = prat->dlPhys;

    for (SO* child = physicsList.psoFirst; child != nullptr; child = child->dlePhys.psoNext)
        EnableSoPhys(child, enablePhysics);

    prat->pvtalo->pfnSetAloVelocityVec(prat, &g_vecZero);
    prat->pvtalo->pfnSetAloAngularVelocityVec(prat, &g_vecZero);
}

void SetRatRats(RAT* prat, RATS rats)
{
    if (prat->rats == rats)
        return;

    if (prat->rats == RATS_Hidden) {
        HideRat(prat, 0);
        prat->prathole = nullptr;
    }

    prat->ptarget->grftak = 14;
    prat->tRatsNext = g_clock.t;

    switch (rats) 
    {
        case RATS_Stopped:
        prat->tRatsNext += GRandInRange(prat->lmDtStop.gMin, prat->lmDtStop.gMax);
        prat->ipaloRender = 0;
        prat->posFrameLast = prat->xf.posWorld;
        break;

        case RATS_Scurry:
        {
            const float angle = GRandInRange(0.0f, 6.2831855f);
            const float distance = GRandInRange(0.0f, prat->sScurry);

            glm::vec3 positionOffset;
            SetVectorCylind(&positionOffset, angle, distance, 0.0f);

            prat->ipaloRender = 0;
            prat->posWanderNext = prat->posOrig + positionOffset;
            prat->tRatsNext += GRandInRange(3.0f, 5.0f);
            break;
        }

        case RATS_Flee:
        ChooseRatFleeTarget(prat);
        prat->ipaloRender = 0;
        prat->tRatsNext += 10.0f;
        break;

        case RATS_Dead:
        {
            glm::vec3* position = &prat->xf.posWorld;
            StartSound((SFXID)58, nullptr, nullptr, position, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            OnDifficultyBreak(&g_difficulty, position, prat->ccoin, prat->fSuppressDifficultyUpdate);
            prat->pvtlo->pfnRemoveLo(prat);
            break;
        }

        case RATS_Hidden:
        HideRat(prat, 1);
        prat->tRatsNext += GRandInRange(3.0f, 15.0f);
        break;
    }

    prat->rats = rats;
    prat->tRats = g_clock.t;
}

void DeleteRat(RAT *prat)
{
	delete prat;
}

RATHOLE* NewRathole()
{
	return new RATHOLE{};
}

void InitSwRatholeDl(SW* psw)
{
	InitDl(&psw->dlRathole, offsetof(RATHOLE, dleRathole));
}

int GetRatholeSize()
{
	return sizeof(RATHOLE);
}

void OnRatholeRemove(RATHOLE* prathole)
{
	OnLoRemove(prathole);
	RemoveDlEntry(&prathole->psw->dlRathole, prathole);
}

void CloneRathole(RATHOLE* prathole, RATHOLE* pratholeBase)
{
    CloneVol(prathole, pratholeBase);

    prathole->dleRathole = pratholeBase->dleRathole;
}

void OnRatholeAdd(RATHOLE* prathole)
{
	OnLoAdd(prathole);
	AppendDlEntry(&prathole->psw->dlRathole, prathole);
}

void DeleteRathole(RATHOLE* prathole)
{
	delete prathole;
}

SNIP s_asnipRat = 
{
    0, (OID)0x268, offsetof(RAT, paloRenderStopped)
};
SNIP s_asnipPostRatLoad = 
{
    4, (OID)0x267, offsetof(RAT, ptarget)
};

SMP s_smpRatSpin = {12.0, 0.0, 0.12};
CLQ s_clqRatSvNearHole = {500, 1.0, 0.0};
