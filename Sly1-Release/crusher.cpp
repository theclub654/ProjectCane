#include "crusher.h"
#include "jt.h"
#include "path.h"
#include "blip.h"
#include "find.h"
#include "rwm.h"
#include "asega.h"
#include "sm.h"
#include "difficulty.h"
#include "puffcharger.h"
#include "timer.h"

static SNIP s_asnipPostCrfodkSpawnerLoad[3] =
{
    { 5, (OID)0x3f8, offsetof(CRFODKSPAWN, prwmRespawn) },
    { 5, (OID)0x3f9, offsetof(CRFODKSPAWN, prwmSpawn) },
    { 1, (OID)0x3fa, offsetof(CRFODKSPAWN, psm) }
};

static float EvalUnitClq(const CLQ& clq, float x)
{
    return glm::clamp(clq.g0 + x * (clq.g1 + x * clq.g2), 0.0f, 1.0f);
}

static float DistanceBetweenSegments(
    const glm::vec3& p0,
    const glm::vec3& p1,
    const glm::vec3& q0,
    const glm::vec3& q1)
{
    const glm::vec3 d0 = p1 - p0;
    const glm::vec3 d1 = q1 - q0;
    const glm::vec3 r = p0 - q0;
    const float a = glm::dot(d0, d0);
    const float e = glm::dot(d1, d1);
    const float f = glm::dot(d1, r);
    float s = 0.0f;
    float t = 0.0f;

    if (a <= 0.0001f && e <= 0.0001f)
        return glm::length(p0 - q0);

    if (a <= 0.0001f) {
        t = glm::clamp(f / e, 0.0f, 1.0f);
    }
    else {
        const float c = glm::dot(d0, r);
        if (e <= 0.0001f) {
            s = glm::clamp(-c / a, 0.0f, 1.0f);
        }
        else {
            const float b = glm::dot(d0, d1);
            const float denom = a * e - b * b;
            if (fabsf(denom) > 0.0001f)
                s = glm::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            t = (b * s + f) / e;
            if (t < 0.0f) {
                t = 0.0f;
                s = glm::clamp(-c / a, 0.0f, 1.0f);
            }
            else if (t > 1.0f) {
                t = 1.0f;
                s = glm::clamp((b - c) / a, 0.0f, 1.0f);
            }
        }
    }

    return glm::length((p0 + d0 * s) - (q0 + d1 * t));
}

void StartupCrusherCtr(CRUSHERCTR* pcrusherctr)
{
	pcrusherctr->pvtcrusherctr = &g_vtcrusherctr;
}

float DtVisibleCrusherctr(CRUSHERCTR* pcrusherctr)
{
	return 0.0f;
}

CRFODB* NewCrfodb()
{
	CRFODB* pcrfodb = new CRFODB{};
    pcrfodb->fKillReported = false;
    return pcrfodb;
}

void InitCrfodb(CRFODB* pcrfodb)
{
	InitStepGuard(pcrfodb);
	pcrfodb->cpoint = 1;
    s_icrfodbNext = 0;
}

void SetCrfodbCpoint(CRFODB* pcrfodk, int cpoint)
{
    pcrfodk->cpoint = cpoint;
}

void* GetCrfodbCpoint(CRFODB* pcrfodk)
{
    return &pcrfodk->cpoint;
}

int GetCrfodbSize()
{
	return sizeof(CRFODB);
}

void CloneCrfodb(CRFODB* pcrfodb, CRFODB* pcrfodbBase)
{
	CloneCrfod(pcrfodb, pcrfodbBase);

	pcrfodb->fDetectLatch = pcrfodbBase->fDetectLatch;
	pcrfodb->cpoint = pcrfodbBase->cpoint;
	pcrfodb->pcrfodkSlot = nullptr;
	pcrfodb->fKillReported = false;
}

void PostCrfodbLoad(CRFODB* pcrfodb)
{
	PostStepguardLoad(pcrfodb);
    pcrfodb->icrfodb = s_icrfodbNext++;
}

void UpdateCrfodbGoal(CRFODB* pcrfodb, int fEnter)
{
    if (pcrfodb->sgs == SGS_ReturnToPatrol) {
        if (fEnter)
            SetStepguardGoal(pcrfodb, &pcrfodb->xf.posWorld);
        return;
    }

    if (pcrfodb->sgs < SGS_Pursue || pcrfodb->sgs > SGS_PursueIdle) 
    {
        UpdateStepguardGoal(pcrfodb, fEnter);
        return;
    }

    glm::vec3 posBest(0.0f);
    float uPenaltyBest = FLT_MAX;
    float radTry = pcrfodb->radTarget;
    float dradTry = 2.0f * 6.2831855 / (float)CRAD_TryMax;

    for (int i = 0; i < CRAD_TryMax; i++) 
    {
        glm::vec3 vHead;
        glm::vec3 posEnd;
        float uPenalty = 0.0f;

        CalcHeadingVector(radTry, &vHead);
        posEnd = pcrfodb->xf.posWorld + vHead * DS_PredictCrfodb;

        if (pcrfodb->ppathzone != nullptr) 
        {
            // ClsgClipEdgeToCbsp may emit as many records as clsgMax.
            // The original function reserves an eight-element LSG work area.
            LSG alsg[8]{};
            float uClip = 1.0f;
            int clsg = ClsgClipEdgeToCbsp(
                pcrfodb->ppathzone->cg.acbsp.data(),
                &pcrfodb->xf.posWorld,
                &posEnd,
                8,
                alsg);

            if (clsg != 0) {
                uClip = alsg[0].au[1];

                if (clsg == 1 && alsg[0].au[0] != 0.0f && alsg[0].au[1] != 1.0f)
                    uClip = alsg[0].au[0];
            }

            uPenalty += U_PenaltyCbsp * glm::clamp((U_PenaltyCbspIgnore - uClip) / U_PenaltyCbspIgnore, 0.0f, 1.0f);
        }

        PO* ppo = PpoCur();
        if (ppo != nullptr) 
        {
            glm::vec3 posEndOther = ppo->xf.posWorld + ppo->xf.v * DT_PredictCrfodb;
            const float sCrusher = DistanceBetweenSegments(
                pcrfodb->xf.posWorld, posEnd, ppo->xf.posWorld, posEndOther);

            float uClose = sCrusher / (R_DistCrusher * (ppo->sRadiusAll + pcrfodb->sRadiusAll));
            uPenalty += U_PenaltyCrusher * (1.0f - glm::clamp(uClose, 0.0f, 1.0f));
        }

        for (CRFOD* pcrfodOther = pcrfodb->psw->dlCrfod.pcrfodFirst; pcrfodOther != nullptr; pcrfodOther = pcrfodOther->dleCrfod.pcrfodNext) 
        {
            if (pcrfodOther == (CRFOD*)pcrfodb)
                continue;

            glm::vec3 posEndOther = pcrfodOther->xf.posWorld + pcrfodOther->xf.v * DT_PredictCrfodb;
            const float sOther = DistanceBetweenSegments(
                pcrfodb->xf.posWorld, posEnd, pcrfodOther->xf.posWorld, posEndOther);

            float uClose = sOther / (R_DistCrfod * (pcrfodOther->sRadiusAll + pcrfodb->sRadiusAll));
            uPenalty += U_PenaltyCrfod * (1.0f - glm::clamp(uClose, 0.0f, 1.0f));
        }

        float drad = RadNormalize(radTry - pcrfodb->radTarget);
        uPenalty += U_PenaltyRad * fabsf(drad * (1.0f / 6.2831855)) * U_RadWorst;

        if (uPenalty < uPenaltyBest) 
        {
            posBest = posEnd;
            uPenaltyBest = uPenalty;
        }

        radTry = RadNormalize(radTry + dradTry);
    }

    SetStepguardGoal(pcrfodb, &posBest);
}

int FDetectCrfodb(CRFODB* pcrfodb)
{
    if (FSwHandsOff(pcrfodb->psw)) 
        return false;

    if (pcrfodb->sgs == SGS_Dying) 
        return false;

    SGS sgsIgnore = (pcrfodb->sgs > SGS_Dying)
        ? SGS_Stun
        : SGS_ReturnToPatrol;

    if (pcrfodb->sgs == sgsIgnore) {
        return false;
    }

    SO* psoTarget = pcrfodb->pvtcrfodb->pfnPsoEnemyStepguard(pcrfodb);
    if (psoTarget == nullptr) {
        return false;
    }

    glm::vec2 dpos = glm::vec2(pcrfodb->xf.posWorld - psoTarget->xf.posWorld);
    float dist = glm::length(dpos);

    if (!pcrfodb->fDetectLatch) {
        if (dist < 900.0) {
            glm::vec2 vTarget = glm::vec2(psoTarget->xf.v);
            glm::vec2 dirToCrfodb = dpos;

            if (glm::length(vTarget) >= 0.0001f &&
                glm::length(dirToCrfodb) >= 0.0001f)
            {
                vTarget = glm::normalize(vTarget);
                dirToCrfodb = glm::normalize(dirToCrfodb);

                if (glm::dot(vTarget, dirToCrfodb) > 0.3f) {
                    pcrfodb->fDetectLatch = true;
                }
            }
        }
    }
    else {
        if (dist > 1200.0) {
            pcrfodb->fDetectLatch = false;
        }
    }

    return pcrfodb->fDetectLatch;
}

int FAbsorbCrfodbWkr(CRFODB* pcrfodb, WKR* pwkr)
{
    if (pcrfodb->sgs == SGS_Stun)
        return 1;

    if (pcrfodb->sgs == SGS_Dying)
        return 1;

    if (!FAbsorbStepguardWkr(pcrfodb, pwkr))
        return 0;

    pcrfodb->fDetectLatch = 0;
    return 1;
}

void DeleteCrfodb(CRFODB* pcrfodb)
{
	delete pcrfodb;
}

CRFOD* NewCrfod()
{
	return new CRFOD{};
}

void InitSwCrfodDl(SW* psw)
{
	InitDl(&psw->dlCrfod, offsetof(CRFOD, dleCrfod));
}

void OnCrfodAdd(CRFOD* pcrfod)
{
	OnStepguardAdd(pcrfod);
	AppendDlEntry(&pcrfod->psw->dlCrfod, pcrfod);
}

void OnCrfodRemove(CRFOD* pcrfod)
{
	// A spawned chicken can be removed immediately after its death animation
	// without delivering the expected state callback.  Preserve the original
	// spawned-object notification at this guaranteed lifecycle point.
	if (FIsBasicDerivedFrom(pcrfod, CID_CRFODB)) {
		CRFODB* pcrfodb = static_cast<CRFODB*>(pcrfod);
		if (pcrfodb->sgs == SGS_Dying && !pcrfodb->fKillReported && g_pcrfodkSpawner != nullptr) {
			pcrfodb->fKillReported = true;
			OnCrfodkSpawnedDestroyed(g_pcrfodkSpawner, pcrfodb);
		}
	}

	OnStepguardRemove(pcrfod);
	RemoveDlEntry(&pcrfod->psw->dlCrfod, pcrfod);
}

void OnCrfodbEnteringSgs(CRFODB* pcrfodb, SGS sgsPrev, ASEG* pasegTargetOverride)
{
    OnStepguardEnteringSgs((STEPGUARD*)pcrfodb, sgsPrev, pasegTargetOverride);

    if (pcrfodb->sgs == SGS_Dying) {
        if (!pcrfodb->fKillReported && g_pcrfodkSpawner != nullptr) {
            pcrfodb->fKillReported = true;
            OnCrfodkSpawnedDestroyed(g_pcrfodkSpawner, pcrfodb);
        }
    }
    else if (pcrfodb->sgs == SGS_Pursue && g_pcrfodkSpawner != nullptr) {
        SetAsegaSpeed(pcrfodb->pasegaSgs, g_pcrfodkSpawner->anotherDifficulty);
    }

    if ((pcrfodb->sgs == SGS_Dying || pcrfodb->sgs == SGS_Pursue) && pcrfodb->pcrfodkSlot != nullptr) {
        pcrfodb->pcrfodkSlot->pasegaSpawn = nullptr;
        pcrfodb->pcrfodkSlot->pcrfodb = nullptr;
        pcrfodb->pcrfodkSlot = nullptr;
    }
}

void CloneCrfod(CRFOD* pcrfod, CRFOD* pcrfodBase)
{
	CloneStepguard(pcrfod, pcrfodBase);

	pcrfod->dleCrfod = pcrfodBase->dleCrfod;
}

int GetCrfodSize()
{
	return sizeof(CRFOD);
}

void DeleteCrfod(CRFOD* pcrfod)
{
	delete pcrfod;
}

CRFODK* NewCrfodk()
{
	CRFODK* pcrfodk = new CRFODK{};
	// Every ordinary chicken contributes one point unless its template
	// explicitly replaces this value while loading.
	pcrfodk->cKillValue = 1;
	pcrfodk->fKillReported = false;
	return pcrfodk;
}

int GetCrfodkSize()
{
	return sizeof(CRFODK);
}

void CloneCrfodk(CRFODK* pcrfodk, CRFODK* pcrfodkBase)
{
	CloneCrfod(pcrfodk, pcrfodkBase);
	pcrfodk->svtPursue = pcrfodkBase->svtPursue;
	pcrfodk->cKillValue = pcrfodkBase->cKillValue > 0 ? pcrfodkBase->cKillValue : 1;
	// This is a runtime association assigned by FireCrfodkIntoSlot; it must
	// never retain the template object's slot pointer.
	pcrfodk->pcrfodkSlot = nullptr;
	pcrfodk->fKillReported = false;
}

SGS SgsNextCrfodkAI(CRFODK* pcrfodk)
{
    SGS sgsNext = SgsNextStepguardAI(pcrfodk);

    if (sgsNext == SGS_Dying) {
        return SGS_Dying;
    }

    PO* ppo = PpoCur();

    if (ppo != nullptr && FIsSoTouching(pcrfodk, ppo)) 
    {
        ZPR zpr;

        sgsNext = SGS_Dying;

        InitZpr(&zpr, ZPK_Blunt, pcrfodk);
        ppo->pvtpo->pfnFTakePoDamage(ppo, &zpr);
    }

    for (CRFODK* pcrfodkOther = (CRFODK*)pcrfodk->psw->dlCrfod.psoFirst; pcrfodkOther != nullptr; pcrfodkOther = (CRFODK*)pcrfodkOther->dleCrfod.psoNext)
    {
        if (pcrfodkOther == pcrfodk) {
            continue;
        }

        if (!FIsBasicDerivedFrom(pcrfodkOther, CID_CRFODK)) {
            continue;
        }

        if (FIsSoTouching(pcrfodk, pcrfodkOther)) {
            SetStepguardSgs(pcrfodkOther, SGS_Dying, nullptr);
        }
    }

    return sgsNext;
}

int FDetectCrfodk(CRFODK* pcrfodk)
{
    if (FSwHandsOff(pcrfodk->psw))
        return false;

    if (pcrfodk->sgs == SGS_Dying || pcrfodk->sgs == SGS_Stun)
        return false;

    return true;
}

int FAbsorbCrfodkWkr(CRFODK* pcrfodk, WKR* pwkr)
{
    if (pcrfodk->sgs == SGS_Stun || pcrfodk->sgs == SGS_Dying) 
        return true;

    if (!FAbsorbStepguardWkr(pcrfodk, pwkr)) 
        return false;

    PO* ppo = PpoCur();
    PO* ppoSource = (PO*)pwkr->ploSource;

    if (ppoSource == ppo) 
    {
        ZPR zpr;

        InitZpr(&zpr, ZPK_Blunt, pcrfodk);
        ppoSource->pvtpo->pfnFTakePoDamage(ppoSource, &zpr);
    }

    return true;
}

void OnCrfodkEnteringSgs(CRFODK* pcrfodk, SGS sgsPrev, ASEG* pasegTargetOverride)
{
    OnStepguardEnteringSgs((STEPGUARD*)pcrfodk, sgsPrev, pasegTargetOverride);

    if (pcrfodk->sgs == SGS_Dying) {
        if (g_pcrfodkSpawner != nullptr) {
            NotifyCrfodkDied(g_pcrfodkSpawner);
        }

        if (pcrfodk->pcrfodkSlot != nullptr) {
            // Normally MSGID_asega_retracted releases this lock.  State entry
            // is a safe fallback for an animation that completed without the
            // subscription message reaching the spawner.
            pcrfodk->pcrfodkSlot->pasegaSpawn = nullptr;
            pcrfodk->pcrfodkSlot->pcrfodk = nullptr;
            pcrfodk->pcrfodkSlot = nullptr;
        }
    }
    else if (pcrfodk->sgs == SGS_Pursue) {
        if (pcrfodk->pcrfodkSlot != nullptr) {
            pcrfodk->pcrfodkSlot->pasegaSpawn = nullptr;
            pcrfodk->pcrfodkSlot->pcrfodk = nullptr;
            pcrfodk->pcrfodkSlot = nullptr;
        }

        SetAsegaSpeed(pcrfodk->pasegaSgs, pcrfodk->svtPursue);
    }
}

void DeleteCrfodk(CRFODK* pcrfodk)
{
	delete pcrfodk;
}

CRFODKSPAWN* NewCrfodkSpawn()
{
    return new CRFODKSPAWN{};
}

void InitCrfodkSpawn(CRFODKSPAWN* pspawn)
{
    InitAlo(pspawn);
    pspawn->cKillGoal = 50;
    pspawn->tGameMax = 90.0f;
    const CRFODKTUNE tune = {
        { 8.0f, 6.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.3f, 0.6f }, { 0.8f, 1.3f }, { 0.0f, 1.0f, 0.0f, 0.0f },
        0.3f, { 3.5f, 5.0f }, { 2.0f, 2.5f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.85f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.8f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }
    };
    pspawn->tuneNormal = tune;
    pspawn->tuneAssist = tune;
    pspawn->tuneAssist.lmRespawnDelayStart = { 12.0f, 12.0f };
    pspawn->tuneAssist.lmRespawnDelayEnd = { 12.0f, 12.0f };
    pspawn->clqSuckToTune = { 0.0f, 1.0f, 0.0f, 0.0f };
    pspawn->tRespawn = FLT_MAX;
    // The original leaves this at zero so state 1021 immediately performs
    // the first regular spawn.  FLT_MAX belongs only to tRespawn here.
    pspawn->tSpawnNext = 0.0f;
    pspawn->cRespawnMax = 2;
    g_pcrfodkSpawner = pspawn;
}

void CloneCrfodkSpawn(CRFODKSPAWN* pspawn, CRFODKSPAWN* pspawnBase)
{
    CloneAlo(pspawn, pspawnBase);
    pspawn->cKillGoal = pspawnBase->cKillGoal;
    pspawn->tGameMax = pspawnBase->tGameMax;
    pspawn->cslot = pspawnBase->cslot;
    std::copy(std::begin(pspawnBase->aslot), std::end(pspawnBase->aslot), std::begin(pspawn->aslot));
    pspawn->prwmRespawn = pspawnBase->prwmRespawn;
    pspawn->prwmSpawn = pspawnBase->prwmSpawn;
    pspawn->psm = pspawnBase->psm;
    pspawn->clqSuckToTune = pspawnBase->clqSuckToTune;
    pspawn->tuneNormal = pspawnBase->tuneNormal;
    pspawn->tuneAssist = pspawnBase->tuneAssist;
}

void DeleteCrfodkSpawn(CRFODKSPAWN* pspawn)
{
    if (g_pcrfodkSpawner == pspawn)
        g_pcrfodkSpawner = nullptr;
    delete pspawn;
}

void AddCrfodkSpawnSlot(CRFODKSPAWN* pspawn, OID oidSlot)
{
    if (pspawn->cslot < (int)std::size(pspawn->aslot))
        pspawn->aslot[pspawn->cslot++].oid = oidSlot;
}

CRFODKTUNE* PcrfodkSpawnerDifficultyData(CRFODKSPAWN* pspawn)
{
    return (&pspawn->tuneNormal) + glm::clamp(pspawn->iDifficultyData, 0, 2);
}

void RecalcCrfodkSpawnerDifficulty(CRFODKSPAWN* pspawn)
{
    const float progress = pspawn->cKillGoal > 0 ? glm::clamp((float)pspawn->cKilled / pspawn->cKillGoal, 0.0f, 1.0f) : 0.0f;
    CRFODKTUNE& t = pspawn->tuneCurrent;
    float u = EvalUnitClq(t.clqProgressToSpawnDelay, progress);
    pspawn->dtSpawnMin = glm::mix(t.lmSpawnDelayStart.gMin, t.lmSpawnDelayEnd.gMin, u);
    pspawn->dtSpawnMax = glm::mix(t.lmSpawnDelayStart.gMax, t.lmSpawnDelayEnd.gMax, u);
    u = EvalUnitClq(t.clqProgressToRespawnDelay, progress);
    pspawn->dtRespawnMin = glm::mix(t.lmRespawnDelayStart.gMin, t.lmRespawnDelayEnd.gMin, u);
    pspawn->dtRespawnMax = glm::mix(t.lmRespawnDelayStart.gMax, t.lmRespawnDelayEnd.gMax, u);
    u = EvalUnitClq(t.clqProgressToDifficulty, progress);
    pspawn->difficultyValue = glm::mix(t.lmDifficultyValue.gMin, t.lmDifficultyValue.gMax, u);
    u = EvalUnitClq(t.clqProgressToSecondary, progress);
    pspawn->anotherDifficulty = glm::mix(t.lmSecondaryDifficulty.gMin, t.lmSecondaryDifficulty.gMax, u);
    u = EvalUnitClq(t.clqProgressToActiveMax, progress);
    pspawn->cActiveMax = (int)glm::mix(t.lmActiveMax.gMin, t.lmActiveMax.gMax, u);
    if (!pspawn->fThresholdReached && t.uRespawnEnable < progress) {
        pspawn->fThresholdReached = 1;
        pspawn->tRespawn = g_clock.t;
    }
}

void PostCrfodkSpawnerLoad(CRFODKSPAWN* pspawn)
{
    PostAloLoad(pspawn);
    SnipAloObjects(pspawn, 3, s_asnipPostCrfodkSpawnerLoad);
    pspawn->psma = PsmaApplySm(pspawn->psm, nullptr, (OID)1019, 0);
    GetSmaCur(pspawn->psma, &pspawn->oidState);
    SubscribeSmaStruct(pspawn->psma, pspawn);
    for (int i = 0; i < pspawn->cslot; ++i) {
        CRFODKSLOT& slot = pspawn->aslot[i];
        const OID oid = slot.oid;
        slot.palo = static_cast<ALO*>(PloFindSwNearest(pspawn->psw, oid, pspawn));
        if (slot.palo != nullptr) {
            slot.pasegSpawn = static_cast<ASEG*>(PloFindSwChild(pspawn->psw, (OID)1015, slot.palo));
            slot.pxfmSpawn = static_cast<XFM*>(PloFindSwChild(pspawn->psw, (OID)1014, slot.palo));
        }
    }
    const float u = EvalUnitClq(pspawn->clqSuckToTune, g_plsCur->uSuck);
    const float* normal = reinterpret_cast<const float*>(&pspawn->tuneNormal);
    const float* assist = reinterpret_cast<const float*>(&pspawn->tuneAssist);
    float* current = reinterpret_cast<float*>(&pspawn->tuneCurrent);
    for (int i = 0; i < 44; ++i)
        current[i] = glm::mix(normal[i], assist[i], u);
    RecalcCrfodkSpawnerDifficulty(pspawn);
}

void NotifyCrfodkDied(CRFODKSPAWN* pspawner)
{
    if (pspawner->cActive == pspawner->cRespawnMax) {
        float dtRespawn = GRandInRange(pspawner->dtRespawnMin, pspawner->dtRespawnMax);
        pspawner->tRespawn = g_clock.t + dtRespawn;
    }

    if (pspawner->cActive > 0)
        --pspawner->cActive;
}

static bool FireCrfodkIntoSlot(CRFODKSPAWN* pspawn, RWM* prwm, CRFODKSLOT& slot)
{
    if (prwm == nullptr || slot.palo == nullptr || slot.pxfmSpawn == nullptr || slot.pasegSpawn == nullptr)
        return false;
    ReloadRwm(prwm);
    SO* pso = prwm->psoAmmo;
    if (pso == nullptr)
        return false;

    if (!FFireRwm(prwm, false))
        return false;

    glm::vec3 pos;
    glm::mat3 mat;
    GetXfmPos(slot.pxfmSpawn, &pos);
    GetXfmMat(slot.pxfmSpawn, &mat);
    pso->pvtalo->pfnTranslateAloToPos(pso, &pos);
    pso->pvtalo->pfnRotateAloToMat(pso, &mat);
    glm::vec3 zero(0.0f);
    pso->pvtalo->pfnSetAloVelocityVec(pso, &zero);
    pso->pvtalo->pfnSetAloAngularVelocityVec(pso, &zero);
    ApplyAseg(slot.pasegSpawn, slot.palo, 0.0f, 1.0f, 0, &slot.pasegaSpawn);
    if (slot.pasegaSpawn != nullptr)
        SubscribeAsegaObject(slot.pasegaSpawn, pspawn);
    if (FIsBasicDerivedFrom(pso, CID_CRFODK)) {
        CRFODK* pcrfodk = static_cast<CRFODK*>(pso);
        pcrfodk->pcrfodkSlot = &slot;
        pcrfodk->svtPursue = pspawn->difficultyValue;
        pcrfodk->fKillReported = false;
        slot.pcrfodk = pcrfodk;
    }
    else if (FIsBasicDerivedFrom(pso, CID_CRFODB)) {
        CRFODB* pcrfodb = static_cast<CRFODB*>(pso);
        pcrfodb->pcrfodkSlot = &slot;
        pcrfodb->fKillReported = false;
        slot.pcrfodb = pcrfodb;
    }
    else {
        slot.pasegaSpawn = nullptr;
        return false;
    }
    return true;
}

void SpawnCrfodk(CRFODKSPAWN* pspawn)
{
    if (pspawn->cSpawned >= pspawn->cActiveMax) {
        pspawn->tSpawnNext = FLT_MAX;
        return;
    }
    CRFODKSLOT* freeSlots[16];
    int count = 0;
    for (int i = 0; i < pspawn->cslot; ++i)
        if (pspawn->aslot[i].pcrfodb == nullptr && pspawn->aslot[i].pasegaSpawn == nullptr)
            freeSlots[count++] = &pspawn->aslot[i];
    if (count == 0)
        return;
    CRFODKSLOT& slot = *freeSlots[NRandInRange(0, count - 1)];
    if (FireCrfodkIntoSlot(pspawn, pspawn->prwmSpawn, slot)) {
        ++pspawn->cSpawned;
        pspawn->tSpawnNext = g_clock.t + GRandInRange(pspawn->dtSpawnMin, pspawn->dtSpawnMax);
    }
}

void RespawnCrfodk(CRFODKSPAWN* pspawn)
{
    if (pspawn->cActive >= pspawn->cRespawnMax) {
        pspawn->tRespawn = FLT_MAX;
        return;
    }
    // The special/red chickens come from the separate RWM at original offset
    // 0x420. Normal chickens are emitted by prwmSpawn at 0x424.
    while (pspawn->cActive < pspawn->cRespawnMax) {
        CRFODKSLOT* freeSlots[16];
        int count = 0;
        for (int i = 0; i < pspawn->cslot && count < (int)std::size(freeSlots); ++i) {
            CRFODKSLOT& slot = pspawn->aslot[i];
            if (slot.pcrfodb == nullptr && slot.pasegaSpawn == nullptr)
                freeSlots[count++] = &slot;
        }

        if (count == 0)
            return;

        CRFODKSLOT& slot = *freeSlots[NRandInRange(0, count - 1)];
        if (!FireCrfodkIntoSlot(pspawn, pspawn->prwmRespawn, slot))
            return;

        ++pspawn->cActive;
        pspawn->tRespawn = g_clock.t + GRandInRange(pspawn->dtRespawnMin, pspawn->dtRespawnMax);
    }

    pspawn->tRespawn = FLT_MAX;
}

void UpdateCrfodkSpawnerDeathDifficulty(CRFODKSPAWN* pspawn)
{
    const float progress = pspawn->cKillGoal > 0 ? glm::clamp((float)pspawn->cKilled / pspawn->cKillGoal, 0.0f, 1.0f) : 0.0f;
    OnDifficultyPlayerDeath(&g_difficulty, 1.0f - progress * 0.8f);
}

void HandleCrfodkSpawnerTimerExpiration(void*, int fActive)
{
    if (fActive != 1 || g_pcrfodkSpawner == nullptr || g_pcrfodkSpawner->psma == nullptr)
        return;
    OID goal, cur;
    GetSmaGoal(g_pcrfodkSpawner->psma, &goal);
    GetSmaCur(g_pcrfodkSpawner->psma, &cur);
    if (goal != (OID)1022 && cur != (OID)1022) {
        SetSmaGoal(g_pcrfodkSpawner->psma, (OID)1023);
        UpdateCrfodkSpawnerDeathDifficulty(g_pcrfodkSpawner);
    }
}

void UpdateCrfodkSpawner(CRFODKSPAWN* pspawn, float dt)
{
    UpdateAlo(pspawn, dt);
    OID state;
    GetSmaCur(pspawn->psma, &state);
    if (state == (OID)1021) {
        if (pspawn->tRespawn < g_clock.t)
            RespawnCrfodk(pspawn);
        if (pspawn->tSpawnNext < g_clock.t)
            SpawnCrfodk(pspawn);
        PO* ppo = PpoCur();
        if (ppo != nullptr)
            ppo->pvtpo->pfnJthsCurrentPo(ppo);
    }
}

void UpdateCrfodkSpawnerBlot(CRFODKSPAWN* pspawn)
{
    OID state;
    GetSmaCur(pspawn->psma, &state);
    if (state == (OID)1021 && FCanShowGameplayBlot()) {
        ShowBlot(&g_puffchargectr);
        ShowBlot(&g_timer);
    }
    else {
        HideBlot(&g_puffchargectr);
        HideBlot(&g_timer);
    }
}

void OnCrfodkSpawnedDestroyed(CRFODKSPAWN* pspawn, CRFODB* pcrfodb)
{
    const int killValue = pcrfodb->cpoint > 0 ? pcrfodb->cpoint : 1;
    const bool releasedSpawnSlot = pspawn->cSpawned > 0;
    pspawn->cKilled = glm::clamp(pspawn->cKilled + killValue, 0, pspawn->cKillGoal);
    if (pspawn->cKilled >= pspawn->cKillGoal) {
        OID cur, goal;
        GetSmaCur(pspawn->psma, &cur);
        GetSmaGoal(pspawn->psma, &goal);
        PO* ppo = PpoCur();
        if (cur != (OID)1023 && goal != (OID)1023 && (ppo == nullptr || ppo->pvtpo->pfnJthsCurrentPo(ppo) != (JTHS)2))
            SetSmaGoal(pspawn->psma, (OID)1022);
        StopTimer(&g_timer);
    }
    if (releasedSpawnSlot)
        --pspawn->cSpawned;

    RecalcCrfodkSpawnerDifficulty(pspawn);
    // SpawnCrfodk parks this timer at FLT_MAX when the live quota is full.
    // Re-arm it as soon as a dying chicken releases a quota entry.
    if (releasedSpawnSlot && pspawn->cKilled < pspawn->cKillGoal)
        pspawn->tSpawnNext = g_clock.t + GRandInRange(pspawn->dtSpawnMin, pspawn->dtSpawnMax);
}

void HandleCrfodkSpawnMessage(CRFODKSPAWN* pspawn, MSGID msgid, void* pv)
{
    HandleAloMessage(pspawn, msgid, pv);
    if (msgid == MSGID_asega_retracted) {
        for (int i = 0; i < pspawn->cslot; ++i)
            if (pspawn->aslot[i].pasegaSpawn == pv)
                pspawn->aslot[i].pasegaSpawn = nullptr;
    }
    else if (msgid == MSGID_sma_transition && pv == pspawn->psma) {
        OID state;
        GetSmaCur(pspawn->psma, &state);
        if (state == (OID)1021) {
            if (pspawn->tSpawnNext == FLT_MAX)
                pspawn->tSpawnNext = g_clock.t;
            g_puffchargectr.pnTotal = &pspawn->cKillGoal;
            g_puffchargectr.pnActual = &pspawn->cKilled;
            SetSwGameplayHud(pspawn->psw, pspawn);
            SetTimer(&g_timer, pspawn->tGameMax);
            SetTimerExpire(&g_timer, 0.0f, HandleCrfodkSpawnerTimerExpiration);
            StartTimer(&g_timer);
        }
        if (pspawn->oidState == (OID)1021) {
            HideBlot(&g_puffchargectr);
            StopTimer(&g_timer);
            SetSwGameplayHud(pspawn->psw, nullptr);
        }
        pspawn->oidState = state;
    }
}

CRBRAIN* NewCrbrain()
{
	return new CRBRAIN{};
}

void InitCrbrain(CRBRAIN* pcrbrain)
{
	InitAlo(pcrbrain);
}

int GetCrbrainSize()
{
	return sizeof(CRBRAIN);
}

void CloneCrbrain(CRBRAIN* pcrbrain, CRBRAIN* pcrbrainBase)
{
	CloneAlo(pcrbrain, pcrbrainBase);

	pcrbrain->cpoint = pcrbrainBase->cpoint;
	pcrbrain->cpointMax = pcrbrainBase->cpointMax;
	pcrbrain->tContest = pcrbrainBase->tContest;
}

void DeleteCrbrain(CRBRAIN* pcrbrain)
{
	delete pcrbrain;
}

CRUSHERCTR g_crusherctr;
int s_icrfodbNext = 0;
int CRAD_TryMax = 16;
float DS_PredictCrfodb = 3000.0;
float U_PenaltyCbspIgnore = 0.75;
float U_PenaltyCbsp = 4.0;
float DT_PredictCrfodb = 3.0;
float R_DistCrusher = 5.0;
float U_PenaltyCrusher = 3.0;
float R_DistCrfod = 2.0;
float U_PenaltyCrfod = 0.5;
float U_PenaltyRad = 1.0;
float U_RadWorst = 2.0;
CRFODKSPAWN* g_pcrfodkSpawner = nullptr;
