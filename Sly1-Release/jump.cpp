#include "jump.h"
#include "actval.h"
#include "bbmark.h"
#include "target.h"
#include "stephide.h"
#include "spire.h"
#include "rail.h"
#include "hnd.h"

void PresetJtAccelJump(JT* pjt)
{
    AccelSoTowardMatSmooth(pjt, g_clock.dt, &pjt->matTarget, &pjt->smpSpin);

    switch (pjt->jtbs)
    {
        case 4:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        return;

        case 6:
        {
            if (pjt->uJoyDeflect > 0.0001f)
            {
                glm::vec3 vAirSteer{};
                SetVectorCylind(&vAirSteer, pjt->radJoy, pjt->uJoyDeflect * pjt->svxyAirSteer, 0.0f);

                glm::vec3 vTargetAir = pjt->vTargetCoast + vAirSteer;
                vTargetAir.z = pjt->xf.v.z;

                AccelSoTowardPosSpring(pjt, nullptr, nullptr, &vTargetAir, &s_clqThrownSteer, g_clock.dt);
            }

            return;
        }

        case 8:
        {
            const float tStartDive = TFindAsegLabel(pjt->pasegDiveAttack, (OID)442);
            const float tEndDive   = TFindAsegLabel(pjt->pasegDiveAttack, (OID)443);

            if (pjt->pasegaCur->tLocal < tEndDive)
            {
                glm::vec3 dvCancelGravity = -pjt->dvGravity;
                AddSoAcceleration((SO*)pjt, &dvCancelGravity);
            }

            glm::vec3 vTargetAir{};

            if (pjt->pasegaCur->tLocal >= tStartDive)
                vTargetAir.z = pjt->xf.v.z;

            AccelSoTowardPosSpring(pjt, nullptr, nullptr, &vTargetAir, &s_clqDiveDamping, g_clock.dt);
            return;
        }

        default:
        break;
    }

    if (pjt->pjsgCur != nullptr)
        return;

    glm::vec3 vAirSteer(pjt->xf.v.x, pjt->xf.v.y, 0.0f);
    const float svAir = glm::length(vAirSteer);

    if (svAir >= 0.0001f)
        vAirSteer /= svAir;
    else
        vAirSteer = g_normalX;

    glm::mat3 matJt{};
    LoadLookAtMatrix(&vAirSteer, &matJt);

    const glm::vec3 vTargetAir = glm::transpose(matJt) * pjt->vTargetCoast;

    float rvForward;
    float rvSide;

    if (pjt->jtbs == 0)
    {
        rvSide = RVY_JtInitAccel;
        rvForward = svAir <= vTargetAir.x ? RVX_JtInitAccel : RVX_JtInitDecel;
    }
    else
    {
        rvSide = RVY_JtAccel;
        rvForward = svAir <= vTargetAir.x ? RVX_JtAccel : RVX_JtDecel;
    }

    glm::vec3 dvLocal{};
    dvLocal.x = rvForward * (vTargetAir.x - svAir);
    dvLocal.y = rvSide * vTargetAir.y;
    dvLocal.z = 0.0f;

    if (pjt->jtbs != 6)
    {
        const float vzTerminal = VZ_JtTerminal * std::sqrt(pjt->rGravity);

        if (pjt->xf.v.z < vzTerminal)
            dvLocal.z = (vzTerminal - pjt->xf.v.z) * RVZ_JtTerminal;
    }

    glm::vec3 dv = matJt * dvLocal;
    AddSoAcceleration(pjt, &dv);
}

void UpdateJtActiveJump(JT* pjt)
{
    glm::vec3 vJoy{};
    SetVectorCylind(&vJoy, pjt->radJoy, pjt->uJoyDeflect * pjt->svxyAirSteer, 0.0f);

    switch (pjt->jtbs)
    {
        case 4:
        case 6:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        break;

        default:
        {
            pjt->vTarget = -(glm::transpose(pjt->matTarget) * vJoy);

            if (((pjt->jtbs == 0) && (pjt->jtjj != 3)) || pjt->fShortenJump)
                pjt->vTargetCoast = vJoy;
            else
            {
                glm::vec3 vHorizontal = pjt->xf.v;
                vHorizontal.z = 0.0f;

                const float speedHorizontal = glm::length(vHorizontal);

                if (speedHorizontal < 0.0001f)
                    vHorizontal = g_normalX;
                else
                    vHorizontal /= speedHorizontal;

                glm::vec3 vTangent{};
                const float speedProjected = SProjectVector(&vJoy, &vHorizontal, nullptr, &vTangent);

                if (speedProjected >= speedHorizontal)
                    pjt->vTargetCoast = vJoy;
                else if (speedProjected > speedHorizontal * -0.25f)
                    pjt->vTargetCoast = vTangent + vHorizontal * speedHorizontal;
                else
                    pjt->vTargetCoast = vJoy;
            }

            break;
        }
    }

    bool fSearchTargets = false;

    switch (pjt->jtbs)
    {
        case 1:
        case 2:
        case 5:
        case 6:
        fSearchTargets = true;
        break;

        case 3:
        fSearchTargets = g_clock.t - pjt->tJtbs > 0.4f;
        break;

        case 0:
        case 4:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        default:
        break;
    }

    if (!fSearchTargets)
        return;

    HBSK* phbskBest = nullptr;
    JMT* pjmtBest = nullptr;

    glm::vec3 posBest{};
    glm::vec3 vBest{};

    float dtBest = 0.0f;
    // HBSK/JMT targeting uses the same integrated jump-path score scale as
    // the other automatic landing targets. 1500 rejects normal barrel-entry
    // trajectories and leaves the barrel as ordinary collision.
    float gBest = G_JtAutoJumpMax;

    for (HBSK* phbsk = g_dlHbsk.phbskFirst; phbsk != nullptr; phbsk = phbsk->dleHbsk.phbskNext)
    {
        // Do not select the basket already attached to Sly as a fresh landing
        // target. It stays linked in g_dlHbsk while HBSKS_InUse.
        if (phbsk->hbsks != HBSKS_Available)
            continue;

        const glm::vec3 dposWorld = phbsk->xf.posWorld - pjt->xf.posWorld;

        if (glm::dot(glm::vec2(dposWorld), glm::vec2(dposWorld)) > 360000.0f)
            continue;

        /*
         * Release check:
         *
         * pjt->xf.posWorld.z <= phbsk->xf.posWorld.z + value_at_HBSK_0x568
         *
         * Rename sJumpTargetHeight to whichever HBSK member occupies offset 0x568.
         */
        if (pjt->xf.posWorld.z > phbsk->xf.posWorld.z + phbsk->dzJumpMax)
            continue;

        float dtCandidate = 0.0f;
        float gCandidate = 0.0f;
        glm::vec3 posCandidate{};
        glm::vec3 vCandidate{};

        MeasureJtJumpToTarget(pjt, &pjt->xf.v, (ALO*)phbsk, &phbsk->xf.posWorld, nullptr, &dtCandidate, &gCandidate, &posCandidate, &vCandidate);

        if (gCandidate > gBest)
            continue;

        glm::vec3 vTarget{};
        CalculateAloMovement(phbsk->paloParent, nullptr, phbsk->xf.posWorld, &vTarget, nullptr, nullptr, nullptr);

        glm::vec3 dpos = pjt->xf.posWorld - phbsk->xf.posWorld;
        glm::vec3 dposProjected = dpos + (pjt->xf.v - vTarget) * glm::min(dtCandidate, 0.25f);

        dpos.z = 0.0f;
        dposProjected.z = 0.0f;

        float u = 0.0f;
        float s = 0.0f;
        FindClosestPointOnLineSegment(&g_vecZero, &dpos, &dposProjected, &u, &s);

        bool fAccept = s <= 100.0f && u != 0.0f;

        if (!fAccept && s <= 25.0f && pjt->xf.v.z <= 0.0f)
            fAccept = true;

        if (!fAccept)
            continue;

        phbskBest = phbsk;
        pjmtBest = nullptr;
        posBest = posCandidate;
        vBest = vCandidate;
        dtBest = dtCandidate;
        gBest = gCandidate;
    }

    for (JMT* pjmt = g_dlJmt.pjmtFirst; pjmt != nullptr; pjmt = pjmt->dleJmt.pjmtNext)
    {
        glm::vec3 posTarget{};
        GetPntPos((PNT*)pjmt, &posTarget);

        const glm::vec3 dposWorld = posTarget - pjt->xf.posWorld;

        if (glm::dot(glm::vec2(dposWorld), glm::vec2(dposWorld)) > 360000.0f)
            continue;

        float dtCandidate = 0.0f;
        float gCandidate = 0.0f;
        glm::vec3 posCandidate{};
        glm::vec3 vCandidate{};

        MeasureJtJumpToTarget(pjt, &pjt->xf.v, pjmt->paloParent, &posTarget, nullptr, &dtCandidate, &gCandidate, &posCandidate, &vCandidate);

        if (gCandidate > gBest)
            continue;

        glm::vec3 vTarget{};
        CalculateAloMovement(pjmt->paloParent, nullptr, posTarget, &vTarget, nullptr, nullptr, nullptr);

        glm::vec3 dpos = pjt->xf.posWorld - posTarget;
        glm::vec3 dposProjected = dpos + (pjt->xf.v - vTarget) * glm::min(dtCandidate, 0.25f);

        dpos.z = 0.0f;
        dposProjected.z = 0.0f;

        float u = 0.0f;
        float s = 0.0f;
        FindClosestPointOnLineSegment(&g_vecZero, &dpos, &dposProjected, &u, &s);

        bool fAccept = s <= pjmt->sRadius && u != 0.0f;

        if (!fAccept && s <= pjmt->sRadius * 0.25f && pjt->xf.v.z <= 0.0f)
            fAccept = true;

        if (!fAccept)
            continue;

        phbskBest = nullptr;
        pjmtBest = pjmt;
        posBest = posCandidate;
        vBest = vCandidate;
        dtBest = dtCandidate;
        gBest = gCandidate;
    }

    /*
     * While already leaving an HBSK, the release refuses to switch directly
     * to another HBSK. A valid JMT can still be selected below.
     */
    if (phbskBest != nullptr && pjt->jts == 2 && pjt->jtbs == 5)
        phbskBest = nullptr;

    if (phbskBest != nullptr)
    {
        if (pjt->phbsk != phbskBest)
        {
            RecalcSwOxfFilterForObject(pjt->psw, (SO*)phbskBest);

            if (pjt->phbsk != nullptr)
                RecalcSwOxfFilterForObject(pjt->psw, (SO*)pjt->phbsk);
        }

        pjt->pactvalJump->tMatch = g_clock.t + dtBest;
        pjt->pactvalJump->posGoal = posBest;
        pjt->pactvalJump->posGoal.z += 75.0f;
        pjt->pactvalJump->vGoal = vBest;
        pjt->pactvalJump->vGoal.z = pjt->xf.v.z + dtBest * pjt->dvGravity.z;
        pjt->phbsk = phbskBest;

        SetJtJts(pjt, 2, 4);
        return;
    }

    if (pjmtBest != nullptr)
    {
        pjt->pactvalJump->tMatch = g_clock.t + dtBest;
        pjt->pactvalJump->posGoal = posBest;
        pjt->pactvalJump->posGoal.z += 75.0f;
        pjt->pactvalJump->vGoal = vBest;
        pjt->pactvalJump->vGoal.z = pjt->xf.v.z + dtBest * pjt->dvGravity.z;
        pjt->pjmtCur = pjmtBest;

        SetJtJts(pjt, 2, 9);
    }
}

int FCheckJtSingleEdge(JT* pjt, SO* psoOther, EDGE* pedge, glm::vec3* pnormal)
{
    //(void)pjt;

    SURF* psurf0 = &psoOther->geomWorld.asurf[static_cast<int16_t>(pedge->aisurf[0])];
    SURF* psurf1 = &psoOther->geomWorld.asurf[static_cast<int16_t>(pedge->aisurf[1])];

    SURF* psurfWall = nullptr;

    if (psurf0->normal.z > 0.7f && psurf1->normal.z < 0.55f)
        psurfWall = psurf1;
    else if (psurf0->normal.z < 0.55f && psurf1->normal.z > 0.7f)
        psurfWall = psurf0;
    else
        return 0;

    const glm::vec3& pos0 = psoOther->geomWorld.apos[static_cast<int16_t>(pedge->aipos[0])];
    const glm::vec3& pos1 = psoOther->geomWorld.apos[static_cast<int16_t>(pedge->aipos[1])];

    const glm::vec3 vecEdge = pos1 - pos0;

    glm::vec3 normal(-vecEdge.y, vecEdge.x, 0.0f);
    const float length = glm::length(normal);

    if (length < 0.0001f)
        normal = g_normalX;
    else
        normal /= length;

    if (glm::dot(normal, psurfWall->normal) < 0.0f)
        normal = -normal;

    *pnormal = normal;
    return 1;
}

int FCheckJtEdgeNormal(JT* pjt, glm::vec3* pposEdge, glm::vec3* pnormal)
{
    constexpr uint64_t SO_FLAG_SKIP_PHYS_ROOT = 1ull << 42;
    constexpr uint64_t SO_FLAG_SKIP_EDGE_TEST = 1ull << 43;

    int cFloorHits = 0;

    glm::mat3 matEdge{};
    matEdge[0] = *pnormal;

    GetNormalVectors(&matEdge[0], &matEdge[1], &g_normalZ, &g_normalZ);

    glm::mat4 mat{};
    LoadMatrixFromPosRot(pposEdge, &matEdge, &mat);

    for (int iProbe = 0; iProbe < 11; ++iProbe)
    {
        glm::vec3 pos0{};
        glm::vec3 pos1{};

        pos0 = glm::vec3(mat * glm::vec4(s_aposCheck[iProbe * 2], 1.0f));
        pos1 = glm::vec3(mat * glm::vec4(s_aposCheck[iProbe * 2 + 1], 1.0f));

        for (OX* pox = pjt->poxa->pox; pox != nullptr; pox = pox->poxNext)
        {
            SO* psoRoot = pox->psoOther;

            const uint64_t rootFlags = *reinterpret_cast<const uint64_t*>(&psoRoot->bspcCamera.absp);

            if ((rootFlags & SO_FLAG_SKIP_PHYS_ROOT) != 0)
                continue;

            for (SO* pso = psoRoot->dlPhys.psoFirst; pso != nullptr; pso = pso->dlePhys.psoNext)
            {
                if (pso->fNoXpsSelf)
                    continue;

                LSG lsg{};
                const int clsg = ClsgClipEdgeToObject(pso, &pos0, &pos1, 1, &lsg);

                if (clsg <= 0)
                    continue;

                /*
                 * The first two segments are clearance checks:
                 *
                 *   probe 0: front vertical clearance
                 *   probe 1: horizontal clearance toward the ledge
                 *
                 * Any collision on either immediately rejects the edge.
                 */
                if (iProbe < 2)
                    return 0;

                /*
                 * au[0] == 0 means the hit occurred at the beginning of the
                 * segment, which the original rejects.
                 */
                if (lsg.au[0] == 0.0f)
                    return 0;

                if (lsg.anormal[0].z > 0.7f)
                    ++cFloorHits;
            }
        }
    }

    return cFloorHits >= 2;
}

int FCheckJtEdgeGrab(JT* pjt, SO* psoOther, glm::vec3* pposEdge, XPK xpk, void* pv, glm::vec3* pnormal)
{
    glm::vec3 normalSum(0.0f);

    if (xpk == XPK_EdgeToSurface)
    {
        SURF* psurf = static_cast<SURF*>(pv);

        if (psurf->normal.z < 0.7f)
            return 0;

        bool foundEdge = false;
        LSG alsg[2]{};

        for (EDGE* pedge = psurf->pedge; pedge != nullptr; pedge = pedge->pedgeNext)
        {
            glm::vec3 normal(0.0f);

            if (!FCheckJtSingleEdge(pjt, psoOther, pedge, &normal))
                continue;

            glm::vec3* ppos0 = &psoOther->geomWorld.apos[pedge->aipos[0]];
            glm::vec3* ppos1 = &psoOther->geomWorld.apos[pedge->aipos[1]];

            if (ClsgClipEdgeToBsp(pjt->bspc.absp.data(), ppos0, ppos1, nullptr, 2, alsg) > 0)
            {
                normalSum += normal;
                foundEdge = true;
            }
        }

        for (EDGE* pedge = psurf->pedgeOther; pedge != nullptr; pedge = pedge->pedgeOtherNext)
        {
            glm::vec3 normal(0.0f);

            if (!FCheckJtSingleEdge(pjt, psoOther, pedge, &normal))
                continue;

            glm::vec3* ppos0 = &psoOther->geomWorld.apos[pedge->aipos[0]];
            glm::vec3* ppos1 = &psoOther->geomWorld.apos[pedge->aipos[1]];

            if (ClsgClipEdgeToBsp(pjt->bspc.absp.data(), ppos0, ppos1, nullptr, 2, alsg) > 0)
            {
                normalSum += normal;
                foundEdge = true;
            }
        }

        if (!foundEdge)
            return 0;

        const float normalLength = glm::length(normalSum);
        *pnormal = normalLength < 0.0001f ? g_normalX : normalSum / normalLength;
    }
    else if (xpk == XPK_EdgeToEdge)
    {
        EDGE* pedge = static_cast<EDGE*>(pv);

        if (!FCheckJtSingleEdge(pjt, psoOther, pedge, pnormal))
            return 0;
    }
    else if (xpk == XPK_PointToSurface)
    {
        GEOM& geom = psoOther->geomWorld;
        glm::vec3* ppos = static_cast<glm::vec3*>(pv);
        const int ipos = static_cast<int>(ppos - geom.apos.data());

        if (geom.mpiposiiedgeMac == nullptr)
            return 0;

        const int iedgeMin = ipos == 0 ? 0 : geom.mpiposiiedgeMac[ipos - 1];
        const int iedgeMac = geom.mpiposiiedgeMac[ipos];

        bool foundEdge = false;

        for (int iedge = iedgeMin; iedge < iedgeMac; ++iedge)
        {
            EDGE* pedge = &geom.aedge[geom.aiedge[iedge]];
            glm::vec3 normal(0.0f);

            if (FCheckJtSingleEdge(pjt, psoOther, pedge, &normal))
            {
                normalSum += normal;
                foundEdge = true;
            }
        }

        if (!foundEdge)
            return 0;

        const float normalLength = glm::length(normalSum);
        *pnormal = normalLength < 0.0001f ? g_normalX : normalSum / normalLength;
    }

    return FCheckJtEdgeNormal(pjt, pposEdge, pnormal);
}

XP* PxpFindJtBestEdge(JT* pjt, int* pixpdBest, glm::vec3* pnormalBest)
{
    if (g_clock.t - pjt->tJts < 0.15f)
        return nullptr;

    XP* pxpBest = nullptr;
    float zBest = -std::numeric_limits<float>::infinity();

    for (OX* pox = pjt->poxa->pox; pox != nullptr; pox = pox->poxNext)
    {
        for (XP* pxp = pox->pxp; pxp != nullptr; pxp = pxp->pxpNext)
        {
            const int ixpd = pjt != static_cast<JT*>(pxp->axpd[0].psoRoot);
            const int ixpdOther = 1 - ixpd;

            SO* psoOther = pxp->axpd[ixpdOther].psoLeaf;

            if (psoOther == nullptr)
                continue;

            SO* psoEgk = psoOther;

            while (psoEgk != nullptr && psoEgk->egk == 0)
                psoEgk = static_cast<SO*>(psoEgk->paloParent);

            if (psoEgk != nullptr && psoEgk->egk == 2)
                continue;

            void* pvEdge = nullptr;

            switch (pxp->xpk)
            {
                case XPK_EdgeToSurface:
                if (ixpd != 0)
                    continue;

                pvEdge = pxp->pv1;
                break;

                case XPK_PointToSurface:
                if (ixpd != 1)
                    continue;

                pvEdge = pxp->pv0;
                break;

                case XPK_EdgeToEdge:
                pvEdge = ixpd == 0 ? pxp->pv1 : pxp->pv0;
                break;

                default:
                continue;
            }

            glm::vec3 normal;

            if (!FCheckJtEdgeGrab(pjt, psoOther, &pxp->axpd[ixpdOther].pos, pxp->xpk, pvEdge, &normal))
                continue;

            if ((pjt->grfpvaXpValid & 2U) == 0 || (pxp->axpd[ixpdOther].psoRoot->grfpvaXpValid & 2U) == 0)
                RecalcSwXpMovement(pjt->psw, 1, &pxp);

            if (g_mpixpdr[ixpd] * pxp->vRelAdj.z > 0.0f)
                continue;

            glm::vec3 posCheck = pxp->axpd[ixpdOther].pos - normal * XY_JtEdgeGrabCheck + glm::vec3(g_normalZ) * Z_JtEdgeGrabCheck;

            if (PbspPointInBspQuick(&posCheck, psoOther->bspc.absp.data()) != nullptr)
                continue;

            const float zEdge = pxp->axpd[ixpdOther].pos.z;

            if (pxpBest == nullptr || zBest <= zEdge)
            {
                pxpBest = pxp;
                zBest = zEdge;
                *pixpdBest = ixpd;
                *pnormalBest = normal;
            }
        }
    }

    return pxpBest;
}

int JtsResetJt(JT* pjt)
{
    if (pjt->uJoyDeflect <= 0.0f)
        return 0;

    if (pjt->pasegblRun != nullptr)
        return 1;

    if (pjt->pasegblSidestep != nullptr)
        return 5;

    return 0;
}

void HandleJtDiveEffect(JT* pjt)
{
    if (!FIsLoInWorld(pjt->psoDiveEffect))
        return;

    constexpr float S_DiveEffectRadius = 300.0f;
    constexpr float S_DiveCoinRadius = 500.0f;
    constexpr float SV_DiveEffect = 1000.0f;
    constexpr float SDV_JtShockEquiv = 3000.0f;

    JoltCm(g_pcm);

    std::vector <SO*> objects;
    IntersectSwBoundingSphere(pjt->psw, pjt, &pjt->xf.posWorld, S_DiveEffectRadius, nullptr, nullptr, objects);

    for (SO* pso : objects)
        pso->fHandleDiveEffect = 1;

    LM lmZ
    {
        pjt->posBaseXp.z - 150.0f,
        pjt->posBaseXp.z + 150.0f
    };

    auto ApplyDiveShock = [&](LO* ploTarget, const glm::vec3& posTarget, const glm::vec3& posEffect)
    {
        glm::vec3 direction = posTarget - pjt->posBaseXp;
        const float length = glm::length(direction);

        if (length < 0.0001f)
            direction = glm::vec3(g_normalX);
        else
            direction /= length;

        WKR wkr{};
        wkr.ploSource = pjt;
        wkr.ploTarget = ploTarget;
        wkr.sftMax = pjt->m * SDV_JtShockEquiv;
        wkr.grftak = 32;
        wkr.pos = posEffect;
        wkr.v = direction * SV_DiveEffect;

        PaloAbsorbWkr(&wkr, 0, nullptr);
    };

    for (TARGET* ptarget = g_dlTarget.ptargetFirst; ptarget != nullptr; ptarget = ptarget->dleTarget.ptargetNext)
    {
        float effectRadius = S_DiveEffectRadius;
        bool handledByIntersectedParent = false;

        for (SO* parent = (SO*)ptarget->paloParent; parent != nullptr; parent = static_cast<SO*>(parent->paloParent))
        {
            if ((parent->pvtlo->grfcid & 2U) != 0 && parent->fHandleDiveEffect)
            {
                handledByIntersectedParent = true;
                break;
            }

            if (FIsBasicDerivedFrom(parent, CID_COIN))
                effectRadius = S_DiveCoinRadius;
        }

        if (handledByIntersectedParent)
            continue;

        glm::vec3 posTarget;
        GetXfmPos(ptarget, &posTarget);

        const float targetRadius = ptarget->sRadiusTarget;
        const float distance = glm::length(posTarget - pjt->posBaseXp);

        if (distance - targetRadius > effectRadius)
            continue;

        if (posTarget.z + targetRadius < lmZ.gMin || posTarget.z - targetRadius > lmZ.gMax)
            continue;

        ApplyDiveShock(ptarget, posTarget, posTarget);
    }

    SBI asbi[32]{};

    for (SO* pso : objects)
    {
        if (!FIsLoInWorld(pso))
            continue;

        bool intersects = false;
        int csbi = 0;

        if (pso->fSphere)
        {
            const float distance = glm::length(pso->xf.posWorld - pjt->posBaseXp);

            if (distance <= S_DiveEffectRadius &&
                lmZ.gMin <= pso->xf.posWorld.z + pso->sRadiusSelf &&
                pso->xf.posWorld.z - pso->sRadiusSelf <= lmZ.gMax)
            {
                intersects = true;
            }
        }
        else if (pso->bspc.absp.size() != 0)
        {
            csbi = CsbiIntersectSphereBsp(&pjt->posBaseXp, S_DiveEffectRadius, pso->bspc.cbsp, pso->bspc.absp.data(), nullptr, &pso->geomWorld, 32, asbi);

            for (int i = 0; i < csbi; ++i)
            {
                if (FCheckLm(&lmZ, asbi[i].pos.z))
                {
                    intersects = true;
                    break;
                }
            }
        }

        if (!intersects)
            continue;

        glm::vec3 posEffect = pso->xf.posWorld;

        if (csbi > 0)
            posEffect = asbi[NRandInRange(0, csbi - 1)].pos;

        ApplyDiveShock(pso, pso->xf.posWorld, posEffect);
    }

    for (SO* pso : objects)
        pso->fHandleDiveEffect = 0;
}

bool FCheckJtStuck(JT* pjt)
{
    if (g_clock.t - pjt->tJtbs < 1.0f)
        return false;

    if (g_clock.t - pjt->tStuckCheck < 1.0f)
        return false;

    pjt->tStuckCheck = g_clock.t;

    if (pjt->fBaseXp != 0 || pjt->poxa == nullptr)
        return false;

    std::vector <glm::vec3> normals;

    for (OX* pox = pjt->poxa->pox; pox != nullptr; pox = pox->poxNext)
    {
        for (XP* pxp = pox->pxp; pxp != nullptr; pxp = pxp->pxpNext)
        {
            glm::vec3 normal = pxp->normal;

            if (pxp->axpd[0].psoRoot != pjt)
                normal = -normal;

            normals.push_back(normal);
        }
    }

    const int normalCount = static_cast<int>(normals.size());

    if (normalCount == 0)
        return false;

    const glm::vec3 gravityDirection = -glm::vec3(g_normalZ);

    std::vector<float> matrix(normalCount * normalCount);
    std::vector<float> target(normalCount);
    std::vector<float> weights(normalCount);

    for (int i = 0; i < normalCount; ++i)
    {
        for (int j = 0; j < normalCount; ++j)
            matrix[i * normalCount + j] = glm::dot(normals[i], normals[j]);

        target[i] = glm::dot(normals[i], gravityDirection);
    }

    SolveInequalities(normalCount, matrix.data(), target.data(), weights.data());

    glm::vec3 remainingDirection = gravityDirection;

    for (int i = 0; i < normalCount; ++i)
        remainingDirection += normals[i] * weights[i];

    return remainingDirection.z > -0.1f;
}

void UpdateJtJump(JT* pjt)
{
    // Choose the heading used while airborne.
    if (pjt->jtcs != -1)
    {
        if (!FTurnJtToTarget(pjt))
            pjt->radTarget = pjt->radJoy;
    }
    else
    {
        switch (pjt->jtbs)
        {
            case 0:
            pjt->radTarget = pjt->radJoy;
            break;

            case 7:
            case 8:
            break;

            case 12:
            GetHshapeHidePos(pjt->phshape, pjt->sParamHshape, nullptr, &pjt->radTarget);
            break;

            case 13:
            GetHpntClosestHidePos(pjt->phpnt, pjt->radHpnt, nullptr, &pjt->radTarget);
            break;

            default:
            // Retail compares horizontal speed (svxy), not the full 3D
            // velocity.  On the touchdown frame the vertical component is
            // still large while XY has collapsed nearly to zero; including Z
            // made atan2(noise.y, noise.x) overwrite the authored approach
            // heading immediately before the landing state was selected.
            if (glm::length(glm::vec2(pjt->xf.v)) > SVXY_JtJumpMatchHeading)
                pjt->radTarget = std::atan2(pjt->xf.v.y, pjt->xf.v.x);
            break;
        }
    }

    // Track how the stick moved during the initial portion of the jump.
    switch (pjt->jtjj)
    {
        case 0:
        if (pjt->uJoyDeflect == 1.0f)
        {
            pjt->jtjj = 1;
            pjt->mpjtjjt[1] = g_clock.t;
        }
        else if (g_clock.t - pjt->mpjtjjt[0] > DT_JtJtjjInit)
        {
            pjt->fShortenJump = 1;
        }
        break;

        case 1:
        if (pjt->uJoyDeflect < 1.0f)
        {
            pjt->jtjj = 2;
            pjt->mpjtjjt[2] = g_clock.t;

            if (g_clock.t - pjt->mpjtjjt[1] < DT_JtJtjjFull)
                pjt->fShortenJump = 1;
        }
        break;

        case 3:
        if (pjt->uJoyDeflect > 0.0f)
            pjt->fShortenJump = 1;
        break;
    }

    // Edge grabs take priority over the normal jump-state update.
    int ixpd = 0;
    glm::vec3 normal(0.0f);
    XP* pxpEdge = PxpFindJtBestEdge(pjt, &ixpd, &normal);

    if (pxpEdge != nullptr)
    {
        const int ixpdOther = 1 - ixpd;
        pjt->psoEdgeCur = pxpEdge->axpd[ixpdOther].psoLeaf;

        ConvertAloPos(nullptr, pjt->psoEdgeCur, &pxpEdge->axpd[ixpdOther].pos, &pjt->posEdgeLocal);

        pjt->xpkEdge = pxpEdge->xpk;
        pjt->pvEdge = (ixpd == 0) ? pxpEdge->pv1 : pxpEdge->pv0;
        pjt->normalEdgeCur = normal;
        
        SetJtJts(pjt, 8, -1);
        return;
    }

    // Retail-only recovery path absent from the supplied prototype.
    if (FCheckJtStuck(pjt))
    {
        StartJtZapJumpToTarget(pjt, 0.0f, nullptr);
        return;
    }

    switch (pjt->jtbs)
    {
        case 0: // Jump init
        if (pjt->xf.v.z < 0.0f)
        {
            SetJtJts(pjt, 2, 2);
            break;
        }

        if (g_clock.t - pjt->tJtbs > DT_JtJumpInit)
        {
            if (!g_joy.IsHeld(BTN_CROSS) && pjt->jtjj != 3)
                pjt->fShortenJump = 1;

            SetJtJts(pjt, 2, 1);
        }
        break;

        case 1: // Jump rise
        case 5: // Jump out
        if (pjt->fBaseXp && g_clock.t - pjt->tJts > 0.15f)
        {
            SetJtJts(pjt, JtsResetJt(pjt), -1);
        }
        else if (pjt->xf.v.z < 0.0f)
        {
            SetJtJts(pjt, 2, 2);
        }
        break;

        case 2: // Jump fall
        if (pjt->fBaseXp)
            SetJtJts(pjt, JtsResetJt(pjt), -1);
        break;

        case 3: // Jump boost
        if (pjt->fBaseXp)
            SetJtJts(pjt, JtsResetJt(pjt), -1);
        break;

        case 4: // Jump into basket
        {
            const float dtMatch = pjt->pactvalJump->tMatch - g_clock.t;
            if (dtMatch < 0.0f)
            {
                pjt->jthk = 2;
                SetJtJts(pjt, 6, 16);
                break;
            }

            glm::vec3 pos;
            glm::vec3 velocity;
            PredictAloTransform(pjt->phbsk, nullptr, dtMatch, &pos, nullptr, &velocity, nullptr);

            pjt->pactvalJump->posGoal = pos;
            pjt->pactvalJump->posGoal.z += 75.0f;
            pjt->pactvalJump->vGoal = velocity;
            pjt->pactvalJump->vGoal.z =
                pjt->xf.v.z + dtMatch * pjt->dvGravity.z;
            break;
        }

        case 6: // Jump thrown
        if (pjt->fBaseXp && g_clock.t - pjt->tJts > 0.15f)
            SetJtJts(pjt, JtsResetJt(pjt), -1);
        break;

        case 7: // Jump smash
        if (pjt->fBaseXp && g_clock.t - pjt->tJtbs > DT_JtMinSmash)
            SetJtJts(pjt, JtsResetJt(pjt), -1);
        break;

        case 8: // Jump dive
        if (pjt->fBaseXp && g_clock.t - pjt->tJtbs > DT_JtMinDive)
        {
            const float tEnd = TFindAsegLabel(pjt->pasegDiveAttack, (OID)443);
            if (pjt->pasegaCur->tLocal < tEnd)
            {
                HandleJtDiveEffect(pjt);
                SeekAsega(pjt->pasegaCur, SEEK_Start, tEnd, pjt->pasegaCur->svtLocal);
            }
        }
        break;

        case 9: // Jump to JMT
        {
            const float dtMatch = pjt->pactvalJump->tMatch - g_clock.t;
            if (dtMatch < 0.0f)
            {
                SetJtJts(pjt, 2, 2);
                break;
            }

            glm::vec3 pos = pjt->pjmtCur->posLocal;
            glm::vec3 velocity(0.0f);
            PredictAloTransformAdjust(pjt->pjmtCur->paloParent, nullptr, dtMatch, &pos, nullptr, &velocity, nullptr);

            pjt->pactvalJump->posGoal = pos;
            pjt->pactvalJump->posGoal.z += 75.0f;
            pjt->pactvalJump->vGoal = velocity;
            pjt->pactvalJump->vGoal.z =
                pjt->xf.v.z + dtMatch * pjt->dvGravity.z;
            break;
        }

        case 10: // Jump to spire
        {
            const float dtMatch = pjt->pactvalJump->tMatch - g_clock.t;
            if (dtMatch < 0.0f)
            {
                pjt->jthk = 5;
                SetJtJts(pjt, 6, 16);
                break;
            }

            glm::vec3 pos = pjt->pspireCur->posLocal;
            glm::vec3 velocity(0.0f);
            PredictAloTransformAdjust(pjt->pspireCur->paloParent, nullptr, dtMatch, &pos, nullptr, &velocity, nullptr);

            pjt->pactvalJump->posGoal = pos;
            pjt->pactvalJump->posGoal.z += 75.0f;
            pjt->pactvalJump->vGoal = velocity;
            pjt->pactvalJump->vGoal.z =
                pjt->xf.v.z + dtMatch * pjt->dvGravity.z;
            break;
        }

        case 11: // Jump to rail
        {
            const float dtMatch = pjt->pactvalJump->tMatch - g_clock.t;
            if (dtMatch < 0.0f)
            {
                pjt->jthk = 4;
                SetJtJts(pjt, 6, 16);
                break;
            }

            glm::vec3 pos(0.0f);
            CRV* pcrv = pjt->prailCur->pcrv.get();
            if (pcrv != nullptr && pcrv->pvtcrv->pfnEvaluateCrvFromU != nullptr)
                pcrv->pvtcrv->pfnEvaluateCrvFromU(pcrv, pjt->uRailCur, &pos, nullptr);

            PredictAloTransformAdjust(pjt->prailCur->paloParent, nullptr, dtMatch, &pos, nullptr, nullptr, nullptr);

            pjt->pactvalJump->posGoal = pos;
            pjt->pactvalJump->posGoal.z += 75.0f;
            break;
        }

        case 12: // Jump to HSHAPE
        case 13: // Jump to HPNT
        {
            const float dtMatch = pjt->pactvalJump->tMatch - g_clock.t;
            HPNT* phpnt = pjt->jtbs == 12
                ? reinterpret_cast<HPNT*>(pjt->phshape)
                : pjt->phpnt;

            if (dtMatch < 0.0f)
            {
                pjt->jthk = phpnt->jthk;
                SetJtJts(pjt, 6, 16);
                break;
            }

            glm::vec3 pos;
            if (pjt->jtbs == 12)
                GetHshapeHidePos(pjt->phshape, pjt->sParamHshape, &pos, nullptr);
            else
                GetHpntClosestHidePos(pjt->phpnt, pjt->radHpnt, &pos, nullptr);

            ConvertAloPos(nullptr, phpnt->paloParent, &pos, &pos);

            glm::vec3 velocity(0.0f);
            PredictAloTransformAdjust(phpnt->paloParent, nullptr, dtMatch, &pos, nullptr, &velocity, nullptr);

            pjt->pactvalJump->posGoal = pos;
            pjt->pactvalJump->posGoal.z += 75.0f;
            pjt->pactvalJump->vGoal = velocity;
            pjt->pactvalJump->vGoal.z =
                pjt->xf.v.z + dtMatch * pjt->dvGravity.z;
            break;
        }

        case 14: // Jump to handle
        {
            const float dtMatch = pjt->pactvalJump->tMatch - g_clock.t;
            if (dtMatch < 0.0f)
            {
                SetJtJts(pjt, 3, 51);
                break;
            }

            MeasureJtJumpToTargetAtTime(dtMatch, pjt, pjt->phndCur->paloParent, &pjt->phndCur->posLocal, nullptr, &pjt->pactvalJump->posGoal, &pjt->pactvalJump->vGoal);
            break;
        }

        case 15: // Jump to pipe
        if (pjt->pactvalJump->tMatch < g_clock.t)
            SetJtJts(pjt, 7, 23);
        break;
    }

    // Retail-only sanity check for target-guided jump trajectories.
    if (pjt->jtbs >= 9 && pjt->jtbs < 16)
    {
        const float t = (pjt->pactvalJump->tMatch - g_clock.t) + g_clock.dt;
        glm::vec3 dpos;
        glm::vec3 velocity;

        EvaluateBezierPos(t, 0.0f, 1.0f, &pjt->xf.pos, &pjt->xf.v, &pjt->pactvalJump->posGoal, &pjt->pactvalJump->vGoal, nullptr, nullptr, &dpos);
        EvaluateBezierPos(t, t, 1.0f, &pjt->xf.pos, &pjt->xf.v, &pjt->pactvalJump->posGoal, &pjt->pactvalJump->vGoal, nullptr, nullptr, &velocity);

        const float estimatedDistance =
            (glm::length(dpos) + glm::length(velocity)) * t * 0.5f;

        if (estimatedDistance > S_JtJumpTargetPathMax)
            SetJtJts(pjt, 2, 2);
    }
}

int FTimeJtJumpToTarget(JT* pjt, glm::vec3* pposJt, glm::vec3* pvJt, glm::vec3* pposTarget, glm::vec3* pvTarget, float* pdtProject)
{
    float roots[2]{};

    const int rootCount = CSolveQuadratic(pjt->dvGravity.z * 0.5f, pvJt->z - pvTarget->z, pposJt->z - pposTarget->z, roots);

    if (rootCount == 0)
        return 0;

    if (rootCount == 2)
        roots[0] = std::max(roots[0], roots[1]);

    if (roots[0] <= 0.0f)
        return 0;

    *pdtProject = roots[0];
    return 1;
}

void GetJtJumpVelocity(JT* pjt, glm::vec3* pv)
{
    float vzJump = pjt->xf.v.z;

    switch (pjt->jtjk)
    {
        case 0:
        case 2:
        {
            const float dz = pjt->xf.posWorld.z - (pjt->posBaseXp.z + DZ_JtStandingJump);
            vzJump = std::sqrt(std::max(2.0f * dz * pjt->dvGravity.z, 0.0f));
            break;
        }

        case 1:
        vzJump = SVZ_JtSwingingJump * std::sqrt(pjt->rGravity);
        break;

        case 3:
        vzJump = SVZ_JtFromWaterJump * std::sqrt(pjt->rGravity);
        break;

        case 4:
        vzJump = SVZ_JtEdgeJump * std::sqrt(pjt->rGravity);
        break;
    }

    *pv = pjt->xf.v;
    pv->z = std::max(pjt->xf.v.z, vzJump);
}

void GetJtJumpToTargetVelocity(JT* pjt, glm::vec3* pv)
{
    *pv = pjt->xf.v;

    const float vz = pv->z;
    const float vzTarget = s_clqVzToVz.g0 + vz * (s_clqVzToVz.g1 + vz * s_clqVzToVz.g2);
    const float vzClamped = glm::clamp(vzTarget, s_lmVz.gMin, s_lmVz.gMax);

    pv->z = std::max(pjt->xf.v.z, vzClamped);
}

void GetJtJumpBoostVelocity(JT* pjt, glm::vec3* pv)
{
    *pv = pjt->xf.v;

    const float vz = pv->z;
    const float vzBoost = s_clqVzToVz.g0 + vz * (s_clqVzToVz.g1 + vz * s_clqVzToVz.g2);
    const float vzClamped = glm::clamp(vzBoost, s_lmVz.gMin, s_lmVz.gMax);

    pv->z = std::max(pjt->xf.v.z, vzClamped);
}

void StartupJmt()
{
	InitDl(&g_dlJmt, offsetof(JMT, dleJmt));
}

void ResetJmtList()
{
	ClearDl(&g_dlJmt);
}

JMT* NewJmt()
{
	return new JMT{};
}

int GetJmtSize()
{
	return sizeof(JMT);
}

void* GetJmtSRadius(JMT* pjmt)
{
	return &pjmt->sRadius;
}

void SetJmtSRadius(JMT* pjmt, float sRadius)
{
	pjmt->sRadius = sRadius;
}

void SetJmtOidSetIgnoreJt(JMT* pjmt, int oidSetIgnoreJt)
{
	pjmt->oidSetIgnoreJt = (OID)oidSetIgnoreJt;
}

void OnJmtAdd(JMT* pjmt)
{
	OnLoAdd(pjmt);
	AppendDlEntry(&g_dlJmt, pjmt);
}

void OnJmtRemove(JMT* pjmt)
{
	OnLoRemove(pjmt);
	RemoveDlEntry(&g_dlJmt, pjmt);
}

void CloneJmt(JMT* pjmt, JMT* pjmtBase)
{
	DLE dleJmt = pjmt->dleJmt;

	ClonePnt(pjmt, pjmtBase);

	// Clone the DLE structure (assuming it's a simple structure, shallow copy should suffice)
	pjmt->dleJmt = pjmtBase->dleJmt;

	// Clone the float sRadius
	pjmt->sRadius = pjmtBase->sRadius;

	// Clone the OID and pointers (assuming these need shallow copying)
	pjmt->oidSetIgnoreJt = pjmtBase->oidSetIgnoreJt;
	pjmt->psoSetIgnoreJt = pjmtBase->psoSetIgnoreJt;

	pjmt->dleJmt = dleJmt;
}

void PostJmtLoad(JMT* pjmt)
{
	PostLoLoad(pjmt);

	if (pjmt->oidSetIgnoreJt != OID_Nil) 
	{
		SO *pso = (SO*)PloFindSwObject(pjmt->psw, 0x104, (OID)pjmt->oidSetIgnoreJt, pjmt);
		pjmt->psoSetIgnoreJt = pso;
	}
}

void DeleteJmt(JMT* pjmt)
{
	delete pjmt;
}

DL g_dlJmt;
