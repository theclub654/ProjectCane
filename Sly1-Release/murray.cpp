#include "murray.h"
#include "rwm.h"
#include "turret.h"
#include "aseg.h"

MURRAY* NewMurray()
{
	return new MURRAY{};
}

void InitMurray(MURRAY* pmurray)
{
	InitStepGuard(pmurray);
    pmurray->uFling = 0.0;
    pmurray->fAutoFadeDying = 0;
}

void PostMurrayLoad(MURRAY* pmurray)
{
    PostStepguardLoad(pmurray);
    SnipAloObjects(pmurray, 2, a_snipPostMurray);

    pmurray->pasegCelebrateKey = PasegFindStepguard(pmurray, (OID)239);
    pmurray->pasegDeath = PasegFindStepguard(pmurray, (OID)364);
    pmurray->pasegStun = PasegFindStepguard(pmurray, (OID)365);
    pmurray->pasegDeathAlt = PasegFindStepguard(pmurray, (OID)362);
    pmurray->pasegStunAlt = PasegFindStepguard(pmurray, (OID)363);
}

int GetMurraySize()
{
	return sizeof(MURRAY);
}

void CloneMurray(MURRAY* pmurray, MURRAY* pmurrayBase)
{
    CloneStepguard(pmurray, pmurrayBase);

    // Shallow copy fields
    pmurray->cpaloEnemy = pmurrayBase->cpaloEnemy;
    for (int i = 0; i < 4; ++i)
        pmurray->apaloEnemy[i] = pmurrayBase->apaloEnemy[i];
    pmurray->pxfmPatrol = pmurrayBase->pxfmPatrol;
    pmurray->pxfmPursue = pmurrayBase->pxfmPursue;
    pmurray->pasegCelebrateKey = pmurrayBase->pasegCelebrateKey;
    pmurray->pturret = pmurrayBase->pturret;
}

void OnMurrayEnteringSgs(MURRAY* pmurray, SGS sgsPrev, ASEG* pasegOverride)
{
    if (pmurray->sgs == SGS_Dying) {
        pasegOverride = pmurray->murrayMode == 4 ? pmurray->pasegDeathAlt : pmurray->pasegDeath;

        if (pmurray->psgg) {
            RemoveSggGuard(pmurray->psgg, pmurray);
        }
    }
    else if (pmurray->sgs == SGS_ReturnToPatrol) {
        pmurray->pxfmPursue = nullptr;
        pmurray->cpaloEnemy = 0;
    }
    else if (pmurray->sgs == SGS_Stun) {
        pasegOverride = pmurray->murrayMode == 4 ? pmurray->pasegStunAlt : pmurray->pasegStun;
    }

    OnStepguardEnteringSgs(pmurray, sgsPrev, pasegOverride);

    if (pmurray->sgs == SGS_Dying || pmurray->sgs == SGS_Stun) {
        pmurray->cStunHits = 0;
    }
}

void OnMurrayExitingSgs(MURRAY* pmurray, SGS sgs)
{
    OnStepguardExitingSgs(pmurray, sgs);

    if (pmurray->sgs == SGS_LostPlayer) 
    {
        if (pmurray->pckCollect == PCK_Gold)
            pmurray->pvtmurray->pfnSetPoPck(pmurray, PCK_Key);

        return;
    }

    if (pmurray->sgs == SGS_Stun && pmurray->pturret) {
        pmurray->pturret->pvtturret->pfnUsePoCharm(pmurray->pturret);
    }
}

void UpdateMurrayGoal(MURRAY* pmurray, int fEnter) 
{
    if (pmurray->sgs == SGS_Discover) {
        if (!fEnter) {
            return;
        }

        for (int i = 0; i < pmurray->cpaloEnemy; ++i) {
            ALO* paloEnemy = pmurray->apaloEnemy[i];

            if (FIsLoInWorld(paloEnemy)) {
                SetStepguardGoal(pmurray, &paloEnemy->xf.posWorld);
                return;
            }
        }

        return;
    }

    if (pmurray->sgs == SGS_ReturnToPatrol) {
        if (pmurray->pasegPatrol) {
            UpdateStepguardGoal(pmurray, fEnter);
            return;
        }

        if (!pmurray->pxfmPatrol) {
            SetStepguardGoal(pmurray, &pmurray->xf.posWorld);
            return;
        }

        glm::vec3 posXfm{};
        GetXfmPos(pmurray->pxfmPatrol, &posXfm);
        SetStepguardGoal(pmurray, &posXfm);
        return;
    }

    if (pmurray->sgs > SGS_Dying && pmurray->sgs < SGS_Attack) {
        if (!pmurray->pxfmPursue) {
            return;
        }

        glm::vec3 posXfm{};
        GetXfmPos(pmurray->pxfmPursue, &posXfm);
        SetStepguardGoal(pmurray, &posXfm);
        return;
    }

    UpdateStepguardGoal(pmurray, fEnter);
}

void UpdateMurraySgs(MURRAY* pmurray) 
{
    switch (pmurray->sgs) 
    {
        case SGS_PatrolIdle: 
        {
            if (pmurray->pasegaPatrol) {
                UpdateStepguardSgs(pmurray);
                return;
            }

            if (pmurray->pxfmPatrol) {
                glm::mat3 matPatrol{};
                GetXfmMat(pmurray->pxfmPatrol, &matPatrol);

                float radTarget = atan2f(matPatrol[0][1], matPatrol[0][0]);
                SetStepguardTargetHeading(pmurray, radTarget, 0);
                return;
            }

            MURRAY* ppoCur = (MURRAY*)PpoCur();
            if (!ppoCur || ppoCur == pmurray) {
                return;
            }

            glm::vec3 dpos = ppoCur->xf.posWorld - pmurray->xf.posWorld;
            float radTarget = atan2f(dpos.y, dpos.x);
            SetStepguardTargetHeading(pmurray, radTarget, 0);
            return;
        }

        case SGS_Discover:
        LookStepguardAtGoal(pmurray);
        return;

        case SGS_LostPlayer: 
        {
            if (pmurray->pasegaSgs->paseg != pmurray->pasegCelebrateKey) {
                return;
            }

            glm::vec3 dpos = g_pcm->pos - pmurray->xf.posWorld;
            float radTarget = atan2f(dpos.y, dpos.x);
            SetStepguardTargetHeading(pmurray, radTarget, 0);
            return;
        }

        case SGS_Dying:
        case SGS_Stun: 
        {
            if (!pmurray->fFaceStunProp) {
                return;
            }

            glm::vec3 dpos = pmurray->xf.posWorld - pmurray->pturret->xf.posWorld;
            float rad = atan2f(dpos.y, dpos.x);

            float radTarget = pmurray->sgs == SGS_Stun ? RadNormalize(rad + 4.0f) : RadNormalize(rad + 4.7f);
            SetStepguardTargetHeading(pmurray, radTarget, 0);
            return;
        }

        case SGS_PursueIdle: 
        {
            if (!pmurray->pxfmPursue) {
                return;
            }

            glm::mat3 matPursue{};
            GetXfmMat(pmurray->pxfmPursue, &matPursue);

            float radTarget = atan2f(matPursue[0][1], matPursue[0][0]);
            SetStepguardTargetHeading(pmurray, radTarget, 0);
            return;
        }

        default:
        UpdateStepguardSgs(pmurray);
        return;
    }
}

int FInvulnerableMurray(MURRAY* pmurray)
{
    if ((g_grfDebugCheats & 1) || FSwHandsOff(pmurray->psw))
        return true;

    return FAbsorbDamageStepguard(pmurray);
}

bool FTakeMurrayDamage(MURRAY* pmurray, ZPR* pzpr)
{
    if (pmurray->pvtmurray->pfnFInvulnerableMurray(pmurray))
        return false;

    pmurray->fFaceStunProp = false;
    pmurray->murrayMode = pzpr->zpk;

    if (pzpr->zpk == 0 && pzpr->ploSource && FIsRwmAmmo(pmurray->pturret->prwm, (SO*)pzpr->ploSource))
        pmurray->fFaceStunProp = true;

    return FTakeStepguardDamage((STEPGUARD*)pmurray, pzpr);
}

bool FAbsorbMurrayWkr(MURRAY* pmurray, WKR* pwkr) 
{
    if (!FCharmAvailable()) 
        pmurray->cStun = 0;
    else if (!pmurray->fFaceStunProp) 
        pmurray->cStun = 1;
    else 
        pmurray->cStun = 0;

    bool fAbsorbed = FAbsorbStepguardWkr(pmurray, pwkr);

    if (fAbsorbed) {
        MSGDMG msgdmg{};
        msgdmg.ploSource = pwkr->ploSource;
        msgdmg.ploDamaged = (LO*)pmurray;
        pmurray->pvtlo->pfnSendLoMessage(pmurray, MSGID_damaged, &msgdmg);
    }

    return fAbsorbed;
}

int FDetectMurray(MURRAY* pmurray)
{
    if (!pmurray->pxfmPursue)
        return false;

    for (int i = 0; i < pmurray->cpaloEnemy; ++i) 
    {
        ALO* paloEnemy = pmurray->apaloEnemy[i];
        STEPGUARD* pstepguardEnemy = (STEPGUARD*)paloEnemy;

        bool fDeadStepguard = FIsBasicDerivedFrom(pstepguardEnemy, CID_STEPGUARD) && pstepguardEnemy->sgs == SGS_Dying;

        if (!fDeadStepguard && FIsLoInWorld(paloEnemy)) 
            return true;
    }

    pmurray->pxfmPursue = nullptr;
    return false;
}

bool FCanMurrayAttack()
{
    return 0;
}

bool FAddMurrayDetectedEnemy(MURRAY* pmurray, STEPGUARD* pstepguard) 
{
    if (!pstepguard || !FIsLoInWorld(pstepguard)) 
        return false;

    if (FIsBasicDerivedFrom(pstepguard, CID_STEPGUARD) && pstepguard->sgs == SGS_Dying)
        return false;

    if (pmurray->cpaloEnemy >= 4)
        return false;

    pmurray->apaloEnemy[pmurray->cpaloEnemy++] = (ALO*)pstepguard;
    return true;
}

void* GetMurrayPursueXfm(MURRAY* pmurray)
{
    return pmurray->pxfmPursue;
}

void SetMurrayPursueXfm(MURRAY* pmurray, XFM* pxfm)
{
    pmurray->pxfmPursue = pxfm;
}

void*GetMurrayPatrolXfm(MURRAY* pmurray)
{
    return pmurray->pxfmPatrol;
}

void SetMurrayPatrolXfm(MURRAY* pmurray, XFM* pxfm)
{
    pmurray->pxfmPatrol = pxfm;
}

void CollectMurrayPrize(MURRAY* pmurray, PCK pck, ALO *paloOther)
{
    CollectPoPrize(pmurray, pck, paloOther);

    if (pck == PCK_Key) 
        SetStepguardSgs(pmurray, SGS_LostPlayer, pmurray->pasegCelebrateKey);
}

int JthsCurrentMurray(MURRAY* pmurray)
{
    if (pmurray->sgs == SGS_Dying) {
        return 2;
    }

    if (pmurray->sgs == SGS_Stun) {
        return 1;
    }

    return 0;
}

void HandleMurrayMessage(MURRAY* pmurray, MSGID msgid, void* pv)
{
    HandleStepguardMessage(pmurray, msgid, pv);

    if (msgid != MSGID_label || pv == nullptr)
        return;

    ASEGLABELMSG* pmsg = static_cast<ASEGLABELMSG*>(pv);

    if (pmsg->pasega == pmurray->pasegaSgs && pmsg->label == 1338)
        HandlePlayerDeath();
}

void DeleteMurray(MURRAY* pmurray)
{
	delete pmurray;
}

SNIP a_snipPostMurray[2] =
{
    { 0x20, (OID)0x167, offsetof(PO, paloCollectTarget[0]) },
    { 0x20, (OID)0x168, offsetof(PO, paloCollectTarget[1]) },
};
