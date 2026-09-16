#include "jetpack.h"

#include "alo.h"
#include "asega.h"
#include "bbmark.h"
#include "blend.h"
#include "clock.h"
#include "cm.h"
#include "game.h"
#include "jt.h"
#include "lgn.h"
#include "rwm.h"
#include "shd.h"
#include "sm.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cmath>

namespace
{
    CLQ s_clqJetpackPosition = { 0.0f, 20.0f, 0.0f, 0.0f };
    CLQ s_clqJetpackVelocity = { 0.0f, 10.0f, 0.0f, 0.0f };
    SMP s_smpJetpackRotation = { 25.0f, 0.0f, 0.5f };
    SMP s_smpJetpackLimit = { 1.5f, 0.0f, 0.75f };
    SMP s_smpJetpackAim = { 20.0f, 0.0f, 0.1f };
    SMP s_smpJetpackBody = { 10.0f, 0.0f, 0.1f };

    constexpr float kTargetX0 = 0.0f;
    constexpr float kTargetX1 = -600.0f;
    constexpr float kTargetX2 = 0.0f;
    constexpr float kTargetY0 = 0.0f;
    constexpr float kTargetY1 = -300.0f;
    constexpr float kTargetY2 = 0.0f;
    constexpr float kAimRangeX = -750.0f;
    constexpr float kAimRangeY = -450.0f;

    SNIP s_asnipJetpack[26] =
    {
        { 0x00, (OID)0x453, offsetof(JETPACK, abl[0].paseg) },
        { 0x00, (OID)0x454, offsetof(JETPACK, abl[1].paseg) },
        { 0x00, (OID)0x455, offsetof(JETPACK, abl[2].paseg) },
        { 0x00, (OID)0x456, offsetof(JETPACK, abl[3].paseg) },
        { 0x00, (OID)0x457, offsetof(JETPACK, abl[4].paseg) },
        { 0x00, (OID)0x458, offsetof(JETPACK, pasegUnknown1) },
        { 0x00, (OID)0x459, offsetof(JETPACK, pasegCharm) },
        { 0x00, (OID)0x45A, offsetof(JETPACK, pasegDead) },
        { 0x00, (OID)0x44E, offsetof(JETPACK, psm) },
        { 0x04, (OID)0x050, offsetof(JETPACK, paloBody) },
        { 0x04, (OID)0x045, offsetof(JETPACK, paloEffectBasis) },
        { 0x04, (OID)0x44C, offsetof(JETPACK, paloJt) },
        { 0x04, (OID)0x44D, offsetof(JETPACK, paloEffect) },
        { 0x00, (OID)0x162, offsetof(JETPACK, pzi.mpccharmpaloCharm[2]) },
        { 0x00, (OID)0x163, offsetof(JETPACK, pzi.mpccharmpaloCharm[3]) },
        { 0x04, (OID)0x160, offsetof(JETPACK, pzi.mpccharmpaloCharm[0]) },
        { 0x20, (OID)0x18D, offsetof(JETPACK, paloUseCharm) },
        { 0x04, (OID)0x45B, offsetof(JETPACK, apaloThruster[0]) },
        { 0x04, (OID)0x45C, offsetof(JETPACK, apaloThruster[1]) },
        { 0x04, (OID)0x465, offsetof(JETPACK, paloBodyLeft) },
        { 0x04, (OID)0x466, offsetof(JETPACK, paloBodyRight) },
        { 0x04, (OID)0x45D, offsetof(JETPACK, prwm) },
        { 0x05, (OID)0x461, offsetof(JETPACK, paloTargetOrigin) },
        { 0x05, (OID)0x45F, offsetof(JETPACK, paloAimBasis) },
        { 0x05, (OID)0x45E, offsetof(JETPACK, paloAimLimit) },
        { 0x05, (OID)0x460, offsetof(JETPACK, paloMovementBasis) }
    };
}

JETPACK* NewJetpack()
{
    return new JETPACK{};
}

void InitJetpack(JETPACK* pjetpack)
{
    InitPo(pjetpack);
    pjetpack->jpk = JETPACKS_Nil;
}

int GetJetpackSize()
{
    return sizeof(JETPACK);
}

void CloneJetpack(JETPACK* pjetpack, JETPACK* pjetpackBase)
{
    ClonePo(pjetpack, pjetpackBase);
}

void PostJetpackLoad(JETPACK* pjetpack)
{
    PostAloLoad(pjetpack);
    SnipAloObjects(pjetpack, 26, s_asnipJetpack);

    // PresetJetpackAccel consumes this before the first active input update.
    // A zero matrix is not a rotation and makes the spring spin indefinitely.
    pjetpack->matBody = pjetpack->xf.mat;

    if (pjetpack->paloEffect != nullptr)
        SetAloTransformBasis(pjetpack->paloEffect, pjetpack->paloEffectBasis, pjetpack->paloEffectBasis);

    pjetpack->psma = PsmaApplySm(pjetpack->psm, pjetpack, OID_Nil, 0);
    EnsureAsegBlendDynamic(pjetpack, sizeof(BL), 5, pjetpack->abl, 0, nullptr, nullptr, &pjetpack->pasegblDynamic);
    pjetpack->abl[0].u = 1.0f;
    if (pjetpack->pasegblDynamic != nullptr)
        ReblendAsegbl(pjetpack->pasegblDynamic, sizeof(BL), 5, pjetpack->abl);

    SetJetpackState(pjetpack, JETPACKS_Active);
    g_lgnr.uAlpha = 0.0f;
    g_lgnr.pshd = PshdFindShader((OID)874);
}

void CalculateJetpackTarget(JETPACK* pjetpack, glm::vec3* pposTarget, glm::vec3* pvTarget)
{
    if (pposTarget != nullptr)
    {
        // The EE pextlw/pcpyld sequence packs these as (0, X, Y, 0).
        // X is lateral motion in the movement basis and Y is vertical motion.
        pposTarget->x = 0.0f;
        pposTarget->y = kTargetX0 + pjetpack->radBodyX *
            (kTargetX1 + pjetpack->radBodyX * kTargetX2);
        pposTarget->z = kTargetY0 + pjetpack->radBodyY *
            (kTargetY1 + pjetpack->radBodyY * kTargetY2);
    }

    if (pvTarget != nullptr)
    {
        pvTarget->x = 0.0f;
        pvTarget->y = pjetpack->dradBodyX * kTargetX1;
        pvTarget->z = pjetpack->dradBodyY * kTargetY1;
    }
}

void PresetJetpackAccel(JETPACK* pjetpack, float dt)
{
    if (pjetpack->jpk < JETPACKS_Active || pjetpack->jpk > JETPACKS_Dead)
    {
        PresetSoAccel(pjetpack, dt);
        return;
    }

    glm::vec3 posTarget{};
    glm::vec3 vTarget{};
    if (pjetpack->jpk < JETPACKS_Dead)
    {
        CalculateJetpackTarget(pjetpack, &posTarget, &vTarget);
        CalculateAloTransformAdjust(pjetpack->paloMovementBasis, nullptr, &posTarget, nullptr, &vTarget, nullptr);
    }
    else
        posTarget = pjetpack->xf.pos;

    AccelSoTowardPosSpring(pjetpack, &posTarget, &s_clqJetpackPosition, &vTarget, &s_clqJetpackVelocity, dt);
    AccelSoTowardMatSmooth(pjetpack, dt, &pjetpack->matBody, &s_smpJetpackRotation);
}

void ProjectJetpackTransform(JETPACK* pjetpack, float dt, int fParentDirty)
{
    (void)fParentDirty;
    ProjectSoTransform(pjetpack, dt, 0);

    if (pjetpack->pvtalo->pfnUpdateAloXfWorld != nullptr)
        pjetpack->pvtalo->pfnUpdateAloXfWorld(pjetpack);
}

void UpdateJetpack(JETPACK* pjetpack, float dt)
{
    UpdatePo(pjetpack, dt);

    if (pjetpack->fExitPending != 0)
    {
        pjetpack->fExitPending = 0;
        if (g_pjt != nullptr)
        {
            g_pjt->pvtlo->pfnAddLo(g_pjt);
            SetPoPlayable(g_pjt, 1);
            SetJtJts(g_pjt, 2, 2);
            g_pjt->pvtalo->pfnMatchAloOtherObject(g_pjt, pjetpack->paloJt);
            g_pjt->posBasePrev = g_pjt->xf.posWorld;
            SwitchToPo(g_pjt);

            ZPR zpr{};
            InitZpr(&zpr, ZPK_Pit, pjetpack);
            g_pjt->pvtpo->pfnFTakePoDamage(g_pjt, &zpr);
        }

        if (pjetpack->paloJt != nullptr)
            pjetpack->paloJt->pvtlo->pfnRemoveLo(pjetpack->paloJt);

        InvalidateSwXpForObject(pjetpack->psw, pjetpack, 7);
    }

    if (pjetpack->fFlash != 0 && pjetpack->jpk != JETPACKS_Charm && g_clock.t - pjetpack->tCharm > 2.0f)
        pjetpack->fFlash = 0;

    glm::vec3 vBasis{};
    if (pjetpack->paloMovementBasis != nullptr)
        CalculateAloMovement(pjetpack->paloMovementBasis, nullptr,
            pjetpack->xf.pos, &vBasis, nullptr, nullptr, nullptr);

    glm::vec3 vRelative = pjetpack->xf.v - vBasis;
    ConvertAloVec(nullptr, pjetpack->paloMovementBasis, &vRelative, &vRelative);
    constexpr float kVelocityBlend = 0.7f;
    float dradVisualY = kVelocityBlend * vRelative.z * -0.0033333334f +
        (1.0f - kVelocityBlend) * pjetpack->dradBodyY;
    float dradVisualX = kVelocityBlend * vRelative.y * -0.0016666667f +
        (1.0f - kVelocityBlend) * pjetpack->dradBodyX;

    // Retail mirrors horizontal body motion when the aim basis lies on the
    // negative-X side of the movement basis.
    if (pjetpack->paloMovementBasis != nullptr &&
        pjetpack->paloAimBasis != nullptr)
    {
        glm::vec3 posAimBasisLocal{};
        ConvertAloPos(nullptr, pjetpack->paloMovementBasis,
            &pjetpack->paloAimBasis->xf.posWorld, &posAimBasisLocal);
        if (posAimBasisLocal.x < 0.0f)
            dradVisualX = -dradVisualX;
    }

    const float sum = std::abs(dradVisualX) + std::abs(dradVisualY);
    if (sum > 1.0f)
    {
        dradVisualX /= sum;
        dradVisualY /= sum;
    }

    // This project's active-object pass runs before its hierarchy update.
    // Preserve the steering rates written by UpdateJetpackActive; otherwise
    // the hierarchy update replaces fresh input with the previous velocity.
    if (PpoCur() != pjetpack || pjetpack->jpk != JETPACKS_Active)
    {
        pjetpack->dradBodyX = dradVisualX;
        pjetpack->dradBodyY = dradVisualY;
    }

    pjetpack->abl[1].u = std::clamp(-dradVisualX, 0.0f, 1.0f);
    pjetpack->abl[2].u = std::clamp( dradVisualX, 0.0f, 1.0f);
    pjetpack->abl[3].u = std::clamp(-dradVisualY, 0.0f, 1.0f);
    pjetpack->abl[4].u = std::clamp( dradVisualY, 0.0f, 1.0f);
    pjetpack->abl[0].u = 1.0f - pjetpack->abl[1].u - pjetpack->abl[2].u -
        pjetpack->abl[3].u - pjetpack->abl[4].u;

    if (pjetpack->pasegblDynamic != nullptr)
        ReblendAsegbl(pjetpack->pasegblDynamic, sizeof(BL), 5, pjetpack->abl);
}

void UpdateJetpackActive(JETPACK* pjetpack, JOY* pjoy, float dt)
{
    if (pjetpack->psma != nullptr && pjoy->IsHeld(BTN_TRIANGLE))
    {
        OID oidState = OID_Nil;
        GetSmaCur(pjetpack->psma, &oidState);
        if (oidState >= (OID)1103)
            oidState = oidState < (OID)1105 ? (OID)1105 : (OID)1103;
        SetSmaGoal(pjetpack->psma, oidState);
    }

    const bool fFlightState = pjetpack->jpk > JETPACKS_Unknown1 &&
        pjetpack->jpk < JETPACKS_Max;
    float inputY = pjoy->y;
    if ((g_pgsCur->grfgs & 2048U) != 0)
        inputY = -inputY;

    // The first retail input block moves the aim cursor.  These are separate
    // accumulators from radBodyX/radBodyY, which drive physical movement.
    if (fFlightState && pjetpack->paloAimBasis != nullptr &&
        pjetpack->paloAimLimit != nullptr)
    {
        glm::vec3 posAimLimit{};
        ConvertAloPos(nullptr, pjetpack->paloAimBasis, &pjetpack->paloAimLimit->xf.posWorld, &posAimLimit);

        const float uAimCenterX = posAimLimit.y / kAimRangeX;
        const float uAimCenterY = posAimLimit.z / kAimRangeY;
        float duAimXMax;
        float duAimXMin;
        float duAimYMax;
        float duAimYMin;
        GSmooth(pjetpack->uAimX, uAimCenterX + 1.0f, 0.0f, &s_smpJetpackLimit, &duAimXMax);
        GSmooth(pjetpack->uAimX, uAimCenterX - 1.0f, 0.0f, &s_smpJetpackLimit, &duAimXMin);
        GSmooth(pjetpack->uAimY, uAimCenterY + 1.0f, 0.0f, &s_smpJetpackLimit, &duAimYMax);
        GSmooth(pjetpack->uAimY, uAimCenterY - 1.0f, 0.0f, &s_smpJetpackLimit, &duAimYMin);

        float duAimXTarget = (duAimXMin + duAimXMax) * 0.5f;
        float duAimYTarget = (duAimYMin + duAimYMax) * 0.5f;
        if (pjetpack->jpk == JETPACKS_Active)
        {
            duAimXTarget = duAimXMin + (pjoy->x + 1.0f) *
                (duAimXMax - duAimXMin) * 0.5f;
            duAimYTarget = duAimYMin + (inputY + 1.0f) *
                (duAimYMax - duAimYMin) * 0.5f;
        }

        pjetpack->duAimX = GSmooth(pjetpack->duAimX, duAimXTarget,
            dt, &s_smpJetpackAim, nullptr);
        pjetpack->duAimY = GSmooth(pjetpack->duAimY, duAimYTarget,
            dt, &s_smpJetpackAim, nullptr);
        pjetpack->uAimX = std::clamp(pjetpack->uAimX + dt * pjetpack->duAimX,
            uAimCenterX - 1.0f, uAimCenterX + 1.0f);
        pjetpack->uAimY = std::clamp(pjetpack->uAimY + dt * pjetpack->duAimY,
            uAimCenterY - 1.0f, uAimCenterY + 1.0f);

        glm::vec3 posAimLocal(posAimLimit.x,
            kAimRangeX * pjetpack->uAimX,
            kAimRangeY * pjetpack->uAimY);
        glm::vec3 posAimWorld{};
        ConvertAloPos(pjetpack->paloAimBasis, nullptr,
            &posAimLocal, &posAimWorld);

        // OID 0x45d is CID_RWM. Retail converts the world aim point back into
        // the aim basis and stores it at RWM+0xf0, which is rwti.pos.
        if (pjetpack->prwm != nullptr)
        {
            glm::vec3 posAimTargetLocal{};
            ConvertAloPos(nullptr, pjetpack->paloAimBasis,
                &posAimWorld, &posAimTargetLocal);
            pjetpack->prwm->rwti.pos = posAimTargetLocal;
        }

        ConvertCmWorldToScreen(g_pcm, &posAimWorld, &g_lgnr.posScreen);
    }

    if (fFlightState)
    {
        float maxBodyX;
        float minBodyX;
        float maxBodyY;
        float minBodyY;
        GSmooth(pjetpack->radBodyX, 1.0f, 0.0f, &s_smpJetpackLimit, &maxBodyX);
        GSmooth(pjetpack->radBodyX, -1.0f, 0.0f, &s_smpJetpackLimit, &minBodyX);
        GSmooth(pjetpack->radBodyY, 1.0f, 0.0f, &s_smpJetpackLimit, &maxBodyY);
        GSmooth(pjetpack->radBodyY, -1.0f, 0.0f, &s_smpJetpackLimit, &minBodyY);

        float targetBodyX = (minBodyX + maxBodyX) * 0.5f;
        float targetBodyY = (minBodyY + maxBodyY) * 0.5f;
        if (pjetpack->jpk == JETPACKS_Active)
        {
            targetBodyX = minBodyX + (pjoy->x + 1.0f) * (maxBodyX - minBodyX) * 0.5f;
            targetBodyY = minBodyY + (inputY + 1.0f) * (maxBodyY - minBodyY) * 0.5f;
        }

        pjetpack->dradBodyX = GSmooth(pjetpack->dradBodyX, targetBodyX, dt, &s_smpJetpackBody, nullptr);
        pjetpack->dradBodyY = GSmooth(pjetpack->dradBodyY, targetBodyY, dt, &s_smpJetpackBody, nullptr);
        pjetpack->radBodyX = std::clamp(pjetpack->radBodyX + dt * pjetpack->dradBodyX, -1.0f, 1.0f);
        pjetpack->radBodyY = std::clamp(pjetpack->radBodyY + dt * pjetpack->dradBodyY, -1.0f, 1.0f);

    }

    if (fFlightState && pjetpack->paloAimBasis != nullptr)
    {
        glm::vec3 vecAim = pjetpack->paloAimBasis->xf.posWorld -
            pjetpack->xf.posWorld;

        if (glm::dot(vecAim, vecAim) > 0.0001f)
        {
            glm::mat3 matAim;
            BuildOrthonormalMatrixZ(&vecAim, &g_normalZ, &matAim);
            SmoothMatrix(&pjetpack->matBody, &matAim, &s_smpJetpackRotation, dt, &pjetpack->matBody, nullptr);
        }
    }
    else
        LoadRotateMatrixRad(pjetpack->radBody, &g_normalZ, &pjetpack->matBody);

    HandleDialogButtons(pjoy);
    ResolveAlo(pjetpack);
}

void RenderJetpackAll(JETPACK* pjetpack, CM* pcm, RO* pro)
{
    glm::vec4 rgbaCel = g_rgbaCel;

    if (pjetpack->fFlash != 0)
    {
        const float age = pjetpack->jpk == JETPACKS_Charm ? 0.0f : (g_clock.t - pjetpack->tCharm) * 0.5f;
        const float pulse = 0.7f + 0.3f * std::cos((g_clock.t - pjetpack->tFlash) * 10.0f);
        const float blend = std::clamp(1.0f - 0.75f * age * age, 0.0f, 1.0f) * std::clamp(pulse, 0.0f, 1.0f);
        rgbaCel = glm::mix(rgbaCel, glm::vec4(160.0f / 255.0f, 0.0f, 0.0f, rgbaCel.a), blend);
    }

    SetAloOverrideCel(pjetpack, &rgbaCel);
    RenderAloAll(pjetpack, pcm, pro);
}

void OnJetpackActive(JETPACK* pjetpack, int fActive, PO* ppoOther)
{
    OnPoActive(pjetpack, fActive, ppoOther);
    if (fActive == 0)
    {
        g_lgnr.pvtblot->pfnSetBlotBlots(&g_lgnr, BLOTS_Hidden);
        pjetpack->fFlash = 0;
    }
    else
        g_lgnr.posScreen = glm::vec3(2000.0f, 0.0f, 0.0f);
}

int FInvulnerableJetpack(JETPACK* pjetpack, ZPK zpk)
{
    if (zpk == ZPK_Pit)
        return 1;
    if (pjetpack->jpk == JETPACKS_Charm || pjetpack->jpk == JETPACKS_Dead)
        return 1;
    if (pjetpack->jpk != JETPACKS_Unknown0 && pjetpack->fFlash == 0)
        return FSwHandsOff(pjetpack->psw) != 0;
    return 1;
}

int FTakeJetpackDamage(JETPACK* pjetpack, ZPR* pzpr)
{
    if (FInvulnerableJetpack(pjetpack, pzpr->zpk))
        return 0;

    if (FCharmAvailable())
    {
        pjetpack->fFlash = 1;
        pjetpack->tFlash = g_clock.t;
        pjetpack->dradBodyX = 0.0f;
        pjetpack->dradBodyY = 3.0f;

        glm::vec3 posTarget{};
        glm::vec3 vTarget{};
        CalculateJetpackTarget(pjetpack, &posTarget, &vTarget);
        CalculateAloTransformAdjust(pjetpack->paloMovementBasis, nullptr, &posTarget, nullptr, &vTarget, nullptr);
        pjetpack->pvtalo->pfnSetAloVelocityVec(pjetpack, &vTarget);

        SetJetpackState(pjetpack, JETPACKS_Charm);
    }
    else
        SetJetpackState(pjetpack, JETPACKS_Dead);

    MSGDMG msgdmg = { pjetpack, pzpr->ploSource };
    pjetpack->pvtlo->pfnSendLoMessage(pjetpack, MSGID_damaged, &msgdmg);
    return 1;
}

bool JthsCurrentJetpack(JETPACK* pjetpack)
{
    return pjetpack->fFlash != 0;
}

void UseJetpackCharm(JETPACK* pjetpack)
{
    UsePoCharm(pjetpack);
    SetJetpackState(pjetpack, JETPACKS_Active);
}

void HandleJetpackMessage(JETPACK* pjetpack, MSGID msgid, void* pv)
{
    if (msgid == MSGID_label && pv != nullptr)
    {
        auto* message = static_cast<ASEGLABELMSG*>(pv);

        if (message->pasega == pjetpack->pasegaCur)
        {
            if (message->label == 350) 
                pjetpack->fCharmEnabled = 1;
            else if (message->label == 351) 
                pjetpack->fCharmEnabled = 0;
            else if (message->label == 1127) 
                pjetpack->fExitPending = 1;
        }
    }

    HandlePoMessage(pjetpack, msgid, pv);
}

void SetJetpackState(JETPACK* pjetpack, JPK jpk)
{
    if (pjetpack->jpk == jpk)
        return;

    if (pjetpack->jpk > JETPACKS_Active && pjetpack->jpk < JETPACKS_Max)
        pjetpack->tCharm = g_clock.t;

    ASEG* paseg = nullptr;
    switch (jpk)
    {
        case JETPACKS_Active:
        g_lgnr.vScreen = glm::vec3(0.0f);
        g_lgnr.posScreen = glm::vec3(2000.0f, 0.0f, 0.0f);
        g_lgnr.pvtblot->pfnSetBlotBlots(&g_lgnr, BLOTS_Visible);
        paseg = pjetpack->pasegblDynamic;
        break;
        case JETPACKS_Unknown1: paseg = pjetpack->pasegUnknown1; break;
        case JETPACKS_Charm: paseg = pjetpack->pasegCharm; pjetpack->tCharm = 0.0f; break;
        case JETPACKS_Dead:
        paseg = pjetpack->pasegDead;
        pjetpack->tCharm = 0.0f;
        if (pjetpack->paloEffect != nullptr)
            SetAloTransformBasis(pjetpack->paloEffect, pjetpack, pjetpack);
        break;
    default: break;
    }

    pjetpack->jpk = jpk;
    pjetpack->tJetpacks = g_clock.t;

    if (paseg != nullptr)
        ApplyAsegCur(paseg, pjetpack, 0.0f, 1.0f, (GRFAPL)1, &pjetpack->pasegaCur);
}

void ResetJetpackTransform(JETPACK* pjetpack)
{
    glm::vec3 pos{};
    glm::vec3 v{};
    CalculateJetpackTarget(pjetpack, &pos, &v);
    CalculateAloTransformAdjust(pjetpack->paloMovementBasis, nullptr, &pos, nullptr, &v, nullptr);
    pjetpack->pvtalo->pfnTranslateAloToPos(pjetpack, &pos);
    pjetpack->pvtalo->pfnSetAloVelocityVec(pjetpack, &v);
}

void DeleteJetpack(JETPACK* pjetpack)
{
    delete pjetpack;
}
