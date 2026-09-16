#include "clkwork.h"
#include "alo.h"
#include "asega.h"
#include "blend.h"
#include "blot.h"
#include "boss.h"
#include "binoc.h"
#include "clock.h"
#include "game.h"
#include "jt.h"
#include "math.h"
#include "po.h"
#include "save.h"
#include "sw.h"
#include "tv.h"
#include "ui.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iterator>

namespace {
constexpr float kIdleBlend = 0.0f;
constexpr float kPredictionTime = 0.5f;
constexpr float kPhase0Speed = 1250.0f;
constexpr float kPhase1ForwardSpeed = 750.0f;
constexpr float kPhase1LateralSpeed = 750.0f;
constexpr float kClkwrkDmgEndingDelay = 3.0f;
float Pos(float x) { return std::max(x, 0.0f); }

CWDK s_cwdkPending = static_cast<CWDK>(-1);
int s_fClkwrkDmgEndingStarted = 0;

SNIP s_asnipClkwork[13] = {
    { 0, (OID)0x468, (int)(offsetof(CLKWORK, aablPhase0) + 3 * sizeof(BL) + offsetof(BL, paseg)) },
    { 0, (OID)0x469, (int)(offsetof(CLKWORK, aablPhase0) + 1 * sizeof(BL) + offsetof(BL, paseg)) },
    { 0, (OID)0x46A, (int)(offsetof(CLKWORK, aablPhase0) + 0 * sizeof(BL) + offsetof(BL, paseg)) },
    { 0, (OID)0x46B, (int)(offsetof(CLKWORK, aablPhase0) + 2 * sizeof(BL) + offsetof(BL, paseg)) },
    { 0, (OID)0x46C, (int)(offsetof(CLKWORK, aablPhase1) + 0 * sizeof(BL) + offsetof(BL, paseg)) },
    { 0, (OID)0x46D, (int)(offsetof(CLKWORK, aablPhase1) + 1 * sizeof(BL) + offsetof(BL, paseg)) },
    { 0, (OID)0x46E, (int)(offsetof(CLKWORK, aablPhase1) + 2 * sizeof(BL) + offsetof(BL, paseg)) },
    { 0, (OID)0x46F, (int)(offsetof(CLKWORK, aablPhase1) + 3 * sizeof(BL) + offsetof(BL, paseg)) },
    { 2, (OID)0x470, (int)(offsetof(CLKWORK, aablTracking) + 0 * sizeof(BL) + offsetof(BL, paseg)) },
    { 2, (OID)0x471, (int)(offsetof(CLKWORK, aablTracking) + 1 * sizeof(BL) + offsetof(BL, paseg)) },
    { 2, (OID)0x472, (int)(offsetof(CLKWORK, aablTracking) + 2 * sizeof(BL) + offsetof(BL, paseg)) },
    { 2, (OID)0x473, (int)(offsetof(CLKWORK, aablTracking) + 3 * sizeof(BL) + offsetof(BL, paseg)) },
    { 2, (OID)0x474, (int)(offsetof(CLKWORK, aablTracking) + 4 * sizeof(BL) + offsetof(BL, paseg)) }
};

SNIP s_asnipClkwrkDmg[3] = {
    { 0, (OID)0x478, (int)(offsetof(CLKWRKDMG, pasegblDamageDefault)) },
    { 0, (OID)0x479, (int)(offsetof(CLKWRKDMG, pasegblDamageSweepAlternate)) },
    { 0, (OID)0x47A, (int)(offsetof(CLKWRKDMG, pasegblDamageSmash)) }
};
}

CLKWORK* NewClkwork() 
{ 
    return new CLKWORK{}; 
}

void InitClkWork(CLKWORK* pclkwork) 
{
    InitSo(pclkwork); 
    pclkwork->cwk = CLKWORKS_Nil; 
}

int GetClkworkSize() 
{ 
    return sizeof(CLKWORK); 
}

void CloneClkwork(CLKWORK* pclkwork, CLKWORK* pclkworkBase)
{
    CloneSo(pclkwork, pclkworkBase);
    pclkwork->cwk = pclkworkBase->cwk;
    pclkwork->tClkworks = pclkworkBase->tClkworks;
    pclkwork->pasegaState = pclkworkBase->pasegaState;
    std::copy(std::begin(pclkworkBase->aablPhase0), std::end(pclkworkBase->aablPhase0), std::begin(pclkwork->aablPhase0));
    pclkwork->pasegblPhase0 = pclkworkBase->pasegblPhase0;
    std::copy(std::begin(pclkworkBase->aablPhase1), std::end(pclkworkBase->aablPhase1), std::begin(pclkwork->aablPhase1));
    pclkwork->pasegblPhase1 = pclkworkBase->pasegblPhase1;
    std::copy(std::begin(pclkworkBase->aablTracking), std::end(pclkworkBase->aablTracking), std::begin(pclkwork->aablTracking));
    pclkwork->pasegblTracking = pclkworkBase->pasegblTracking;
    pclkwork->pasegaTracking = pclkworkBase->pasegaTracking;
    pclkwork->psoWeakPoint = pclkworkBase->psoWeakPoint;
    pclkwork->pasegblDamage = pclkworkBase->pasegblDamage;
    pclkwork->pasegaDamage = pclkworkBase->pasegaDamage;
}

void DeleteClkwork(CLKWORK* pclkwork) 
{ 
    delete pclkwork; 
}

void PostClkWorkLoad(CLKWORK* pclkwork)
{
    PostAloLoad(pclkwork);
    SnipAloObjects(pclkwork, 13, s_asnipClkwork);
    EnsureAsegBlendDynamic(pclkwork, sizeof(BL), 4, pclkwork->aablPhase0, 0, nullptr, nullptr, &pclkwork->pasegblPhase0);
    pclkwork->aablPhase0[3].u = 1.0f;
    ReblendAsegbl(pclkwork->pasegblPhase0, sizeof(BL), 4, pclkwork->aablPhase0);
    EnsureAsegBlendDynamic(pclkwork, sizeof(BL), 4, pclkwork->aablPhase1, 0, nullptr, nullptr, &pclkwork->pasegblPhase1);
    pclkwork->aablPhase1[0].u = 1.0f;
    ReblendAsegbl(pclkwork->pasegblPhase1, sizeof(BL), 4, pclkwork->aablPhase1);
    EnsureAsegBlendDynamic(pclkwork, sizeof(BL), 5, pclkwork->aablTracking, 0, nullptr, nullptr, &pclkwork->pasegblTracking);
    pclkwork->aablTracking[0].u = 1.0f;
    ReblendAsegbl(pclkwork->pasegblTracking, sizeof(BL), 5, pclkwork->aablTracking);
    SetClkworkState(pclkwork, CLKWORKS_Phase0);
    SetClkworkTracking(pclkwork, 1);
}

void UpdateClkwork(CLKWORK* pclkwork, float dt)
{
    UpdateSo(pclkwork, dt);

    glm::vec3 movementLocal{};

    if (pclkwork->cwk > CLKWORKS_Nil && pclkwork->cwk < CLKWORKS_Defeated) {
        // Retail only calculates this movement vector while Clockwerk is in
        // one of the two active boss phases. The same local-space vector is
        // then reused by the tracking blend below.
        glm::vec3 movementWorld{};
        CalculateAloMovement(pclkwork, nullptr, pclkwork->xf.posWorld,
            &movementWorld, nullptr, nullptr, nullptr);
        movementLocal = glm::transpose(pclkwork->xf.matWorld) * movementWorld;

        glm::vec3 posPredicted{}, velocityPredicted{};
        glm::mat3 matPredicted(1.0f);
        PredictAloTransform(pclkwork, nullptr, kPredictionTime,
            &posPredicted, &matPredicted, &velocityPredicted, nullptr);
        const glm::vec3 velocityLocal = glm::transpose(matPredicted) * velocityPredicted;

        if (pclkwork->cwk == CLKWORKS_Phase0) {
            float d = glm::clamp(velocityLocal.x / kPhase0Speed, -1.0f, 1.0f), scale = 1.0f - kIdleBlend;
            pclkwork->aablPhase0[3].u = kIdleBlend; pclkwork->aablPhase0[0].u = Pos(d) * scale;
            pclkwork->aablPhase0[2].u = Pos(-d) * scale; pclkwork->aablPhase0[1].u = (1.0f - Pos(d) - Pos(-d)) * scale;
            ReblendAsegbl(pclkwork->pasegblPhase0, sizeof(BL), 4, pclkwork->aablPhase0);
        } else {
            float f = glm::clamp(velocityLocal.x / kPhase1ForwardSpeed, 0.0f, 1.0f);
            float s = glm::clamp(velocityLocal.y / kPhase1LateralSpeed, -1.0f, 1.0f);
            float scale = (1.0f - kIdleBlend) / std::max(1.0f, std::abs(f) + std::abs(s)); f *= scale; s *= scale;
            pclkwork->aablPhase1[1].u = f; pclkwork->aablPhase1[2].u = Pos(s); pclkwork->aablPhase1[3].u = Pos(-s);
            pclkwork->aablPhase1[0].u = 1.0f - f - std::abs(s);
            ReblendAsegbl(pclkwork->pasegblPhase1, sizeof(BL), 4, pclkwork->aablPhase1);
        }
    }
    if (PO* player = PpoCur()) {
        glm::vec3 local{}; ConvertAloPos(nullptr, pclkwork, &player->xf.posWorld, &local);
        glm::vec3 normal = movementLocal;
        glm::vec3 x(1.0f, 0.0f, 0.0f);
        float angle = std::min(RadBetweenVectors(x, local, &normal), 1.0f);
        float up = angle * normal.z, side = -angle * normal.y;
        pclkwork->aablTracking[3].u = Pos(up); pclkwork->aablTracking[4].u = Pos(-up);
        pclkwork->aablTracking[1].u = Pos(side); pclkwork->aablTracking[2].u = Pos(-side);
        pclkwork->aablTracking[0].u = 1.0f - Pos(up) - Pos(-up) - Pos(side) - Pos(-side);
        ReblendAsegbl(pclkwork->pasegblTracking, sizeof(BL), 5, pclkwork->aablTracking);
    }
    if (pclkwork->pasegaDamage && pclkwork->pasegaDamage->svtLocal == 0.0f) { RetractAsega(pclkwork->pasegaDamage); pclkwork->pasegaDamage = nullptr; }
}

void UpdateClkworkBlot(CLKWORK* pclkwork)
{
    bool fShow = false;

    if (pclkwork->cwk > CLKWORKS_Nil &&
        pclkwork->cwk < CLKWORKS_Defeated &&
        FCanShowGameplayBlot()) {
        if (g_binoc.binocs == BINOCS_Instruct)
            fShow = g_tvLeft.pspeakerDraw == nullptr && g_tvRight.pspeakerDraw == nullptr;
        else
            fShow = true;
    }

    if (fShow)
        ShowBlot(&g_boss);
    else
        HideBlot(&g_boss);
}

int FAbsorbClkworkWkr(CLKWORK* pclkwork, WKR* pwkr)
{
    if (pwkr && (pwkr->grftak & 16U) && pwkr->ploTarget == pclkwork->psoWeakPoint) {
        DecrementBossHealth(&g_boss);
        PlayClkworkDamageAnimation(pclkwork);
        HandleLoSpliceEvent(pclkwork, 2, 0, nullptr);
    }
    return 1;
}

void SetClkworkState(CLKWORK* pclkwork, CWK cwk)
{
    if (cwk == pclkwork->cwk) return;
    ASEGBL* pasegbl = cwk == CLKWORKS_Phase0 ? pclkwork->pasegblPhase0 : cwk == CLKWORKS_Phase1 ? pclkwork->pasegblPhase1 : nullptr;
    if (cwk > CLKWORKS_Nil && cwk < CLKWORKS_Defeated) {
        g_boss.uHealthTarget = 1.0f; g_boss.cPhasesRemaining = g_boss.cPhaseMax;
        g_boss.cHealthCurrent = g_boss.cHealthPerPhase; g_boss.uHealthDisplay = 0.0f; SetSwGameplayHud(pclkwork->psw, pclkwork);
    } else if (cwk >= CLKWORKS_Defeated && cwk < CLKWORKS_Max) { HideBlot(&g_boss); SetSwGameplayHud(pclkwork->psw, nullptr); }
    ApplyAsegCur(pasegbl, pclkwork, 0.0f, 1.0f, (GRFAPL)1, &pclkwork->pasegaState); pclkwork->cwk = cwk; pclkwork->tClkworks = g_clock.t;
}

void SetClkworkTracking(CLKWORK* pclkwork, int fTracking)
{
    ApplyAsegCur(fTracking ? pclkwork->pasegblTracking : nullptr, pclkwork, 0.0f, 1.0f, (GRFAPL)1, &pclkwork->pasegaTracking);
}
void SetClkworkDamageAnimation(CLKWORK* pclkwork, OID oid)
{
    pclkwork->pasegblDamage = oid == OID_Nil ? nullptr : reinterpret_cast<ASEGBL*>(PloFindSwObject(pclkwork->psw, 257, oid, pclkwork));
}
void PlayClkworkDamageAnimation(CLKWORK* pclkwork)
{
    ApplyAsegCur(pclkwork->pasegblDamage, pclkwork, 0.0f, 1.0f, (GRFAPL)1, &pclkwork->pasegaDamage);
}

CLKWRKDMG* NewClkwrkDmg() 
{ 
    return new CLKWRKDMG{}; 
}

void InitClkwrkDmg(CLKWRKDMG* pclkwrkdmg)
{
    InitSo(pclkwrkdmg);

    pclkwrkdmg->cHitsRemaining = 16;
    s_cwdkPending = static_cast<CWDK>(-1);
    s_fClkwrkDmgEndingStarted = 0;
}

int GetClkwrkDmgSize() 
{ 
    return sizeof(CLKWRKDMG); 
}

void CloneClkwrkDmg(CLKWRKDMG* pclkwrkdmg, CLKWRKDMG* pclkwrkdmgBase)
{
    CloneSo(pclkwrkdmg, pclkwrkdmgBase);
    pclkwrkdmg->cwdk = pclkwrkdmgBase->cwdk;
    pclkwrkdmg->tClkwrkDmg = pclkwrkdmgBase->tClkwrkDmg;
    pclkwrkdmg->cHitsRemaining = pclkwrkdmgBase->cHitsRemaining;
    pclkwrkdmg->pasegblDamageDefault = pclkwrkdmgBase->pasegblDamageDefault;
    pclkwrkdmg->pasegblDamageSweepAlternate = pclkwrkdmgBase->pasegblDamageSweepAlternate;
    pclkwrkdmg->pasegblDamageSmash = pclkwrkdmgBase->pasegblDamageSmash;
    pclkwrkdmg->pasegaDamage = pclkwrkdmgBase->pasegaDamage;
}

void DeleteClkwrkDmg(CLKWRKDMG* pclkwrkdmg) 
{ 
    delete pclkwrkdmg; 
}

void PostClkwrkDmgLoad(CLKWRKDMG* pclkwrkdmg)
{
    PostAloLoad(pclkwrkdmg);
    SnipAloObjects(pclkwrkdmg, 3, s_asnipClkwrkDmg);
}

void UpdateClkwrkDmg(CLKWRKDMG* pclkwrkdmg, float dt)
{
    UpdateSo(pclkwrkdmg, dt);

    if (pclkwrkdmg->pasegaDamage && pclkwrkdmg->pasegaDamage->svtLocal == 0.0f) {
        RetractAsega(pclkwrkdmg->pasegaDamage);
        pclkwrkdmg->pasegaDamage = nullptr;
    }

    if (s_cwdkPending != static_cast<CWDK>(-1))
        SetClkwrkDmgState(pclkwrkdmg, s_cwdkPending);

    if (pclkwrkdmg->cwdk == CLKWRKDMGS_Defeated &&
        kClkwrkDmgEndingDelay < g_clock.t - pclkwrkdmg->tClkwrkDmg &&
        !s_fClkwrkDmgEndingStarted) {
        s_fClkwrkDmgEndingStarted = 1;
        PlayEndingFromCompletionFlag(2);
    }
}

void UpdateClkwrkDmgBlot(CLKWRKDMG* pclkwrkdmg)
{
    bool fShow = pclkwrkdmg->cwdk == CLKWRKDMGS_Active && FCanShowGameplayBlot();
    if (fShow && g_binoc.binocs == BINOCS_Instruct)
        fShow = g_tvLeft.pspeakerDraw == nullptr && g_tvRight.pspeakerDraw == nullptr;

    if (fShow)
        ShowBlot(&g_boss);
    else
        HideBlot(&g_boss);
}

int FAbsorbClkworkDmgWkr(CLKWRKDMG* pclkwrkdmg, WKR* pwkr)
{
    if (pwkr && (pwkr->grfic & 32U)) {
        pclkwrkdmg->cHitsRemaining = std::max(pclkwrkdmg->cHitsRemaining - 1, 0);
        DecrementBossHealth(&g_boss);

        ASEGBL* pasegblDamage = nullptr;
        if (pclkwrkdmg->cHitsRemaining == 0) {
            s_cwdkPending = CLKWRKDMGS_Defeated;
        }
        else if (!(pwkr->grftak & GRFTAK_Sweep)) {
            pasegblDamage = (pwkr->grftak & GRFTAK_Smash)
                ? pclkwrkdmg->pasegblDamageSmash
                : pclkwrkdmg->pasegblDamageDefault;
        }
        else if (!g_pjt || g_pjt->jtcs == JTCS_SweepFore || g_pjt->jtcs == JTCS_StandingSweepFore) {
            pasegblDamage = pclkwrkdmg->pasegblDamageDefault;
        }
        else {
            pasegblDamage = pclkwrkdmg->pasegblDamageSweepAlternate;
        }

        ApplyAsegCur(pasegblDamage, pclkwrkdmg, 0.0f, 1.0f, (GRFAPL)1, &pclkwrkdmg->pasegaDamage);
        pwkr->grfic &= ~32U;
        HandleLoSpliceEvent(pclkwrkdmg, 2, 0, nullptr);
    }

    return FAbsorbSoWkr(pclkwrkdmg, pwkr);
}

void SetClkwrkDmgState(CLKWRKDMG* pclkwrkdmg, CWDK cwdk)
{
    if (cwdk == pclkwrkdmg->cwdk)
        return;

    s_cwdkPending = static_cast<CWDK>(-1);
    if (cwdk == CLKWRKDMGS_Active) {
        g_boss.tHealthChanged = -1.0f;
        g_boss.cHealthCurrent = 4;
        g_boss.cHealthTotal = 16;
        g_boss.uHealthTarget = 1.0f;
        g_boss.cPhaseMax = 4;
        g_boss.cPhasesRemaining = 4;
        g_boss.cHealthPerPhase = 4;
        g_boss.uHealthDisplay = 0.0f;
        SetSwGameplayHud(pclkwrkdmg->psw, pclkwrkdmg);
    }
    else if (cwdk == CLKWRKDMGS_Defeated) {
        SetWorldComplete(GAMEWORLD_Clockwerk);
        AutosaveCurrentGame(&g_saveData);
    }

    pclkwrkdmg->cwdk = cwdk;
    pclkwrkdmg->tClkwrkDmg = g_clock.t;
}
