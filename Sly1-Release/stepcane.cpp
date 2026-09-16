#include "stepcane.h"
#include "jt.h"
#include "target.h"
#include "actseg.h"
#include "pipe.h"
#include "hnd.h"
#include "rchm.h"
#include "actadj.h"
#include "sm.h"

void SetJtJtcs(JT* pjt, int jtcs)
{
    const int jtcsOld = pjt->jtcs;

    if (jtcsOld == jtcs)
        return;

    /*
     * The release contains a transition table at 0x00274BA0.
     * It identifies transitions that continue/reverse the current cane
     * animation instead of starting an unrelated animation.
     */
    bool fContinueAseg = false;

    if (jtcs != -1 && jtcsOld != -1)
        fContinueAseg = jtcs == s_mjtcsjtcsSweepTransition[jtcsOld];

    if (pjt->psoSweepEffect != nullptr && !fContinueAseg)
        pjt->psoSweepEffect->pvtlo->pfnRemoveLo(pjt->psoSweepEffect);

    pjt->grfjtsc &= ~2u;

    switch (jtcsOld)
    {
        case 0:
        pjt->grfpvaXpValid = 0;
        break;

        case 1:
        StartSound(pjt->phndCur->sfxidRelease, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        pjt->grfpvaXpValid = 0;
        break;

        case 2:
        StartSound(pjt->ppipeCur->sfxidRelease, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        pjt->grfpvaXpValid = 0;
        break;

        case 6:
        case 7:
        case 8:
        case 9:
        if (pjt->jts == 0)
            SeekSma(pjt->psmaCur, (OID)326);
        break;
    }

    float t = 0.0f;
    float svt = 1.0f;
    float dtForce = 0.0f;

    ASEGBL* pasegbl = nullptr;
    int sfxid = -1;

    /*
     * State 3 selects an actual forward/back attack.
     *
     * Raw JTCS values:
     *   3 = SweepInit
     *   4 = SweepFore
     *   5 = SweepBack
     *   6 = StandingSweepFore
     *   7 = StandingSweepBack
     */
    if (jtcs == 3)
    {
        pjt->cpaloIgnoreEffect = 0;
        pjt->tSweepPending = -1.0f;

        glm::vec3 posHookLocal(0.0f);
        ConvertAloPos(nullptr, reinterpret_cast<ALO*>(pjt), &pjt->paloHook->xf.posWorld, &posHookLocal);

        const glm::vec3 posSweep = (posHookLocal + pjt->paloRightHand->xf.pos) * 0.5f;
        const bool fSweepFore = g_fForceFore != 0 || (g_fForceBack == 0 && posSweep.y < 0.0f);

        if (pjt->jts == 0)
            jtcs = fSweepFore ? 6 : 7;
        else
            jtcs = fSweepFore ? 4 : 5;
    }

    switch (jtcs)
    {
        case 0:
        pasegbl = pjt->prchmReach->pasegbl;
        sfxid = 13;
        t = pasegbl->amrsgc[0].amrsg[1].t;
        svt = 0.0f;

        pjt->grfpvaXpValid = 0;
        pjt->phndUnhook = nullptr;
        pjt->ppipeUnhook = nullptr;
        pjt->tContact = g_clock.t + 0.1f;
        break;

        case 1:
        case 2:
        pjt->grfpvaXpValid = 0;
        break;

        case 4:
        pjt->cpaloIgnoreEffect = 0;

        ChooseJtSweepTarget(pjt, pjt->ablSweepAttackFore, pjt->pasegblSweepAttackFore);

        pasegbl = pjt->pasegblSweepAttackFore;
        dtForce = DT_JtSweepForce;
        sfxid = 11;
        pjt->tContact = g_clock.t + DT_JtSweepForeContact;
        break;

        case 5:
        pjt->cpaloIgnoreEffect = 0;

        ChooseJtSweepTarget(pjt, pjt->ablSweepAttackBack, pjt->pasegblSweepAttackBack);

        pasegbl = pjt->pasegblSweepAttackBack;
        dtForce = DT_JtSweepForce;
        sfxid = 11;
        pjt->tContact = g_clock.t + DT_JtSweepBackContact;
        break;

        case 6:
        pjt->cpaloIgnoreEffect = 0;

        ChooseJtSweepTarget(pjt, pjt->ablStandingSweepAttackFore, pjt->pasegblStandingSweepAttackFore);

        pasegbl = pjt->pasegblStandingSweepAttackFore;
        dtForce = DT_JtSweepForce;
        sfxid = 11;
        pjt->tContact = g_clock.t + DT_JtSweepForeContact;

        if (IsmsFindSmOptional(pjt->psmIdle, (OID)335) >= 0)
            SeekSma(pjt->psmaCur, (OID)335);
        break;

        case 7:
        pjt->cpaloIgnoreEffect = 0;

        ChooseJtSweepTarget(pjt, pjt->ablStandingSweepAttackBack, pjt->pasegblStandingSweepAttackBack);

        pasegbl = pjt->pasegblStandingSweepAttackBack;
        dtForce = DT_JtSweepForce;
        sfxid = 11;
        pjt->tContact = g_clock.t + DT_JtSweepBackContact;

        if (IsmsFindSmOptional(pjt->psmIdle, (OID)335) >= 0)
            SeekSma(pjt->psmaCur, (OID)335);
        break;

        case 8:
        pasegbl = pjt->pasegblStandingSweepForeReturn;
        break;

        case 9:
        pasegbl = pjt->pasegblStandingSweepBackReturn;
        break;

        case 10:
        pjt->grfpvaXpValid = 0;
        pasegbl = pjt->pasegblPipeEnter;
        break;

        case 11:
        pjt->grfpvaXpValid = 0;
        pasegbl = pjt->pasegblPipeExit;
        break;

        case -1:
        pjt->ptargetCur = nullptr;
        pjt->phndCur = nullptr;
        pjt->ppipeCur = nullptr;
        break;
    }

    if (fContinueAseg)
    {
        ASEGA* pasegaOld = pjt->pasegaJtcs;

        t = pasegaOld->tLocal;

        if (pasegaOld->svtLocal == 0.0f)
        {
            if (t == 0.0f)
                svt = -1.0f;
            else if (t == pasegaOld->paseg->tMax)
                svt = 1.0f;
        }
        else
            svt = pasegaOld->svtLocal;

        svt /= pasegaOld->svtMaster;
        sfxid = -1;
    }
    else
    {
        pjt->fCaneInterruptible = 0;
    }

    const bool fUsingKeyOld = jtcsOld == 2 || jtcsOld == 11;
    const bool fUsingKeyNew = jtcs == 2 || jtcs == 11;

    if (fUsingKeyOld != fUsingKeyNew)
    {
        if (fUsingKeyNew)
        {
            pjt->paloKey->pvtalo->pfnAddLo(pjt->paloKey);
            pjt->paloKey->pvtalo->pfnMatchAloOtherObject(pjt->paloKey, pjt->paloTool);
            pjt->paloTool->pvtalo->pfnRemoveLo(pjt->paloTool);

            /*
             * The release assigns jtcs here before applying the animation.
             */
            pjt->jtcs = (JTCS)jtcs;
        }
        else
        {
            pjt->paloTool->pvtalo->pfnAddLo(pjt->paloTool);
            pjt->paloTool->pvtalo->pfnMatchAloOtherObject(pjt->paloTool, pjt->paloKey);
            pjt->paloKey->pvtalo->pfnRemoveLo(pjt->paloKey);
        }
    }

    pjt->jtcs = (JTCS)jtcs;
    pjt->cposHook = 0;
    pjt->tJtcs = g_clock.t;

    ApplyAsegCur(pasegbl, pjt, t, svt, 1, &pjt->pasegaJtcs);

    if (dtForce != 0.0f && pjt->pasegaJtcs != nullptr)
    {
        for (ACTSEG* pactseg = pjt->pasegaJtcs->dlActseg.pactsegFirst; pactseg != nullptr; pactseg = pactseg->dleAsega.pactsegNext)
        {
            if (pactseg->ackPos == ACK_Smooth)
            {
                pactseg->ackPos = ACK_SmoothForce;
                pactseg->tMatch = g_clock.t + dtForce;
            }

            if (pactseg->ackRot == ACK_Smooth)
            {
                pactseg->ackRot = ACK_SmoothForce;
                pactseg->tMatch = g_clock.t + dtForce;
            }
        }
    }

    if (sfxid != -1)
        StartSound((SFXID)sfxid, nullptr, nullptr, &pjt->xf.posWorld, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
}

void UpdateJtCane(JT* pjt)
{
    if (pjt->ptargetCur != nullptr && !FIsLoInWorld(reinterpret_cast<LO*>(pjt->ptargetCur)))
    {
        pjt->ptargetCur = nullptr;
        pjt->phndCur = nullptr;

        if (pjt->jtcs != 0)
            return;

        SetJtJtcs(pjt, -1);
    }

    if (pjt->jtcs != 0)
        return;

    if (pjt->ptargetCur != nullptr)
        TrackJtTarget(pjt, pjt->prchmReach, pjt->ptargetCur);
    else if (pjt->ppipeCur != nullptr)
        TrackJtPipe(pjt, pjt->prchmReach, pjt->ppipeCur, &pjt->sPipeCur);
}

void UpdateJtActiveCane(JT* pjt, JOY* pjoy)
{
    constexpr unsigned short GRFBTN_Sweep = 128;

    bool fCanSweep = true;
    const bool fSweepPressed = pjoy->IsPressed(BTN_SQUARE);

    switch (pjt->jts)
    {
        case 0:
        if (pjt->pasegblStandingSweepAttackFore != nullptr && pjt->pasegblStandingSweepAttackBack != nullptr)
        {
            if (pjt->jtcs == 4)
                SetJtJtcs(pjt, 6);
            else if (pjt->jtcs == 5)
                SetJtJtcs(pjt, 7);
        }
        break;

        case 2:
        case 4:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        fCanSweep = false;
        break;

        case 3:
        if (fSweepPressed)
        {
            fCanSweep = false;
            SetJtJts(pjt, 2, 2);
        }
        break;

        case 6:
        if (pjt->jthk != 4 && pjt->jthk != 5)
            fCanSweep = false;
        break;

        default:
        break;
    }

    if (fSweepPressed)
        pjt->tSweepPending = g_clock.t;

    int jtcsNext = pjt->jtcs;

    if (jtcsNext == -1)
    {
        if (fCanSweep && g_clock.t - pjt->tSweepPending < 0.35f)
            jtcsNext = 3;
    }
    else if (jtcsNext >= 4 && jtcsNext <= 7)
    {
        if (fCanSweep && pjt->fCaneInterruptible != 0 && g_clock.t - pjt->tSweepPending < 0.35f)
            jtcsNext = 3;
    }

    SetJtJtcs(pjt, jtcsNext);
}

void ChooseJtAttackTarget(JT* pjt, GRFTAK grftak, glm::vec3* pposNaturalLocal, float dtProj, float dradMax, TARGET** pptarget, glm::vec3* pdposProj)
{
    constexpr float S_TargetBestInitial = 300.0f;

    TARGET* ptargetBest = nullptr;
    glm::vec3 dposBest(0.0f);
    float sBest = S_TargetBestInitial;

    glm::mat3 matJtProj(1.0f);
    LoadRotateMatrixRad(pjt->radTarget, &g_normalZ, &matJtProj);

    const glm::vec3 posJtProj = pjt->xf.pos + pjt->xf.v * dtProj;

    glm::vec3 posNatural;
    ConvertAloPos(pjt, nullptr, pposNaturalLocal, &posNatural);

    glm::vec3 posNaturalProj;
    posNaturalProj.x = matJtProj[0].x * pposNaturalLocal->x + matJtProj[1].x * pposNaturalLocal->y + matJtProj[2].x * pposNaturalLocal->z;
    posNaturalProj.y = matJtProj[0].y * pposNaturalLocal->x + matJtProj[1].y * pposNaturalLocal->y + matJtProj[2].y * pposNaturalLocal->z;
    posNaturalProj.z = matJtProj[0].z * pposNaturalLocal->x + matJtProj[1].z * pposNaturalLocal->y + matJtProj[2].z * pposNaturalLocal->z;
    posNaturalProj += posJtProj;

    const glm::vec3 dposAdjust = posNaturalProj - posNatural;

    for (TARGET* ptarget = g_dlTarget.ptargetFirst; ptarget != nullptr; ptarget = ptarget->dleTarget.ptargetNext)
    {
        if ((ptarget->grftak & grftak) == 0)
            continue;

        glm::vec3 posTargetParentProj(0.0f);
        glm::mat3 matTargetParentProj(1.0f);

        PredictAloTransform(ptarget->paloParent, nullptr, dtProj, &posTargetParentProj, &matTargetParentProj, nullptr, nullptr);

        glm::vec3 posTarget;
        GetXfmPos(ptarget, &posTarget);
        posTarget += dposAdjust;

        glm::vec3 posTargetProj;
        posTargetProj.x = matTargetParentProj[0].x * ptarget->posLocal.x + matTargetParentProj[1].x * ptarget->posLocal.y + matTargetParentProj[2].x * ptarget->posLocal.z;
        posTargetProj.y = matTargetParentProj[0].y * ptarget->posLocal.x + matTargetParentProj[1].y * ptarget->posLocal.y + matTargetParentProj[2].y * ptarget->posLocal.z;
        posTargetProj.z = matTargetParentProj[0].z * ptarget->posLocal.x + matTargetParentProj[1].z * ptarget->posLocal.y + matTargetParentProj[2].z * ptarget->posLocal.z;
        posTargetProj += posTargetParentProj;

        float u;
        float s;
        FindClosestPointOnLineSegment(&posNaturalProj, &posTarget, &posTargetProj, &u, &s);

        if (ptarget->paloParent != nullptr && (ptarget->paloParent->paloRoot->pvtlo->grfcid & 8) != 0)
            s += DS_JtAttackPoBonus;

        if (s >= sBest)
            continue;

        const glm::vec3 dposTarget = posTarget - pjt->xf.posWorld;
        const float radTarget = std::atan2(dposTarget.y, dposTarget.x);
        const float dradTarget = RadNormalize(radTarget - pjt->radTarget);

        if (std::abs(dradTarget) > dradMax)
            continue;

        ptargetBest = ptarget;
        dposBest = posTargetProj - posJtProj;
        sBest = s;
    }

    *pptarget = ptargetBest;
    *pdposProj = dposBest;
}

void ChooseJtSweepTarget(JT* pjt, BL* ablSweep, ASEGBL* pasegbl)
{
    TARGET* ptarget = nullptr;
    glm::vec3 dposTarget(0.0f);

    const float dradMax = pjt->jts == 0 ? 3.15f : 1.6f;

    ChooseJtAttackTarget(pjt, 2, &s_posSweep, 0.15f, dradMax, &ptarget, &dposTarget);

    float uSweep = 1.0f;

    if (ptarget != nullptr)
        uSweep = std::clamp((dposTarget.z + 50.0f) / 110.0f, 0.0f, 1.0f);

    if (g_uSweepFixed >= 0.0f)
        uSweep = g_uSweepFixed;

    ablSweep[0].u = 1.0f - uSweep;
    ablSweep[1].u = uSweep;

    ReblendAsegbl(pasegbl, sizeof(BL), 2, ablSweep);

    pjt->phndCur = nullptr;
    pjt->ptargetCur = ptarget;
    pjt->ppipeCur = nullptr;

    if (ptarget != nullptr && pjt->jts == 0)
    {
        pjt->radTarget = std::atan2(dposTarget.y, dposTarget.x);
        FixStepAngularVelocity(pjt);
    }
}

void ChooseJtRushTarget(JT* pjt)
{
    TARGET* ptarget = nullptr;
    glm::vec3 dposTarget(0.0f);

    ChooseJtAttackTarget(pjt, 8, &s_posRush, 0.25f, 1.0f, &ptarget, &dposTarget);

    pjt->ptargetCur = ptarget;

    if (ptarget != nullptr)
    {
        pjt->radTarget = std::atan2(dposTarget.y, dposTarget.x);
        FixStepAngularVelocity(pjt);
    }
}

void ChooseJtSmashTarget(JT* pjt)
{
    TARGET* ptarget = nullptr;
    glm::vec3 dposTarget(0.0f);

    ChooseJtAttackTarget(pjt, 4, &s_posSmash, 0.25f, 3.15f, &ptarget, &dposTarget);

    pjt->ptargetCur = ptarget;

    if (ptarget != nullptr)
    {
        pjt->radTarget = std::atan2(dposTarget.y, dposTarget.x);
        FixStepAngularVelocity(pjt);
    }
}
