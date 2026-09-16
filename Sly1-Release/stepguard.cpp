#include "stepguard.h"
#include "jt.h"
#include "hide.h"
#include "rwm.h"
#include "waypoint.h"
#include "chkpnt.h"
#include "emitter.h"
#include "acp.h"
#include "path.h"
#include "actseg.h"
#include "expl.h"
#include "bbmark.h"
#include "alarm.h"
#include "target.h"
#include "decoy.h"
#include "ub.h"
#include "gomer.h"

STEPGUARD* NewStepguard()
{
	return new STEPGUARD{};
}

void InitStepGuard(STEPGUARD* pstepguard)
{
    InitStep(pstepguard);

    pstepguard->egk = EGK_No;

    pstepguard->oidPathzone = OID_Nil;

    pstepguard->lmAttackS.gMin = 0.0f;
    pstepguard->lmAttackS.gMax = 300.0f;

    pstepguard->lmAttackT.gMin = 0.2f;
    pstepguard->lmAttackT.gMax = 0.5f;

    pstepguard->radAttackMax = 0.17453294f;
    pstepguard->sgs = SGS_Nil;

    for (int i = 0; i < SGS_Max; ++i)
    {
        pstepguard->mpsgssga[i].oidAseg = OID_Nil;
        pstepguard->mpsgssgp[i].oidPhys = OID_Nil;
    }

    pstepguard->ichkDead = IchkAllocChkmgr(&g_chkmgr);
    pstepguard->fFirstFrame = 1;
    pstepguard->oidEnemyExternal = OID_Nil;
    pstepguard->ccoin = -1;

    pstepguard->clqSToUPursue.g0 = 1.0f;

    SetPoPlayable(reinterpret_cast<PO*>(pstepguard), 0);

    pstepguard->rGravity = 1.5f;
    pstepguard->uIdleStart = -1.0f;

    pstepguard->lmDtFreefall.gMin = 1.0f;
    pstepguard->lmDtFreefall.gMax = 2.5f;

    pstepguard->tLostPlayer = FLT_MAX;

    pstepguard->ficg.grficBomb = ' ';
    pstepguard->pstepguardBase = pstepguard;

    pstepguard->tTaunt = FLT_MAX;

    pstepguard->lmDtTaunt.gMin = 0.0f;
    pstepguard->lmDtTaunt.gMax = 0.8f;

    pstepguard->fFaceDamageDirection = 1;

    pstepguard->ovrSgs.oid = OID_Nil;

    pstepguard->uFling = 1.0f;
    pstepguard->fAutoFadeDying = 1;
    pstepguard->muContact = 1.25f;
}

void LoadStepGuardFromBrx(STEPGUARD* pstepguard, CBinaryInputStream* pbis)
{
    LoadSoFromBrx(pstepguard, pbis);
    SnipAloObjects(pstepguard, 1, s_asnipStepguardLoad);
    LoadStepguardAnimations(pstepguard);
    LoadStepguardPhys(pstepguard);
}

void* GetStepguardLmAttackS(STEPGUARD* pstepguard)
{
    return &pstepguard->lmAttackS;
}

void SetStepguardLmAttackS(STEPGUARD* pstepguard, LM lmAttackS)
{
    pstepguard->lmAttackS = lmAttackS;
}

void* GetStepguardLmAttackT(STEPGUARD* pstepguard)
{
    return &pstepguard->lmAttackT;
}

void SetStepguardLmAttackT(STEPGUARD* pstepguard, LM lmAttackT)
{
    pstepguard->lmAttackT = lmAttackT;
}

void* GetStepguardRadAttackMax(STEPGUARD* pstepguard)
{
    return &pstepguard->radAttackMax;
}

void SetStepguardRadAttackMax(STEPGUARD* pstepguard, float radAttackMax)
{
    pstepguard->radAttackMax = radAttackMax;
}

void* GetStepguardDtFade(STEPGUARD* pstepguard)
{
    return &pstepguard->dtFade;
}

void SetStepguardDtFade(STEPGUARD* pstepguard, float dtFade)
{
    pstepguard->dtFade = dtFade;
}

void* GetStepguardCStun(STEPGUARD* pstepguard)
{
    return &pstepguard->cStun;
}

void SetStepguardCStun(STEPGUARD* pstepguard, int cStun)
{
    pstepguard->cStun = cStun;
}

void* GetStepguardFTumbleStun(STEPGUARD* pstepguard)
{
    return &pstepguard->fTumbleStun;
}

void SetStepguardFTumbleStun(STEPGUARD* pstepguard, int fTumbleStun)
{
    pstepguard->fTumbleStun = fTumbleStun;
}

void* GetStepguardLmDtFreefall(STEPGUARD* pstepguard)
{
    return &pstepguard->lmDtFreefall;
}

void SetStepguardLmDtFreefall(STEPGUARD* pstepguard, LM lmDtFreefall)
{
    pstepguard->lmDtFreefall = lmDtFreefall;
}

void* GetStepguardFLockLook(STEPGUARD* pstepguard)
{
    return &pstepguard->fLockLook;
}

void SetStepguardFLockLook(STEPGUARD* pstepguard, int fLockLook)
{
    pstepguard->fLockLook = fLockLook;
}

void* GetStepguardOidEnemyExternal(STEPGUARD* pstepguard)
{
    return &pstepguard->oidEnemyExternal;
}

void SetStepguardOidEnemyExternal(STEPGUARD* pstepguard, int oidEnemyExternal)
{
    pstepguard->oidEnemyExternal = (OID)oidEnemyExternal;
}

void* GetStepguardFRebindEnemy(STEPGUARD* pstepguard)
{
    return &pstepguard->fRebindEnemy;
}

void SetStepguardFRebindEnemy(STEPGUARD* pstepguard, int fRebindEnemy)
{
    pstepguard->fRebindEnemy = fRebindEnemy;
}

void* GetStepguardFFlying(STEPGUARD* pstepguard)
{
    return &pstepguard->fFlying;
}

void SetStepguardFFlying(STEPGUARD* pstepguard, int fFlying)
{
    pstepguard->fFlying = fFlying;
}

void* GetStepguardDzFlying(STEPGUARD* pstepguard)
{
    return &pstepguard->dzFlying;
}

void SetStepguardDzFlying(STEPGUARD* pstepguard, float dzFlying)
{
    pstepguard->dzFlying = dzFlying;
}

void* GetStepguardClqSToUPursue(STEPGUARD* pstepguard)
{
    return &pstepguard->clqSToUPursue;
}

void SetStepguardClqSToUPursue(STEPGUARD* pstepguard, CLQ clqSToUPursue)
{
    pstepguard->clqSToUPursue = clqSToUPursue;
}

void* GetStepguardRpk(STEPGUARD* pstepguard)
{
    return &pstepguard->rpk;
}

void SetStepguardRpk(STEPGUARD* pstepguard, RPK rpk)
{
    pstepguard->rpk = rpk;
}

void* GetStepguardCcoin(STEPGUARD* pstepguard)
{
    return &pstepguard->ccoin;
}

void SetStepguardCcoin(STEPGUARD* pstepguard, int ccoin)
{
    pstepguard->ccoin = ccoin;
}

void* GetStepguardPosBreakLocal(STEPGUARD* pstepguard)
{
    return &pstepguard->posBreakLocal;
}

void SetStepguardPosBreakLocal(STEPGUARD* pstepguard, glm::vec3 posBreakLocal)
{
    pstepguard->posBreakLocal = posBreakLocal;
}

void* GetStepguardFNoDifficultyUpdate(STEPGUARD* pstepguard)
{
    return &pstepguard->fNoDifficultyUpdate;
}

void SetStepguardFNoDifficultyUpdate(STEPGUARD* pstepguard, int fNoDifficultyUpdate)
{
    pstepguard->fNoDifficultyUpdate = fNoDifficultyUpdate;
}

void SetStepguardUIdleStart(STEPGUARD* pstepguard, float uIdleStart)
{
    pstepguard->uIdleStart = uIdleStart;
}

void* GetStepguardUFling(STEPGUARD* pstepguard)
{
    return &pstepguard->uFling;
}

void SetStepguardUFling(STEPGUARD* pstepguard, float uFling)
{
    pstepguard->uFling = uFling;
}

void* GetStepguardFAutoFadeDying(STEPGUARD* pstepguard)
{
    return &pstepguard->fAutoFadeDying;
}

void SetStepguardFAutoFadeDying(STEPGUARD* pstepguard, int fAutoFadeDying)
{
    pstepguard->fAutoFadeDying = fAutoFadeDying;
}

void* GetStepguardSgs(STEPGUARD* pstepguard)
{
    return &pstepguard->sgs;
}

void* GetStepguardFBypassEffectHitSuppression(STEPGUARD* pstepguard)
{
    return &pstepguard->fBypassEffectHitSuppression;
}

void SetStepguardFBypassEffectHitSuppression(STEPGUARD* pstepguard, int fBypassEffectHitSuppression)
{
    pstepguard->fBypassEffectHitSuppression = fBypassEffectHitSuppression;
}

void* GetStepguardLmDtTaunt(STEPGUARD* pstepguard)
{
    return &pstepguard->lmDtTaunt;
}

void SetStepguardLmDtTaunt(STEPGUARD* pstepguard, LM lmDtTaunt)
{
    pstepguard->lmDtTaunt = lmDtTaunt;
}

void* GetStepguardFFaceDamageDirection(STEPGUARD* pstepguard)
{
    return &pstepguard->fFaceDamageDirection;
}

void SetStepguardFFaceDamageDirection(STEPGUARD* pstepguard, int fFaceDamageDirection)
{
    pstepguard->fFaceDamageDirection = fFaceDamageDirection;
}

void* GetStepguardMuContact(STEPGUARD* pstepguard)
{
    return &pstepguard->muContact;
}

void SetStepguardMuContact(STEPGUARD* pstepguard, float muContact)
{
    pstepguard->muContact = muContact;
}

void* GetStepguardFAbsorbDamage(STEPGUARD* pstepguard)
{
    return &pstepguard->fAbsorbDamage;
}

void SetStepguardFAbsorbDamage(STEPGUARD* pstepguard, int fAbsorbDamage)
{
    pstepguard->fAbsorbDamage = fAbsorbDamage;
}

void* GetStepguardOvrOid(STEPGUARD* pstepguard)
{
    return &pstepguard->ovrSgs.oid;
}

void SetStepguardOvrOid(STEPGUARD* pstepguard, int oid)
{
    pstepguard->ovrSgs.oid = (OID)oid;
}

void* GetStepguardFAlignToGround(STEPGUARD* pstepguard)
{
    return &pstepguard->fAlignToGround;
}

void SetStepguardFAlignToGround(STEPGUARD* pstepguard, int fAlignToGround)
{
    pstepguard->fAlignToGround = fAlignToGround;
}

void CloneStepguard(STEPGUARD* pstepguard, STEPGUARD* pstepguardBase)
{
    SGG* psgg = pstepguard->psgg;
    int ichkDead = pstepguard->ichkDead;

    ClonePo(pstepguard, pstepguardBase);

    pstepguard->radJoy = pstepguardBase->radJoy;
    pstepguard->uJoyDeflect = pstepguardBase->uJoyDeflect;
    pstepguard->radTarget = pstepguardBase->radTarget;
    pstepguard->uTarget = pstepguardBase->uTarget;
    pstepguard->zBaseTarget = pstepguardBase->zBaseTarget;
    pstepguard->vTarget = pstepguardBase->vTarget;
    pstepguard->matTarget = pstepguardBase->matTarget;
    pstepguard->fBaseXp = pstepguardBase->fBaseXp;
    pstepguard->tBaseXp = pstepguardBase->tBaseXp;
    pstepguard->uFootfall = pstepguardBase->uFootfall;
    pstepguard->tLastFootfall = pstepguardBase->tLastFootfall;
    pstepguard->dzBase = pstepguardBase->dzBase;
    pstepguard->iposBase = pstepguardBase->iposBase;
    pstepguard->posBaseXp = pstepguardBase->posBaseXp;
    pstepguard->normalBaseXp = pstepguardBase->normalBaseXp;
    pstepguard->psoBaseXp = pstepguardBase->psoBaseXp;
    pstepguard->psoPhys = pstepguardBase->psoPhys;
    pstepguard->smpSpin = pstepguardBase->smpSpin;
    pstepguard->smpCompress = pstepguardBase->smpCompress;
    pstepguard->smpExpand = pstepguardBase->smpExpand;
    pstepguard->clqDradToURun = pstepguardBase->clqDradToURun;
    pstepguard->rGravity = pstepguardBase->rGravity;

    // These two pointers address vertices inside geomWorld.apos. CloneSo owns
    // a separate vector in this port, so translate the source vertex indices.
    auto RebaseStepWorldPos = [pstepguard, pstepguardBase](const glm::vec3* ppos) -> glm::vec3*
    {
        if (ppos == nullptr || pstepguardBase->geomWorld.apos.empty())
            return nullptr;

        const std::ptrdiff_t ipos = ppos - pstepguardBase->geomWorld.apos.data();
        if (ipos < 0 || ipos >= static_cast<std::ptrdiff_t>(pstepguard->geomWorld.apos.size()))
            return nullptr;

        return &pstepguard->geomWorld.apos[ipos];
    };

    pstepguard->pposBase = RebaseStepWorldPos(pstepguardBase->pposBase);
    pstepguard->pposBumper = RebaseStepWorldPos(pstepguardBase->pposBumper);
    pstepguard->paloLeftFoot = pstepguardBase->paloLeftFoot;
    pstepguard->paloRightFoot = pstepguardBase->paloRightFoot;
    pstepguard->paloLeftHand = pstepguardBase->paloLeftHand;
    pstepguard->paloRightHand = pstepguardBase->paloRightHand;
    pstepguard->paloLeftHeel = pstepguardBase->paloLeftHeel;
    pstepguard->paloRightHeel = pstepguardBase->paloRightHeel;
    pstepguard->paloLeftBall = pstepguardBase->paloLeftBall;
    pstepguard->paloRightBall = pstepguardBase->paloRightBall;

    pstepguard->sgs = pstepguardBase->sgs;
    pstepguard->tSgs = pstepguardBase->tSgs;
    pstepguard->grfsgsc = pstepguardBase->grfsgsc;
    pstepguard->sgsNext = pstepguardBase->sgsNext;
    pstepguard->paloHead = pstepguardBase->paloHead;
    pstepguard->lmAttackS = pstepguardBase->lmAttackS;
    pstepguard->lmAttackT = pstepguardBase->lmAttackT;
    pstepguard->tAttackNext = pstepguardBase->tAttackNext;
    pstepguard->radAttackMax = pstepguardBase->radAttackMax;
    pstepguard->pasegPatrol = pstepguardBase->pasegPatrol;
    pstepguard->pasegaPatrol = pstepguardBase->pasegaPatrol;

    for (int i = 0; i < 17; ++i)
    {
        pstepguard->mpsgssga[i] = pstepguardBase->mpsgssga[i];
        pstepguard->mpsgssgp[i] = pstepguardBase->mpsgssgp[i];
    }

    pstepguard->pasegaSgs = pstepguardBase->pasegaSgs;
    pstepguard->csgmap = pstepguardBase->csgmap;

    for (int i = 0; i < 8; ++i)
        pstepguard->asgmap[i] = pstepguardBase->asgmap[i];

    pstepguard->pasegPrev = pstepguardBase->pasegPrev;
    pstepguard->uAsegaPrev = pstepguardBase->uAsegaPrev;
    pstepguard->psoPhysDefault = pstepguardBase->psoPhysDefault;
    pstepguard->posSearch = pstepguardBase->posSearch;
    pstepguard->radTargetSearch = pstepguardBase->radTargetSearch;
    pstepguard->tPatrolReturn = pstepguardBase->tPatrolReturn;
    pstepguard->tPatrolPrev = pstepguardBase->tPatrolPrev;
    pstepguard->svtPatrolPrev = pstepguardBase->svtPatrolPrev;
    pstepguard->rpk = pstepguardBase->rpk;
    pstepguard->oidPathzone = pstepguardBase->oidPathzone;
    pstepguard->ppathzone = pstepguardBase->ppathzone;
    pstepguard->cposPath = pstepguardBase->cposPath;
    pstepguard->iposPath = pstepguardBase->iposPath;

    for (int i = 0; i < 16; ++i)
        pstepguard->aposPath[i] = pstepguardBase->aposPath[i];

    pstepguard->posGoal = pstepguardBase->posGoal;
    pstepguard->posGoalPath = pstepguardBase->posGoalPath;
    pstepguard->fPathValid = pstepguardBase->fPathValid;
    pstepguard->tPathValid = pstepguardBase->tPathValid;
    pstepguard->cStun = pstepguardBase->cStun;
    pstepguard->fTumbleStun = pstepguardBase->fTumbleStun;
    pstepguard->dtFade = pstepguardBase->dtFade;
    pstepguard->pwaypoint = pstepguardBase->pwaypoint;
    pstepguard->csge = pstepguardBase->csge;

    for (int i = 0; i < 4; ++i)
        pstepguard->asge[i] = pstepguardBase->asge[i];

    pstepguard->pxpEffect = pstepguardBase->pxpEffect;
    pstepguard->vThrow = pstepguardBase->vThrow;
    pstepguard->fBypassEffectHitSuppression = pstepguardBase->fBypassEffectHitSuppression;
    pstepguard->csgda = pstepguardBase->csgda;

    for (int i = 0; i < 4; ++i)
        pstepguard->asgda[i] = pstepguardBase->asgda[i];

    pstepguard->fExplExplicit = pstepguardBase->fExplExplicit;
    pstepguard->oidExpl = pstepguardBase->oidExpl;
    pstepguard->pexpl = pstepguardBase->pexpl;
    pstepguard->fRwmExplicit = pstepguardBase->fRwmExplicit;
    pstepguard->oidRwm = pstepguardBase->oidRwm;
    pstepguard->prwm = pstepguardBase->prwm;
    pstepguard->fFirstFrame = pstepguardBase->fFirstFrame;
    pstepguard->uIdleStart = pstepguardBase->uIdleStart;
    pstepguard->fFlying = pstepguardBase->fFlying;
    pstepguard->dzFlying = pstepguardBase->dzFlying;
    pstepguard->radTilt = pstepguardBase->radTilt;
    pstepguard->clqSToUPursue = pstepguardBase->clqSToUPursue;
    pstepguard->fEnemyExternal = pstepguardBase->fEnemyExternal;
    pstepguard->fRebindEnemy = pstepguardBase->fRebindEnemy;
    pstepguard->psoEnemyExternal = pstepguardBase->psoEnemyExternal;
    pstepguard->oidEnemyExternal = pstepguardBase->oidEnemyExternal;
    pstepguard->psoEnemyBound = pstepguardBase->psoEnemyBound;
    pstepguard->psoSubscribe = pstepguardBase->psoSubscribe;
    pstepguard->ccoin = pstepguardBase->ccoin;
    pstepguard->posBreakLocal = pstepguardBase->posBreakLocal;
    pstepguard->fNoDifficultyUpdate = pstepguardBase->fNoDifficultyUpdate;
    // Preserve the original hierarchy that owns the stripped ASEG objects.
    // RWM ammunition can be cloned from an already-cloned template; using that
    // immediate clone as the lookup root makes LoadStepguardAnimations search a
    // hierarchy from which the animations have already been snipped.
    pstepguard->pstepguardBase = pstepguardBase->pstepguardBase != nullptr
        ? pstepguardBase->pstepguardBase
        : pstepguardBase;
    pstepguard->fAnimatedJump = pstepguardBase->fAnimatedJump;
    pstepguard->fLockHeading = pstepguardBase->fLockHeading;
    pstepguard->fLockLook = pstepguardBase->fLockLook;
    pstepguard->fLockState = pstepguardBase->fLockState;
    pstepguard->fExternalAI = pstepguardBase->fExternalAI;
    pstepguard->fFreefall = pstepguardBase->fFreefall;
    pstepguard->fFreefallAseg = pstepguardBase->fFreefallAseg;
    pstepguard->dxFreefall = pstepguardBase->dxFreefall;
    pstepguard->dtFreefall = pstepguardBase->dtFreefall;
    pstepguard->lmDtFreefall = pstepguardBase->lmDtFreefall;
    pstepguard->tFreefallLand = pstepguardBase->tFreefallLand;
    pstepguard->posFreefallLand = pstepguardBase->posFreefallLand;
    pstepguard->vFreefallLand = pstepguardBase->vFreefallLand;
    pstepguard->uFling = pstepguardBase->uFling;
    pstepguard->fAutoFadeDying = pstepguardBase->fAutoFadeDying;
    pstepguard->fAbsorbDamage = pstepguardBase->fAbsorbDamage;
    pstepguard->fFaceDamageDirection = pstepguardBase->fFaceDamageDirection;
    pstepguard->lmDtTaunt = pstepguardBase->lmDtTaunt;
    pstepguard->muContact = pstepguardBase->muContact;
    pstepguard->ovrSgs = pstepguardBase->ovrSgs;
    pstepguard->tGiveUpSearch = pstepguardBase->tGiveUpSearch;
    pstepguard->tStopPatrol = pstepguardBase->tStopPatrol;
    pstepguard->tTaunt = pstepguardBase->tTaunt;
    pstepguard->tLostPlayer = pstepguardBase->tLostPlayer;
    pstepguard->fIncapacitated = pstepguardBase->fIncapacitated;
    pstepguard->fAlignToGround = pstepguardBase->fAlignToGround;
    pstepguard->ichkDead = ichkDead;
    pstepguard->psgg = psgg;

}

void BindStepguard(STEPGUARD* pstepguard)
{
    BindAlo(pstepguard);

    // Optional pathzone binding
    if (pstepguard->oidPathzone != OID_Nil)
        pstepguard->ppathzone = (PATHZONE*)PloFindSwNearest(pstepguard->psw, (OID)pstepguard->oidPathzone, pstepguard);
    // Optional external enemy binding
    if (pstepguard->oidEnemyExternal != OID_Nil)
        RebindStepguardEnemy(pstepguard);
    // Explosion binding
    if (!pstepguard->fExplExplicit) 
        InferExpl(&pstepguard->pexpl, pstepguard);
    else if (pstepguard->oidExpl != OID_Nil) 
        pstepguard->pexpl = (EXPL*)PloFindSwObject(pstepguard->psw, 0x101, (OID)pstepguard->oidExpl, pstepguard);
}

void PostStepguardLoadCallback(STEPGUARD* pstepguard, MSGID msgid, void* pv)
{
    pstepguard->pvtstepguard->pfnRemoveLo(pstepguard);
}

void PostStepguardLoad(STEPGUARD* pstepguard)
{
    PostStepLoad(pstepguard);
    SnipAloObjects(pstepguard, 2, s_asnipPostStepguardLoad);

    if (pstepguard->psgg == nullptr)
    {
        pstepguard->psgg = PsggNew(pstepguard->psw);
        AddSggGuard(pstepguard->psgg, pstepguard);
    }

    LoadStepguardAnimations(pstepguard);

    for (int i = 0; i < 17; i++) 
    {
        SO* psoPhys = pstepguard->mpsgssgp[i].psoPhys;

        if (psoPhys != nullptr)
        {
            pstepguard->psoPhys = psoPhys;
            pstepguard->psoPhysDefault = psoPhys;
            pstepguard->pvtso->pfnCloneSoPhys(pstepguard, psoPhys, 1);
            break;
        }
    }

    for (int i = 0; i < pstepguard->csge; i++) 
    {
        SGE* psge = &pstepguard->asge[i];

        psge->pso = (SO*)PloFindSwObject(pstepguard->psw, 257, psge->oidEffect, pstepguard);

        if (psge->pso != nullptr)
        {
            psge->pso->fNoXpsSelf = 1;
            psge->pso->pvtlo->pfnRemoveLo(psge->pso);
        }
    }

    if (pstepguard->fRwmExplicit == 0) {
        pstepguard->prwm = (RWM*)PloFindSwObjectByClass(pstepguard->psw, 257, (CID)140, pstepguard);
    }
    else if (pstepguard->oidRwm != OID_Nil) {
        pstepguard->prwm = (RWM*)PloFindSwObject(pstepguard->psw, 257, (OID)pstepguard->oidRwm, pstepguard);
    }

    SetStepGravityRatio(pstepguard, pstepguard->rGravity);

    SetStepguardSgs(pstepguard, SGS_Reset, nullptr);

    if (FGetChkmgrIchk(&g_chkmgr, pstepguard->ichkDead))
        PostSwCallback(pstepguard->psw, (PFNMQ)PostStepguardLoadCallback, pstepguard, MSGID_callback, nullptr);
}

void SetStepguardSgs(STEPGUARD* pstepguard, SGS sgs, ASEG* pasegTargetOverride)
{
    SGS sgsPrev = pstepguard->sgs;

    pstepguard->grfsgsc &= ~1u;

    if (sgsPrev == sgs)
        return;

    // Retail passes the state being entered here.  The exit handler uses it to
    // decide whether the patrol ASEGA should be retained (Patrol -> PatrolIdle)
    // or retracted and have its time/speed saved.
    pstepguard->pvtstepguard->pfnOnStepguardExitingSgs(pstepguard, sgs);

    pstepguard->sgs = sgs;
    pstepguard->tSgs = g_clock.t;

    pstepguard->pvtstepguard->pfnOnStepguardEnteringSgs(pstepguard, sgsPrev, pasegTargetOverride);

    void* apvArgs[2];
    apvArgs[0] = &sgsPrev;
    apvArgs[1] = &pstepguard->sgs;

    HandleLoSpliceEvent(pstepguard, 15, 2, apvArgs);
}

int FIgnoreStepguardIntersection(STEPGUARD* pstepguard, SO* psoOther)
{
    if (FIgnoreSoIntersection(pstepguard, psoOther))
        return 1;

    if (pstepguard->prwm != nullptr && FIsRwmAmmo(pstepguard->prwm, psoOther))
        return 1;

    if (pstepguard->sgs == SGS_Dying) 
    {
        if (psoOther->fPhys) 
            return 0;

        return 1;
    }

    return 0;
}

void OnStepguardAdd(STEPGUARD* pstepguard)
{
	OnPoAdd(pstepguard);

    if (pstepguard->psgg != nullptr)
        AddSggGuard(pstepguard->psgg, pstepguard);
}

void OnStepguardRemove(STEPGUARD* pstepguard)
{
	OnPoRemove(pstepguard);

    if (pstepguard->pasegaSgs != nullptr) {
        RetractAsega(pstepguard->pasegaSgs);
        pstepguard->pasegaSgs = nullptr;
    }
    if (pstepguard->pasegaPatrol != nullptr) {
        RetractAsega(pstepguard->pasegaPatrol);
        pstepguard->pasegaPatrol = nullptr;
    }
    if (pstepguard->psgg != nullptr) {
        RemoveSggGuard(pstepguard->psgg, pstepguard);
    }

    pstepguard->grfsgsc = 0;
    pstepguard->fPathValid = 0;
    pstepguard->tLostPlayer = 3.4028235e+38;
    pstepguard->tAttackNext = 0.0;
    pstepguard->tFreefallLand = 0.0;
    pstepguard->tGiveUpSearch = 3.4028235e+38;
    pstepguard->tStopPatrol = 3.4028235e+38;
    pstepguard->tTaunt = 3.4028235e+38;

}

void PresetStepguardAccel(STEPGUARD* pstepguard, float dt)
{
    glm::vec3 posNext;
    glm::vec3 velocityNext;
    glm::vec3 vNext;
    glm::vec3 dv;
    if (pstepguard->fAnimatedJump != 0) {
        ASEGA* pasega = pstepguard->pasegaSgs;
        ACP* pacp = pasega->paseg->pchnStrip->pacp.get();

        // Animated root motion is driven by the ACP velocity channel.  Using
        // its position channel here turns a positional offset into a velocity
        // and then divides it by dt, producing the enormous launch impulse.
        pacp->pvtacp->pfnEvaluateAcp(pacp, (ALO*)pstepguard, pasega->tLocal, pasega->svtLocal, 0, nullptr, &velocityNext);

        dv = (velocityNext + pstepguard->vTarget) / dt;
        vNext = glm::mat3(pstepguard->matTarget) * dv;

        AddSoAcceleration((SO*)pstepguard, &vNext);
        AccelSoTowardMatSmooth((SO*)pstepguard, g_clock.dt, &pstepguard->matTarget, &pstepguard->smpSpin);
        return;
    }

    if (pstepguard->fFlying != 0) {
        float dot = glm::dot(glm::vec3(pstepguard->xf.mat[0]), glm::vec3(pstepguard->matTarget[0]));
        float drad = acosf(glm::clamp(dot, -1.0f, 1.0f));
        float u = s_clqDradToUFly.g0 + fabsf(drad) * (s_clqDradToUFly.g1 + fabsf(drad) * s_clqDradToUFly.g2);
        float uFly = u;
        float sPursue = -1.0f;
        float uPursue = 1.0f;

        u = glm::clamp(u, 0.0f, 1.0f);

        if (pstepguard->sgs == SGS_Pursue) {
            sPursue = glm::length(glm::vec3(pstepguard->posGoalPath) - glm::vec3(pstepguard->xf.posWorld));
            uPursue = pstepguard->clqSToUPursue.g0 + sPursue * (pstepguard->clqSToUPursue.g1 + sPursue * pstepguard->clqSToUPursue.g2);

            u *= glm::clamp(uPursue, 0.0f, 1.0f);
        }

        posNext = -(glm::mat3(pstepguard->xf.matWorld) * glm::vec3(pstepguard->vTarget)) * u;

        AccelSoTowardPosSpring((SO*)pstepguard, nullptr, nullptr, &posNext, pstepguard->pclqPosDamping.get(), dt);
        AccelSoTowardMatSmooth((SO*)pstepguard, g_clock.dt, &pstepguard->matTarget, &pstepguard->smpSpin);
        return;
    }

    if (pstepguard->tFreefallLand > g_clock.t) {
        float tRemaining = pstepguard->tFreefallLand - g_clock.t;
        float dtStep = glm::min(tRemaining, dt);

        EvaluateBezierPos(tRemaining, dtStep, 1.0f, &pstepguard->xf.pos, &pstepguard->xf.v, &pstepguard->posFreefallLand, &pstepguard->vFreefallLand, &posNext, &vNext, nullptr);

        dv = (vNext - glm::vec3(pstepguard->xf.v)) / dt;
        LimitVectorLength(&dv, 20000.0f, &dv);
        AddSoAcceleration((SO*)pstepguard, &dv);
        return;
    }

    if (pstepguard->sgs == SGS_Stun && pstepguard->fTumbleStun != 0) {
        float u = 1.0f;

        if (pstepguard->pasegaSgs != nullptr) {
            u = pstepguard->pasegaSgs->tLocal / pstepguard->pasegaSgs->paseg->tMax;
        }

        u = s_clqUdtToU.g0 + u * (s_clqUdtToU.g1 + u * s_clqUdtToU.g2);
        u = glm::clamp(u, 0.0f, 1.0f);

        CLQ clqSpring = s_clqStunSpring;
        CLQ clqDamping = s_clqStunDamping;

        clqSpring.g0 *= u;
        clqSpring.g1 *= u;
        clqSpring.g2 *= u;

        clqDamping.g0 *= u;
        clqDamping.g1 *= u;
        clqDamping.g2 *= u;

        AccelSoTowardMatSpring(pstepguard, &pstepguard->matTarget, &clqSpring, &g_vecZero, &clqDamping, dt);
        AddSoAcceleration(pstepguard, &pstepguard->dvGravity);
        return;
    }

    PresetStepAccel(pstepguard, dt);
}

float DradEnemyStepguard(STEPGUARD* pstepguard)
{
    SO* psoEnemy = pstepguard->pvtstepguard->pfnPsoEnemyStepguard(pstepguard);

    if (psoEnemy == nullptr)
        return 0.0f;

    glm::vec3 dpos = psoEnemy->xf.posWorld - pstepguard->xf.posWorld;

    float radEnemy = atan2f(dpos.y, dpos.x);

    glm::vec3 forward = pstepguard->xf.matWorld[0];
    float radForward = atan2f(forward.y, forward.x);

    return RadNormalize(radEnemy - radForward);
}

void MoveStepguardToGoal(STEPGUARD* pstepguard)
{
    pstepguard->pvtstepguard->pfnUpdateStepguardPath(pstepguard);

    const glm::vec3 currentPosition = pstepguard->xf.posWorld;

    // Skip path points that have already been reached.
    while (pstepguard->iposPath < pstepguard->cposPath)
    {
        const glm::vec3& pathPosition = pstepguard->aposPath[pstepguard->iposPath];

        const glm::vec2 pathDeltaPlanar = glm::vec2(pathPosition) - glm::vec2(currentPosition);

        if (glm::length(pathDeltaPlanar) > 50.0f)
            break;

        ++pstepguard->iposPath;
    }

    glm::vec3 direction{ 0.0f };

    if (pstepguard->iposPath < pstepguard->cposPath)
    {
        // Face the next unreached path point.
        direction = pstepguard->aposPath[pstepguard->iposPath] - currentPosition;
    }
    else if (pstepguard->cposPath >= 2)
    {
        // The path is complete. Preserve the direction of its final segment.
        direction = pstepguard->aposPath[pstepguard->cposPath - 1] - pstepguard->aposPath[pstepguard->cposPath - 2];
    }

    float targetHeading = pstepguard->radTarget;

    if (glm::length(glm::vec2(direction)) > 1.0f)
        targetHeading = std::atan2(direction.y, direction.x);

    float targetTilt = 0.0f;

    if (pstepguard->fFlying != 0)
    {
        const glm::vec3 goalDelta = pstepguard->posGoalPath - currentPosition;
        const float distanceToGoal = glm::length(glm::vec2(goalDelta));

        float dzGoal = goalDelta.z;

        switch (pstepguard->sgs)
        {
            case SGS_Search:
            case SGS_SearchIdle:
            case SGS_Discover:
            case SGS_Taunt:
            case SGS_Pursue:
            case SGS_PursueIdle:
            case SGS_Attack:
            case SGS_AttackIdle:
            dzGoal += pstepguard->dzFlying;
            break;

            default:
            break;
        }

        float distanceTilt = s_clqSToUTilt.g0 + distanceToGoal * (s_clqSToUTilt.g1 + distanceToGoal * s_clqSToUTilt.g2);
        distanceTilt = glm::clamp(distanceTilt, s_lmUTiltS.gMin, s_lmUTiltS.gMax);

        const float absoluteDz = std::abs(dzGoal);

        float heightTilt = s_clqDzToUTilt.g0 + absoluteDz * (s_clqDzToUTilt.g1 + absoluteDz * s_clqDzToUTilt.g2);
        heightTilt = glm::clamp(heightTilt, s_lmUTiltDz.gMin, s_lmUTiltDz.gMax);

        targetTilt = distanceTilt * heightTilt * RAD_StepguardTiltMax;

        if (dzGoal < 0.0f)
            targetTilt = -targetTilt;
    }

    SetStepguardTargetHeading(pstepguard, targetHeading, targetTilt);
}

void SetStepguardTargetHeading(STEPGUARD* pstepguard, float radTarget, float radTilt)
{
    if (pstepguard == nullptr || FStepguardLookLocked(pstepguard))
        return;

    pstepguard->radTarget = radTarget;
    pstepguard->radTilt = radTilt;

    if (pstepguard->fFlying == 0)
        LoadRotateMatrixRad(radTarget, &g_normalZ, &pstepguard->matTarget);
    else
        LoadRotateMatrixPanTilt(radTarget, radTilt, &pstepguard->matTarget);
}

void UpdateStepguardAI(STEPGUARD* pstepguard)
{
    if (pstepguard->fLockState != 0 || pstepguard->fExternalAI != 0)
        return;

    if (g_pjt != nullptr && g_pjt->fStun != 0)
        return;

    while (true)
    {
        const SGS sgsNew = pstepguard->pvtstepguard->pfnSgsNextStepguardAI(pstepguard);

        if (sgsNew == pstepguard->sgs)
            break;

        SetStepguardSgs(pstepguard, sgsNew, nullptr);
    }
}

void UpdateStepguardDrive(STEPGUARD* pstepguard)
{
    float radTarget = 0.0f;
    float radTilt = 0.0f;
    float svDrive = 0.0f;

    float* pradTiltTarget = pstepguard->fFlying ? &radTilt : NULL;

    CalculateAloDrive(pstepguard, nullptr, nullptr, g_clock.dt, pstepguard->radTarget, &radTarget, pradTiltTarget, &svDrive);

    float svTarget = glm::length(pstepguard->vTarget);

    if (svDrive > 0.0001f && svTarget > 0.0001f)
    {
        const float svtOld = pstepguard->pasegaPatrol->svtLocal;
        const float ratio = svTarget / svDrive;
        const float svtNew = svtOld * ratio;

        pstepguard->pasegaPatrol->svtLocal = svtNew;
    }

    SetStepguardTargetHeading(pstepguard, radTarget, radTilt);
}

void GetStepguardOnFinalPathSegment(STEPGUARD* pstepguard, int* pfFinalSegment)
{
    *pfFinalSegment = pstepguard->fPathValid != 0 && pstepguard->cposPath - 2 <= pstepguard->iposPath;
}

void UpdateStepguard(STEPGUARD* pstepguard, float dt)
{
    UpdateStep(pstepguard, dt);

    // -------------------------------------------------------------------------
    // Refresh an explicitly bound enemy
    // -------------------------------------------------------------------------

    if (pstepguard->fRebindEnemy != 0)
        RebindStepguardEnemy(pstepguard);

    SO* psoEnemyBound = pstepguard->psoEnemyBound;

    if (psoEnemyBound != nullptr)
    {
        const bool enemyInWorld = FIsLoInWorld(reinterpret_cast<LO*>(psoEnemyBound)) != 0;
        const bool groupIsAttacking = pstepguard->psgg != nullptr && pstepguard->psgg->sggs == SGGS_Attack;

        if (!enemyInWorld || !groupIsAttacking)
            SetStepguardEnemyBound(pstepguard, nullptr);
    }
    else
    {
        const bool groupIsAttacking = pstepguard->psgg != nullptr && pstepguard->psgg->sggs == SGGS_Attack;

        if (!groupIsAttacking)
        {
            SO* psoEnemy = pstepguard->pvtstepguard->pfnPsoEnemyStepguard(pstepguard);

            // If this guard normally targets Sly, allow it to switch to Sly's
            // currently active decoy.
            if (g_pjt != nullptr &&
                psoEnemy == g_pjt &&
                g_pjt->pdecoy != nullptr &&
                FIsLoInWorld(g_pjt->pdecoy))
            {
                SetStepguardEnemyBound(pstepguard, g_pjt->pdecoy);

                if (pstepguard->pvtstepguard->pfnFDetectStepguard(pstepguard) == 0)
                {
                    SetStepguardEnemyBound(pstepguard, nullptr);
                }
                else if (pstepguard->psgg != nullptr)
                {
                    SGG* psgg = pstepguard->psgg;

                    // Share the discovered decoy with the other guards.
                    for (int i = 0; i < psgg->cpstepguard; ++i)
                    {
                        STEPGUARD* pstepguardOther = psgg->apstepguard[i];

                        if (pstepguardOther != nullptr && pstepguardOther != pstepguard)
                            SetStepguardEnemyBound(pstepguardOther, g_pjt->pdecoy);
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // Finish a freefall when the guard finds ground
    // -------------------------------------------------------------------------

    if (pstepguard->fFreefall != 0 && PxpFindSoGround(pstepguard, nullptr, nullptr) != nullptr)
        pstepguard->pvtstepguard->pfnDoStepguardFreefallLanding(pstepguard);

    HandleStepguardGrfsgsc(pstepguard);
    UpdateStepguardAI(pstepguard);

    // -------------------------------------------------------------------------
    // Calculate target velocity from the current state animation
    // -------------------------------------------------------------------------

    pstepguard->vTarget = glm::vec3(0.0f);

    ASEGA* pasegaSgs = pstepguard->pasegaSgs;

    if (pasegaSgs != nullptr && pasegaSgs->paseg != nullptr && pasegaSgs->paseg->pchnStrip != nullptr)
    {
        ACP* pacp = pasegaSgs->paseg->pchnStrip->pacp.get();

        if (pacp != nullptr)
        {
            pacp->pvtacp->pfnEvaluateAcp(pacp, pstepguard, pasegaSgs->tLocal, pasegaSgs->svtLocal, static_cast<GRFEVAL>(0), nullptr, &pstepguard->vTarget);
            pstepguard->vTarget *= -1.0f;

        }
    }

    pstepguard->pvtstepguard->pfnUpdateStepguardGoal(pstepguard, 0);
    pstepguard->pvtstepguard->pfnUpdateStepguardSgs(pstepguard);

    if (pstepguard->psgg != nullptr)
        EnsureSggCallback(pstepguard->psgg);

    UpdateStepguardEffect(pstepguard);

    // -------------------------------------------------------------------------
    // Align the target orientation to the ground
    // -------------------------------------------------------------------------

    if (pstepguard->fAlignToGround != 0)
    {
        int ixpdGround = 0;
        XP* pxpGround = PxpFindSoGround(pstepguard, nullptr, &ixpdGround);

        glm::vec3 normal = g_normalZ;

        if (pxpGround != nullptr)
            normal = pxpGround->normal * g_mpixpdr[ixpdGround];

        glm::mat3 headingMatrix(1.0f);
        glm::mat3 groundMatrix(1.0f);

        LoadRotateMatrixRad(pstepguard->radTarget, &g_normalZ, &headingMatrix);
        BuildRotateVectorsMatrix(&g_normalZ, &normal, &groundMatrix);

        pstepguard->matTarget = groundMatrix * headingMatrix;
    }

    ResolveAlo(pstepguard);

    pstepguard->fFirstFrame = 0;
}

SGS SgsNextStepguardAI(STEPGUARD* pstepguard)
{
#if 0
    if (pstepguard == nullptr)
        return SGS_Nil;

    const SGS sgsCurrent = pstepguard->sgs;
    const SGGS sggs = SggsGetStepguard(pstepguard);

    if (static_cast<unsigned>(sgsCurrent) >= 17)
        return sgsCurrent;

    if (pstepguard->tTaunt < g_clock.t &&
        sgsCurrent != SGS_Dying &&
        sgsCurrent != SGS_Stun &&
        pstepguard->mpsgssga[10].paseg != nullptr)
        return SGS_Taunt;

    SGS sgsNext = sgsCurrent;

    switch (sgsCurrent)
    {
    case SGS_Reset:
        switch (sggs)
        {
        case SGGS_Patrol:
            sgsNext = pstepguard->pasegPatrol != nullptr ? SGS_Patrol : SGS_PatrolIdle;
            break;
        case SGGS_Search:
            sgsNext = SGS_Search;
            break;
        case SGGS_Attack:
            sgsNext = SGS_Discover;
            break;
        case SGGS_Dead:
            sgsNext = SGS_Dying;
            break;
        default:
            break;
        }
        break;

    case SGS_Patrol:
    case SGS_PatrolIdle:
    case SGS_PatrolWaypoint:
        if (sggs != SGGS_Patrol && pstepguard->tStopPatrol <= g_clock.t)
            sgsNext = SGS_Reset;
        break;

    case SGS_ReturnToPatrol:
        if (sggs == SGGS_Patrol)
            FReachedStepguardGoal(pstepguard);
        break;

    case SGS_Search:
        if (sggs == SGGS_Search)
            FReachedStepguardGoal(pstepguard);
        break;

    case SGS_Discover:
        if (pstepguard->pasegaSgs == nullptr)
            sgsNext = SGS_Pursue;
        break;

    case SGS_LostPlayer:
    case SGS_Taunt:
    case SGS_Stun:
        if (pstepguard->pasegaSgs == nullptr)
            sgsNext = SGS_Reset;
        break;

    case SGS_Pursue:
        if (sggs == SGGS_Attack)
        {
            if (pstepguard->pasegaSgs == nullptr)
                sgsNext = SGS_PursueIdle;
            else if (pstepguard->mpsgssga[SGS_Attack].paseg != nullptr &&
                     pstepguard->pvtstepguard->pfnFCanStepguardAttack(pstepguard) != 0)
                sgsNext = SGS_Attack;
            else if (FReachedStepguardGoal(pstepguard) &&
                     pstepguard->mpsgssga[SGS_PursueIdle].paseg != nullptr)
                sgsNext = SGS_PursueIdle;
        }
        break;

    case SGS_PursueIdle:
        if (sggs == SGGS_Attack)
        {
            if (pstepguard->mpsgssga[SGS_Attack].paseg != nullptr &&
                pstepguard->pvtstepguard->pfnFCanStepguardAttack(pstepguard) != 0)
                sgsNext = SGS_Attack;
            else if (pstepguard->fPathValid == 0 &&
                     pstepguard->mpsgssga[SGS_Pursue].paseg != nullptr)
                sgsNext = SGS_Pursue;
        }
        break;

    case SGS_Attack:
        if (sggs == SGGS_Attack && pstepguard->pasegaSgs == nullptr)
            sgsNext = SGS_PursueIdle;
        break;

    default:
        break;
    }

    return sgsNext;
#else
    const int group = static_cast<int>(SggsGetStepguard(pstepguard));
    int state = static_cast<int>(pstepguard->sgs);

    if (pstepguard->tTaunt < g_clock.t && state != 11 && state != 16 &&
        pstepguard->mpsgssga[10].paseg != nullptr)
    {
        return static_cast<SGS>(10);
    }

    if (static_cast<unsigned>(state) >= 17)
        return static_cast<SGS>(state);

    switch (state)
    {
    case 0:
        if (group == 0)
            state = 4;
        else if (group == 1)
            state = 5;
        else if (group == 2)
            state = 7;
        break;

    case 1:
        if (group == 0)
        {
            if (pstepguard->pasegaPatrol == nullptr || pstepguard->dms == 1)
                state = 2;
        }
        else if (group == 1)
        {
            if (pstepguard->tStopPatrol < g_clock.t)
                state = 5;
        }
        else if (group == 2 && pstepguard->tStopPatrol < g_clock.t)
        {
            state = 7;
        }
        break;

    case 2:
        if (group == 0)
        {
            if (pstepguard->pasegaPatrol != nullptr && pstepguard->dms == 0)
                state = 1;
        }
        else if (group == 1)
        {
            if (pstepguard->tStopPatrol < g_clock.t)
                state = 5;
        }
        else if (group == 2 && pstepguard->tStopPatrol < g_clock.t)
        {
            state = 7;
        }
        break;

    case 3:
        if (group == 1)
        {
            if (pstepguard->tStopPatrol < g_clock.t)
                state = 5;
        }
        else if (group == 2 && pstepguard->tStopPatrol < g_clock.t)
        {
            state = 7;
        }
        break;

    case 4:
        if (group == 0)
        {
            if (FReachedStepguardGoal(pstepguard))
                state = 1;
        }
        else if (group == 1)
        {
            if (pstepguard->tStopPatrol < g_clock.t)
                state = 5;
        }
        else if (group == 2 && pstepguard->tStopPatrol < g_clock.t)
        {
            state = 7;
        }
        break;

    case 5:
        if (group == 0)
            state = 9;
        else if (group == 1)
        {
            if (FReachedStepguardGoal(pstepguard))
                state = 6;
        }
        else if (group == 2)
            state = 7;
        break;

    case 6:
        if (group == 0)
        {
            if (pstepguard->tGiveUpSearch < g_clock.t)
                state = 9;
        }
        else if (group == 1)
        {
            if (pstepguard->fPathValid == 0)
                state = 5;
        }
        else if (group == 2)
            state = 7;
        break;

    case 7:
        if (pstepguard->pasegaSgs == nullptr)
            state = 12;
        break;

    case 8:
    case 10:
    case 16:
        if (pstepguard->pasegaSgs == nullptr)
            state = 0;
        break;

    case 9:
        if (group == 0)
        {
            if (pstepguard->pasegaSgs == nullptr)
                state = 4;
        }
        else if (group == 1)
            state = 5;
        else if (group == 2 && pstepguard->tStopPatrol < g_clock.t)
            state = 7;
        break;

    case 11:
    case 15:
        break;

    case 12:
        if (group == 0 || group == 1)
        {
            if (pstepguard->tLostPlayer < g_clock.t)
                state = 8;
        }
        else if (group == 2)
        {
            if (pstepguard->pasegaSgs == nullptr)
                state = 13;
            else if (pstepguard->mpsgssga[14].paseg != nullptr &&
                     pstepguard->pvtstepguard->pfnFCanStepguardAttack(pstepguard) != 0)
                state = 14;
            else if (FReachedStepguardGoal(pstepguard) &&
                     pstepguard->mpsgssga[13].paseg != nullptr)
                state = 13;
        }
        break;

    case 13:
        if (group == 0 || group == 1)
        {
            if (pstepguard->tLostPlayer < g_clock.t)
                state = 8;
        }
        else if (group == 2)
        {
            if (pstepguard->mpsgssga[14].paseg != nullptr &&
                pstepguard->pvtstepguard->pfnFCanStepguardAttack(pstepguard) != 0)
                state = 14;
            else if (pstepguard->fPathValid == 0 &&
                     pstepguard->mpsgssga[12].paseg != nullptr)
                state = 12;
        }
        break;

    case 14:
        if (group == 0 || group == 1)
        {
            if (pstepguard->tLostPlayer < g_clock.t)
                state = 8;
        }
        else if (group == 2 && pstepguard->pasegaSgs == nullptr)
            state = 12;
        break;
    }

    return static_cast<SGS>(state);
#endif
}

void SetStepguardGoal(STEPGUARD* pstepguard, glm::vec3* pposGoal)
{
    glm::vec3 posGoalPath = *pposGoal;

    pstepguard->posGoal = posGoalPath;

    if (pstepguard->ppathzone != nullptr) {
        FindPathzoneClosestPoint(pstepguard->ppathzone, &posGoalPath, &posGoalPath);
    }

    if (pstepguard->fPathValid) {
        float dist = glm::length(pstepguard->posGoalPath - posGoalPath);

        if (dist <= 40.0) {
            return;
        }

        pstepguard->fPathValid = 0;
    }

    pstepguard->posGoalPath = posGoalPath;
}

int FReachedStepguardGoal(STEPGUARD* pstepguard)
{
    if (!pstepguard->fPathValid)
        return 0;

    return (pstepguard->iposPath == pstepguard->cposPath);
}

int FFilterStepguardJump(STEPGUARD* pstepguard, SO* pso)
{
    if (pso->fNoXpsSelf && !(pso->pvtlo->grfcid & 0x80 && pso->gBuoyancy != 0.0f))
        return 0;

    if (FIsBasicDerivedFrom((BASIC*)pso->paloRoot, CID_PO))
        return 0;

    if (FIsBasicDerivedFrom((BASIC*)pso->paloRoot, CID_FLY))
        return 0;

    if (FIsBasicDerivedFrom((BASIC*)pso->paloRoot, CID_RAT))
        return 0;

    return 1;
}

void UpdateStepguardPath(STEPGUARD* pstepguard)
{
    if (pstepguard == nullptr)
        return;

    const float pathAge = g_clock.t - pstepguard->tPathValid;

    if (pstepguard->fPathValid != 0 && pathAge <= 0.5f)
        return;

    glm::vec3 currentPosition = pstepguard->xf.posWorld;
    glm::vec3 goalPosition = pstepguard->posGoalPath;

    // -------------------------------------------------------------------------
    // Normal pathfinding
    // -------------------------------------------------------------------------

    if (pstepguard->fFreefallAseg == 0)
    {
        pstepguard->cposPath = 0;

        if (pstepguard->ppathzone != nullptr)
            pstepguard->cposPath = CposFindPathzonePath(pstepguard->ppathzone, &currentPosition, &pstepguard->posGoalPath, 16, pstepguard->aposPath);

        // Fall back to a direct two-point path if no PATHZONE route was found.
        if (pstepguard->cposPath == 0)
        {
            pstepguard->cposPath = 2;
            pstepguard->aposPath[0] = currentPosition;
            pstepguard->aposPath[1] = goalPosition;
        }

    }

    // -------------------------------------------------------------------------
    // Freefall path
    // -------------------------------------------------------------------------

    else
    {
        constexpr float kPathAdvanceStep = 100.0f;
        constexpr float kGroundTraceBelow = 100.0f;
        constexpr float kGroundTraceAbove = 500.0f;

        const glm::vec3 displacement = goalPosition - currentPosition;
        const float distanceToGoal = glm::length(displacement);
        const float requestedDistance = std::abs(pstepguard->dxFreefall);
        const float initialDistance = std::min(requestedDistance, distanceToGoal);

        pstepguard->cposPath = 2;
        pstepguard->aposPath[0] = currentPosition;

        glm::vec3 candidatePosition = goalPosition;

        if (distanceToGoal > 0.0001f && requestedDistance < distanceToGoal)
            candidatePosition = currentPosition + displacement * (requestedDistance / distanceToGoal);

        glm::vec3 pathPosition = candidatePosition;

        if (pstepguard->ppathzone != nullptr)
            FindPathzoneClosestPoint(pstepguard->ppathzone, &candidatePosition, &pathPosition);

        // If snapping to the PATHZONE moved the point too close to the guard,
        // advance farther toward the goal and try again.
        if (pstepguard->ppathzone != nullptr && distanceToGoal > 0.0001f)
        {
            float distanceAlongPath = initialDistance;
            const float minimumAcceptedDistance = initialDistance * 0.5f;

            while (glm::length(pathPosition - currentPosition) < minimumAcceptedDistance && distanceAlongPath < distanceToGoal)
            {
                distanceAlongPath = std::min(distanceAlongPath + kPathAdvanceStep, distanceToGoal);
                candidatePosition = currentPosition + displacement * (distanceAlongPath / distanceToGoal);
                FindPathzoneClosestPoint(pstepguard->ppathzone, &candidatePosition, &pathPosition);
            }
        }

        // ---------------------------------------------------------------------
        // Project the selected path point onto the ground.
        // ---------------------------------------------------------------------

        glm::vec3 traceBottom = pathPosition - g_normalZ * kGroundTraceBelow;
        glm::vec3 traceTop = pathPosition + g_normalZ * kGroundTraceAbove;

        std::vector <SO*> candidates;
        IntersectSwBoundingBox(pstepguard->psw, pstepguard, &traceBottom, &traceTop, (PFNFILTER)FFilterStepguardJump, pstepguard, candidates);

        if (!candidates.empty())
        {
            LSG hit{};

            SO* hitObject = PsoHitTestLineObjects(1, &traceTop, &traceBottom, candidates, &hit);

            if (hitObject != nullptr)
            {
                float baseOffset = -pstepguard->geomLocal.apos[pstepguard->iposBase].z - 1.0f;

                if (pstepguard->fIncapacitated != 0)
                    baseOffset += pstepguard->dzBase;

                pathPosition = hit.apos[0] + g_normalZ * baseOffset;
            }
        }

        pstepguard->aposPath[1] = pathPosition;
    }

    // Begin traversing the newly generated path.
    pstepguard->iposPath = 0;
    pstepguard->fPathValid = 1;
    pstepguard->tPathValid = g_clock.t;
}

void LookStepguardAtGoal(STEPGUARD* pstepguard)
{
    float radTarget;
    float radTilt;

    DecomposeSphere(&pstepguard->posGoal, &pstepguard->xf.posWorld, &radTarget, &radTilt, nullptr);
    SetStepguardTargetHeading(pstepguard, radTarget, radTilt);
}

void UpdateStepguardGoal(STEPGUARD* pstepguard, int fEnter)
{
    switch (pstepguard->sgs)
    {
        case SGS_ReturnToPatrol:
        {
            if (fEnter == 0)
                return;

            glm::vec3 posReturn = pstepguard->posOrig;

            if (pstepguard->pasegPatrol != nullptr)
            {
                if (pstepguard->rpk == RPK_Closest)
                    FindAsegClosestPoint(pstepguard->pasegPatrol, pstepguard, &pstepguard->xf.posWorld, pstepguard->tPatrolPrev, &pstepguard->tPatrolReturn, &posReturn, nullptr);
                else if (pstepguard->rpk == RPK_Previous)
                {
                    pstepguard->tPatrolReturn = pstepguard->tPatrolPrev;

                    CHN* pchn = PchnFindAseg(pstepguard->pasegPatrol, pstepguard);

                    if (pchn != nullptr && pchn->pacp != nullptr)
                        pchn->pacp->pvtacp->pfnEvaluateAcp(pchn->pacp.get(), pstepguard, pstepguard->tPatrolReturn, 1.0f, static_cast<GRFEVAL>(0), &posReturn, nullptr);
                }

                pstepguard->svtPatrolPrev = 1.0f;
            }

            SetStepguardGoal(pstepguard, &posReturn);
            break;
        }

        case SGS_Search:
        if (fEnter != 0)
            SetStepguardGoal(pstepguard, &pstepguard->posSearch);
        break;

        case SGS_Discover:
        case SGS_Taunt:
        case SGS_Pursue:
        case SGS_PursueIdle:
        case SGS_Attack:
        {
            SO* psoEnemy = pstepguard->pvtstepguard->pfnPsoEnemyStepguard(pstepguard);

            if (psoEnemy != nullptr)
                SetStepguardGoal(pstepguard, &psoEnemy->xf.posWorld);

            break;
        }

        default:
        break;
    }
}
void OnStepguardWaypointActive(STEPGUARD* pstepguard)
{
    WAYPOINT* pwaypoint = pstepguard->pwaypoint;

    if (pwaypoint->paseg == nullptr) {
        return;
    }

    OVR ovr;
    ovr.oid = (OID)pwaypoint->oid;
    ovr.palo = (ALO*)pstepguard;

    ApplyAsegOvr(pwaypoint->paseg, pstepguard, 1, &ovr, 0.0f, 1.0f, 1, &pstepguard->pasegaPatrol);
    SubscribeAsegaObject(pstepguard->pasegaPatrol, (LO*)pwaypoint);

    OID oidProxy = OidProxyLoPreferred(pstepguard);
    ACTSEG* pactseg = PactsegFindAsega(pstepguard->pasegaPatrol, oidProxy);

    if (pactseg == (ACTSEG*)pstepguard->pactPos) {
        pactseg->ackPos = ACK_Drive;
    }

    if (pactseg == (ACTSEG*)pstepguard->pactRot) {
        pactseg->ackRot = ACK_Drive;
    }
}

void UpdateStepguardSgs(STEPGUARD* pstepguard)
{
    PsoEnemyStepguard(pstepguard);

    switch (pstepguard->sgs) 
    {
        case SGS_Patrol:
        case SGS_PatrolIdle:
        if (FDrivenAlo(pstepguard)) {
            UpdateStepguardDrive(pstepguard);
            return;
        }
        break;

        case SGS_PatrolWaypoint:
        if (pstepguard->pasegaPatrol != nullptr) {
            float radTarget;
            float radTilt = 0.0f;
            float svTarget;

            CalculateAloDrive((ALO*)pstepguard, nullptr, nullptr, g_clock.dt, pstepguard->radTarget, &radTarget, pstepguard->fFlying ? &radTilt : nullptr, &svTarget);
            SetStepguardTargetHeading(pstepguard, radTarget, radTilt);
        }

        EnsureWpsgCallback(pstepguard->pwaypoint->pwpsg.get());
        return;

        case SGS_ReturnToPatrol:
        case SGS_Search:
        case SGS_Pursue:
        MoveStepguardToGoal(pstepguard);
        return;

        case SGS_Discover:
        case SGS_Taunt:
        case SGS_PursueIdle:
        case SGS_Attack:
        LookStepguardAtGoal(pstepguard);
        break;

        default:
        break;
    }
}

void SetStepguardSgsExternal(STEPGUARD* pstepguard, SGS sgs, ASEG* pasegTargetOverride)
{
    if (sgs == SGS_Nil)
    {
        pstepguard->fExternalAI = 0;
    }
    else
    {
        pstepguard->fExternalAI = 1;
        SetStepguardSgs(pstepguard, sgs, pasegTargetOverride);
    }
}

void OnStepguardExitingSgs(STEPGUARD* pstepguard, SGS sgsNext)
{
    if (pstepguard->pasegaSgs != nullptr)
        pstepguard->uAsegaPrev = pstepguard->pasegaSgs->tLocal / pstepguard->pasegaSgs->paseg->tMax;

    pstepguard->fAnimatedJump = 0;
    pstepguard->fLockHeading = 0;
    pstepguard->fLockState = 0;
    pstepguard->fFreefall = 0;

    for (int i = 0; i < pstepguard->csge; i++)
    {
        SO* pso = pstepguard->asge[i].pso;

        if (pso != nullptr)
            pso->pvtlo->pfnRemoveLo(pso);
    }

    switch (pstepguard->sgs)
    {
        case SGS_Patrol:
        if (pstepguard->pasegaPatrol != nullptr && sgsNext != SGS_PatrolIdle)
        {
            pstepguard->tPatrolPrev = pstepguard->pasegaPatrol->tLocal;
            pstepguard->svtPatrolPrev = pstepguard->pasegaPatrol->svtLocal;

            RetractAsega(pstepguard->pasegaPatrol);
            pstepguard->pasegaPatrol = nullptr;
        }
        break;

        case SGS_PatrolWaypoint:
        if (pstepguard->pasegaPatrol != nullptr)
        {
            RetractAsega(pstepguard->pasegaPatrol);
            pstepguard->pasegaPatrol = nullptr;
        }

        pstepguard->pwaypoint->pvtlo->pfnUnsubscribeLoObject(pstepguard->pwaypoint, pstepguard);
        SetWaypointWps(pstepguard->pwaypoint, WPS_Passive);
        pstepguard->pwaypoint = nullptr;

        if (sgsNext == SGS_Patrol)
            pstepguard->tPatrolReturn = pstepguard->tPatrolPrev;

        break;

        case SGS_LostPlayer:
        pstepguard->tLostPlayer = FLT_MAX;
        break;

        case SGS_Dying:
        pstepguard->sgs = sgsNext;
        RecalcSwOxfFilterForObject(g_psw, pstepguard);
        [[fallthrough]];

        case SGS_Stun:
        pstepguard->fIncapacitated = 0;
        break;

        case SGS_Attack:
        {
            float dtAttack = GRandInRange(pstepguard->lmAttackT.gMin, pstepguard->lmAttackT.gMax);
            pstepguard->tAttackNext = g_clock.t + dtAttack;
            break;
        }
    }

    if (pstepguard->pasegaSgs != nullptr)
    {
        RetractAsega(pstepguard->pasegaSgs);
        pstepguard->pasegaSgs = nullptr;

        pstepguard->grfsgsc &= ~2u;
    }
}

void OnStepguardEnteringSgs(STEPGUARD* pstepguard, SGS sgsPrev, ASEG* pasegTargetOverride)
{
    if (pstepguard == nullptr)
        return;

    const SGS sgs = pstepguard->sgs;

    // Use the animation assigned to the new state unless the caller supplied
    // a damage/special-state override.
    if (pasegTargetOverride == nullptr && sgs >= SGS_Reset && sgs < SGS_Max)
        pasegTargetOverride = pstepguard->mpsgssga[sgs].paseg;

    SO* psoPhys = nullptr;

    if (sgs >= SGS_Reset && sgs < SGS_Max)
        psoPhys = pstepguard->mpsgssgp[sgs].psoPhys;

    if (psoPhys == nullptr)
        psoPhys = pstepguard->psoPhysDefault;

    SetStepPhys(pstepguard, psoPhys, 0);
    pstepguard->pvtstepguard->pfnUpdateStepguardGoal(pstepguard, 1);

    switch (sgs)
    {
    case SGS_Patrol:
    {
        if (pstepguard->pasegPatrol != nullptr && sgsPrev != SGS_PatrolIdle)
        {
            float svtPatrol = pstepguard->svtPatrolPrev;

            // Patrol references loaded by the post-load SNIP are written
            // directly into pasegPatrol, bypassing SetStepguardPatrolAnimation.
            // On the first activation there is therefore no previously saved
            // ASEGA speed.  Retail starts that initial traversal forward; once
            // an ASEGA has run, OnStepguardExitingSgs saves its actual signed
            // speed and that value must be preserved when patrol resumes.
            if (pstepguard->fFirstFrame != 0 && std::abs(svtPatrol) < 0.0001f)
                svtPatrol = 1.0f;

            ApplyAseg(pstepguard->pasegPatrol, reinterpret_cast<ALO*>(pstepguard), pstepguard->tPatrolReturn, svtPatrol, static_cast<GRFAPL>(1), &pstepguard->pasegaPatrol);

            const OID oidPreferred = OidProxyLoPreferred(reinterpret_cast<LO*>(pstepguard));
            ACT* pact = PactsegFindAsega(pstepguard->pasegaPatrol, oidPreferred);

            if (pact == pstepguard->pactPos)
                pact->ackPos = ACK_Drive;

            if (pact == pstepguard->pactRot)
                pact->ackRot = ACK_Drive;

        }

        break;
    }

    case SGS_PatrolIdle:
    {
        if (pstepguard->pasegaPatrol == nullptr)
            SetStepguardTargetHeading(pstepguard, pstepguard->eulOrig.z, 0.0f);
        else
            SetAsegaSpeed(pstepguard->pasegaPatrol, 1.0f);

        break;
    }

    case SGS_PatrolWaypoint:
    {
        if (pstepguard->pwaypoint != nullptr && pstepguard->pwaypoint->wps == WPS_Active)
            OnStepguardWaypointActive(pstepguard);

        break;
    }

    case SGS_SearchIdle:
    {
        pstepguard->tGiveUpSearch = FLT_MAX;
        SetStepguardTargetHeading(pstepguard, pstepguard->radTargetSearch, 0.0f);
        break;
    }

    case SGS_Discover:
    {
        pstepguard->tStopPatrol = FLT_MAX;
        break;
    }

    case SGS_Taunt:
    {
        pstepguard->tTaunt = FLT_MAX;
        break;
    }

    case SGS_Dying:
    {
        glm::vec3 breakPosition{};

        ConvertAloPos(pstepguard, nullptr, &pstepguard->posBreakLocal, &breakPosition);
        OnDifficultyBreak(&g_difficulty, &breakPosition, pstepguard->ccoin, pstepguard->fNoDifficultyUpdate);
        RecalcSwOxfFilterForObject(g_psw, pstepguard);

        if (pstepguard->pasegPatrol != nullptr)
        {
            std::vector <WAYPOINT*> waypoints;

            GetAsegWaypoints(pstepguard->pasegPatrol, pstepguard, nullptr, waypoints);

            for (WAYPOINT* waypoint : waypoints)
            {
                if (waypoint != nullptr && waypoint->pwpsg != nullptr)
                    RemoveWpsgWaypoint(waypoint->pwpsg.get(), waypoint);
            }
        }

        SetChkmgrIchk(&g_chkmgr, pstepguard->ichkDead);

        if (pstepguard->psoSubscribe != nullptr)
        {
            pstepguard->psoSubscribe->pvtlo->pfnUnsubscribeLoObject(pstepguard->psoSubscribe, pstepguard);
            pstepguard->psoSubscribe = nullptr;
        }

        if (pstepguard->fAutoFadeDying != 0)
        {
            float fadeDelay = pstepguard->dtFade;

            if (pasegTargetOverride != nullptr)
                fadeDelay += pasegTargetOverride->tMax;

            FadeAloOut(pstepguard, fadeDelay);
        }

        pstepguard->fIncapacitated = 1;
        break;
    }

        case SGS_Stun:
        {
            pstepguard->fIncapacitated = 1;
            --pstepguard->cStun;
            break;
        }

        default:
        break;
    }

    // -------------------------------------------------------------------------
    // Start the state's animation.
    // -------------------------------------------------------------------------

    if (pasegTargetOverride != nullptr)
    {
        float uStart = 0.0f;

        if (sgs == SGS_PatrolIdle && pstepguard->fFirstFrame != 0)
        {
            uStart = pstepguard->uIdleStart;

            if (uStart < 0.0f)
                uStart = GRandInRange(0.0f, 1.0f);
        }
        else if (pstepguard->pasegPrev != nullptr)
        {
            if (pasegTargetOverride == pstepguard->pasegPrev)
                uStart = pstepguard->uAsegaPrev;
            else
            {
                // Preserve normalized animation time when the old and new
                // animations belong to the same SGMAP group.
                for (int imap = 0; imap < pstepguard->csgmap; ++imap)
                {
                    const SGMAP& map = pstepguard->asgmap[imap];

                    bool containsPrevious = false;
                    bool containsTarget = false;

                    for (int ioid = 0; ioid < 4; ++ioid)
                    {
                        containsPrevious |= map.aoid[ioid] == pstepguard->pasegPrev->oid;
                        containsTarget |= map.aoid[ioid] == pasegTargetOverride->oid;
                    }

                    if (containsPrevious && containsTarget)
                    {
                        uStart = pstepguard->uAsegaPrev;
                        break;
                    }
                }
            }
        }

        GRFAPL grfapl = static_cast<GRFAPL>(uStart != 0.0f ? 3 : 1);

        if (sgsPrev == SGS_Dying)
            grfapl = static_cast<GRFAPL>(static_cast<unsigned>(grfapl) | 4U);

        const float tStart = uStart * pasegTargetOverride->tMax;

        if (pstepguard->ovrSgs.oid == OID_Nil || pstepguard->ovrSgs.palo == nullptr)
            ApplyAseg(pasegTargetOverride, pstepguard, tStart, 1.0f, grfapl, &pstepguard->pasegaSgs);
        else
            ApplyAsegOvr(pasegTargetOverride, pstepguard, 1, &pstepguard->ovrSgs, tStart, 1.0f, grfapl, &pstepguard->pasegaSgs);

        pstepguard->pasegPrev = pasegTargetOverride;
        pstepguard->uAsegaPrev = 0.0f;
    }

    // Establish the initial heading for the new state.
    switch (sgs)
    {
        case SGS_Patrol:
        case SGS_ReturnToPatrol:
        case SGS_Search:
        case SGS_Pursue:
        MoveStepguardToGoal(pstepguard);
        break;

        case SGS_Discover:
        case SGS_Taunt:
        case SGS_PursueIdle:
        case SGS_Attack:
        LookStepguardAtGoal(pstepguard);
        break;

        default:
        break;
    }
}

SGGS SggsGetStepguard(STEPGUARD* pstepguard)
{
    if (pstepguard->psgg != nullptr)
        return pstepguard->psgg->sggs;
    
    return SGGS_Dead;
}

int FAbsorbStepguardWkr(STEPGUARD* pstepguard, WKR* pwkr)
{
    if (pstepguard->fAbsorbDamage != 0)
        return 1;

    // Ammunition belonging to this guard's ranged weapon is ignored.
    if (pstepguard->prwm != nullptr && pwkr->ploSource != nullptr)
    {
        LO* source = pwkr->ploSource;

        if ((source->pvtlo->grfcid & 2U) != 0 && FIsRwmAmmo(pstepguard->prwm, (SO*)pwkr->ploSource))
            return 1;
    }

    const long absorbed = FAbsorbSoWkr(pstepguard, pwkr);

    if (absorbed == 0)
        return 0;

    // Only damage interactions carrying this flag stun or kill the guard.
    if ((pwkr->grfic & 32U) == 0)
        return absorbed;

    // Find the most recently configured damage-animation override matching
    // this attack type.
    ASEG* pasegTargetOverride = nullptr;

    for (int i = pstepguard->csgda - 1; i >= 0; --i)
    {
        SGDA& sgda = pstepguard->asgda[i];

        if ((sgda.grftak & pwkr->grftak) != 0)
        {
            pasegTargetOverride = sgda.paseg;
            break;
        }
    }

    // Turn to face the direction from which the damage arrived.
    if (pstepguard->fFaceDamageDirection != 0)
    {
        const float speed = glm::length(pwkr->v);

        if (speed > 0.0001f)
        {
            pstepguard->fLockHeading = 0;

            const float incomingHeading = std::atan2(-pwkr->v.y, -pwkr->v.x);
            const float headingDelta = RadNormalize(incomingHeading - pstepguard->radTarget);
            const float targetHeading = RadNormalize(pstepguard->radTarget + headingDelta * pstepguard->uFling);

            SetStepguardTargetHeading(pstepguard, targetHeading, pstepguard->radTilt);
            FixStepAngularVelocity(pstepguard);
        }
    }

    // cStun is effectively the remaining nonfatal hit count.
    const SGS damageState = pstepguard->cStun < 1 ? SGS_Dying : SGS_Stun;

    SetStepguardSgs(pstepguard, damageState, pasegTargetOverride);

    // Trigger the optional death/hit explosion.
    if (pstepguard->pexpl != nullptr)
    {
        EXPLSO explso{};

        explso.grfexplso = 6;
        explso.paloReference = pstepguard;

        if (pwkr->ploSource != nullptr && (pwkr->ploSource->pvtlo->grfcid & 1U) != 0)
        {
            const ALO* sourceAlo = reinterpret_cast<const ALO*>(pwkr->ploSource);
            explso.vec = pstepguard->xf.posWorld - sourceAlo->xf.posWorld;
        }
        else
            explso.vec = pwkr->v;

        const float directionLength = glm::length(explso.vec);

        if (directionLength > 0.0001f)
            explso.vec /= directionLength;
        else
            explso.vec = g_normalX;

        explso.posOrigin = pstepguard->xf.posWorld;
        explso.posOrigin.z = pwkr->pos.z;

        pstepguard->pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pstepguard->pexpl, &explso);
    }

    MSGDMG msgdmg{};
    msgdmg.ploDamaged = pstepguard;
    msgdmg.ploSource = pwkr->ploSource;
    pstepguard->pvtlo->pfnSendLoMessage(pstepguard, MSGID_damaged, &msgdmg);

    return absorbed;
}

int FTakeStepguardDamage(STEPGUARD* pstepguard, ZPR* pzpr)
{
    WKR wkr = {};

    wkr.ploSource = pzpr->ploSource;

    if (pzpr->zpk == ZPK_Crush) {
        wkr.grftak = 4;
    }
    else if (wkr.ploSource != nullptr && (wkr.ploSource->pvtlo->grfcid & 32) != 0) {
        wkr.grftak = 16;
    }
    else {
        wkr.grftak = 2;
    }

    wkr.pos = pstepguard->xf.posWorld;
    wkr.ploTarget = (LO*)pstepguard;

    ALO* paloAbsorbed = PaloAbsorbWkr(&wkr, 0, nullptr);

    return paloAbsorbed != nullptr;
}

bool FAbsorbDamageStepguard(STEPGUARD* pstepguard)
{
    return (pstepguard->fAbsorbDamage != 0);
}

int FCanStepguardTauntDamage(STEPGUARD* pstepguard, SO* psoEnemy, SO* psoDamage)
{
    if (psoEnemy != pstepguard->psoSubscribe) {
        return 0;
    }

    switch (pstepguard->sgs) 
    {
        case SGS_Taunt:
        case SGS_Dying:
        case SGS_Stun:
        return 0;

        case SGS_Pursue:
        case SGS_PursueIdle:
        case SGS_Attack:
        return 1;

        default:
        break;
    }

    if (psoDamage == nullptr) {
        return 0;
    }

    if (psoDamage->paloRoot == pstepguard->paloRoot) {
        return 1;
    }

    return pstepguard->prwm != nullptr && FIsRwmAmmo(pstepguard->prwm, psoDamage);
}

void HandleStepguardGrfsgsc(STEPGUARD* pstepguard)
{
    if (pstepguard->grfsgsc == 0) 
        return;

    if ((pstepguard->grfsgsc & 2) != 0) 
    {
        ASEGA* pasega = pstepguard->pasegaSgs;

        if (pasega != nullptr) {
            if (pstepguard->sgs == SGS_Dying) {
                pasega->tLocal = pasega->paseg->tMax;
                pasega->dtPause = FLT_MAX;
            }
            else {
                RetractAsega(pasega);
                pstepguard->pasegaSgs = nullptr;
            }
        }

        pstepguard->grfsgsc &= ~2;
    }

    if ((pstepguard->grfsgsc & 1) != 0) {
        SetStepguardSgs(pstepguard, pstepguard->sgsNext, nullptr);
    }
}

void DoStepguardFreefallJump(STEPGUARD* pstepguard)
{
    int iposNext = glm::min(pstepguard->iposPath, pstepguard->cposPath - 1);

    glm::vec3 posNext = pstepguard->aposPath[iposNext];
    glm::vec3 dposNext = posNext - pstepguard->xf.posWorld;

    // Retail uses the complete displacement here.  Using only XY shortens the
    // flight time whenever the landing point differs in height, which inflates
    // the launch velocity and can throw the guard beyond the arena.
    float distanceToLanding = glm::length(dposNext);
    float dtRatio = GLimitLm(&pstepguard->lmDtFreefall, distanceToLanding / fabsf(pstepguard->dxFreefall));
    float dtFreefall = dtRatio * pstepguard->dtFreefall;

    glm::vec3 v = (dposNext / dtFreefall) - (pstepguard->dvGravity * (0.5f * dtFreefall));

    pstepguard->pvtalo->pfnSetAloVelocityVec(pstepguard, &v);

    pstepguard->tFreefallLand = g_clock.t + dtFreefall;
    pstepguard->posFreefallLand = posNext;
    pstepguard->vFreefallLand = v + pstepguard->dvGravity * dtFreefall;

    if (distanceToLanding > 1.0f) {
        float radTarget = atan2f(dposNext.y, dposNext.x);

        if (pstepguard->dxFreefall < 0.0f) {
            radTarget += glm::pi<float>();
        }

        SetStepguardTargetHeading(pstepguard, radTarget, 0.0f);

        glm::vec3 localVelocity = glm::transpose(pstepguard->matTarget) * (-v);

        pstepguard->vTarget = localVelocity;
    }
    else 
        pstepguard->vTarget = glm::vec3(0.0f);
}

void DoStepguardFreefallLanding(STEPGUARD* pstepguard)
{
    pstepguard->fFreefall = 0;
    pstepguard->fPathValid = 0;
    float dtLocal = TFindAsegLabel(pstepguard->pasegaSgs->paseg, (OID)433);
    SeekAsega(pstepguard->pasegaSgs, SEEK_Start, dtLocal, 1.0);
    pstepguard->tFreefallLand = 0.0;
}

void HandleStepguardMessage(STEPGUARD* pstepguard, MSGID msgid, void* pv)
{
    HandlePoMessage(pstepguard, msgid, pv);

    switch (msgid)
    {
    case MSGID_asega_limit:
    {
        ASEGA* pasega = static_cast<ASEGA*>(pv);

        if (pasega == pstepguard->pasegaSgs)
        {
            pstepguard->grfsgsc |= 2;
        }
        else if (pasega == pstepguard->pasegaPatrol)
        {
            SetStepguardPatrolAnimation(pstepguard, nullptr);
        }

        break;
    }

    case MSGID_water_entered:
    {
        const auto* message = static_cast<const STEP_GUARD_WATER_MSG*>(pv);

        if (message->pstepguard == pstepguard && pstepguard->fFreefall != 0)
            pstepguard->pvtstepguard->pfnDoStepguardFreefallLanding(pstepguard);

        break;
    }

    case MSGID_label:
    {
        const auto* message = static_cast<const STEP_GUARD_LABEL_MSG*>(pv);
        ASEGA* pasega = message->pasega;

        if (pasega == nullptr || pasega != pstepguard->pasegaSgs || pasega->paseg == nullptr)
            break;

        ASEG* aseg = pasega->paseg;

        switch (message->label)
        {
            // Prototype label 300.
        case 312:
        {
            if (aseg->pchnStrip == nullptr || aseg->pchnStrip->pacp == nullptr)
                break;

            glm::vec3 velocityLocal{};

            aseg->pchnStrip->pacp->pvtacp->pfnEvaluateAcp(aseg->pchnStrip->pacp.get(), reinterpret_cast<ALO*>(pstepguard), message->tLocal, pasega->svtLocal, static_cast<GRFEVAL>(0), nullptr, &velocityLocal);

            pstepguard->vTarget = -velocityLocal;

            glm::vec3 velocityWorld = pstepguard->matTarget * velocityLocal;
            pstepguard->pvtstepguard->pfnSetSoVelocityVec(pstepguard, &velocityWorld);
            break;
        }

        // Fire ranged weapon.
        case 421:
        {
            RWM* prwm = pstepguard->prwm;

            if (prwm == nullptr)
                break;

            // A configured target OID is not useful when PostRwmLoad could not
            // resolve it. In that case, aim at the guard's current enemy just
            // like an RWM with no explicit target.
            if (prwm->rwti.palo == nullptr && prwm->rwti.ptarget == nullptr)
                prwm->rwti.palo = reinterpret_cast<ALO*>(pstepguard->pvtstepguard->pfnPsoEnemyStepguard(pstepguard));

            FFireRwm(prwm, 1);
            break;
        }

        // Begin animated jump.
        case 422:
        {
            if (PeaFindAsegLabel(aseg, (OID)423) == nullptr || aseg->pchnStrip == nullptr || aseg->pchnStrip->pacp == nullptr)
                break;

            pstepguard->fAnimatedJump = 1;

            glm::vec3 velocityLocal{};

            aseg->pchnStrip->pacp->pvtacp->pfnEvaluateAcp(aseg->pchnStrip->pacp.get(), reinterpret_cast<ALO*>(pstepguard), message->tLocal, pasega->svtLocal, static_cast<GRFEVAL>(0), nullptr, &velocityLocal);

            pstepguard->vTarget = -velocityLocal;

            glm::vec3 velocityWorld = pstepguard->matTarget * velocityLocal;
            pstepguard->pvtstepguard->pfnSetSoVelocityVec(pstepguard, &velocityWorld);
            break;
        }

        // End animated jump.
        case 423:
            pstepguard->fAnimatedJump = 0;
            break;

        case 424:
            pstepguard->fLockHeading = 1;
            break;

        case 425:
            pstepguard->fLockHeading = 0;
            break;

        case 426:
            pstepguard->fLockState = 1;
            break;

        case 427:
            pstepguard->fLockState = 0;
            break;

            // Configure freefall animation data.
        case 428:
        {
            pstepguard->fPathValid = 0;
            pstepguard->fFreefallAseg = 1;

            EA* peaJump = PeaFindAsegLabel(aseg, (OID)430);
            EA* peaApex = PeaFindAsegLabel(aseg, (OID)431);
            EA* peaLoop = PeaFindAsegLabel(aseg, (OID)432);
            EA* peaLanding = PeaFindAsegLabel(aseg, (OID)433);

            if (peaJump == nullptr)
                break;

            if (peaApex == nullptr || peaLanding == nullptr || aseg->pchnStrip == nullptr || aseg->pchnStrip->pacp == nullptr)
                break;

            EA* peaEnd = peaLoop != nullptr ? peaLoop : peaLanding;

            glm::vec3 posJump{};
            glm::vec3 posEnd{};

            aseg->pchnStrip->pacp->pvtacp->pfnEvaluateAcp(aseg->pchnStrip->pacp.get(), pstepguard, peaJump->tLocal, 0.0f, static_cast<GRFEVAL>(0), &posJump, nullptr);
            aseg->pchnStrip->pacp->pvtacp->pfnEvaluateAcp(aseg->pchnStrip->pacp.get(), pstepguard, peaEnd->tLocal, 0.0f, static_cast<GRFEVAL>(0), &posEnd, nullptr);

            pstepguard->dxFreefall = posEnd.x - posJump.x;

            if (aseg->svtMaster != 0.0f)
                pstepguard->dtFreefall = (peaEnd->tLocal - peaJump->tLocal) / aseg->svtMaster;

            break;
        }

        case 429:
            pstepguard->fFreefallAseg = 0;
            break;

        case 430:
            pstepguard->pvtstepguard->pfnDoStepguardFreefallJump(pstepguard);
            break;

        case 431:
            pstepguard->fFreefall = 1;
            break;

        case 433:
        {
            if (pstepguard->fFreefall == 0 || pstepguard->pasegaSgs == nullptr)
                break;

            EA* peaLoop = PeaFindAsegLabel(aseg, (OID)432);
            const float tSeek = peaLoop != nullptr ? peaLoop->tLocal : message->tLocal;
            const float svtSeek = peaLoop != nullptr ? 1.0f : 0.0f;

            SeekAsega(pstepguard->pasegaSgs, SEEK_Start, tSeek, svtSeek);
            break;
        }

        case 434:
            pstepguard->fAbsorbDamage = 1;
            break;

        case 435:
            pstepguard->fAbsorbDamage = 0;
            break;

        default:
            break;
        }

        break;
    }

    case MSGID_waypoint:
    {
        const auto* message = static_cast<const STEP_GUARD_WAYPOINT_MSG*>(pv);

        if (message->pasega != pstepguard->pasegaPatrol || message->pwaypoint == nullptr)
            break;

        pstepguard->pwaypoint = message->pwaypoint;
        pstepguard->pwaypoint->pvtlo->pfnSubscribeLoObject(reinterpret_cast<LO*>(pstepguard->pwaypoint), reinterpret_cast<LO*>(pstepguard));
        pstepguard->sgsNext = SGS_PatrolWaypoint;
        pstepguard->grfsgsc |= 1;

        WPS wps = WPS_Active;
        const OID oidSync = (OID)pstepguard->pwaypoint->oidSync;

        if (oidSync != OID_Nil && pstepguard->psgg != nullptr)
        {
            if (pstepguard->psgg->oidSync == OID_Nil)
                pstepguard->psgg->oidSync = oidSync;

            if (pstepguard->psgg->oidSync == oidSync)
                wps = WPS_Sync;
        }

        SetWaypointWps(pstepguard->pwaypoint, wps);
        break;
    }

    case MSGID_waypoint_transition:
    {
        const auto* message = static_cast<const STEP_GUARD_WAYPOINT_TRANSITION_MSG*>(pv);

        if (message->pwaypoint != pstepguard->pwaypoint)
            break;

        if (message->transition == 0)
        {
            pstepguard->sgsNext = SGS_Patrol;
            pstepguard->grfsgsc |= 1;
        }
        else if (message->transition == 2 && pstepguard->sgs == SGS_PatrolWaypoint)
        {
            OnStepguardWaypointActive(pstepguard);
        }

        break;
    }

        case MSGID_damaged:
        {
            const auto* message = static_cast<const STEP_GUARD_DAMAGED_MSG*>(pv);

            SO* psoSource = message->psoSource;
            SO* psoAmmo = message->psoAmmo;

            if (psoSource == nullptr || (psoSource->pvtlo->grfcid & 2U) == 0)
                psoSource = nullptr;

            if (psoAmmo == nullptr || (psoAmmo->pvtlo->grfcid & 2U) == 0)
                psoAmmo = nullptr;

            if (!FCanStepguardTauntDamage(pstepguard, psoSource, psoAmmo))
                break;

            const glm::vec3 sourcePosition = psoSource->xf.posWorld;
            const float guardDistance = glm::length(pstepguard->xf.posWorld - sourcePosition);

            LO* nearbyObjects[32]{};

            const int objectCount = CploFindSwObjectsByClass(pstepguard->psw, static_cast<GRFFSO>(5), CID_STEPGUARD, nullptr, 32, nearbyObjects);

            int eligibleGuardCount = 0;
            int closerGuardCount = 0;

            for (int i = 0; i < objectCount; ++i)
            {
                STEPGUARD* other = reinterpret_cast<STEPGUARD*>(nearbyObjects[i]);

                if (other == nullptr || other == pstepguard || !FCanStepguardTauntDamage(other, psoSource, psoAmmo))
                    continue;

                ++eligibleGuardCount;

                const float otherDistance = glm::length(other->xf.posWorld - sourcePosition);

                if (otherDistance < guardDistance)
                    ++closerGuardCount;
            }

            if (closerGuardCount == 0)
            {
                pstepguard->tTaunt = g_clock.t + pstepguard->lmDtTaunt.gMin;

            }
            else
            {
                const float uMin = static_cast<float>(closerGuardCount - 1) / static_cast<float>(eligibleGuardCount);
                const float uMax = static_cast<float>(closerGuardCount) / static_cast<float>(eligibleGuardCount);
                const float u = GRandInRange(uMin, uMax);

                pstepguard->tTaunt = g_clock.t + glm::mix(pstepguard->lmDtTaunt.gMin, pstepguard->lmDtTaunt.gMax, u);

            }

            break;
        }

        default:
        break;
    }
}

SGAS SgasGetStepguard(STEPGUARD* pstepguard)
{
    return SGAS_Force;
}

int FCanStepguardAttack(STEPGUARD* pstepguard)
{
    SO* psoEnemy = PsoEnemyStepguard(pstepguard);

    if (psoEnemy == nullptr)
        return 0;

    if ((psoEnemy->pvtpo->grfcid & 8U) != 0 &&
        psoEnemy->pvtpo->pfnJthsCurrentPo((PO*)psoEnemy) != JTHS_Normal)
        return 0;

    float sEnemy = glm::length(pstepguard->xf.posWorld - psoEnemy->xf.posWorld);

    if (!FCheckLm(&pstepguard->lmAttackS, sEnemy))
        return 0;

    float dradEnemy = DradEnemyStepguard(pstepguard);
    float uDrad = s_clqSdistToUdrad.g0 + sEnemy * (s_clqSdistToUdrad.g1 + sEnemy * s_clqSdistToUdrad.g2);

    uDrad = glm::clamp(uDrad, 0.0f, 1.0f);

    if (uDrad * fabsf(dradEnemy) > pstepguard->radAttackMax)
        return 0;

    if (g_clock.t < pstepguard->tAttackNext)
        return 0;

    if (pstepguard->prwm != nullptr && !FEnsureRwmLoaded(pstepguard->prwm))
        return 0;

    return 1;
}

void RenderStepguardSelf(STEPGUARD* pstepguard, CM* pcm, RO* pro)
{
    RenderStepSelf(pstepguard, pcm, pro);
}

int FValidSgs(SGS sgs)
{
    return sgs < 17;
}

void UseStepguardAnimation(STEPGUARD* pstepguard, SGS sgs, OID oidAseg)
{
    int fvalid = FValidSgs(sgs);

    if (fvalid != 0) 
        pstepguard->mpsgssga[sgs].oidAseg = oidAseg;
}

void UseStepguardAnimationImmediate(STEPGUARD* pstepguard, SGS sgs, OID oidAseg)
{
    if (!FValidSgs(sgs)) {
        return;
    }

    SGA* psga = &pstepguard->mpsgssga[sgs];

    psga->oidAseg = oidAseg;

    if (oidAseg == OID_Nil) {
        psga->paseg = nullptr;
    }
    else {
        psga->paseg = PasegFindStepguard(pstepguard, oidAseg);
    }
}

void UseStepguardDeathAnimation(STEPGUARD* pstepguard, GRFTAK grftak, OID oidAseg)
{
    int csgda = pstepguard->csgda;

    if (csgda < 4) 
    {
        pstepguard->csgda = csgda + 1;
        pstepguard->asgda[csgda].oidAseg = oidAseg;
        pstepguard->asgda[csgda].grftak = grftak;
    }
}

ASEG* PasegFindStepguard(STEPGUARD* pstepguard, OID oidAseg)
{
    ASEG *paseg = (ASEG*)PloFindSwObject(pstepguard->psw, 257, oidAseg, pstepguard->pstepguardBase);

    if (paseg != nullptr) 
    {
        paseg->fDefault = 0;
        StripAsegAlo(paseg, (ALO*)pstepguard);
        SnipLo(paseg);
    }
    return paseg;
}

ASEG* GetStepguardSgsAseg(STEPGUARD* pstepguard, SGS sgs)
{
    if (pstepguard == nullptr || !FValidSgs(sgs))
        return nullptr;

    return pstepguard->mpsgssga[sgs].paseg;
}

void LoadStepguardAnimations(STEPGUARD* pstepguard)
{
    //
    // Per-state animations
    //
    for (int i = 0; i < 17; i++) {
        SGA* psga = &pstepguard->mpsgssga[i];

        if (psga->oidAseg != OID_Nil) 
        {
            psga->paseg = PasegFindStepguard(pstepguard, psga->oidAseg);
            psga->oidAseg = OID_Nil;
        }
    }

    //
    // Dynamic animation overrides
    //
    for (int i = 0; i < pstepguard->csgda; i++) {
        SGDA* psgda = &pstepguard->asgda[i];

        if (psgda->oidAseg != OID_Nil) 
        {
            psgda->paseg = PasegFindStepguard(pstepguard, psgda->oidAseg);
            psgda->oidAseg = OID_Nil;
        }
    }

}

void UseStepguardExpl(STEPGUARD* pstepguard, OID oidExpl)
{
    pstepguard->oidExpl = oidExpl;
    pstepguard->fExplExplicit = 1;
}

void UseStepguardRwm(STEPGUARD* pstepguard, OID oidRwm)
{
    pstepguard->oidRwm = oidRwm;
    pstepguard->fRwmExplicit = 1;
}

void UseStepguardPhys(STEPGUARD* pstepguard, SGS sgs, OID oidPhys)
{
    int fValid = FValidSgs(sgs);

    if (fValid != 0) 
        pstepguard->mpsgssgp[sgs].oidPhys = oidPhys;
}

void LoadStepguardPhys(STEPGUARD* pstepguard)
{
    for (int i = 0; i < 17; i++) 
    {
        SGP* psgp = &pstepguard->mpsgssgp[i];

        if (psgp->oidPhys == OID_Nil) 
            continue;

        psgp->psoPhys = (SO*)PloFindSwObject(pstepguard->psw, 257, psgp->oidPhys, pstepguard);

        if (psgp->psoPhys != nullptr) 
            SnipLo(psgp->psoPhys);
    }
}

void AddStepguardEffect(STEPGUARD* pstepguard, OID oidEffect, ZPK zpk)
{
    int csge = pstepguard->csge;

    if (csge < 4) 
    {
        pstepguard->csge = csge + 1;
        pstepguard->asge[csge].zpk = zpk;
        pstepguard->asge[csge].oidEffect = oidEffect;
    }
}

void SetStepguardPathzone(STEPGUARD* pstepguard, OID oidPathzone)
{
    PATHZONE* ppathzone;

    if (oidPathzone == OID_Nil) 
        ppathzone = nullptr;
    else 
    {
        ppathzone = (PATHZONE*)PloFindSwNearest(pstepguard->psw, oidPathzone, pstepguard);

        if (ppathzone == nullptr)
            return;
      
    }
    if (ppathzone != pstepguard->ppathzone)
        pstepguard->fPathValid = 0;

    pstepguard->ppathzone   = ppathzone;
    pstepguard->oidPathzone = oidPathzone;
}

void* GetStepguardPathzone(STEPGUARD* pstepguard)
{
    return &pstepguard->oidPathzone;
}

SO* PsoEnemyStepguard(STEPGUARD* pstepguard)
{
    SO* psoEnemy = pstepguard->psoEnemyBound;

    if (psoEnemy == nullptr) {
        if (pstepguard->fEnemyExternal == 0) {
            psoEnemy = (SO*)PpoCur();
        }
        else {
            psoEnemy = pstepguard->psoEnemyExternal;
        }
    }

    SO* psoSubscribed = (SO*)PpoCur();

    if (psoSubscribed == nullptr) {
        psoSubscribed = pstepguard->psoSubscribe;
    }
    else if (psoSubscribed == psoEnemy) {
        if ((psoEnemy)->pvtpo->pfnJthsCurrentPo((PO*)psoEnemy) == 2) {
            psoEnemy = nullptr;
        }

        psoSubscribed = pstepguard->psoSubscribe;
    }
    else {
        psoSubscribed = pstepguard->psoSubscribe;
    }

    if (psoEnemy != psoSubscribed) {
        if (psoSubscribed != nullptr) {
            psoSubscribed->pvtlo->pfnUnsubscribeLoObject((LO*)psoSubscribed, (LO*)pstepguard);
        }

        if (psoEnemy != nullptr) {
            psoEnemy->pvtlo->pfnSubscribeLoObject((LO*)psoEnemy, (LO*)pstepguard);
        }

        pstepguard->psoSubscribe = psoEnemy;
    }

    return psoEnemy;
}

SO* GetStepguardEnemy(STEPGUARD* pstepguard)
{
    return PsoEnemyStepguard(pstepguard);
}

void SetStepguardEnemyObject(STEPGUARD* pstepguard, SO* psoEnemy)
{
    pstepguard->psoEnemyExternal = psoEnemy;
    pstepguard->fEnemyExternal = 1;
}

void RebindStepguardEnemy(STEPGUARD* pstepguard)
{
    LO* aplo[32];
    int cplo = CploFindSwObjects(pstepguard->psw, 5, (OID)pstepguard->oidEnemyExternal, nullptr, 32, aplo);

    glm::vec3 stepguardPredictedPos = pstepguard->xf.pos + glm::vec3(pstepguard->xf.v.x, pstepguard->xf.v.y, 0.0f) * 0.34999999f;

    SO* psoBest = nullptr;
    float distBestSq = FLT_MAX;

    for (int i = 0; i < cplo; i++) {
        SO* pso = (SO*)aplo[i];

        glm::vec3 enemyPredictedPos = pso->xf.pos + glm::vec3(pso->xf.v.x, pso->xf.v.y, 0.0f) * 0.34999999f;

        float distSq = glm::length2(stepguardPredictedPos - enemyPredictedPos);

        // Bias toward keeping the current enemy if it is still close.
        if (pso == pstepguard->psoEnemyExternal) {
            distSq *= 0.5f;
        }

        if (distSq < distBestSq) {
            distBestSq = distSq;
            psoBest = pso;
        }
    }

    pstepguard->psoEnemyExternal = psoBest;
    pstepguard->fEnemyExternal = 1;
}

void SetStepguardEnemyBound(STEPGUARD* pstepguard, SO* pso)
{
    pstepguard->psoEnemyBound = pso;
}

void AdjustStepguardNewXp(STEPGUARD* pstepguard, XP* pxp, int ixpd)
{
    if (pstepguard->sgs == SGS_Dying || pstepguard->sgs == SGS_Stun) 
    {
        pxp->mu = MU_StepguardDying;
        pxp->elas = ELAS_StepguardDying;
        return;
    }

    if (!FCheckStepXpBase(pstepguard, pxp, ixpd)) 
    {
        pxp->elas = 0.0f;
        pxp->mu = 0.0f;
        return;
    }

    pxp->elas = 0.0f;
    pxp->mu = pstepguard->muContact;
}

void AdjustStepguardDz(STEPGUARD* pstepguard, GRFADJ grfadj, DZ* pdz, int ixpd, float dt)
{
    AdjustStepDz(pstepguard, grfadj, pdz, ixpd, dt);

    if ((((pstepguard->sgs == SGS_Dying) || (pstepguard->sgs == SGS_Stun)) && ((grfadj & 18U) == 18)) && (0.7 < (pdz->pxp->normal).z))
        pdz->sfMax = 0.0;
}

void SetStepguardAttackAngleMax(STEPGUARD* pstepguard, float degAttackMax)
{
    pstepguard->radAttackMax = degAttackMax * 0.017453294;
}

void* GetStepguardAttackAngleMax(STEPGUARD* pstepguard)
{
    return &pstepguard->radAttackMax;
}

void AddStepguardAlarm(STEPGUARD* pstepguard, ALARM* palarm)
{
    EnsureSggAlarm(pstepguard->psgg, palarm);
}

void MatchStepguardAnimationPhase(STEPGUARD* pstepguard, OID oid0, OID oid1, OID oid2, OID oid3)
{
    if (pstepguard->csgmap >= 8) 
        return;

    SGMAP* psgmap = &pstepguard->asgmap[pstepguard->csgmap++];

    psgmap->aoid[0] = oid0;
    psgmap->aoid[1] = oid1;
    psgmap->aoid[2] = oid2;
    psgmap->aoid[3] = oid3;
}

void AddStepguardCustomXps(STEPGUARD* pstepguard, SO* psoOther, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxp)
{
    if (pstepguard->sgs != SGS_Dying && pstepguard->sgs != SGS_Stun) {
        AddStepCustomXpsBase((STEP*)pstepguard, psoOther, pbspPruned, ppxp);
    }

    SO* psoEnemy = PsoEnemyStepguard(pstepguard);

    if (psoOther != psoEnemy) 
    {
        if (psoOther->paloParent != nullptr) 
            return;

        if (FIsBasicDerivedFrom(psoOther, CID_DECOY) == 0) 
            return;
    }

    for (int isge = 0; isge < pstepguard->csge; isge++) {
        SO* psoEffect = pstepguard->asge[isge].pso;

        if (FIsLoInWorld(psoEffect) == 0) 
            continue;

        if (psoOther->fSphere) 
            MarkSoContactsSphereBsp(psoOther, &psoOther->xf.posWorld, psoOther->sRadiusSelf, psoEffect, psoEffect->bspc.cbsp, psoEffect->bspc.absp.data(), nullptr, &pstepguard->pxpEffect);
        
        else if (abspPruned != nullptr) 
            MarkSoContactsBspBsp(psoEffect, psoOther, cbspPruned, abspPruned, pbspPruned, &pstepguard->pxpEffect);

    }
}

void ApplyStepguardThrow(STEPGUARD* pstepguard, PO* ppo)
{
    ppo->pvtalo->pfnSetAloVelocityVec(ppo, &pstepguard->vThrow);
 
    if (FIsBasicDerivedFrom(ppo, CID_JT) != 0) 
        SetJtJts((JT*)ppo, JTS_Zap, JTBS_Zap_DeadInPit);
}

void UpdateStepguardEffect(STEPGUARD* pstepguard)
{
    SO* enemy = PsoEnemyStepguard(pstepguard);

    if (enemy == nullptr)
        return;

    auto ClearEffects = [&]()
    {
        FreeSwXpList(pstepguard->psw, pstepguard->pxpEffect);
        pstepguard->pxpEffect = nullptr;
    };

    if (pstepguard->fIncapacitated != 0 || pstepguard->pxpEffect == nullptr)
    {
        ClearEffects();
        return;
    }

    // Prevent Sly's current attack contact from immediately triggering the
    // guard's attached effect objects.
    if (pstepguard->fBypassEffectHitSuppression == 0 && g_pjt != nullptr && (enemy == reinterpret_cast<SO*>(g_pjt) || enemy == pstepguard->psoEnemyBound))
    {
        TARGET* target = g_pjt->ptargetCur;

        if (target != nullptr && target->paloParent != nullptr && target->paloParent->paloRoot == pstepguard)
        {
            const bool recentSweep = g_pjt->jtcs > JTCS_SweepInit && g_pjt->jtcs < JTCS_Max && g_clock.t - g_pjt->tJtcs < 0.25f;
            const bool recentRush = g_pjt->jts == JTS_Rush && g_clock.t - g_pjt->tJts < 0.25f;
            const bool recentThrownJump = g_pjt->jtbs > JTBS_Jump_Thrown && g_pjt->jtbs < JTBS_Jump_Target && g_clock.t - g_pjt->tJtbs < 0.25f;

            if (recentSweep || recentRush || recentThrownJump)
            {
                ClearEffects();
                return;
            }
        }
    }

    for (XP* pxp = pstepguard->pxpEffect; pxp != nullptr; pxp = pxp->pxpNext)
    {
        const bool guardIsSide1 = pxp->axpd[0].psoRoot != pstepguard;
        const int guardSide = guardIsSide1 ? 1 : 0;
        const int otherSide = 1 - guardSide;

        SO* effectObject = pxp->axpd[guardSide].psoLeaf;
        SO* otherObject = pxp->axpd[otherSide].psoRoot;

        if (effectObject == nullptr || otherObject == nullptr)
            continue;

        SGE* effect = nullptr;

        for (int i = 0; i < pstepguard->csge; ++i)
        {
            if (pstepguard->asge[i].pso == effectObject && FIsLoInWorld(effectObject))
            {
                effect = &pstepguard->asge[i];
                break;
            }
        }

        if (effect == nullptr)
            continue;

        // Non-PO targets receive a normal work/damage record.
        if (!FIsBasicDerivedFrom(otherObject, CID_PO))
        {
            WKR wkr{};

            wkr.ploSource = pstepguard;
            wkr.ploTarget = otherObject;
            wkr.pos = pstepguard->xf.posWorld;
            wkr.grftak = 2;

            CalculateAloMovement(effectObject, nullptr, pxp->axpd[otherSide].pos, &wkr.v, nullptr, nullptr, nullptr);
            PaloAbsorbWkr(&wkr, 0, nullptr);
        }
        else
        {
            ZPR zpr{};
            InitZpr(&zpr, effect->zpk, pstepguard);

            if (zpr.zpk == ZPK_Blunt || (zpr.zpk == ZPK_Max && pstepguard->fFlying == 0))
            {
                glm::vec3 throwVelocity{};

                CalculateAloMovement(effectObject, nullptr, pxp->axpd[otherSide].pos, &throwVelocity, nullptr, nullptr, nullptr);

                throwVelocity *= R_StepguardElas + 1.0f;

                if (glm::length(throwVelocity) > SV_StepguardThrowJtMax)
                    throwVelocity = glm::normalize(throwVelocity) * SV_StepguardThrowJtMax;

                throwVelocity.z = std::max(throwVelocity.z, SV_StepguardThrowJtMin);

                pstepguard->vThrow = throwVelocity;
                zpr.pfnzap = (PFNZAP)ApplyStepguardThrow;
            }
            else if (zpr.zpk == ZPK_Max)
            {
                pstepguard->vThrow = glm::vec3(0.0f);
                zpr.pfnzap = (PFNZAP)ApplyStepguardThrow;
            }

            otherObject->pvtpo->pfnFTakePoDamage((PO*)otherObject, &zpr);
        }

        // The effect objects are one-shot and disappear after activation.
        for (int i = 0; i < pstepguard->csge; ++i)
        {
            if (pstepguard->asge[i].pso != nullptr)
                pstepguard->asge[i].pso->pvtlo->pfnRemoveLo(pstepguard->asge[i].pso);
        }

        break;
    }

    ClearEffects();
}

void SetStepguardPatrolAnimation(STEPGUARD* pstepguard, ASEG* pasegPatrol)
{
    if (pasegPatrol == pstepguard->pasegPatrol) {
        return;
    }

    if (pstepguard->pasegaPatrol != nullptr) {
        RetractAsega(pstepguard->pasegaPatrol);
        pstepguard->pasegaPatrol = nullptr;
    }

    pstepguard->pasegPatrol = pasegPatrol;

    if (SggsGetStepguard(pstepguard) == SGGS_Patrol) {
        pstepguard->sgsNext = SGS_Reset;
        pstepguard->grfsgsc |= 1;
    }

    if (pstepguard->pasegPatrol != nullptr) {
        FindAsegClosestPoint(pstepguard->pasegPatrol, pstepguard, &pstepguard->xf.posWorld, 0.0f, &pstepguard->tPatrolPrev, nullptr, nullptr);
    }
}

int FInflictStepguardZap(STEPGUARD* pstepguard, XP* pxp, ZPR* pzpr)
{
    return 1;
}

int FDetectStepguard(STEPGUARD* pstepguard)
{
    return 0;
}

int FCheckStepguardEnemyHidden(STEPGUARD* pstepguard)
{
    JT* pjt = (JT*)pstepguard->pvtstepguard->pfnPsoEnemyStepguard(pstepguard);

    if (!FIsBasicDerivedFrom((BASIC*)pjt, CID_JT)) 
        return 0;

    if (pjt->jts != JTS_Hide) 
        return 0;
    
    glm::vec3 dpos = pjt->xf.posWorld - pstepguard->xf.posWorld;

    // Use the release values here.  The proto JTBS table is shifted at the
    // hide substates, so its named Hide_Stand value is not valid here.
    switch ((int)pjt->jthk)
    {
        case 0:
        {
            if (pjt->phshape != nullptr && pjt->phshape->fDetect)
                return 0;
            

            if (pjt->phpnt != nullptr && pjt->phpnt->fDetect)
                return 0;
            

            glm::vec3 facing = pjt->xf.matWorld[0];

            return glm::dot(dpos, facing) < 0.0f;
        }

        case 1:
        {
            if (pjt->phshape != nullptr && pjt->phshape->fDetect)
                return 0;
            

            if (pjt->phpnt != nullptr && pjt->phpnt->fDetect) 
                return 0;
            

            glm::vec3 facing = pjt->xf.matWorld[0];

            return glm::dot(dpos, facing) > 0.0f;
        }

        case 2:
        return (int)pjt->jtbs == 16;

        case 3:
        return (GetAvailableVaultFlags() & 73728U) != 0;

        default:
        return 0;
    }
}

bool FStepguardLookLocked(STEPGUARD* pstepguard)
{
    if (pstepguard->fLockHeading)
        return true;

    if (pstepguard->fLockLook)
        return true;

    if (g_pjt == nullptr)
        return false;

    if (!g_pjt->fStun)
        return false;

    return true;
}

int GetStepguardSize()
{
	return sizeof(STEPGUARD);
}

void DeleteStepguard(STEPGUARD* pstepguard)
{
	delete pstepguard;
}

SGG* NewSgg()
{
    return new SGG{};
}

SGG* PsggNew(SW* psw)
{
    SGG* pvContext = (SGG*)PloNew(CID_SGG, psw, nullptr, (OID)1078, -1);
    PostSwCallback(g_psw, (PFNMQ)PostSggLoadCallback, pvContext, MSGID_callback, nullptr);
    return pvContext;
}

void InitSgg(SGG* psgg)
{
    InitLo(psgg);

    psgg->oidSync = OID_Nil;
}

int GetSggSize()
{
    return sizeof(SGG);
}

void CloneSgg(SGG* psgg, SGG* psggBase)
{
    CloneLo(psgg, psggBase);

    psgg->posEnemyLast = psggBase->posEnemyLast;

    // Copy SGGS (assuming it's trivially copyable)
    psgg->sggs = psggBase->sggs;

    // Copy simple float values
    psgg->tSggs = psggBase->tSggs;

    // Copy integer values
    psgg->cpstepguard = psggBase->cpstepguard;
    psgg->coidStepguard = psggBase->coidStepguard;
    psgg->cpxfmSearch = psggBase->cpxfmSearch;
    psgg->coidXfmSearch = psggBase->coidXfmSearch;
    psgg->cpalarm = psggBase->cpalarm;
    psgg->fCallback = psggBase->fCallback;

    // Copy arrays (assuming shallow copy for the pointers)
    for (int i = 0; i < 16; ++i)
    {
        psgg->apstepguard[i] = psggBase->apstepguard[i];
        psgg->aoidStepguard[i] = psggBase->aoidStepguard[i];
        psgg->apxfmSearch[i] = psggBase->apxfmSearch[i];
        psgg->aoidXfmSearch[i] = psggBase->aoidXfmSearch[i];
    }

    // Copy pointer members (assuming shallow copy is sufficient)
    for (int i = 0; i < 4; ++i)
    {
        psgg->apalarm[i] = psggBase->apalarm[i];
    }

    // Copy other pointer members
    psgg->oidSync = psggBase->oidSync;
    psgg->pexcAttack = psggBase->pexcAttack;

    // Copy detect flag
    psgg->fDetect = psggBase->fDetect;
}

void AddSggGuard(SGG* psgg, STEPGUARD* pstepguard)
{
    if (psgg->cpstepguard == 0)
        SetSggSggs(psgg, SGGS_Patrol);

    if (psgg->cpstepguard >= 16)
        return;

    psgg->apstepguard[psgg->cpstepguard++] = pstepguard;
}

void AddSggGuardName(SGG* psgg, OID oidGuard)
{
    if (psgg->coidStepguard < 16)
    {
        psgg->aoidStepguard[psgg->coidStepguard] = oidGuard;
        psgg->coidStepguard++;
    }
}

void AddSggSearchXfmName(SGG* psgg, OID oidXfm)
{
    if (psgg->coidXfmSearch < 16)
    {
        psgg->aoidXfmSearch[psgg->coidXfmSearch] = oidXfm;
        psgg->coidXfmSearch++;
    }
}

void RemoveSggGuard(SGG* psgg, STEPGUARD* pstepguard)
{
    int istepguard = -1;

    for (int i = 0; i < psgg->cpstepguard; i++)
    {
        if (psgg->apstepguard[i] == pstepguard)
        {
            istepguard = i;
            break;
        }
    }

    if (istepguard < 0)
        return;

    int cRemaining = psgg->cpstepguard - 1;

    for (int i = istepguard; i < cRemaining; i++)
        psgg->apstepguard[i] = psgg->apstepguard[i + 1];

    psgg->apstepguard[cRemaining] = nullptr;
    psgg->cpstepguard = cRemaining;

    if (psgg->cpstepguard < 1)
        SetSggSggs(psgg, SGGS_Dead);
}

void BindSgg(SGG* psgg)
{
    if (psgg == nullptr)
        return;

    for (int i = 0; i < psgg->coidStepguard; ++i)
    {
        LO* plo = PloFindSwNearest(g_psw, (OID)psgg->aoidStepguard[i], psgg);

        if (plo == nullptr || psgg->cpstepguard >= 16)
            continue;

        STEPGUARD* pstepguard = static_cast<STEPGUARD*>(plo);

        psgg->apstepguard[psgg->cpstepguard++] = pstepguard;
        pstepguard->psgg = psgg;
    }

    for (int i = 0; i < psgg->coidXfmSearch; ++i)
    {
        LO* plo = PloFindSwNearest(g_psw, (OID)psgg->aoidXfmSearch[i], psgg);

        if (plo == nullptr || psgg->cpxfmSearch >= 16)
            continue;

        psgg->apxfmSearch[psgg->cpxfmSearch++] = static_cast<XFM*>(plo);
    }

    PostSwCallback(psgg->psw, (PFNMQ)PostSggLoadCallback,
        psgg, MSGID_callback, nullptr);
}

void PostSggLoadCallback(SGG* psgg, MSGID msgid, void* pv)
{
    struct StepguardWaypoints {
        int count;
        std::vector<WAYPOINT*> waypoints;
    };

    std::vector<StepguardWaypoints> awaypoints(psgg->cpstepguard);

    for (int i = 0; i < psgg->cpstepguard; ++i) {
        STEPGUARD* pstepguard = psgg->apstepguard[i];

        if (pstepguard->pasegPatrol != nullptr) {
            GetAsegWaypoints(pstepguard->pasegPatrol, reinterpret_cast<ALO*>(pstepguard), &awaypoints[i].count, awaypoints[i].waypoints);
        }
    }

    for (int i = 0; i < psgg->cpstepguard; ++i) {
        for (WAYPOINT* pwaypoint : awaypoints[i].waypoints) {
            if (pwaypoint == nullptr) {
                continue;
            }

            if (pwaypoint->pwpsg != nullptr) {
                continue;
            }

            pwaypoint->pwpsg = PwpsgNew();
            pwaypoint->pwpsg->psgg = psgg;
            AddWpsgWaypoint(pwaypoint->pwpsg.get(), pwaypoint);

            if (pwaypoint->oidSync == OID_Nil) {
                continue;
            }

            for (int j = i + 1; j < psgg->cpstepguard; ++j) {
                for (WAYPOINT* psyncWaypoint : awaypoints[j].waypoints) {
                    if (psyncWaypoint == nullptr) {
                        continue;
                    }

                    if (psyncWaypoint->oidSync != pwaypoint->oidSync) {
                        continue;
                    }

                    psyncWaypoint->pwpsg = pwaypoint->pwpsg;
                    AddWpsgWaypoint(pwaypoint->pwpsg.get(), psyncWaypoint);
                }
            }
        }
    }
}

void EnsureSggCallback(SGG* psgg)
{
    if ((psgg->sggs != SGGS_Dead) && (psgg->fCallback == 0)) {
        psgg->fCallback = 1;
        PostSwCallback(g_psw, (PFNMQ)UpdateSggCallback, psgg, MSGID_Nil, nullptr);
    }
}

SO* PsoEnemySgg(SGG* psgg)
{
    STEPGUARD* pstepguard = psgg->apstepguard[0];

    return pstepguard->pvtstepguard->pfnPsoEnemyStepguard(pstepguard);
}

void UpdateSggCallback(SGG* psgg, MSGID msgid, void* pv)
{
    if (psgg == nullptr)
        return;

    psgg->fCallback = false;

    if (psgg->sggs == SGGS_Dead)
        return;

    psgg->fDetect = FDetectSgg(psgg);

    while (true)
    {
        const SGGS sggsNext = SggsNextSgg(psgg);

        if (sggsNext == psgg->sggs)
            break;

        SetSggSggs(psgg, sggsNext);
    }
}

SGGS SggsNextSgg(SGG* psgg)
{
    if (psgg == nullptr)
        return SGGS_Dead;

    switch (psgg->sggs)
    {
        case SGGS_Patrol:
        {
            if (!psgg->fDetect)
                return SGGS_Patrol;

            for (int i = 0; i < psgg->cpstepguard; ++i)
            {
                STEPGUARD* pstepguard = psgg->apstepguard[i];

                if (pstepguard == nullptr || !FIsBasicDerivedFrom(pstepguard, CID_SMARTGUARD))
                    return SGGS_Attack;
            }

            StartSound((SFXID)63, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
            return SGGS_Attack;
        }

        case SGGS_Search:
        {
            if (psgg->fDetect)
                return SGGS_Attack;

            if (FAbandonSggSearch(psgg))
                return SGGS_Patrol;

            return SGGS_Search;
        }

        case SGGS_Attack:
        {
            if (!psgg->fDetect)
                return SGGS_Search;

            return SGGS_Attack;
        }

        default:
        return psgg->sggs;
    }
}

int FDetectSgg(SGG* psgg)
{
    for (int i = 0; i < psgg->cpstepguard; ++i)
    {
        STEPGUARD* pstepguard = psgg->apstepguard[i];

        if (pstepguard == nullptr || pstepguard->pvtstepguard->pfnFDetectStepguard == nullptr)
            continue;

        const int fDetect = pstepguard->pvtstepguard->pfnFDetectStepguard(pstepguard);

        if (!fDetect)
            continue;

        SO* psoEnemy = PsoEnemySgg(psgg);

        if (psoEnemy != nullptr)
            psgg->posEnemyLast = psoEnemy->xf.posWorld;

        return 1;
    }

    return 0;
}

int FAbandonSggSearch(SGG* psgg)
{
    int fAbandon = 1;

    for (STEPGUARD* pstepguard : psgg->apstepguard)
    {
        if (pstepguard == nullptr ||
            pstepguard->pvtstepguard->pfnSgasGetStepguard == nullptr)
        {
            continue;
        }

        const int sgas = pstepguard->pvtstepguard->pfnSgasGetStepguard(pstepguard);

        if (sgas == 2)
            return 1;

        if (sgas == 0)
            fAbandon = 0;
    }

    return fAbandon;
}

void EnsureSggAlarm(SGG* psgg, ALARM* palarm)
{
    for (int i = 0; i < psgg->cpalarm; ++i)
    {
        if (psgg->apalarm[i] == palarm)
            return;
    }

    if (psgg->cpalarm >= 4)
        return;

    psgg->apalarm[psgg->cpalarm++] = palarm;
}

void TriggerSggAlarms(SGG* psgg, ALTK altk)
{
    for (int i = 0; i < psgg->cpalarm; ++i)
    {
        ALARM* palarm = psgg->apalarm[i];

        if (palarm != nullptr)
            TriggerAlarm(palarm, altk);
    }
}

void SetSggSggs(SGG* psgg, SGGS sggs)
{
    const SGGS sggsPrevious = psgg->sggs;

    // Retail does no exit/entry work when the requested group state is
    // already active.  Besides avoiding duplicate alarm transitions, this
    // preserves the timers established when the state was first entered.
    if (sggsPrevious == sggs)
        return;

    // Perform cleanup associated with leaving the current state.
    switch (sggsPrevious)
    {
        case SGGS_Search:
        {
            for (int i = 0; i < psgg->cpstepguard; ++i)
            {
                STEPGUARD* pstepguard = psgg->apstepguard[i];

                if (pstepguard == nullptr)
                    continue;

                const float dtGiveUp = GRandInRange(g_lmDtGiveUpSearch.gMin, g_lmDtGiveUpSearch.gMax);
                pstepguard->tGiveUpSearch = g_clock.t + dtGiveUp;
            }

            break;
        }

        case SGGS_Attack:
        {
            if (psgg->pexcAttack != nullptr)
            {
                UnsetExcitementHyst(psgg->pexcAttack);
                psgg->pexcAttack = nullptr;
            }

            for (int i = 0; i < psgg->cpstepguard; ++i)
            {
                STEPGUARD* pstepguard = psgg->apstepguard[i];

                if (pstepguard == nullptr)
                    continue;

                const float dtLostPlayer = GRandInRange(g_lmDtLostPlayer.gMin, g_lmDtLostPlayer.gMax);
                pstepguard->tLostPlayer = g_clock.t + dtLostPlayer;
            }

            break;
        }

        default:
        break;
    }

    // Perform setup associated with entering the new state.
    switch (sggs)
    {
        case SGGS_Patrol:
        TriggerSggAlarms(psgg, ALTK_Untrigger);
        break;

        case SGGS_Search:
        AssignSggSearchPoints(psgg);
        break;

        case SGGS_Attack:
        {
            psgg->pexcAttack = PexcSetExcitement(107);
            TriggerSggAlarms(psgg, ALTK_Trigger);

            for (int i = 0; i < psgg->cpstepguard; ++i)
            {
                STEPGUARD* pstepguard = psgg->apstepguard[i];

                if (pstepguard == nullptr)
                    continue;

                bool fDetect = false;

                if (pstepguard->pvtstepguard->pfnFDetectStepguard != nullptr)
                    fDetect = pstepguard->pvtstepguard->pfnFDetectStepguard(pstepguard) != 0;

                if (fDetect)
                {
                    pstepguard->tStopPatrol = g_clock.t;
                }
                else
                {
                    const float dtDiscover = GRandInRange(g_lmDtDiscover.gMin, g_lmDtDiscover.gMax);
                    pstepguard->tStopPatrol = g_clock.t + dtDiscover;
                }
            }

            break;
        }

        case SGGS_Dead:
        TriggerSggAlarms(psgg, ALTK_Disable);
        break;

        default:
        break;
    }

    psgg->sggs = sggs;
    psgg->tSggs = g_clock.t;
}

void AssignSggSearchPoints(SGG* psgg)
{
    if (psgg->cpstepguard <= 0)
        return;

    std::vector<bool> afXfmUsed(psgg->cpxfmSearch, false);

    const int cAssignedMax = std::min(psgg->cpstepguard, psgg->cpxfmSearch);
    int ipstepguard = 0;

    // Assign configured search transforms nearest to the enemy's last position.
    for (; ipstepguard < cAssignedMax; ++ipstepguard)
    {
        int ipxfmBest = -1;
        float sBestSq = std::numeric_limits<float>::max();

        for (int ipxfm = 0; ipxfm < psgg->cpxfmSearch; ++ipxfm)
        {
            if (afXfmUsed[ipxfm])
                continue;

            glm::vec3 posSearch;
            GetXfmPos(psgg->apxfmSearch[ipxfm], &posSearch);

            const glm::vec3 dpos = posSearch - psgg->posEnemyLast;
            const float sSq = glm::dot(dpos, dpos);

            if (sSq < sBestSq)
            {
                sBestSq = sSq;
                ipxfmBest = ipxfm;
            }
        }

        if (ipxfmBest < 0)
            break;

        afXfmUsed[ipxfmBest] = true;

        STEPGUARD* pstepguard = psgg->apstepguard[ipstepguard];
        XFM* pxfmSearch = psgg->apxfmSearch[ipxfmBest];

        glm::mat3 matSearch;

        GetXfmPos(pxfmSearch, &pstepguard->posSearch);
        GetXfmMat(pxfmSearch, &matSearch);

        pstepguard->radTargetSearch = std::atan2(matSearch[0].y, matSearch[0].x);
    }

    if (ipstepguard >= psgg->cpstepguard)
        return;

    constexpr float kTwoPi = 6.2831855f;
    constexpr float kInvTwoPi = 0.15915494f;

    const float dRadSearch = kTwoPi / static_cast<float>(psgg->cpstepguard);
    const float sSearch = static_cast<float>(psgg->cpstepguard) * S_StepguardSearchSpacing * kInvTwoPi;

    float radSearch = 0.0f;

    if (SO* psoEnemy = PsoEnemySgg(psgg))
        radSearch = std::atan2(psoEnemy->xf.v.y, psoEnemy->xf.v.x);

    // Generate fallback search positions around the last known enemy position.
    for (; ipstepguard < psgg->cpstepguard; ++ipstepguard)
    {
        STEPGUARD* pstepguard = psgg->apstepguard[ipstepguard];

        const float uRad = GRandInRange(g_lmUDradSearch.gMin, g_lmUDradSearch.gMax);
        const float radTarget = RadNormalize(radSearch + uRad * dRadSearch);
        const float uRadius = GRandInRange(g_lmUSSearch.gMin, g_lmUSSearch.gMax);

        glm::vec3 dposSearch;
        SetVectorCylind(&dposSearch, radTarget, uRadius * sSearch, 0.0f);

        pstepguard->radTargetSearch = radTarget;
        pstepguard->posSearch = psgg->posEnemyLast + dposSearch;

        radSearch = RadNormalize(radSearch + dRadSearch);
    }
}

void DeleteSgg(SGG* psgg)
{
    delete psgg;
}

SNIP s_asnipStepguardLoad[1] = {2, (OID)0x195, offsetof(STEPGUARD, mpsgssgp[16].psoPhys)};
SNIP s_asnipPostStepguardLoad[2] = 
{
    2, (OID)0x194, offsetof(STEPGUARD, pasegPatrol),
    6, (OID)0x50,  offsetof(STEPGUARD, paloHead)
};

float MU_StepguardDying = 0.85;
float ELAS_StepguardDying = 0.0;
float MU_StepguardBase = 1.25;
CLQ s_clqDradToUFly  = { 1.0, -1.0, 0.0 };
CLQ s_clqStunSpring  = { 0.0, 15.0, 1.5 };
CLQ s_clqStunDamping = { 0.0, 2.5, 0.25 };
CLQ s_clqUdtToU = {0.0, 0.0, 1.0};
CLQ s_clqSdistToUdrad = {0.0, 0.0049999999, 0.0};
glm::vec3 kDefaultHitNormal = { 1.0, 0.0, 0.0 };
LM g_lmDtGiveUpSearch = { 0.0, 4.0 };
LM g_lmDtLostPlayer = {0.0, 0.3};
LM g_lmDtDiscover = {0.0, 1.5};
float S_StepguardSearchSpacing = 700;
LM g_lmUDradSearch = {-0.2, 0.2};
LM g_lmUSSearch = { 0.8, 1.2 };
float R_StepguardElas = 0.25;
float SV_StepguardThrowJtMin = 600;
float SV_StepguardThrowJtMax = 1200;
CLQ s_clqSToUTilt = {1.0, -0.001, 0.0};
LM  s_lmUTiltDz = {0.0, 1.0};
float RAD_StepguardTiltMax = 1.0;
LM s_lmUTiltS = {0.25, 1.0};
CLQ s_clqDzToUTilt = {0.0, 0.0049999999, 0.0};
