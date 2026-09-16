#include "stephide.h"
#include "jt.h"
#include "pzo.h"
#include "jump.h"
#include "rail.h"
#include "actval.h"
#include "pipe.h"
#include "rail.h"
#include "spire.h"
#include "hnd.h"
#include "steprun.h"
#include "actadj.h"
#include "stepside.h"
#include "stepcane.h"
#include "stephang.h"
#include "stepguard.h"

int JtbsChooseJtHide(JT* pjt, LO* ploForce, int* pjthk)
{
    int hideType = -1;
    float distanceBest = ploForce != nullptr ? -FLT_MAX : S_JtFlattenMax;

    HSHAPE* phshapeBest = nullptr;
    HPNT* phpntBest = nullptr;
    HBSK* phbskBest = nullptr;
    VAULT* pvaultBest = nullptr;

    float sHshapeBest = 0.0f;

    for (HSHAPE* phshape = g_dlHshape.phshapeFirst; phshape != nullptr; phshape = phshape->dleHshape.phshapeNext)
    {
        glm::vec3 posJtLocal;
        glm::vec3 posClosest;
        float sHshape = 0.0f;

        ConvertAloPos(nullptr, phshape->paloParent, &pjt->xf.posWorld, &posJtLocal);

        CRV* pcrv = phshape->pcrv.get();
        if (pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
            pcrv->pvtcrv->pfnFindCrvClosestPointAll(pcrv, &posJtLocal, nullptr, &posClosest, nullptr, nullptr, &sHshape);

        const float distance = glm::distance(posJtLocal, posClosest);

        if (distance < distanceBest || phshape == ploForce)
        {
            distanceBest = std::min(distanceBest, distance);
            hideType = 0;
            phshapeBest = phshape;
            sHshapeBest = sHshape;
        }
    }

    for (HPNT* phpnt = g_dlHpnt.phpntFirst; phpnt != nullptr; phpnt = phpnt->dleHpnt.phpntNext)
    {
        glm::vec3 posHpnt;
        GetPntPos(reinterpret_cast<PNT*>(phpnt), &posHpnt);

        const float distance = std::abs(glm::distance(posHpnt, pjt->xf.posWorld) - phpnt->sFlattenRadius);

        if (distance < distanceBest || phpnt == ploForce)
        {
            distanceBest = std::min(distanceBest, distance);
            hideType = 1;
            phpntBest = phpnt;
        }
    }

    for (HBSK* phbsk = g_dlHbsk.phbskFirst; phbsk != nullptr; phbsk = phbsk->dleHbsk.phbskNext)
    {
        // An attached basket remains in the global HBSK list. Retail only
        // considers available baskets unless the caller explicitly forces one.
        if (phbsk->hbsks != HBSKS_Available && phbsk != ploForce)
            continue;

        const float distance = std::abs(glm::distance(phbsk->xf.posWorld, pjt->xf.posWorld) - phbsk->sFlattenRadius);

        if (distance < distanceBest || phbsk == ploForce)
        {
            distanceBest = std::min(distanceBest, distance);
            hideType = 2;
            phbskBest = phbsk;
        }
    }

    /*
     * Release-only searches assign:
     *
     * hideType 5: vault
     * hideType 6: HND reach target
     * hideType 7: secondary/dynamic HSHAPE reach target
     *
     * Keep their existing search code here until the final three jump-table
     * destinations are recovered.
     */

    if (pjt->psw->pvault != nullptr &&
        FCanOpenVault(pjt->psw->pvault) &&
        g_plsCur->abitClue.m_cbitSet >= pjt->psw->cclueAll &&
        !FWouldCompleteVaultSet(pjt->psw->pvault->fVault))
    {
        glm::vec3 posVault;
        GetXfmPos(pjt->psw->pvault->pxfmJt, &posVault);

        const float distance = glm::distance(posVault, pjt->xf.posWorld);

        if (distance < S_JtVaultTrigger || pjt->psw->pvault == ploForce)
        {
            hideType = 5;
            pvaultBest = pjt->psw->pvault;
        }
    }

    switch (hideType)
    {
        case 0:
        *pjthk = phshapeBest->jthk;
        pjt->phshape = phshapeBest;
        pjt->sParamHshape = sHshapeBest;
        return 21;

        case 1:
        *pjthk = phpntBest->jthk;
        pjt->phpnt = phpntBest;
        return 21;

        case 2:
        *pjthk = JTHK_Basket;
        pjt->phbsk = phbskBest;
        return 21;

        case 5:
        *pjthk = JTHK_Vault;
        pjt->pvaultCur = pvaultBest;
        return 16;

        default:
        if ((GetAvailableVaultFlags() & 0x12000) == 0)
        {
            *pjthk = -1;
            return -1;
        }

        *pjthk = 3;
        return 16;
    }
}

void MeasureJtJumpToTarget(JT* pjt, glm::vec3* pvJump, ALO* paloTarget, glm::vec3* pposTarget, glm::vec3* pdvTarget, float* pdtJump, float* pgInteg, glm::vec3* pposPredict, glm::vec3* pvPredict)
{
    glm::vec3 vTarget;
    CalculateAloMovement(paloTarget, nullptr, *pposTarget, &vTarget, nullptr, nullptr, nullptr);

    float dtPredict;
    if (!FTimeJtJumpToTarget(pjt, pjt->pposBase, pvJump, pposTarget, &vTarget, &dtPredict))
    {
        const float gravityZ = pjt->dvGravity.z;
        const float relativeVz = pvJump->z - vTarget.z;
        const float dtClosest = std::max(-relativeVz / gravityZ, 0.0f);

        if (pdtJump != nullptr)
            *pdtJump = dtClosest;

        if (pgInteg != nullptr)
        {
            const float dz = pjt->pposBase->z - pposTarget->z;
            const float dzProjected = dz + relativeVz * dtClosest + 0.5f * gravityZ * dtClosest * dtClosest;

            *pgInteg = 1010000.0f - dzProjected;
        }

        return;
    }

    const float dtTargetPrediction = std::min(dtPredict, 0.15f);
    glm::vec3 posPredict = *pposTarget + vTarget * dtTargetPrediction;

    if (pdvTarget != nullptr)
        vTarget += *pdvTarget;

    if (pdtJump != nullptr)
        *pdtJump = dtPredict;

    if (pgInteg != nullptr)
    {
        glm::vec3 dvStart;
        glm::vec3 dvFinish;

        EvaluateBezierPos(dtPredict, 0.0f, 1.0f, pjt->pposBase, pvJump, &posPredict, &vTarget, nullptr, nullptr, &dvStart);
        EvaluateBezierPos(dtPredict, dtPredict, 1.0f, pjt->pposBase, pvJump, &posPredict, &vTarget, nullptr, nullptr, &dvFinish);

        const float distanceEstimate = 0.5f * (glm::length(glm::vec2(dvStart)) + glm::length(glm::vec2(dvFinish))) * dtPredict;
        *pgInteg = std::min(distanceEstimate, 1000000.0f);
    }

    if (pposPredict != nullptr)
        *pposPredict = posPredict;

    if (pvPredict != nullptr)
        *pvPredict = vTarget;
}

void GetJtRailLanding(JT* pjt, RAIL* prail, float uRail, glm::vec3* ppos, glm::vec3* pv)
{
    glm::vec3 posLocal;
    glm::vec3 tangentLocal;

    if (prail->pcrv->pvtcrv->pfnEvaluateCrvFromU != nullptr)
        prail->pcrv->pvtcrv->pfnEvaluateCrvFromU(prail->pcrv.get(), uRail, &posLocal, &tangentLocal);

    glm::vec3 pos;
    glm::vec3 tangent;

    ConvertAloPos(prail->paloParent, nullptr, &posLocal, &pos);
    ConvertAloVec(prail->paloParent, nullptr, &tangentLocal, &tangent);

    const float runSpeed = glm::length(pjt->ablrunRail[1].v);

    glm::vec3 vDesired;
    SetVectorCylind(&vDesired, pjt->radJoy, runSpeed * pjt->uJoyDeflect, 0.0f);

    const float svDesired = glm::dot(vDesired, tangent);
    const float sRail = prail->pcrv->pvtcrv->pfnSFromCrvU(prail->pcrv.get(), uRail);

    SMP smpRailLimit{};
    smpRailLimit.svFast = -pjt->ablrunRail[1].v.x;
    smpRailLimit.svSlow = -pjt->ablrunRail[0].v.x;
    smpRailLimit.dtFast = DT_JtRailLimit;

    float svMin;
    GSmooth(sRail, DS_JtRailLimit, g_clock.dt, &smpRailLimit, &svMin);

    const float sRailMax = prail->pcrv->pvtcrv->pfnSMaxCrv(prail->pcrv.get());

    float svMax;
    GSmooth(sRail, sRailMax - DS_JtRailLimit, g_clock.dt, &smpRailLimit, &svMax);

    *ppos = pos;
    *pv = tangent * glm::clamp(svDesired, svMin, svMax);
}

float GMeasureJumpRail(MJR* pmjr, float u)
{
    glm::vec3 posTarget;
    glm::vec3 vTarget;
    float cost;

    GetJtRailLanding(pmjr->pjt, pmjr->prail, u, &posTarget, &vTarget);
    MeasureJtJumpToTarget(pmjr->pjt, &pmjr->vJump, pmjr->prail->paloParent, &posTarget, &vTarget, nullptr, &cost, nullptr, nullptr);

    return cost;
}

float GMeasureJumpHpnt(MJHPNT* pmjhpnt, float u)
{
    glm::vec3 posTarget;
    float cost;

    GetHpntClosestHidePos(pmjhpnt->phpnt, u, &posTarget, nullptr);
    MeasureJtJumpToTarget(pmjhpnt->pjt, &pmjhpnt->vJump, pmjhpnt->phpnt->paloParent, &posTarget, nullptr, nullptr, &cost, nullptr, nullptr);

    return cost;
}

float GMeasureJumpToHshape(MJH* pmjh, float s)
{
    glm::vec3 posTarget;
    float cost;

    GetHshapeHidePos(pmjh->phshape, s, &posTarget, nullptr);
    MeasureJtJumpToTarget(pmjh->pjt, &pmjh->vJump, pmjh->phshape->paloParent, &posTarget, nullptr, nullptr, &cost, nullptr, nullptr);

    return cost;
}

void MeasureJtJumpToTargetAtTime(float dt, JT* pjt, ALO* paloTargetParent, const glm::vec3* pposTargetLocal, float* pgMeasure, glm::vec3* pposGoal, glm::vec3* pvGoal)
{
    const float targetRadius = 250.0;

    const glm::vec3 posBallistic =
        pjt->xf.pos +
        pjt->xf.v * dt +
        pjt->dvGravity * (0.5f * dt * dt);

    const glm::vec3 vBallistic =
        pjt->xf.v +
        pjt->dvGravity * dt;

    glm::vec3 posTarget = *pposTargetLocal;
    PredictAloTransformAdjust(paloTargetParent, nullptr, dt, &posTarget, nullptr, nullptr, nullptr);

    const glm::vec3 dpos = posTarget - posBallistic;
    const float distSq = glm::dot(dpos, dpos);

    glm::vec3 posAdjusted = posBallistic;
    glm::vec3 vAdjusted = vBallistic;

    if (distSq < targetRadius * targetRadius)
    {
        if (pgMeasure != nullptr)
            *pgMeasure = glm::length(dpos) - targetRadius;
    }
    else if (std::abs(dpos.z) <= targetRadius && dt != 0.0f)
    {
        const glm::vec3 dposHorizontal(dpos.x, dpos.y, 0.0f);
        const float horizontalDistance = glm::length(dposHorizontal);

        const glm::vec3 horizontalDirection =
            horizontalDistance < 0.0001f
            ? glm::vec3(0.0f)
            : dposHorizontal / horizontalDistance;

        // Horizontal distance needed to reach the edge of the target sphere
        // at the current vertical separation.
        const float horizontalCorrection =
            horizontalDistance -
            std::sqrt(targetRadius * targetRadius - dpos.z * dpos.z);

        const glm::vec3 accelerationCorrection =
            horizontalDirection * (2.0f * horizontalCorrection / (dt * dt));

        if (pgMeasure != nullptr)
        {
            const float measure =
                (2.0f * 2.0 * horizontalCorrection) / dt;

            *pgMeasure = std::min(measure, 1000000.0f);
        }

        posAdjusted += accelerationCorrection * (0.5f * dt * dt);
        vAdjusted += accelerationCorrection * dt;
    }
    else
    {
        // Large penalty tells the minimizer this time is not a viable solution.
        if (pgMeasure != nullptr)
            *pgMeasure = std::abs(dpos.z) + 1010000.0f;
    }

    if (pposGoal != nullptr)
        *pposGoal = posAdjusted;

    if (pvGoal != nullptr)
        *pvGoal = vAdjusted;
}

float GMeasureJumpTargetTime(MJT* pmjt, float dt)
{
    float g;

    MeasureJtJumpToTargetAtTime(dt, pmjt->pjt, pmjt->paloTargetParent, &pmjt->posTargetLocal, &g, nullptr, nullptr);

    return g;
}

float GMeasureJumpPipe(MJP* pmjp, float s)
{
    glm::vec3 posTargetLocal{};

    CRV* pcrv = pmjp->ppipe->pcrv.get();

    if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
        pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, s, &posTargetLocal, nullptr);

    MJT mjt{};
    mjt.pjt = pmjp->pjt;
    mjt.paloTargetParent = pmjp->ppipe->paloParent;
    mjt.posTargetLocal = posTargetLocal;

    float g;

    MinimizeRange((PFNGG)GMeasureJumpTargetTime, &mjt, 0.5f, 0.25f, 0.25f, 3.0f, nullptr, &g);

    return g;
}

bool FFindJtObstruction(JT* pjt, float dt, PFNFILTER pfnFilter, void* pvFilter, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, glm::vec3* pposFinal)
{
    glm::vec3 bboxMin = *ppos0;
    glm::vec3 bboxMax = *ppos1;

    if (pposFinal != nullptr)
    {
        bboxMin = glm::min(bboxMin, *pposFinal);
        bboxMax = glm::max(bboxMax, *pposFinal);
    }

    std::vector<SO*> objects;
    IntersectSwBoundingBox(pjt->psw, reinterpret_cast<SO*>(pjt), &bboxMin, &bboxMax, pfnFilter, pvFilter, objects);

    glm::vec3 posPrev{};
    glm::vec3 posCur{};

    for (float t = 0.0f;; t = std::min(t + 0.1f, dt))
    {
        EvaluateBezierPos(dt, t, 1.0f, ppos0, pv0, ppos1, pv1, &posCur, nullptr, nullptr);

        if (t != 0.0f)
        {
            LSG lsg{};
            SO* psoHit = PsoHitTestLineObjects(static_cast<GRFHTL>(0), &posPrev, &posCur, objects, &lsg);

            if (psoHit != nullptr && glm::distance(lsg.apos[0], *ppos1) > 100.0f)
                return true;
        }

        if (t == dt)
            break;

        posPrev = posCur;
    }

    if (pposFinal != nullptr)
    {
        LSG lsg{};
        SO* psoHit = PsoHitTestLineObjects(static_cast<GRFHTL>(0), &posPrev, pposFinal, objects, &lsg);

        if (psoHit != nullptr && lsg.au[0] != 1.0f && glm::distance(lsg.apos[0], *pposFinal) > 50.0f)
            return true;
    }

    return false;
}

float GAdjustJtLandingScore(float g, float gPenalty, JT* pjt, LO* ploCandidate)
{
    g += gPenalty;

    if (ploCandidate == pjt->ploJumpSource)
        g += 1000.0;;

    return g;
}

int JtbsChooseJtLanding(JT* pjt, LO* ploForce)
{
    constexpr int HOK_None = -1;
    constexpr int HOK_Shape = 0;
    constexpr int HOK_Point = 1;
    constexpr int HOK_Spire = 3;
    constexpr int HOK_Rail = 4;
    constexpr int HOK_Target = 6;
    constexpr int HOK_Pipe = 7;

    constexpr float S_TargetSearchMax = 1000.0f;

    constexpr float DS_RailLimit = 50.0f;
    constexpr float DS_PipeSearch = 100.0f;
    constexpr float DS_PipeEnd = 50.0f;
    constexpr float DS_HshapeSearch = 100.0f;
    constexpr float DS_HshapeEnd = 30.0f;

    constexpr float G_SpirePenalty = -250.0f;
    constexpr float G_RailPenalty = 0.0f;
    constexpr float G_TargetPenalty = 0.0f;

    // Release globals at 0x00274e30 and 0x00274e34.
    constexpr float G_PipePenalty = 0.0f;
    constexpr float G_HshapePenalty = 0.0f;

    constexpr float G_HpntPenalty = 500.0f;
    constexpr float G_CircularTargetScale = 1.25f;

    constexpr float DT_SearchStart = 0.5f;
    constexpr float DT_SearchStep = 0.25f;
    constexpr float DT_SearchMin = 0.25f;
    constexpr float DT_SearchMax = 3.0f;

    constexpr float RAD_SearchStep = 0.1f;
    constexpr float RAD_SearchMin = -3.1415927f;
    constexpr float RAD_SearchMax = 3.1415927f;

    int bestKind = HOK_None;
    // A forced target must be accepted regardless of the normal automatic
    // landing score. This matches the release's -FLT_MAX initialization.
    float gBest = ploForce != nullptr ? -FLT_MAX : G_JtAutoJumpMax;
    float dtBest = 0.0f;

    glm::vec3 posBest(0.0f);
    glm::vec3 vBest(0.0f);
    glm::vec3 vJump(0.0f);

    SPIRE* pspireBest = nullptr;
    RAIL* prailBest = nullptr;
    HND* phndBest = nullptr;
    PIPE* ppipeBest = nullptr;
    HPNT* phpntBest = nullptr;
    HSHAPE* phshapeBest = nullptr;

    float uRailBest = 0.0f;
    float sPipeBest = 0.0f;
    float radHpntBest = 0.0f;
    float sHshapeBest = 0.0f;

    GetJtJumpToTargetVelocity(pjt, &vJump);

    auto CandidateLo = [](auto* object) -> LO*
    {
        return reinterpret_cast<LO*>(object);
    };

    auto IsForced = [&](auto* object) -> bool
    {
        return ploForce != nullptr && CandidateLo(object) == ploForce;
    };

    auto ShouldInspect = [&](auto* object) -> bool
    {
        return ploForce == nullptr || CandidateLo(object) == ploForce;
    };

    auto CommitCandidate = [&](int kind, float g, float dt, const glm::vec3& posGoal, const glm::vec3& vGoal)
        {
            bestKind = kind;
            gBest = g;
            dtBest = dt;
            posBest = posGoal;
            vBest = vGoal;
        };

    // ---------------------------------------------------------------------
    // Spires
    // ---------------------------------------------------------------------

    for (SPIRE* pspire = pjt->psw->dlSpire.pspireFirst; pspire != nullptr; pspire = pspire->dleSpire.pspireNext)
    {
        if (!ShouldInspect(pspire))
            continue;

        glm::vec3 posTarget(0.0f);
        glm::vec3 posGoal(0.0f);
        glm::vec3 vGoal(0.0f);
        float dt = 0.0f;
        float g = 0.0f;

        GetPntPos(reinterpret_cast<PNT*>(pspire), &posTarget);

        if (!IsForced(pspire) && pjt->xf.pos.z - posTarget.z > pspire->dzJumpTargetMax)
            continue;

        MeasureJtJumpToTarget(pjt, &vJump, pspire->paloParent, &posTarget, nullptr, &dt, &g, &posGoal, &vGoal);

        // The release replaces the predicted target Z velocity with Sly's
        // ballistic velocity before checking the Bezier path for obstruction.
        vGoal.z = vJump.z + dt * pjt->dvGravity.z;

        if (!IsForced(pspire))
        {
            if (g >= G_JtAutoJumpMax)
                continue;

            g = GAdjustJtLandingScore(g, G_SpirePenalty, pjt, CandidateLo(pspire));

            if (g >= gBest)
                continue;

            const int obstructed = FFindJtObstruction(pjt, dt, nullptr, nullptr, pjt->pposBase, &vJump, &posGoal, &vGoal, nullptr);
            if (obstructed)
                continue;
        }

        CommitCandidate(HOK_Spire, g, dt, posGoal, vGoal);
        pspireBest = pspire;
    }

    // ---------------------------------------------------------------------
    // Rails
    // ---------------------------------------------------------------------

    for (RAIL* prail = pjt->psw->dlRail.prailFirst; prail != nullptr; prail = prail->dleRail.prailNext)
    {
        if (!ShouldInspect(prail))
            continue;

        CRV* pcrv = prail->pcrv.get();

        if (pcrv == nullptr)
            continue;

        glm::vec3 posJtLocal(0.0f);
        glm::vec3 posClosestLocal(0.0f);
        glm::vec3 posClosestWorld(0.0f);
        float uClosest = 0.0f;

        ConvertAloPos(nullptr, prail->paloParent, pjt->pposBase, &posJtLocal);

        if (pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
            pcrv->pvtcrv->pfnFindCrvClosestPointAll(pcrv, &posJtLocal, nullptr, &posClosestLocal, nullptr, &uClosest, nullptr);

        ConvertAloPos(prail->paloParent, nullptr, &posClosestLocal, &posClosestWorld);

        if (!IsForced(prail) && pjt->xf.pos.z - posClosestWorld.z > prail->dzJumpTargetMax)
            continue;

        MJR mjr{};
        mjr.pjt = pjt;
        mjr.prail = prail;
        mjr.vJump = vJump;

        const float du = DuGetCrvSearchIncrement(pcrv);
        const float sMax = pcrv->pvtcrv->pfnSMaxCrv(pcrv);
        const float uMin = pcrv->pvtcrv->pfnUFromCrvS(pcrv, DS_RailLimit);
        const float uMax = pcrv->pvtcrv->pfnUFromCrvS(pcrv, sMax - DS_RailLimit);

        float uRail = 0.0f;
        float g = 0.0f;

        MinimizeRange(reinterpret_cast<PFNGG>(GMeasureJumpRail), &mjr, uClosest, du, uMin, uMax, &uRail, &g);

        if (!IsForced(prail))
        {
            if (g >= G_JtAutoJumpMax)
                continue;

            g = GAdjustJtLandingScore(g, G_RailPenalty, pjt, CandidateLo(prail));

            if (g >= gBest)
                continue;
        }

        glm::vec3 posTarget(0.0f);
        glm::vec3 vTarget(0.0f);
        glm::vec3 posGoal(0.0f);
        glm::vec3 vGoal(0.0f);
        float dt = 0.0f;

        GetJtRailLanding(pjt, prail, uRail, &posTarget, &vTarget);
        MeasureJtJumpToTarget(pjt, &vJump, prail->paloParent, &posTarget, &vTarget, &dt, nullptr, &posGoal, &vGoal);

        // As with spires, obstruction testing uses Sly's final ballistic Z
        // velocity rather than the target object's vertical velocity.
        vGoal.z = vJump.z + dt * pjt->dvGravity.z;

        if (!IsForced(prail) && FFindJtObstruction(pjt, dt, nullptr, nullptr, pjt->pposBase, &vJump, &posGoal, &vGoal, nullptr))
            continue;

        CommitCandidate(HOK_Rail, g, dt, posGoal, vGoal);
        prailBest = prail;
        uRailBest = uRail;
    }

    // ---------------------------------------------------------------------
    // Handles / hook targets
    // ---------------------------------------------------------------------

    for (HND* phnd = g_dlTarget.phndFirst; phnd != nullptr; phnd = phnd->dleTarget.phndNext)
    {
        if (!ShouldInspect(phnd))
            continue;

        if (!IsForced(phnd))
        {
            if (phnd == pjt->phndUnhook)
                continue;

            if ((phnd->grftak & 1) == 0)
                continue;

            if (!FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(phnd), CID_HND))
                continue;
        }

        glm::vec3 posTargetWorld(0.0f);
        GetXfmPos(reinterpret_cast<XFM*>(phnd), &posTargetWorld);

        if (!IsForced(phnd))
        {
            if (pjt->xf.pos.z - posTargetWorld.z > phnd->dzJumpTargetMax)
                continue;

            if (glm::distance2(posTargetWorld, pjt->xf.posWorld) > S_TargetSearchMax * S_TargetSearchMax)
                continue;
        }

        MJT mjt{};
        mjt.pjt = pjt;
        mjt.paloTargetParent = phnd->paloParent;
        mjt.posTargetLocal = phnd->posLocal;

        float dt = 0.0f;
        float g = 0.0f;

        MinimizeRange(reinterpret_cast<PFNGG>(GMeasureJumpTargetTime), &mjt, DT_SearchStart, DT_SearchStep, DT_SearchMin, DT_SearchMax, &dt, &g);

        if (!IsForced(phnd))
        {
            if (g >= G_JtAutoJumpMax)
                continue;

            g = GAdjustJtLandingScore(g, G_TargetPenalty, pjt, CandidateLo(phnd));

            if (g >= gBest)
                continue;
        }

        glm::vec3 posGoal(0.0f);
        glm::vec3 vGoal(0.0f);

        MeasureJtJumpToTargetAtTime(dt, pjt, phnd->paloParent, &phnd->posLocal, nullptr, &posGoal, &vGoal);

        if (!IsForced(phnd))
        {
            glm::vec3 posFinal(0.0f);
            ConvertAloPos(phnd->paloParent, nullptr, &phnd->posLocal, &posFinal);

            if (FFindJtObstruction(pjt, dt, nullptr, nullptr, &pjt->xf.posWorld, &pjt->xf.v, &posGoal, &vGoal, &posFinal))
                continue;
        }

        // The common finalization code adds 75 back.
        posGoal.z -= 75.0f;

        CommitCandidate(HOK_Target, g, dt, posGoal, vGoal);
        phndBest = phnd;
    }

    // ---------------------------------------------------------------------
    // Pipes
    // ---------------------------------------------------------------------

    for (PIPE* ppipe = g_dlPipe.ppipeFirst; ppipe != nullptr; ppipe = ppipe->dlePipe.ppipeNext)
    {
        if (!ShouldInspect(ppipe))
            continue;

        CRV* pcrv = ppipe->pcrv.get();

        if (pcrv == nullptr)
            continue;

        glm::vec3 posJtLocal(0.0f);
        glm::vec3 posClosestLocal(0.0f);
        glm::vec3 posClosestWorld(0.0f);
        float sClosest = 0.0f;

        ConvertAloPos(nullptr, ppipe->paloParent, &pjt->xf.posWorld, &posJtLocal);

        if (pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
            pcrv->pvtcrv->pfnFindCrvClosestPointAll(pcrv, &posJtLocal, nullptr, &posClosestLocal, nullptr, nullptr, &sClosest);

        ConvertAloPos(ppipe->paloParent, nullptr, &posClosestLocal, &posClosestWorld);

        const float dzPipe = pjt->xf.pos.z - posClosestWorld.z;
        const float distSqPipe = glm::distance2(pjt->xf.posWorld, posClosestWorld);

        if (!IsForced(ppipe))
        {
            if (dzPipe > ppipe->dzJumpTargetMax)
                continue;

            if (distSqPipe > S_TargetSearchMax * S_TargetSearchMax)
                continue;
        }

        MJP mjp{};
        mjp.pjt = pjt;
        mjp.ppipe = ppipe;

        const float sMax = pcrv->pvtcrv->pfnSMaxCrv(pcrv);

        float sPipe = 0.0f;
        float gPipe = 0.0f;

        MinimizeRange(reinterpret_cast<PFNGG>(GMeasureJumpPipe), &mjp, sClosest, DS_PipeSearch, DS_PipeEnd, sMax - DS_PipeEnd, &sPipe, &gPipe);

        if (!IsForced(ppipe))
        {
            if (gPipe >= G_JtAutoJumpMax)
                continue;

            gPipe = GAdjustJtLandingScore(gPipe, G_PipePenalty, pjt, CandidateLo(ppipe));

            if (gPipe >= gBest)
                continue;
        }

        glm::vec3 posTargetLocal(0.0f);

        if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
            pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, sPipe, &posTargetLocal, nullptr);

        MJT mjt{};
        mjt.pjt = pjt;
        mjt.paloTargetParent = ppipe->paloParent;
        mjt.posTargetLocal = posTargetLocal;

        float dt = 0.0f;

        MinimizeRange(reinterpret_cast<PFNGG>(GMeasureJumpTargetTime), &mjt, DT_SearchStart, DT_SearchStep, DT_SearchMin, DT_SearchMax, &dt, nullptr);

        glm::vec3 posGoal(0.0f);
        glm::vec3 vGoal(0.0f);

        MeasureJtJumpToTargetAtTime(dt, pjt, ppipe->paloParent, &posTargetLocal, nullptr, &posGoal, &vGoal);

        if (!IsForced(ppipe))
        {
            glm::vec3 posFinal(0.0f);
            ConvertAloPos(ppipe->paloParent, nullptr, &posTargetLocal, &posFinal);

            if (FFindJtObstruction(pjt, dt, nullptr, nullptr, &pjt->xf.posWorld, &pjt->xf.v, &posGoal, &vGoal, &posFinal))
                continue;
        }

        // The common finalization code adds 75 back.
        posGoal.z -= 75.0f;

        // The release commits gPipe, not the time-search measure.
        CommitCandidate(HOK_Pipe, gPipe, dt, posGoal, vGoal);
        ppipeBest = ppipe;
        sPipeBest = sPipe;
    }

    // ---------------------------------------------------------------------
    // HPNTs
    // ---------------------------------------------------------------------

    for (HPNT* phpnt = g_dlHpnt.phpntFirst; phpnt != nullptr; phpnt = phpnt->dleHpnt.phpntNext)
    {
        if (!ShouldInspect(phpnt))
            continue;

        glm::vec3 posCenter(0.0f);
        GetPntPos(reinterpret_cast<PNT*>(phpnt), &posCenter);

        // The release does not bypass this check for a forced HPNT.
        if (pjt->xf.pos.z - posCenter.z > phpnt->dzJumpTargetMax)
            continue;

        const glm::vec3 dpos = pjt->xf.posWorld - posCenter;

        // The release multiplies this angle by an HPNT direction value.
        // Use phpnt->radDirection here once that member has been identified.
        const float radStart = std::atan2(dpos.y, dpos.x);

        MJHPNT mjhpnt{};
        mjhpnt.pjt = pjt;
        mjhpnt.phpnt = phpnt;
        mjhpnt.vJump = pjt->xf.v;

        float radHpnt = 0.0f;
        float g = 0.0f;

        MinimizeRange(reinterpret_cast<PFNGG>(GMeasureJumpHpnt), &mjhpnt, radStart, RAD_SearchStep, RAD_SearchMin, RAD_SearchMax, &radHpnt, &g);

        g *= G_CircularTargetScale;

        if (!IsForced(phpnt))
        {
            if (g >= G_JtAutoJumpMax)
                continue;

            g = GAdjustJtLandingScore(g, G_HpntPenalty, pjt, CandidateLo(phpnt));

            if (g >= gBest)
                continue;
        }

        glm::vec3 posTarget(0.0f);
        glm::vec3 posGoal(0.0f);
        glm::vec3 vGoal(0.0f);
        float dt = 0.0f;

        GetHpntClosestHidePos(phpnt, radHpnt, &posTarget, nullptr);
        MeasureJtJumpToTarget(pjt, &pjt->xf.v, phpnt->paloParent, &posTarget, nullptr, &dt, nullptr, &posGoal, &vGoal);

        if (!IsForced(phpnt) && FFindJtObstruction(pjt, dt, nullptr, nullptr, pjt->pposBase, &pjt->xf.v, &posGoal, &vGoal, nullptr))
            continue;

        vGoal.z = pjt->xf.v.z + dt * pjt->dvGravity.z;

        CommitCandidate(HOK_Point, g, dt, posGoal, vGoal);
        phpntBest = phpnt;
        radHpntBest = radHpnt;
    }

    // ---------------------------------------------------------------------
    // HSHAPEs
    // ---------------------------------------------------------------------

    for (HSHAPE* phshape = g_dlHshape.phshapeFirst; phshape != nullptr; phshape = phshape->dleHshape.phshapeNext)
    {
        if (!ShouldInspect(phshape))
            continue;

        CRV* pcrv = phshape->pcrv.get();

        if (pcrv == nullptr)
            continue;

        glm::vec3 posJtLocal(0.0f);
        glm::vec3 posClosestLocal(0.0f);
        glm::vec3 posClosestWorld(0.0f);
        float sClosest = 0.0f;

        ConvertAloPos(nullptr, phshape->paloParent, pjt->pposBase, &posJtLocal);

        if (pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
            pcrv->pvtcrv->pfnFindCrvClosestPointAll(pcrv, &posJtLocal, nullptr, &posClosestLocal, nullptr, nullptr, &sClosest);

        ConvertAloPos(phshape->paloParent, nullptr, &posClosestLocal, &posClosestWorld);

        // The release does not bypass this check for a forced HSHAPE.
        if (pjt->xf.pos.z - posClosestWorld.z > phshape->dzMax)
            continue;

        MJH mjh{};
        mjh.pjt = pjt;
        mjh.phshape = phshape;
        mjh.vJump = pjt->xf.v;

        const float sMax = pcrv->pvtcrv->pfnSMaxCrv(pcrv);

        float sHshape = 0.0f;
        float g = 0.0f;

        MinimizeRange(reinterpret_cast<PFNGG>(GMeasureJumpToHshape), &mjh, sClosest, DS_HshapeSearch, DS_HshapeEnd, sMax - DS_HshapeEnd, &sHshape, &g);

        g *= G_CircularTargetScale;

        if (!IsForced(phshape))
        {
            if (g >= G_JtAutoJumpMax)
                continue;

            g = GAdjustJtLandingScore(g, G_HshapePenalty, pjt, CandidateLo(phshape));

            if (g >= gBest)
                continue;
        }

        glm::vec3 posTarget(0.0f);
        glm::vec3 posGoal(0.0f);
        glm::vec3 vGoal(0.0f);
        float dt = 0.0f;

        GetHshapeHidePos(phshape, sHshape, &posTarget, nullptr);
        MeasureJtJumpToTarget(pjt, &pjt->xf.v, phshape->paloParent, &posTarget, nullptr, &dt, nullptr, &posGoal, &vGoal);

        if (!IsForced(phshape) && FFindJtObstruction(pjt, dt, nullptr, nullptr, pjt->pposBase, &pjt->xf.v, &posGoal, &vGoal, nullptr))
            continue;

        vGoal.z = pjt->xf.v.z + dt * pjt->dvGravity.z;

        CommitCandidate(HOK_Shape, g, dt, posGoal, vGoal);
        phshapeBest = phshape;
        sHshapeBest = sHshape;
    }

    // ---------------------------------------------------------------------
    // Release-only landing acceptance gate
    // ---------------------------------------------------------------------

    if (0.0 != 0 && g_clock.t > GRandInRange(30.0f, 120.0f))
        return pjt->jtbs;

    int jtbs = pjt->jtbs;

    if (bestKind == HOK_None)
    {
        StartSound(
            static_cast<SFXID>(126),
            nullptr,
            nullptr,
            &pjt->xf.posWorld,
            3000.0f,
            300.0f,
            0.6f,
            0.0f,
            0.0f,
            nullptr,
            nullptr);

        return pjt->jtbs;
    }

    switch (bestKind)
    {
        case HOK_Shape:
        pjt->sParamHshape = sHshapeBest;
        pjt->phshape = phshapeBest;
        jtbs = 12;
        break;

        case HOK_Point:
        pjt->radHpnt = radHpntBest;
        pjt->phpnt = phpntBest;
        jtbs = 13;
        break;

        case HOK_Spire:
        pjt->pspireCur = pspireBest;
        jtbs = 10;
        break;

        case HOK_Rail:
        pjt->uRailCur = uRailBest;
        pjt->prailCur = prailBest;
        pjt->sRailCur = prailBest->pcrv->pvtcrv->pfnSFromCrvU(prailBest->pcrv.get(), uRailBest);
        jtbs = 11;
        break;

        case HOK_Target:
        pjt->ppipeCur = nullptr;
        pjt->ptargetCur = reinterpret_cast<TARGET*>(phndBest);
        pjt->phndCur = phndBest;
        SetJtJtcs(pjt, 0);
        UpdateJtCane(pjt);
        jtbs = 14;
        break;
    
        case HOK_Pipe:
        pjt->sPipeCur = sPipeBest;
        pjt->ppipeCur = ppipeBest;
        pjt->phndCur = nullptr;
        pjt->ptargetCur = nullptr;
        SetJtJtcs(pjt, 0);
        UpdateJtCane(pjt);
        jtbs = 15;
        break;

        default:
        return pjt->jtbs;
    }

    pjt->pactvalJump->tMatch = g_clock.t + dtBest;
    pjt->pactvalJump->posGoal = posBest;
    pjt->pactvalJump->posGoal.z += 75.0f;
    pjt->pactvalJump->vGoal = vBest;

    return jtbs;
}

void UpdateJtActiveHide(JT* pjt, JOY* pjoy)
{
    // The release executable has two jump/basket substates before the hide
    // block.  jt.h still carries the older prototype numbering, so use the
    // release values here (the SetJtJts switch is also release-numbered).
    constexpr JTBS JTBS_ReleaseHideStand = static_cast<JTBS>(16);
    constexpr JTBS JTBS_ReleaseHideSidestep = static_cast<JTBS>(17);
    constexpr JTBS JTBS_ReleaseHidePeekLeft = static_cast<JTBS>(18);
    constexpr JTBS JTBS_ReleaseHidePeekRight = static_cast<JTBS>(19);
    constexpr JTBS JTBS_ReleaseHideRun = static_cast<JTBS>(20);
    constexpr JTBS JTBS_ReleaseHideSpin = static_cast<JTBS>(21);

    int jts = pjt->jts;
    int jtbs = pjt->jtbs;

    switch (pjt->jthk)
    {
        // -----------------------------------------------------------------
        // Flatten / duck hiding
        // -----------------------------------------------------------------

    case JTHK_Flatten:
    case JTHK_Duck:
    {
        glm::vec3 posHide(0.0f);
        float radHide = 0.0f;

        if (pjt->phshape != nullptr)
            GetHshapeClosestHidePos(pjt->phshape, &pjt->sParamHshape, &posHide, &radHide);
        else if (pjt->phpnt != nullptr)
            GetHpntHidePos(pjt->phpnt, pjt->radHpnt, &posHide, &radHide);
        else
            GetHbskClosestHidePos(pjt->phbsk, &posHide, &radHide);

        const glm::vec3& vBlend =
            pjt->jthk == JTHK_Flatten
            ? pjt->ablssFlatten[4].v
            : pjt->ablssDuck[2].v;

        const float svHide = glm::length(vBlend) * pjt->pasegaCur->svtLocal;

        // Only the horizontal position is matched.
        posHide.z = pjt->xf.pos.z;

        SMP smpHide{};
        smpHide.svFast = svHide;
        smpHide.svSlow = 0.0f;
        smpHide.dtFast = DT_JtFlattenSmooth;

        glm::vec3 vecRailLocal(0.0f);

        // Use the equivalent overload from your implementation if
        // PosSmooth returns through a hidden return-storage parameter.
        PosSmooth(pjt->xf.pos, posHide, g_clock.dt, &smpHide, &vecRailLocal);

        const float radRail =
            pjt->jthk == JTHK_Flatten
            ? radHide + glm::half_pi<float>()
            : radHide - glm::half_pi<float>();

        glm::vec3 normalRail(0.0f);
        glm::vec3 normalJoy(0.0f);

        SetVectorCylind(&normalRail, radRail, 1.0f, 0.0f);
        SetVectorCylind(&normalJoy, pjt->radJoy, 1.0f, 0.0f);

        float uRail = glm::clamp(R_JtHideSpline * glm::dot(normalRail, normalJoy), -1.0f, 1.0f);

        uRail *= pjt->uJoyDeflect;

        // Release-only input dead zone.
        if (std::abs(uRail) < 0.2f)
            uRail = 0.0f;

        uRail *= svHide;

        const float svRemainingSq = std::max(
            0.0f,
            svHide * svHide - glm::dot(vecRailLocal, vecRailLocal));

        const float svRemaining = std::sqrt(svRemainingSq);

        // The original min logic preserves the sign of uRail.
        float svRail = std::min(svRemaining, uRail);
        const float svRailRequested = svRail;

        if (pjt->phshape != nullptr &&
            pjt->phshape->pcrv != nullptr &&
            !pjt->phshape->pcrv->fClosed)
        {
            float svMin = 0.0f;
            float svMax = 0.0f;

            GSmooth(pjt->sParamHshape, 30.0f, g_clock.dt, &smpHide, &svMin);

            const float sMax =
                pjt->phshape->pcrv->mpicvs[
                    pjt->phshape->pcrv->ccv - 1];

            GSmooth(pjt->sParamHshape, sMax - 30.0f, g_clock.dt, &smpHide, &svMax);

            svRail = glm::clamp(svRail, svMin, svMax);
        }

        const glm::vec3 vRail = normalRail * svRail;
        const glm::vec3 vCombined = vecRailLocal + vRail;

        // Convert the desired world-space movement into Sly-local space,
        // then negate it as done by the release matrix operations.
        pjt->vTarget = -(glm::transpose(pjt->matTarget) * vCombined);

        if (glm::length(pjt->vTarget) > SV_JtFlattenSidestepMin)
        {
            jtbs = JTBS_ReleaseHideSidestep;

            if (pjt->jthk == JTHK_Flatten)
            {
                if (glm::length(vecRailLocal) > 45.0f)
                {
                    SetJtHfdk(pjt, HFDK_Match);
                }
                else if (pjt->vTarget.y > 20.0f)
                {
                    SetJtHfdk(pjt, HFDK_Right);
                }
                else if (pjt->vTarget.y < -20.0f)
                {
                    SetJtHfdk(pjt, HFDK_Left);
                }
            }
        }
        else
        {
            pjt->vTarget = glm::vec3(0.0f);
            jtbs = JTBS_ReleaseHideStand;

            const uint32_t grfhp =
                pjt->phshape != nullptr
                ? pjt->phshape->grfhp
                : 0;

            if (svRailRequested < -SV_JtFlattenSidestepMin &&
                (grfhp & 1U) != 0)
            {
                jtbs =
                    pjt->jthk == JTHK_Flatten
                    ? JTBS_ReleaseHidePeekRight
                    : JTBS_ReleaseHidePeekLeft;
            }
            else if (svRailRequested > SV_JtFlattenSidestepMin &&
                (grfhp & 2U) != 0)
            {
                jtbs =
                    pjt->jthk == JTHK_Flatten
                    ? JTBS_ReleaseHidePeekLeft
                    : JTBS_ReleaseHidePeekRight;
            }

            if (pjt->jthk == JTHK_Flatten)
                SetJtHfdk(pjt, HFDK_Stand);
        }

        pjt->radTarget = radHide;

        if (pjt->jtbs == JTBS_ReleaseHideSpin)
        {
            const float radCurrent = std::atan2(
                pjt->xf.mat[0].y,
                pjt->xf.mat[0].x);

            const float drad = RadNormalize(
                radCurrent - pjt->radTarget);

            if (std::abs(drad) > RAD_JtExitSpin)
                jtbs = JTBS_ReleaseHideSpin;
        }

        break;
    }

    // -----------------------------------------------------------------
    // Basket
    // -----------------------------------------------------------------

    case JTHK_Basket:
    {
        if (pjt->jtbs != JTBS_ReleaseHideStand &&
            pjt->jtbs != JTBS_ReleaseHideRun)
        {
            break;
        }

        if (pjt->uJoyDeflect > 0.0f)
        {
            jtbs = JTBS_ReleaseHideRun;

            SetStepRunTarget(pjt, pjt->radJoy, pjt->uJoyDeflect, pjt->ablrunBasket, pjt->pasegblBasket);
        }
        else
        {
            pjt->vTarget = glm::vec3(0.0f);
            jtbs = JTBS_ReleaseHideStand;
        }

        break;
    }

    // -----------------------------------------------------------------
    // Nonchalant / shadow-sneak
    // -----------------------------------------------------------------

    case JTHK_Nonchalant:
    {
        if ((GetAvailableVaultFlags() & 8192U) == 0)
        {
            pjt->vTarget = glm::vec3(0.0f);
            jtbs = JTBS_ReleaseHideStand;
            break;
        }

        if (pjt->uJoyDeflect > 0.0f)
        {
            jtbs = JTBS_ReleaseHideRun;

            SetStepRunTarget(pjt, pjt->radJoy, pjt->uJoyDeflect * U_JtShadowSneakMax, pjt->ablrunRun, pjt->pasegblRun);
        }
        else
        {
            pjt->vTarget = glm::vec3(0.0f);
            jtbs = JTBS_ReleaseHideStand;
        }

        break;
    }

    // -----------------------------------------------------------------
    // Rail
    // -----------------------------------------------------------------

    case JTHK_Rail:
    {
        RAIL* prail = pjt->prailCur;
        CRV* pcrv = prail->pcrv.get();
        ALO* paloRail = prail->paloParent;

        glm::vec3 posBaseLocal(0.0f);
        glm::vec3 posRailLocal(0.0f);
        glm::vec3 tangentRailLocal(0.0f);

        ConvertAloPos(nullptr, paloRail, pjt->pposBase, &posBaseLocal);

        if (pcrv->pvtcrv->pfnFindCrvClosestPointFromU != nullptr)
            pcrv->pvtcrv->pfnFindCrvClosestPointFromU(pcrv, &posBaseLocal, pjt->uRailCur, nullptr, &posRailLocal, &tangentRailLocal, &pjt->uRailCur, &pjt->sRailCur);

        glm::vec3 posRail(0.0f);
        glm::vec3 tangentRail(0.0f);

        ConvertAloPos(paloRail, nullptr, &posRailLocal, &posRail);
        ConvertAloVec(paloRail, nullptr, &tangentRailLocal, &tangentRail);

        tangentRail.z = 0.0f;

        if (prail->fSlippery &&
            !pcrv->fClosed &&
            (pjt->uRailCur <= 0.0f ||
                pjt->uRailCur >= pcrv->mpicvu[pcrv->ccv - 1]))
        {
            jts = JTS_Jump;
            jtbs = JTBS_Jump_Fall;
            break;
        }

        if (glm::length2(tangentRail) < 0.0001f)
            tangentRail = g_normalX;
        else
            tangentRail = glm::normalize(tangentRail);

        float radRail = std::atan2(tangentRail.y, tangentRail.x);

        glm::vec3 normalJoy(0.0f);
        SetVectorCylind(&normalJoy, pjt->radJoy, 1.0f, 0.0f);

        glm::vec3 normalControl = tangentRail;

        // A rope/control curve can override the direction used to
        // interpret joystick input.
        if (prail->prope != nullptr)
        {
            SHAPE* pshapeRope =
                reinterpret_cast<SHAPE*>(prail->prope);

            if (pshapeRope->pcrv != nullptr)
            {
                glm::vec3 posRopeLocal(0.0f);
                glm::vec3 tangentRopeLocal(0.0f);

                ConvertAloPos(nullptr, pshapeRope->paloParent, pjt->pposBase, &posRopeLocal);

                if (pshapeRope->pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
                    pshapeRope->pcrv->pvtcrv->pfnFindCrvClosestPointAll(pshapeRope->pcrv.get(), &posRopeLocal, nullptr, nullptr, &tangentRopeLocal, nullptr, nullptr);

                ConvertAloVec(pshapeRope->paloParent, nullptr, &tangentRopeLocal, &normalControl);
            }
        }

        normalControl.z = 0.0f;

        if (glm::length2(normalControl) < 0.0001f)
            normalControl = g_normalX;
        else
            normalControl = glm::normalize(normalControl);

        const float joyDotRail =
            glm::dot(normalJoy, normalControl);

        // Release direction latch with hysteresis.
        switch (pjt->jtMoveDir)
        {
        case 0:
            if (pjt->uJoyDeflect < 0.25f)
            {
                pjt->jtMoveDir = -1;
            }
            else if (joyDotRail < -0.7f)
            {
                pjt->jtMoveDir = 1;
            }
            break;

        case 1:
            if (pjt->uJoyDeflect < 0.25f)
            {
                pjt->jtMoveDir = -1;
            }
            else if (joyDotRail > 0.7f)
            {
                pjt->jtMoveDir = 0;
            }
            break;

        case -1:
            if (pjoy->uDeflect > 0.35f)
            {
                pjt->jtMoveDir =
                    joyDotRail < 0.0f ? 1 : 0;
            }
            break;

        default:
            pjt->jtMoveDir = -1;
            break;
        }

        if (pjt->jtMoveDir == 0)
            pjt->fRailReverse = 0;
        else if (pjt->jtMoveDir == 1)
            pjt->fRailReverse = 1;

        float uRun = 0.0f;

        if (!prail->fSlippery)
        {
            SMP smpRail{};

            smpRail.svSlow = -pjt->ablrunRail[0].v.z;

            smpRail.svFast = -pjt->ablrunRail[1].posPlant.z;

            smpRail.dtFast = DT_JtRailLimit;

            float svEnd = smpRail.svFast;

            if (!pcrv->fClosed)
            {
                if (!pjt->fRailReverse)
                {
                    float svLimit = 0.0f;

                    GSmooth(pjt->sRailCur, pcrv->pvtcrv->pfnSMaxCrv(pcrv) - DS_JtRailLimit, g_clock.dt, &smpRail, &svLimit);
                    svEnd = svLimit;
                }
                else
                {
                    float svLimit = 0.0f;
                    GSmooth(pjt->sRailCur, DS_JtRailLimit, g_clock.dt, &smpRail, &svLimit);

                    svEnd = -svLimit;
                }
            }

            float uInput = pjt->uJoyDeflect;

            const float uEndLimit =
                (svEnd - smpRail.svSlow) /
                (smpRail.svFast - smpRail.svSlow);

            if (uInput >= 0.0f)
                uInput = std::min(uInput, uEndLimit);
            else
                uInput = 0.0f;

            uRun = uInput;
        }

        if (pjt->fRailReverse)
            radRail = RadNormalize(radRail + glm::pi<float>());

        if (uRun > 0.0f)
        {
            jtbs = JTBS_ReleaseHideRun;

            SetStepRunTarget(pjt, radRail, uRun, pjt->ablrunRail, pjt->pasegblRail);
        }
        else
        {
            pjt->radTarget = radRail;
            pjt->vTarget = glm::vec3(0.0f);
            jtbs = JTBS_ReleaseHideStand;
        }

        break;
    }

    // -----------------------------------------------------------------
    // Spire
    // -----------------------------------------------------------------

    case JTHK_Spire:
    {
        pjt->vTarget = glm::vec3(0.0f);
        pjt->radTarget = pjt->radJoy;
        break;
    }

    // -----------------------------------------------------------------
    // Reach
    // -----------------------------------------------------------------

    case JTHK_Reach:
    {
        FTurnJtToTarget(pjt);
        pjt->vTarget = glm::vec3(0.0f);
        break;
    }

    default:
    {
        pjt->vTarget = glm::vec3(0.0f);
        break;
    }
    }

    SetJtJts(pjt, jts, jtbs);
}

void MatchJtXmgRail(JT* pjt, XMG* pxmg, ACTADJ* pactadj)
{
    if (!pxmg->fValid)
    {
        pactadj->dposGoal = glm::vec3(0.0f);
        return;
    }

    RAIL* prail = pjt->prailCur;
    CRV* pcrv = prail->pcrv.get();

    float dsRail = pxmg->posExtrem.x;

    if (pjt->fRailReverse)
        dsRail = -dsRail;

    const float sMin = pcrv->mpicvs[0];
    const float sMax = pcrv->pvtcrv->pfnSMaxCrv(pcrv);
    const float sUnbounded = pjt->sRailCur + dsRail;
    const float sRail = pcrv->fClosed != 0
        ? GModPositive(sUnbounded, sMax)
        : std::clamp(sUnbounded, sMin, sMax);

    glm::vec3 posRailLocal(0.0f);
    glm::vec3 posRail(0.0f);

    if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
        pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, sRail, &posRailLocal, nullptr);

    ConvertAloPos(prail->paloParent, reinterpret_cast<ALO*>(pjt), &posRailLocal, &posRail);

    LSG lsg{};

    const int clsg = ClsgClipEdgeToSphere(&pxmg->posSphere, pxmg->sSphere, &pxmg->posExtrem, &posRail, 1, &lsg);

    if (clsg > 0)
        pactadj->dposGoal = lsg.apos[1] - pxmg->posExtrem;

}

void UpdateJtHide(JT* pjt)
{
    constexpr int JTBS_HideStand = 16;
    constexpr int JTBS_HideSidestep = 17;
    constexpr int JTBS_HideRun = 20;

    constexpr int JTHK_Flatten = 0;
    constexpr int JTHK_Duck = 1;
    constexpr int JTHK_Basket = 2;
    constexpr int JTHK_Nonchalant = 3;
    constexpr int JTHK_Rail = 4;

    if (pjt->jtbs == JTBS_HideSidestep)
    {
        BLSS* pblss = nullptr;
        ASEGBL* pasegbl = nullptr;
        SFF* psff = nullptr;

        switch (pjt->jthk)
        {
        case JTHK_Flatten:
            pblss = pjt->ablssFlatten;
            pasegbl = pjt->pasegblFlatten;
            psff = &pjt->sffFlatten;
            break;

        case JTHK_Duck:
            pblss = pjt->ablssDuck;
            pasegbl = pjt->pasegblDuck;
            psff = &pjt->sffDuck;
            break;

        case JTHK_Nonchalant:
            pblss = pjt->ablssSidestep;
            pasegbl = pjt->pasegblSidestep;
            psff = &pjt->sffSidestep;
            break;

        default:
            return;
        }

        AdjustJtSidestep(pjt, pblss, pasegbl);

        ASEGA* pasega = pjt->pasegaCur;
        const float uFootfall = GModPositive((pasega->tLocal + -0.033 * pasega->svtLocal) / pasega->paseg->tMax, 1.0f);

        if (pjt->ptnCur == pjt->ptnHideDynamic && pjt->hfdk > 0 && pjt->hfdk < 3)
        {
            const float uFlatten = (pjt->hfdk == 1) ? pjt->ablssFlatten[1].u : pjt->ablssFlatten[0].u;
            pjt->uFlattenClamp = std::min(pjt->uFlattenClamp, uFlatten);
        }

        UpdateStepFootfall(reinterpret_cast<STEP*>(pjt), uFootfall, psff);
        return;
    }

    if (pjt->jtbs == JTBS_HideStand)
    {
        if (pjt->jthk == JTHK_Rail)
        {
            RebuildJtXmg(pjt, pjt->paloLeftBall, 5.0f, pjt->paloLeftHeel, pjt->pactadjLeftFoot, &pjt->xmgLeftBall);
            RebuildJtXmg(pjt, pjt->paloRightBall, 5.0f, pjt->paloRightHeel, pjt->pactadjRightFoot, &pjt->xmgRightBall);

            MatchJtXmgRail(pjt, &pjt->xmgLeftBall, pjt->pactadjLeftFoot);
            MatchJtXmgRail(pjt, &pjt->xmgRightBall, pjt->pactadjRightFoot);
        }

        return;
    }

    if (pjt->jtbs == JTBS_HideRun)
    {
        switch (pjt->jthk)
        {
            case JTHK_Basket:
            AdjustStepRun(reinterpret_cast<STEP*>(pjt), pjt->ablrunBasket, pjt->pasegblBasket, pjt->pasegaCur);
            break;

            case JTHK_Nonchalant:
            AdjustStepRun(reinterpret_cast<STEP*>(pjt), pjt->ablrunRun, pjt->pasegblRun, pjt->pasegaCur);
            break;

            case JTHK_Rail:
            AdjustStepRun(reinterpret_cast<STEP*>(pjt), pjt->ablrunRail, pjt->pasegblRail, pjt->pasegaCur);
            break;
        }

        ASEGA* pasega = pjt->pasegaCur;
        const float uFootfall = GModPositive((pasega->tLocal + -0.033 * pasega->svtLocal) / pasega->paseg->tMax, 1.0f);

        UpdateStepFootfall(reinterpret_cast<STEP*>(pjt), uFootfall, &pjt->sffRun);
    }
}

void UpdateJtInternalXpsHide(JT* pjt)
{
    constexpr int JTHK_Rail = 4;
    constexpr int JTHK_Spire = 5;

    constexpr int GRFAHX_Rail = 34;
    constexpr int GRFAHX_RailEnd = 42;
    constexpr int GRFAHX_Spire = 35;

    constexpr float DS_JtRailLimit = 50.0f;

    if (pjt->jthk == JTHK_Spire)
    {
        glm::vec3 posSpire(0.0f);
        GetPntPos(reinterpret_cast<PNT*>(pjt->pspireCur), &posSpire);
        AddJtHookXps(pjt, GRFAHX_Spire, reinterpret_cast<LO*>(pjt->pspireCur), &posSpire, nullptr, pjt->pposBase);
        return;
    }

    if (pjt->jthk != JTHK_Rail)
        return;

    RAIL* prail = pjt->prailCur;
    ALO* paloRail = prail->paloParent;
    CRV* pcrv = prail->pcrv.get();

    glm::vec3 posBaseLocal(0.0f);
    glm::vec3 posRailLocal(0.0f);
    glm::vec3 tangentRailLocal(0.0f);
    glm::vec3 posRail(0.0f);
    glm::vec3 tangentRail(0.0f);

    ConvertAloPos(nullptr, paloRail, pjt->pposBase, &posBaseLocal);

    if (pcrv->pvtcrv->pfnFindCrvClosestPointFromU != nullptr)
        pcrv->pvtcrv->pfnFindCrvClosestPointFromU(pcrv, &posBaseLocal, pjt->uRailCur, nullptr, &posRailLocal, &tangentRailLocal, &pjt->uRailCur, &pjt->sRailCur);

    ConvertAloPos(paloRail, nullptr, &posRailLocal, &posRail);
    ConvertAloVec(paloRail, nullptr, &tangentRailLocal, &tangentRail);

    int grfahx = GRFAHX_Rail;

    if (!prail->fSlippery && !pcrv->fClosed)
    {
        const float sMin = pcrv->mpicvs[0];
        const float sMax = pcrv->mpicvs[pcrv->ccv - 1];

        const float svRail = glm::dot(pjt->xf.v, tangentRail);
        const glm::vec3 vTargetWorld = pjt->xf.mat * pjt->vTarget;
        const float svTarget = glm::dot(vTargetWorld, tangentRail);

        const bool fNearStart = pjt->sRailCur < sMin + DS_JtRailLimit;
        const bool fNearEnd = pjt->sRailCur > sMax - DS_JtRailLimit;

        const bool fMovingPastStart = svRail < -2.0f || svTarget > -2.0f;
        const bool fMovingPastEnd = svRail > 2.0f || svTarget < 2.0f;

        if (fNearStart && fMovingPastStart)
        {
            if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
                pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, sMin, &posRailLocal, nullptr);

            ConvertAloPos(paloRail, nullptr, &posRailLocal, &posRail);
            grfahx = GRFAHX_RailEnd;
        }
        else if (fNearEnd && fMovingPastEnd)
        {
            if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
                pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, sMax, &posRailLocal, nullptr);

            ConvertAloPos(paloRail, nullptr, &posRailLocal, &posRail);

            tangentRail = -tangentRail;
            grfahx = GRFAHX_RailEnd;
        }
    }

    AddJtHookXps(pjt, grfahx, prail, &posRail, &tangentRail, pjt->pposBase);
}

void SetJtHfdk(JT* pjt, HFDK hfdk)
{
    if (hfdk == pjt->hfdk)
        return;

    pjt->hfdk = hfdk;
    pjt->tHfdk = g_clock.t;
    pjt->uFlattenClamp = 0.0f;

    switch (hfdk)
    {
        case HFDK_Left:
        case HFDK_Right:
        pjt->jtfock = (pjt->ptnCur == pjt->ptnHideDynamic) ? JTFOCK_Focus : JTFOCK_Relax;
        break;

        case HFDK_Stand:
        pjt->jtfock = JTFOCK_Clamp;
        break;

        default:
        break;
    }
}

void ResetJtDynamicTunnel(JT* pjt)
{
    TN* ptn = pjt->ptnHideDynamic;

    ptn->grftnd = 5;
    ptn->ctsurf = 0;
    ptn->ctbsp = 0;
    ptn->pcrv = nullptr;

    ptn->tnfn = g_tnfnDynamic;

    ptn->fPanOnEntry = 1;
    ptn->priCamera = 3;
    ptn->fCylinder = 0;
    ptn->fSwitchInAir = 0;
    ptn->fPanOnIdle = 0;

    ptn->dradSlack = 0.0f;
    ptn->sClosest = 0.0f;

    SetTnTnsOverride(ptn, TNS_Out);
}

void PresetJtAccelHide(JT* pjt)
{
    if (pjt->jthk == JTHK_Rail && pjt->prailCur->fSlippery)
    {
        RAIL* prail = pjt->prailCur;
        CRV* pcrv = prail->pcrv.get();

        glm::vec3 tangentLocal(0.0f);
        glm::vec3 tangent(0.0f);

        if (pcrv->pvtcrv->pfnEvaluateCrvFromU != nullptr)
            pcrv->pvtcrv->pfnEvaluateCrvFromU(pcrv, pjt->uRailCur, nullptr, &tangentLocal);

        ConvertAloVec(prail->paloParent, nullptr, &tangentLocal, &tangent);

        const float svRail = glm::dot(tangent, pjt->xf.v);
        const float sdvGravity = glm::dot(tangent, pjt->dvGravity);

        float sdvMax = 0.0f;
        float sdvMin = 0.0f;

        GSmooth(svRail, prail->svMax, g_clock.dt, &s_smpSvRailMax, &sdvMax);
        GSmooth(svRail, -prail->svMax, g_clock.dt, &s_smpSvRailMax, &sdvMin);

        const float sdvTarget = std::clamp(prail->rdvGravity * sdvGravity, sdvMin, sdvMax);
        glm::vec3 dv = tangent * (sdvTarget - sdvGravity);

        AddSoAcceleration(pjt, &dv);
    }

    PresetJtAccelBase(pjt);
}

bool FJtSafeFromStepguards(JT* pjt)
{
    LO* objects[64];
    int count = CploFindSwObjectsByClass(pjt->psw, 5, CID_SGG, nullptr, 64, objects);

    for (int i = 0; i < count; ++i)
    {
        SGG* psgg = static_cast<SGG*>(objects[i]);

        if (psgg->sggs == SGGS_Search || psgg->sggs == SGGS_Attack)
            return false;
    }

    return true;
}
