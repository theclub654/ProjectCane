#include "step.h"
#include "bbmark.h"
#include "jt.h"

STEP* NewStep()
{
	return new STEP{};
}

void InitStep(STEP* pstep)
{
    InitPo(pstep);

    pstep->rGravity = 1.0f;
    pstep->gBuoyancy = G_StepBuoyancyDefault;

    SetStepSpinRatio(pstep, 1.0f);

    pstep->clqDradToURun = s_clqDradToURun;
    pstep->smpExpand = s_smpExpand;
    pstep->smpCompress = s_smpCompress;
}

int GetStepSize()
{
	return sizeof(STEP);
}

void* GetStepDzBase(STEP* pstep)
{
    return &pstep->dzBase;
}

void SetStepDzBase(STEP* pstep, float dzBase)
{
    pstep->dzBase = dzBase;
}

void CloneStep(STEP* pstep, STEP* pstepBase)
{
    ClonePo(pstep, pstepBase);

    pstep->paloLeftFoot = pstepBase->paloLeftFoot;
    pstep->paloRightFoot = pstepBase->paloRightFoot;
    pstep->paloLeftHand = pstepBase->paloLeftHand;
    pstep->paloRightHand = pstepBase->paloRightHand;
    pstep->paloLeftHeel = pstepBase->paloLeftHeel;
    pstep->paloRightHeel = pstepBase->paloRightHeel;
    pstep->paloLeftBall = pstepBase->paloLeftBall;
    pstep->paloRightBall = pstepBase->paloRightBall;

    // Copy simple float values
    pstep->radJoy = pstepBase->radJoy;
    pstep->uJoyDeflect = pstepBase->uJoyDeflect;
    pstep->radTarget = pstepBase->radTarget;
    pstep->uTarget = pstepBase->uTarget;
    pstep->zBaseTarget = pstepBase->zBaseTarget;

    // Copy glm::vec3 (trivially copyable)
    pstep->vTarget = pstepBase->vTarget;

    // Copy mat3 (trivially copyable)
    pstep->matTarget = pstepBase->matTarget;

    // Copy flags and int values
    pstep->fBaseXp = pstepBase->fBaseXp;
    pstep->tBaseXp = pstepBase->tBaseXp;
    pstep->uFootfall = pstepBase->uFootfall;
    pstep->tLastFootfall = pstepBase->tLastFootfall;
    pstep->dzBase = pstepBase->dzBase;
    pstep->iposBase = pstepBase->iposBase;

    // Copy glm::vec3 (for position and normal)
    pstep->posBaseXp = pstepBase->posBaseXp;
    pstep->normalBaseXp = pstepBase->normalBaseXp;

    // Copy pointers to ALO (assumed shallow copy)
    pstep->psoBaseXp = pstepBase->psoBaseXp;
    pstep->psoPhys = pstepBase->psoPhys;

    // Copy SMP structures (assuming they are trivially copyable)
    pstep->smpSpin = pstepBase->smpSpin;
    pstep->smpCompress = pstepBase->smpCompress;
    pstep->smpExpand = pstepBase->smpExpand;

    // Copy CLQ (assuming shallow copy is sufficient)
    pstep->clqDradToURun = pstepBase->clqDradToURun;

    // Copy gravity and related float values
    pstep->rGravity = pstepBase->rGravity;

    // These are pointers into geomWorld.apos, not shared object references.
    // CloneSo gives this STEP its own world geometry, so preserve the pointed
    // vertex indices and rebase both pointers onto the clone's storage.
    auto RebaseWorldPos = [pstep, pstepBase](const glm::vec3* pposBase) -> glm::vec3*
    {
        if (pposBase == nullptr || pstepBase->geomWorld.apos.empty())
            return nullptr;

        const glm::vec3* first = pstepBase->geomWorld.apos.data();
        const std::ptrdiff_t ipos = pposBase - first;

        if (ipos < 0 || ipos >= static_cast<std::ptrdiff_t>(pstep->geomWorld.apos.size()))
            return nullptr;

        return &pstep->geomWorld.apos[ipos];
    };

    pstep->pposBase = RebaseWorldPos(pstepBase->pposBase);
    pstep->pposBumper = RebaseWorldPos(pstepBase->pposBumper);

    // Copy other variables that hold data about footfall and base positions
    pstep->paloLeftFoot = pstepBase->paloLeftFoot;
    pstep->paloRightFoot = pstepBase->paloRightFoot;
    pstep->paloLeftHand = pstepBase->paloLeftHand;
    pstep->paloRightHand = pstepBase->paloRightHand;
    pstep->paloLeftHeel = pstepBase->paloLeftHeel;
    pstep->paloRightHeel = pstepBase->paloRightHeel;
    pstep->paloLeftBall = pstepBase->paloLeftBall;
    pstep->paloRightBall = pstepBase->paloRightBall;
}

void PostStepLoad(STEP* pstep)
{
    PostAloLoad(pstep);
    SnipAloObjects(pstep, 6, s_asnipPostLoad);
    LimitStepHands(pstep, 1);

    pstep->paloLeftFoot =
        pstep->paloLeftBall != nullptr
        ? pstep->paloLeftBall
        : pstep->paloLeftHeel;

    pstep->paloRightFoot =
        pstep->paloRightBall != nullptr
        ? pstep->paloRightBall
        : pstep->paloRightHeel;

    pstep->matTarget = glm::mat3(1.0);
}

void LimitStepHands(STEP* pstep, int fLimit)
{
    constexpr uint32_t GRFALOX_IK = 32;
    constexpr uint32_t GRFIK_LIMIT = 2;

    ALO* apaloHand[2] = { pstep->paloRightHand,pstep->paloLeftHand };

    for (int ihand = 0; ihand < 2; ihand++) {
        ALO* paloHand = apaloHand[ihand];

        if (paloHand == nullptr || paloHand->palox == nullptr) {
            continue;
        }

        ALOX* palox = paloHand->palox.get();

        if ((palox->grfalox & GRFALOX_IK) == 0) {
            continue;
        }

        if (fLimit != 0) {
            palox->ikh.grfik |= GRFIK_LIMIT;
        }
        else {
            palox->ikh.grfik &= ~GRFIK_LIMIT;
        }
    }
}

void RetractStepExtremity(STEP* pstep, ALO* paloExtrem, float sRadius, ALO* paloAdjust, ALO* paloIkh, XMG* pxmg)
{
    pxmg->fRetractValid = 0;

    ALOX* palox = paloIkh->palox.get();

    if (palox == nullptr || (palox->grfalox & 0x20U) == 0)
        return;

    ALO* paloShoulder = palox->ikh.paloShoulder;

    if (paloShoulder == nullptr)
        return;

    glm::vec3 posIkhWorld{};
    ConvertAloPos(paloIkh, nullptr, &palox->ikh.posIkh, &posIkhWorld);

    const glm::vec3 deltaIkh = posIkhWorld - paloShoulder->xf.posWorld;
    const float ikhLength = glm::length(deltaIkh);

    const glm::vec3 directionIkh =
        ikhLength < 0.0001f
        ? g_normalX
        : deltaIkh / ikhLength;

    pxmg->sRetract = ikhLength - 15.0f;

    if (pxmg->sRetract < 0.0001f)
        return;

    pxmg->posRetractStart = paloExtrem->xf.posWorld - g_normalZ * sRadius;
    pxmg->posRetractEnd = pxmg->posRetractStart - directionIkh * pxmg->sRetract;

    // Initializes the current closest-hit parameter before clipping.
    pxmg->lsg.au[0] = -1;

    pxmg->fRetractValid = 1;
}

void ClipStepRetractionToObject(STEP* pstep, SO* pso, BSP* pbspPruned, XMG* pxmg)
{
    LSG alsg[2]{};

    const int clsg = ClsgClipEdgeToObjectPruned(pso, pbspPruned, &pxmg->posRetractEnd, &pxmg->posRetractStart, 2, alsg);

    if (clsg <= 0)
        return;

    int ilsg = 0;

    // Ignore a hit located exactly at the beginning of the segment.
    if (alsg[0].au[0] == 0.0f)
        ilsg = 1;

    if (ilsg >= clsg)
        return;

    // Keep the closest hit found so far.
    if (alsg[ilsg].au[0] < pxmg->lsg.au[0])
        pxmg->lsg = alsg[ilsg];
}

void ApplyStepExtremityRetraction(STEP* pstep, ALO* paloExtrem, ALO* paloAdjust, XMG* pxmg)
{
    const bool fHit = pxmg->lsg.au[0] != -1;

    // The retraction result is consumed only once.
    pxmg->fRetractValid = 0;

    if (!fHit || pxmg->sRetract < 0.0001f)
        return;

    const glm::vec3 retractDirection = (pxmg->posRetractStart - pxmg->posRetractEnd) / pxmg->sRetract;
    const glm::vec3 hitAdjustment = pxmg->lsg.apos[0] - pxmg->posRetractStart;

    glm::vec3 posAdjustWorld = paloAdjust->xf.posWorld + hitAdjustment + retractDirection;
    glm::vec3 posAdjustLocal{};

    ConvertAloPos(nullptr, paloAdjust->paloParent, &posAdjustWorld, &posAdjustLocal);
    paloAdjust->pvtalo->pfnTranslateAloToPos(paloAdjust, &posAdjustLocal);

    glm::vec3 movement{};
    CalculateAloMovement(paloAdjust, nullptr, paloExtrem->xf.posWorld, &movement, nullptr, nullptr, nullptr);

    glm::vec3 hitNormalWorld = pxmg->lsg.anormal[0];
    float normalVelocity = glm::dot(movement, hitNormalWorld);

    if (normalVelocity >= 0.0f)
        return;

    glm::vec3 hitNormalLocal{};
    ConvertAloVec(nullptr, paloAdjust->paloParent, &hitNormalWorld, &hitNormalLocal);

    // Remove only the component of velocity entering the surface.
    glm::vec3 velocity = paloAdjust->xf.v - hitNormalLocal * normalVelocity;
    paloAdjust->pvtalo->pfnSetAloVelocityVec(paloAdjust, &velocity);
}

void RenderStepSelf(STEP* pstep, CM* pcm, RO* pro)
{
    RenderSoSelf(pstep, pcm, pro);
}

void ReadStepJoystick(STEP* pstep, JOY* pjoy)
{
    static float uDeflectPrevious = 0.0f;

    const float uDeflectThreshold = (uDeflectPrevious > 0.25f) ? (uDeflectPrevious - 0.25f) : 0.0f;

    if (pjoy->uDeflect > uDeflectThreshold)
        AdjustCmJoy(g_pcm, pjoy, JOYID_Left, &pstep->radJoy);
    else
        pstep->radJoy = pstep->radTarget;

    uDeflectPrevious = pjoy->uDeflect;
    pstep->uJoyDeflect = uDeflectPrevious;
}

void UpdateStepFootfall(STEP* pstep, float u, SFF* psff)
{
    const float uPrevious = pstep->uFootfall;
    pstep->uFootfall = u;

    if (g_clock.t - pstep->tLastFootfall < DT_StepFootfallInterval)
        return;

    float volume = 0.0f;

    const float uFromLeft = GModPositive(u - psff->au[0], 1.0f);
    const float uPreviousFromLeft = GModPositive(uPrevious - psff->au[0], 1.0f);

    if (uFromLeft < 0.5f && uPreviousFromLeft > 0.5f)
        volume = psff->auVol[0];
    else
    {
        const float uFromRight = GModPositive(u - psff->au[1], 1.0f);
        const float uPreviousFromRight = GModPositive(uPrevious - psff->au[1], 1.0f);

        if (uFromRight >= 0.5f || uPreviousFromRight <= 0.5f)
            return;

        volume = psff->auVol[1];
    }

    constexpr int kReleaseFootfallFirst = 71;
    constexpr int kReleaseFootfallLast = 74;
    int isfxid = NRandInRange(kReleaseFootfallFirst, kReleaseFootfallLast);

    if (isfxid == s_sfxidLast)
    {
        ++isfxid;
        if (isfxid > kReleaseFootfallLast)
            isfxid = kReleaseFootfallFirst;
    }

    s_sfxidLast = isfxid;

    StartSound(static_cast<SFXID>(isfxid), nullptr, nullptr, &pstep->xf.posWorld, 1000.0f, 200.0f,
               volume, 0.0f, 0.0f, nullptr, nullptr);
    NextSneakyFootstep();
}

void UpdateStepPhys(STEP* pstep)
{
    SO* psoPhys = pstep->psoPhys;

    if (psoPhys == nullptr) {
        return;
    }

    int cpos = psoPhys->geomLocal.cpos;
    glm::vec3* aposPhysLocal = psoPhys->geomLocal.apos.data();
    glm::vec3* aposStepLocal = pstep->geomLocal.apos.data();
    glm::vec3* avStepLocal = pstep->plvo->mpiposvLocal.data();

    for (int ipos = 0; ipos < cpos; ipos++)
    {
        glm::vec3 pos = psoPhys->xf.pos
            + psoPhys->xf.mat[0] * aposPhysLocal[ipos].x
            + psoPhys->xf.mat[1] * aposPhysLocal[ipos].y
            + psoPhys->xf.mat[2] * aposPhysLocal[ipos].z;

        for (int iaxis = 0; iaxis < 3; iaxis++)
        {
            float sOld = aposStepLocal[ipos][iaxis];
            float sNew = GSmooth(sOld, pos[iaxis], g_clock.dt, &s_smpPhys, nullptr);

            avStepLocal[ipos][iaxis] = (sNew - sOld) / g_clock.dt;
        }
    }
}

void SetStepPhys(STEP* pstep, SO* pso, int fForceSnap)
{
    if (pso == pstep->psoPhys)
        return;

    bool fChanged = pso == nullptr;

    if (pso != nullptr && pstep->plvo != nullptr)
    {
        // The original uses the candidate physical shape's counts for both
        // loops.  The STEP geometry was cloned with enough storage for every
        // candidate, so clamping here changes which vertices get switched.
        const int cpos = pso->geomLocal.cpos;

        for (int ipos = 0; ipos < cpos; ++ipos)
        {
            const glm::vec3 posLocal = pso->xf.pos + pso->xf.mat * pso->geomLocal.apos[ipos];

            bool fInside = true;

            if (!fForceSnap)
            {
                for (int isurf = 0; isurf < pso->geomLocal.csurf; ++isurf)
                {
                    const SURF& surf = pstep->geomLocal.asurf[isurf];

                    if (glm::dot(posLocal, surf.normal) - surf.gDot > 1.0f)
                    {
                        fInside = false;
                        break;
                    }
                }
            }

            if (!fInside)
                continue;

            pstep->geomLocal.apos[ipos] = posLocal;
            pstep->plvo->mpiposvLocal[ipos] = glm::vec3(0.0f);
            fChanged = true;
        }
    }

    if (fChanged)
    {
        InvalidateSwXpForObject(pstep->psw, pstep, 7);

        if (pstep->plvo != nullptr)
            ProjectSoLvo(pstep, 0.0f);
    }

    pstep->psoPhys = pso;
}

void UpdateStep(STEP* pstep, float dt)
{
    UpdatePo(pstep, dt);
    UpdateStepPhys(pstep);
}

CT CtTorqueStep(STEP* pstep)
{
    return CT_Locked;
}

void PropagateStepForce(STEP* pstep, GRFSG grfsg, XP* pxp, int ixpd, DZ* pdz, FX* afx)
{
    CT ctSaved = pstep->constrTorque.ct;

    pstep->constrTorque.ct = pstep->pvtstep->pfnCtTorqueStep(pstep);

    PropagateSoForce(pstep, grfsg, pxp, ixpd, pdz, afx);

    pstep->constrTorque.ct = ctSaved;
}

void RotateStepToMat(STEP* pstep, glm::mat3& pmat)
{
    RotateSoToMat(pstep, pmat);

    float rad = atan2f(pmat[0][1], pmat[0][0]);

    pstep->radJoy = rad;
    pstep->radTarget = rad;

    pstep->pvtstep->pfnUpdateStepMatTarget(pstep);
}

void CloneStepPhys(STEP* pstep, SO* psoPhys, int cposExtra)
{
    CloneSoPhys(pstep, psoPhys, cposExtra);

    if (pstep->geomLocal.cpos <= 0)
        return;

    if (pstep->geomLocal.apos.size() < static_cast<size_t>(pstep->geomLocal.cpos))
        pstep->geomLocal.apos.resize(pstep->geomLocal.cpos);

    if (pstep->geomWorld.apos.size() < static_cast<size_t>(pstep->geomWorld.cpos))
        pstep->geomWorld.apos.resize(pstep->geomWorld.cpos);

    int cposTotal = pstep->geomLocal.cpos;
    int cposReal = cposTotal - cposExtra;

    if (cposReal <= 0 || cposExtra <= 0)
        return;

    int iposBase = cposTotal - 1;
    int iposLowest = -1;
    float zLowest = FLT_MAX;

    glm::mat4 dmat(1.0f);
    LoadMatrixFromPosRot(&pstep->xf.posWorld, &pstep->xf.matWorld, &dmat);

    for (int ipos = 0; ipos < cposReal; ++ipos)
    {
        float z = pstep->geomLocal.apos[ipos].z;

        if (z < zLowest)
        {
            zLowest = z;
            iposLowest = ipos;
            pstep->pposBumper = &pstep->geomWorld.apos[ipos];
        }
    }

    if (iposLowest < 0)
        return;

    glm::vec3 posBase =
        pstep->geomLocal.apos[iposLowest] +
        glm::vec3(0.0f, 0.0f, -pstep->dzBase);

    pstep->geomLocal.apos[iposBase] = posBase;
    pstep->geomWorld.apos[iposBase] = glm::vec3(dmat * glm::vec4(posBase, 1.0f));

    pstep->iposBase = iposBase;
    pstep->pposBase = &pstep->geomWorld.apos[iposBase];

}

void PresetStepAccel(STEP* pstep, float dt)
{
    PresetSoAccel(pstep, dt);
    PresetStepAccelBase(pstep);
}

void PresetStepAccelBase(STEP* pstep)
{
    AccelSoTowardMatSmooth((SO*)pstep, g_clock.dt, &pstep->matTarget, &pstep->smpSpin);

    if (pstep->psoPhys == nullptr) {
        return;
    }

    glm::vec3* pposBase = pstep->fBaseXp ? &pstep->posBaseXp : pstep->pposBase;
    glm::vec3 dpos = pstep->xf.posWorld - *pposBase;

    // Keep the base point stationary under angular acceleration.  The PS2
    // VU sequence is dw x dpos; reversing these operands applies the exact
    // opposite tangential acceleration and makes grounded steps drift.
    glm::vec3 dv = glm::cross(pstep->xf.dw, dpos);
    ApplySoConstraintLocal(pstep, &pstep->constrForce, &dv, &dv, nullptr);
    AddSoAcceleration(pstep, &dv);

    glm::vec3 v = glm::cross(dpos, pstep->xf.w);
    dv = glm::cross(v, pstep->xf.w);

    ApplySoConstraintLocal(pstep, &pstep->constrForce, &dv, &dv, nullptr);
    AddSoAcceleration(pstep, &dv);

}

int FCheckStepXpBase(STEP* pstep, XP* pxp, int ixpd)
{
    if (pstep->psoPhys == nullptr || pxp->normal.z * g_mpixpdr[ixpd] < 0.7f) {
        return 0;
    }

    if (pxp->xpk == XPK_SphereToPoint) {
        return ixpd == 1 && pxp->pv1 == pstep->pposBase;
    }

    if (pxp->xpk == XPK_PointToSurface) {
        return ixpd == 0 && pxp->pv0 == pstep->pposBase;
    }

    if (pxp->xpk == XPK_Internal) {
        return ixpd == 0 && pxp->pv0 == pstep->pposBase;
    }

    return 0;
}

void AdjustStepNewXp(STEP* pstep, XP* pxp, int ixpd)
{
    if (pstep->pvtstep->pfnFCheckStepXpBase(pstep, pxp, ixpd)) {
        pxp->elas = 0.0f;
        pxp->mu = MU_StepInternal;
    }
    else {
        pxp->elas = 0.0f;
        pxp->mu = 0.0f;
    }
}

void AdjustStepDz(STEP* pstep, GRFADJ grfadj, DZ* pdz, int ixpd, float dt)
{
    AdjustStepDzBase(pstep, grfadj, pdz, ixpd);
}

void AdjustStepDzBase(STEP* pstep, GRFADJ grfadj, DZ* pdz, int ixpd)
{

}

void UpdateStepMatTarget(STEP* pstep)
{
    LoadRotateMatrixRad(pstep->radTarget, &g_normalZ, &pstep->matTarget);
}

void AdjustStepXpVelocity(STEP* pstep, XP* pxp, int ixpd)
{
    if (pstep->pvtstep->pfnFCheckStepXpBase(pstep, pxp, ixpd))
        AdjustStepXpVelocityBase(pstep, pxp, ixpd);
}

void UpdateStepXfWorld(STEP* pstep)
{
    UpdateSoXfWorld(pstep);

    // radTarget and the STEP's authored facing are both in local space.
    float currentRad = atan2f(pstep->xf.mat[0][1], pstep->xf.mat[0][0]);

    // Difference between desired angle and current angle.
    float drad = RadNormalize(pstep->radTarget - currentRad);

    float absDrad = fabsf(drad);
    float uRun = pstep->clqDradToURun.g0 + absDrad * (pstep->clqDradToURun.g1 + absDrad * pstep->clqDradToURun.g2);

    // Clamp 0..1
    pstep->uTarget = glm::clamp(uRun, 0.0f, 1.0f);
}

void AdjustStepXpVelocityBase(STEP* pstep, XP* pxp, int ixpd)
{
    float svStep = 0.0f;
    float normalStep = 0.0f;
    float gStep = 0.0f;
    float svXp = 0.0f;
    float svExpand = 0.0f;
    float svCompress = 0.0f;

    if (pstep->psoPhys != nullptr && pstep->dzBase != 0.0f) {
        glm::vec3 axisZ = pstep->xf.matWorld[2];
        normalStep = glm::dot(pxp->normal, axisZ) * g_mpixpdr[ixpd];

        if (pxp->gDist < -2.0f) {
            // The retail VU select chooses the larger operand here, then
            // negates it for the displacement along the step's Z axis.
            float dz = glm::max((pxp->gDist + 1.0f) / normalStep, pstep->geomLocal.apos[pstep->iposBase].z + 10.0f);

            pxp->axpd[ixpd].pos -= axisZ * dz;
            pxp->gDist = glm::dot(pxp->axpd[0].pos - pxp->axpd[1].pos, pxp->normal);
        }

        glm::vec3 dpos = *pstep->pposBase - pxp->axpd[ixpd].pos;
        glm::vec3 dv = pxp->axpd[0].v - pxp->axpd[1].v;

        SMP smpExpand = pstep->smpExpand;
        SMP smpCompress = pstep->smpCompress;

        smpExpand.svFast *= pstep->rGravity;
        smpCompress.svFast *= pstep->rGravity;

        gStep = glm::dot(dpos, axisZ);
        svXp = glm::dot(dv, pxp->normal) / normalStep;

        GSmooth(gStep, 0.0f, g_clock.dt, &smpExpand, &svExpand);
        GSmooth(gStep, -pstep->dzBase, g_clock.dt, &smpCompress, &svCompress);

        svStep = glm::max(svXp, svCompress);
        svStep += GLimitAbs(svExpand - svStep, (g_clock.dt * (smpCompress.svFast - smpCompress.svSlow)) / smpCompress.dtFast);
    }

    glm::vec3 vTarget = pstep->matTarget[0] * pstep->vTarget.x + pstep->matTarget[1] * pstep->vTarget.y + pstep->matTarget[2] * pstep->vTarget.z;
    vTarget -= pxp->normal * glm::dot(vTarget, pxp->normal);

    pxp->axpd[ixpd].v += pstep->xf.matWorld[2] * -svStep + vTarget * pstep->uTarget;

}

void AdjustStepXps(STEP* pstep)
{
    XP* pxpFree = nullptr;
    XP* pxpBest = nullptr;

    pstep->fBaseXp = 0;

    float gBest = FLT_MAX;

    for (OX* pox = pstep->poxa->pox; ; pox = pox->poxNext)
    {
        XP* pxp = pox != nullptr ? pox->pxp : pstep->pxpInternal;

        for (; pxp != nullptr; pxp = pxp->pxpNext)
        {
            int ixpdStep = pstep != (STEP*)pxp->axpd[0].psoRoot;
            const bool fBaseCandidate = pstep->pvtstep->pfnFCheckStepXpBase(pstep, pxp, ixpdStep) != 0;

            if (!fBaseCandidate) {
                continue;
            }

            int ixpdOther = 1 - ixpdStep;
            float gBase = glm::dot(pstep->xf.posWorld - pxp->axpd[ixpdOther].pos, pstep->xf.matWorld[2]);

            if (gBase < gBest) {
                gBest = gBase;
                pxpBest = pxp;

                pstep->fBaseXp = 1;
                pstep->tBaseXp = g_clock.t;
                pstep->posBaseXp = pxp->axpd[ixpdOther].pos;
                pstep->normalBaseXp = pxp->normal * g_mpixpdr[ixpdStep];
                // The original reads XPD + 4 here, which is psoLeaf.  Keep the
                // exact contacted child so moving/hierarchical supports use
                // their own transform rather than the aggregate root's.
                pstep->psoBaseXp = pxp->axpd[ixpdOther].psoLeaf;
            }
        }

        if (pox == nullptr) {
            break;
        }
    }

    if (pstep->psoPhys == nullptr) {
        return;
    }

    for (OX* pox = pstep->poxa->pox; ; pox = pox->poxNext) {
        XP** ppxp = pox != nullptr ? &pox->pxp : &pstep->pxpInternal;

        while (*ppxp != nullptr) {
            XP* pxp = *ppxp;
            int ixpdStep = pstep != (STEP*)pxp->axpd[0].psoRoot;

            if (pxp->axpd[ixpdStep].psoLeaf != (SO*)pstep) {
                ppxp = &pxp->pxpNext;
                continue;
            }

            bool removeXp = false;

            if (pxp->xpk == XPK_PointToSurface && pxp->pv0 == pstep->pposBumper) {
                removeXp = pstep->fBaseXp != 0;
            }
            else if (pxp->xpk == XPK_SphereToPoint && pxp->pv1 == pstep->pposBumper) {
                removeXp = pstep->fBaseXp != 0;
            }
            else if (pxp != pxpBest && ((pxp->xpk == XPK_PointToSurface && pxp->pv0 == pstep->pposBase) || (pxp->xpk == XPK_SphereToPoint && pxp->pv1 == pstep->pposBase) || (pxp->xpk == XPK_Internal && pxp->pv0 == pstep->pposBase))) {
                removeXp = true;
            }

            if (removeXp) {
                *ppxp = pxp->pxpNext;
                pxp->pxpNext = pxpFree;
                pxpFree = pxp;
                continue;
            }

            if (pxp->xpk == XPK_EdgeToEdge || (ixpdStep != 0 && (pxp->xpk == XPK_PointToSurface || pxp->xpk == XPK_SphereToPoint || pxp->xpk == XPK_SphereToEdge || pxp->xpk == XPK_SphereToSurface))) {
                if (pxp->normal.z * g_mpixpdr[ixpdStep] > 0.0f) {
                    int ixpdOther = 1 - ixpdStep;

                    glm::vec3 normal = pxp->normal * g_mpixpdr[ixpdStep];
                    normal.z = 0.0f;

                    if (glm::length(normal) < 0.0001f) {
                        normal = g_normalX;
                    }
                    else {
                        normal = glm::normalize(normal);
                    }

                    float gDist = glm::dot(pxp->axpd[ixpdStep].pos - pxp->axpd[ixpdOther].pos, normal);
                    glm::vec3 pos = pxp->axpd[ixpdOther].pos + normal * gDist;

                    PxpAdd((SO*)pstep, pxp->axpd[ixpdOther].psoLeaf, &pos, &normal, gDist, XPK_Internal, nullptr, nullptr, &pxp->pxpNext);
                }
            }

            ppxp = &pxp->pxpNext;
        }

        if (pox == nullptr) {
            FreeSwXpList(pstep->psw, pxpFree);
            return;
        }

        pox->poxOther->pxp = pox->pxp;
    }
}

void AddStepCustomXps(STEP* pstep, SO* psoOther, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst)
{
    AddStepCustomXpsBase(pstep, psoOther, pbspPruned, ppxpFirst);
}

void AddStepCustomXpsBase(STEP* pstep, SO* psoOther, BSP* pbspPruned, XP** ppxpFirst)
{
    LSG lsg{};
    if (pstep->psoPhys == nullptr || pstep->pposBase == nullptr) {
        return;
    }

    const int clsg = ClsgClipEdgeToObjectPruned(
        psoOther, pbspPruned, &pstep->xf.posWorld, pstep->pposBase, 1, &lsg);

    if (clsg <= 0) {
        return;
    }

    if (lsg.au[0] <= 0.0f) {
        return;
    }

    glm::vec3* ppos = pstep->pposBase;
    glm::vec3 dpos = *ppos - lsg.apos[0];

    if (lsg.lsgk == LSGK_EdgeToBsp)
    {
        float penetration = glm::dot(dpos, lsg.anormal[0]);
        PxpAdd(pstep, psoOther, ppos, lsg.anormal, penetration, XPK_PointToSurface, ppos, lsg.data.bsp.apsurf[0], ppxpFirst);

    }
    else if (lsg.lsgk == LSGK_EdgeToSphere)
    {
        lsg.anormal[0] = -lsg.anormal[0];

        float penetration = -glm::dot(dpos, lsg.anormal[0]);
        PxpAdd(psoOther, pstep, ppos, lsg.anormal, penetration, XPK_SphereToPoint, nullptr, ppos, ppxpFirst);

    }
}

void FixStepAngularVelocity(STEP* pstep)
{
    glm::vec3 w = pstep->xf.w;

    float radCur = atan2f(pstep->xf.mat[0][1], pstep->xf.mat[0][0]);

    RadSmooth(radCur, pstep->radTarget, 0.0f, &pstep->smpSpin, &w.z);
    SetAloAngularVelocityVec(pstep, &w);
}

void PredictStepPosition(STEP* pstep, float dtOffset, glm::vec3* ppos, glm::vec3* pv)
{
    PredictAloPosition(pstep, dtOffset, ppos, pv);
}

void PredictStepRotation(STEP* pstep, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    float radCurrent = atan2f(pstep->xf.mat[0][1], pstep->xf.mat[0][0]);

    float sw = 0.0f;
    float rad = RadSmooth(radCurrent, pstep->radTarget, dtOffset, &pstep->smpSpin, &sw);

    *pmat = glm::mat3(
        cosf(rad), sinf(rad), 0.0f,
        -sinf(rad), cosf(rad), 0.0f,
        0.0f, 0.0f, 1.0f
    );

    *pw = glm::vec3(0.0f, 0.0f, sw);
}

void SetStepSpinRatio(STEP* pstep, float r)
{
    pstep->smpSpin = s_smpSpinStepDefault;

    pstep->smpSpin.svFast *= r;
    pstep->smpSpin.svSlow *= r;
    pstep->smpSpin.dtFast *= 1.0f / sqrtf(r);
}

void SetStepGravityRatio(STEP* pstep, float rGravity)
{
    float rGravityNew = rGravity < 0.1f ? 0.1f : rGravity;
    float rGravityOld = pstep->rGravity;
    float scale = rGravityNew / rGravityOld;

    SetSoMass(pstep, (pstep->m * pstep->rGravity) / rGravityNew);

    pstep->rGravity = rGravityNew;

    pstep->dvGravity.x *= scale;
    pstep->dvGravity.y *= scale;
    pstep->dvGravity.z *= scale;
}

void DeleteStep(STEP* pstep)
{
	delete pstep;
}

SMP s_smpSpinStepDefault{25.0, 0.0, 0.25};
float MU_StepInternal = 1.5;
float g_mpixpdr[2] = {1.0, -1.0};
float G_StepBuoyancyDefault = 0.5;
CLQ s_clqDradToURun = {1.0, -1.0, 0.0};
SMP s_smpExpand = {3000.0, 0.0, 2.0};
SMP s_smpCompress = {3000.0, 0.0, 0.25};
SNIP s_asnipPostLoad[6] = 
{
    6, (OID)0x36, offsetof(STEP, paloRightHand),
    6, (OID)0x35, offsetof(STEP, paloLeftHand),
    6, (OID)0x37, offsetof(STEP, paloLeftBall),
    6, (OID)0x3A, offsetof(STEP, paloRightBall),
    6, (OID)0x38, offsetof(STEP, paloLeftHeel),
    6, (OID)0x3B, offsetof(STEP, paloRightHeel),
};
SMP s_smpPhys = {300.0, 0.0, 0.25};
float DT_StepFootfallInterval = 0.1;
