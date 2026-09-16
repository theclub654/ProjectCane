#include "mgv.h"
#include "aseg.h"
#include "asega.h"
#include "clock.h"
#include "difficulty.h"
#include "ctr.h"
#include "expl.h"
#include "find.h"
#include "game.h"
#include "input.h"
#include "pnt.h"
#include "sm.h"
#include "stepzap.h"
#include "sw.h"
#include "stepzap.h"

#include <cstddef>
#include <cstring>
#include <algorithm>
#include <cfloat>

namespace
{
    int s_cSecurityCodesTotal = 13;
    SMP s_smpMgvSpin = { 20.0f, 0.0f, 0.3f };
    SMP s_smpMgvAim = { 40.0f, 0.0f, 0.3f };

    constexpr float SV_MgvMax = 1200.0f;
    constexpr float U_MgvMoveMin = 0.25f;
    constexpr float U_MgvAimMin = 0.1f;
    constexpr float SDV_MgvMax = 3000.0f;
    constexpr float S_MgvDamage = 150.0f;
    constexpr float RT_MgvDamage = 0.2f;

    void ApplyMgvThrowVelocity(void* pvSource, PO* ppo)
    {
        ALO* paloSource = static_cast<ALO*>(static_cast<LO*>(pvSource));
        MGV* pmgv = static_cast<MGV*>(ppo);
        glm::vec3 velocity = pmgv->xf.posWorld - paloSource->xf.posWorld;
        velocity.z = 0.0f;

        const float length = glm::length(velocity);
        if (length < 0.0001f)
            velocity = g_normalX;
        else
            velocity /= length;

        velocity *= 1000.0f;
        pmgv->pvtalo->pfnSetAloVelocityVec(pmgv, &velocity);
    }
}

CTR g_securitycodectr{};
CTR g_mgvhealthctr{};

void StartupSecurityCodeCtr(CTR* pctr)
{
    pctr->pvtctr = &g_vtctr;
}

void StartupMgvHealthCtr(CTR* pctr)
{
    pctr->pvtctr = &g_vtctr;
}

SNIP s_asnipMgv[12] =
{
    { 0x04, (OID)0x525, offsetof(MGV, psoAimTarget) },
    { 0x04, (OID)0x528, offsetof(MGV, pblipgController) },
    { 0x02, (OID)0x162, offsetof(MGV, pzi.mpccharmpaloCharm[2]) },
    { 0x02, (OID)0x163, offsetof(MGV, pzi.mpccharmpaloCharm[3]) },
    { 0x06, (OID)0x160, offsetof(MGV, pzi.mpccharmpaloCharm[0]) },
    { 0x00, (OID)0x527, offsetof(MGV, psmAim) },
    { 0x00, (OID)0x529, offsetof(MGV, pasegInactive) },
    { 0x00, (OID)0x52a, offsetof(MGV, pasegActive) },
    { 0x00, (OID)0x52b, offsetof(MGV, pasegCharm) },
    { 0x00, (OID)0x52c, offsetof(MGV, pasegDead) },
    { 0x00, (OID)0x52d, offsetof(MGV, pasegImpact) },
    { 0x00, (OID)0x52e, offsetof(MGV, pasegDamage) }
};

MGV* NewMgv()
{
	return new MGV{};
}

void InitMgv(MGV* pmgv)
{
	InitPo(pmgv);

	pmgv->clives = 5;
	pmgv->mgvs = MGVS_Nil;
	pmgv->cpntRespawn = 0;
	pmgv->cploSpawned = 0;
}

int GetMgvSize()
{
	return sizeof(MGV);
}

void LoadMgvFromBrx(MGV* pmgv, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pmgv, pbis);
}

void CloneMgv(MGV* pmgv, MGV* pmgvBase)
{
    ClonePo(pmgv, pmgvBase);
}

void PostMgvLoad(MGV* pmgv)
{
    PostAloLoad(pmgv);
    pmgv->sRadiusSelfCached = pmgv->sRadiusSelf;
    SnipAloObjects(pmgv, 12, s_asnipMgv);
    pmgv->psmaAim = PsmaApplySm(pmgv->psmAim, pmgv, OID_Nil, 0);
    pmgv->radMove = atan2f(pmgv->xf.mat[0].y, pmgv->xf.mat[0].x);
    if (pmgv->psoAimTarget)
        pmgv->radAim = atan2f(pmgv->psoAimTarget->xf.mat[0].y, pmgv->psoAimTarget->xf.mat[0].x);
    if (pmgv->pblipgController)
    {
        pmgv->gAimControllerX = pmgv->pblipgController->clqScale.g2;
        pmgv->gAimControllerY = pmgv->pblipgController->clqScale.gUnused;
    }
    g_securitycodectr.pnActual = &pmgv->cSecurityCodes;
    g_securitycodectr.pnTotal = &s_cSecurityCodesTotal;
    g_mgvhealthctr.pnActual = &pmgv->clives;
    g_mgvhealthctr.dgDisplayMax = 4.0f;
    SetSwGameplayHud(pmgv->psw, pmgv);
    if (pmgv->cploSpawned < 16)
        pmgv->aploSpawned[pmgv->cploSpawned++] = pmgv;
    SetMgvMgvs(pmgv, MGVS_Active);
}

void UpdateMgv(MGV* pmgv, float dt)
{
    UpdatePo(pmgv, dt);

    if (pmgv->ploSpecialContact)
    {
        EXPL* pexpl = reinterpret_cast<EXPL*>(
            PloFindSwObjectByClass(pmgv->psw, (GRFFSO)257, CID_EXPL,
                pmgv->ploSpecialContact));
        if (pexpl)
            ExplodeExpl(pexpl);

        ++pmgv->cSecurityCodes;
        pmgv->ploSpecialContact->pvtlo->pfnRemoveLo(pmgv->ploSpecialContact);
        pmgv->ploSpecialContact = nullptr;
    }

    if (pmgv->psoDeathSource)
    {
        ApplyMgvDeathThrow(pmgv, pmgv->psoDeathSource);
        pmgv->psoDeathSource = nullptr;
    }

    if (!pmgv->pasegaDamage)
    {
        if (pmgv->fDamagePending)
        {
            ApplyAseg(pmgv->pasegDamage, pmgv, 0.0f, 1.0f, 0,
                &pmgv->pasegaDamage);
            --pmgv->clives;
            g_mgvhealthctr.nDisplay = -1;
        }
    }
    else if (pmgv->pasegaDamage->svtLocal == 0.0f)
    {
        RetractAsega(pmgv->pasegaDamage);
        pmgv->pasegaDamage = nullptr;
    }
    pmgv->fDamagePending = 0;

    float uDamage = 0.0f;
    if (pmgv->pasegaDamage)
    {
        uDamage = pmgv->pasegaDamage->tLocal /
            pmgv->pasegaDamage->paseg->tMax;
        g_rtClockPowerUp = RT_MgvDamage;
    }
    else
    {
        g_rtClockPowerUp = 1.0f;
    }

    const float sRadiusDamage =
        uDamage * S_MgvDamage + (1.0f - uDamage) * pmgv->sRadiusSelfCached;
    pmgv->sRadiusSelf = sRadiusDamage;
    pmgv->sRadiusAll = sRadiusDamage;

    if (pmgv->psoAimTarget)
    {
        const float radBody = atan2f(pmgv->xf.mat[0].y, pmgv->xf.mat[0].x);
        glm::mat3 matAimTarget;
        glm::mat3 matAimSmooth;
        LoadRotateMatrixRad(pmgv->radAim - radBody, &g_normalZ, &matAimTarget);
        SmoothMatrix(&pmgv->psoAimTarget->xf.mat, &matAimTarget, &s_smpMgvAim, dt, &matAimSmooth, nullptr);
        pmgv->psoAimTarget->pvtalo->pfnRotateAloToMat(
            pmgv->psoAimTarget, &matAimSmooth);
    }

    if (pmgv->fCharmUsed && g_clock.t - pmgv->tCharmState > 2.0f)
        pmgv->fCharmUsed = 0;

    glm::vec4 rgbaCel(0.0f, 1.0f, 16.0f / 255.0f, 128.0f / 255.0f);
    if (pmgv->fCharmUsed)
    {
        const float uState = pmgv->mgvs == MGVS_Charm
            ? 0.0f
            : (g_clock.t - pmgv->tMgvs) * 0.5f;
        const float wave = std::cos((g_clock.t - pmgv->tCharmUsed) * 10.0f);

        const float uStateCel = glm::clamp(1.0f - 0.75f * uState * uState, 0.0f, 1.0f);
        const float uFlashCel = glm::clamp(0.7f + 0.3f * wave, 0.0f, 1.0f);
        const float blend = uStateCel * uFlashCel;

        const glm::vec4 rgbaFlash(1.0f, 0.0f, 0.0f, rgbaCel.a);
        rgbaCel = glm::mix(rgbaCel, rgbaFlash, blend);
    }

    SetAloOverrideCel(pmgv, &rgbaCel);

    if (pmgv->pblipgController) {
        pmgv->pblipgController->clqScale.g2 = pmgv->uAim * pmgv->gAimControllerX;
        pmgv->pblipgController->clqScale.gUnused = pmgv->uAim * pmgv->gAimControllerY;
        pmgv->pblipgController->field_0x32c = 0;
    }
}

void UpdateMgvBlot(MGV*)
{
    if (FCanShowGameplayBlot())
    {
        ShowBlot(&g_securitycodectr);
        ShowBlot(&g_mgvhealthctr);
    }
    else
    {
        HideBlot(&g_securitycodectr);
        HideBlot(&g_mgvhealthctr);
    }
}

void UpdateMgvActive(MGV* pmgv, JOY* pjoy, float)
{
    // The retail MGV advances out of its one-shot charm/death state when the
    // current animation reaches zero playback speed.  In particular, Dead
    // must become TryAgain here; SetMgvMgvs then installs the prompt.
    if (pmgv->pasegaCur != nullptr && pmgv->pasegaCur->svtLocal == 0.0f)
    {
        if (pmgv->mgvs == MGVS_Charm)
            UseMgvCharm(pmgv);
        else if (pmgv->mgvs == MGVS_Dead)
            SetMgvMgvs(pmgv, MGVS_TryAgain);
    }

    if (pmgv->mgvs == MGVS_Active)
    {
        if (pjoy->uDeflect > 0.0f)
            AdjustCmJoy(g_pcm, pjoy, JOYID_Left, &pmgv->radMove);

        pmgv->uMove = std::max(pjoy->uDeflect, U_MgvMoveMin);
        SetVectorCylind(&pmgv->vMove, pmgv->radMove, SV_MgvMax * pjoy->uDeflect, 0.0f);

        pmgv->uAim = std::max(pjoy->uDeflect2, U_MgvAimMin);
        if (pjoy->uDeflect2 > 0.0f)
        {
            AdjustCmJoy(g_pcm, pjoy, JOYID_Right, &pmgv->radAim);
            if (pmgv->psmaAim)
                SetSmaGoal(pmgv->psmaAim, (OID)1157);
        }
        else if (pmgv->psmaAim)
        {
            SetSmaGoal(pmgv->psmaAim, (OID)1156);
        }
    }
    else
    {
        pmgv->uMove = U_MgvMoveMin;
        pmgv->vMove = glm::vec3(0.0f);
    }

    HandleDialogButtons(pjoy);
    ResolveAlo(pmgv);
}

void PresetMgvAccel(MGV* pmgv, float dt)
{
    PresetSoAccel(pmgv, dt);

    glm::mat3 matTarget;
    LoadRotateMatrixPanTilt(pmgv->radMove, 0.0f, &matTarget);
    AccelSoTowardMatSmooth(pmgv, dt, &matTarget, &s_smpMgvSpin);

    if (dt <= 0.0f)
        return;

    glm::vec3 accel = pmgv->vMove - pmgv->xf.v;
    LimitVectorLength(&accel, SDV_MgvMax * pmgv->uMove * dt, &accel);
    accel /= dt;
    AddSoAcceleration(pmgv, &accel);
}

void AdjustMgvNewXp(MGV* pmgv, XP* pxp, int ixpd)
{
    LO* other = pxp->axpd[1 - ixpd].psoRoot;
    if (!other) return;
    if (FIsBasicDerivedFrom(other, CID_BREAK)) {
        if (pmgv->clives < 1) { if (!pmgv->pasegaDamage) pmgv->psoDeathSource = (SO*)other; }
        else pmgv->fDamagePending = 1;
    } else if (FMatchesLoName(other, (OID)1327)) pmgv->ploSpecialContact = other;
}

void AdjustMgvXpVelocity(MGV* pmgv, XP* pxp, int ixpd)
{
    if (!pmgv->pasegaDamage)
        return;

    XPD& xpd = pxp->axpd[ixpd];
    glm::vec3 direction = xpd.pos - pmgv->xf.posWorld;
    direction /= glm::length(direction);

    const float sv = (S_MgvDamage - pmgv->sRadiusSelfCached) /
        pmgv->pasegaDamage->paseg->tMax;
    xpd.v += direction * sv;
}
int FIsMgvInvulnerable(MGV* pmgv, ZPK) { return FInvulnerablePo(pmgv); }

int FTakeMgvDamage(MGV* pmgv, ZPR* pzpr)
{
    if (FIsMgvInvulnerable(pmgv, pzpr->zpk))
        return 0;

    if (pzpr->zpk == ZPK_Blunt && pzpr->pfnzap)
        pzpr->pfnzap(pzpr->pv, pmgv);

    if ((pmgv->mgvs > MGVS_Active && pmgv->mgvs < MGVS_Max) ||
        pmgv->fCharmUsed)
        return 1;

    if (FCharmAvailable())
    {
        SetMgvMgvs(pmgv, MGVS_Charm);
        pmgv->fCharmUsed = 1;
        pmgv->tCharmUsed = g_clock.t;
    }
    else
    {
        SetMgvMgvs(pmgv, MGVS_Dead);
    }

    MSGDMG msgdmg = { pmgv, pzpr->ploSource };
    pmgv->pvtlo->pfnSendLoMessage(pmgv, MSGID_damaged, &msgdmg);
    return 1;
}

bool FIsMgvTakingDamage(MGV* pmgv)
{
    return pmgv->mgvs > MGVS_Active || pmgv->fCharmUsed != 0;
}

void UseMgvCharm(MGV* pmgv) { UsePoCharm(pmgv); SetMgvMgvs(pmgv, MGVS_Active); }

void SetMgvMgvs(MGV* pmgv, MGVS mgvs)
{
    if (pmgv->mgvs == mgvs) return;
    if (pmgv->mgvs == MGVS_Charm) { pmgv->clives = 3; pmgv->tCharmState = g_clock.t; }
    pmgv->uAim = 0.0f;

    if (mgvs > MGVS_Active && mgvs < MGVS_TryAgain)
    {
        const float rad = RadNormalize(
            atan2f(pmgv->xf.v.y, pmgv->xf.v.x) + 3.1415927f);
        pmgv->radAim = rad;
        pmgv->radMove = rad;

        glm::vec3 angularVelocity(0.0f);
        const float radBody = atan2f(pmgv->xf.mat[0].y, pmgv->xf.mat[0].x);
        RadSmooth(radBody, pmgv->radMove, 0.0f, &s_smpMgvSpin,
            &angularVelocity.z);
        pmgv->pvtalo->pfnSetAloAngularVelocityVec(pmgv, &angularVelocity);

        pmgv->clives = 0;
        g_mgvhealthctr.nDisplay = -1;

        if (mgvs == MGVS_Dead)
            OnDifficultyPlayerDeath(&g_difficulty, 1.0f);
    }

    ASEG* paseg = nullptr; float t = 0.0f;
    switch (mgvs) {
    case MGVS_Inactive: paseg = pmgv->pasegInactive; break;
    case MGVS_Active: paseg = pmgv->pasegActive; break;
    case MGVS_Charm: paseg = pmgv->pasegCharm; pmgv->tCharmState = 0.0f; break;
    case MGVS_Dead: paseg = pmgv->pasegDead; break;
    case MGVS_TryAgain:
        SetPrompt(&g_prompt, 0, PRK_TryAgain);
        paseg = pmgv->pasegDead;
        if (paseg) t = paseg->tMax;
        break;
    default: break;
    }
    if (paseg) ApplyAsegCur(paseg, pmgv, t, 1.0f, (GRFAPL)1, &pmgv->pasegaCur);
    pmgv->mgvs = mgvs; pmgv->tMgvs = g_clock.t;
}

void ApplyMgvDeathThrow(MGV* pmgv, LO* ploSource)
{
    ZPR zpr;
    InitZpr(&zpr, ZPK_Blunt, ploSource);
    zpr.pfnzap = ApplyMgvThrowVelocity;
    zpr.pv = ploSource;
    FTakeMgvDamage(pmgv, &zpr);
}

void AddMgvRespawnPoint(MGV* pmgv, PNT* ppnt)
{
    if (pmgv->cpntRespawn < 32)
        pmgv->appntRespawn[pmgv->cpntRespawn++] = ppnt;
}

void RespawnMgvObject(MGV* pmgv, LO* plo)
{
    int iplo = 0;
    while (iplo < pmgv->cploSpawned && pmgv->aploSpawned[iplo] != plo)
        ++iplo;

    if (!FIsLoInWorld(plo))
    {
        glm::vec3 posBest(0.0f);
        float distanceBest = 0.0f;

        for (int ipnt = 0; ipnt < pmgv->cpntRespawn; ++ipnt)
        {
            glm::vec3 posCandidate;
            GetPntPos(pmgv->appntRespawn[ipnt], &posCandidate);

            float distanceNearest = FLT_MAX;
            for (int i = 0; i < pmgv->cploSpawned; ++i)
            {
                LO* ploOther = pmgv->aploSpawned[i];
                if (ploOther && FIsLoInWorld(ploOther))
                {
                    const ALO* paloOther = static_cast<const ALO*>(ploOther);
                    const glm::vec2 delta(
                        posCandidate.x - paloOther->xf.posWorld.x,
                        posCandidate.y - paloOther->xf.posWorld.y);
                    distanceNearest = std::min(distanceNearest, glm::length(delta));
                }
            }

            if (distanceNearest > distanceBest)
            {
                distanceBest = distanceNearest;
                posBest = posCandidate;
            }
        }

        ALO* palo = static_cast<ALO*>(plo);
        FadeAloIn(palo, 1.0f);
        posBest.z = palo->xf.posWorld.z;
        palo->pvtalo->pfnTranslateAloToPos(palo, &posBest);


        if (FIsBasicDerivedFrom(plo, CID_BREAK))
        {
            glm::vec3 velocity;
            SetVectorCylind(&velocity,
                GRandInRange(-3.1415927f, 3.1415927f),
                GRandInRange(0.0f, 300.0f), 0.0f);
            palo->pvtalo->pfnSetAloVelocityVec(palo, &velocity);

            glm::vec3 angularVelocity(0.0f, 0.0f, GRandInRange(-1.0f, 1.0f));
            palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &angularVelocity);
        }
    }

    if (iplo < 16)
    {
        pmgv->aploSpawned[iplo] = plo;
        pmgv->cploSpawned = std::max(pmgv->cploSpawned, iplo + 1);
    }

}

void DeleteMgv(MGV *pmgv)
{
	delete pmgv;
}
