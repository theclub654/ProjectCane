#include "suv.h"
#include "sm.h"
#include "pnt.h"
#include "shape.h"
#include "chkpnt.h"
#include "stepguard.h"
#include "murray.h"
#include "scores.h"
#include "van.h"
#include "timer.h"
#include "bomb.h"
#include "break.h"
#include "sound.h"
#include <cstdio>

SUV* NewSuv()
{
	return new SUV{};
}

int GetSuvSize()
{
    return sizeof(SUV);
}

void InitSuv(SUV* psuv)
{
	InitPo(psuv);

    psuv->sRadiusFrontWheel = 50.0f;
    psuv->sRadiusRearWheel = 50.0f;

    psuv->svMax = 3000.0f;
    psuv->dyMax = 1300.0f;

    psuv->muSxp = 2.0f;

    psuv->cLapMax = 3;

    psuv->clqTune = s_clqTune;

    psuv->asvrb[0] = s_asvrb;
    psuv->asvrb[1] = s_asvrb;
    psuv->svrb = s_asvrb;

    ResetSuv(psuv);
}

void SetSuvSRadiusFrontWheel(SUV* psuv, float sRadiusFrontWheel)
{
    psuv->sRadiusFrontWheel = sRadiusFrontWheel;
}

void* GetSuvSRadiusFrontWheel(SUV* psuv)
{
    return &psuv->sRadiusFrontWheel;
}

// Offset 0x614 - g_aeopid[1081]
void SetSuvSRadiusRearWheel(SUV* psuv, float sRadiusRearWheel)
{
    psuv->sRadiusRearWheel = sRadiusRearWheel;
}

void* GetSuvSRadiusRearWheel(SUV* psuv)
{
    return &psuv->sRadiusRearWheel;
}

// Offset 0x618 - g_aeopid[1082]
void SetSuvSvMax(SUV* psuv, float svMax)
{
    psuv->svMax = svMax;
}

void* GetSuvSvMax(SUV* psuv)
{
    return &psuv->svMax;
}

// Offset 0x61C - g_aeopid[1083]
void SetSuvDyMax(SUV* psuv, float dyMax)
{
    psuv->dyMax = dyMax;
}

void* GetSuvDyMax(SUV* psuv)
{
    return &psuv->dyMax;
}

void SetSuvCLapMax(SUV* psuv, int cLapMax)
{
    psuv->cLapMax = cLapMax;
}

void* GetSuvCLapMax(SUV* psuv)
{
    return &psuv->cLapMax;
}

void SetSuvPpathzone(SUV* psuv, PATHZONE* ppathzone)
{
    psuv->ppathzone = ppathzone;
}

void* GetSuvPpathzone(SUV* psuv)
{
    return &psuv->ppathzone;
}

void SetSuvSuvgk(SUV* psuv, SUVGK suvgk)
{
    psuv->suvgk = suvgk;
}

void* GetSuvSuvgk(SUV* psuv)
{
    return &psuv->suvgk;
}

void SetSuvPexplDirt(SUV* psuv, EXPL* pexplDirt)
{
    psuv->pexplDirt = pexplDirt;
}

void* GetSuvPexplDirt(SUV* psuv)
{
    return &psuv->pexplDirt;
}

void SetSuvPexplDust(SUV* psuv, EXPL* pexplDust)
{
    psuv->pexplDust = pexplDust;
}

void* GetSuvPexplDust(SUV* psuv)
{
    return &psuv->pexplDust;
}

void SetSuvPemitterBoost(SUV* psuv, EMITTER* pemitterBoost)
{
    psuv->pemitterBoost = pemitterBoost;
}

void* GetSuvPemitterBoost(SUV* psuv)
{
    return &psuv->pemitterBoost;
}

void SetSuvMuSxp(SUV* psuv, float muSxp)
{
    psuv->muSxp = muSxp;
}

void* GetSuvMuSxp(SUV* psuv)
{
    return &psuv->muSxp;
}

void SetSuvCBoost(SUV* psuv, int cBoost)
{
    psuv->cBoost = cBoost;
}

void* GetSuvCBoost(SUV* psuv)
{
    return &psuv->cBoost;
}

void SetSuvClqDuToRsvAhead(SUV* psuv, CLQ clq)
{
    psuv->svrb.clqDuToRsvAhead = clq;
}

void* GetSuvClqDuToRsvAhead(SUV* psuv)
{
    return &psuv->svrb.clqDuToRsvAhead;
}

void SetSuvClqDuToRsvBehind(SUV* psuv, CLQ clq)
{
    psuv->svrb.clqDuToRsvBehind = clq;
}

void* GetSuvClqDuToRsvBehind(SUV* psuv)
{
    return &psuv->svrb.clqDuToRsvBehind;
}

void SetSuvLmRsvRandom(SUV* psuv, LM lm)
{
    psuv->svrb.lmRsvRandom = lm;
}

void* GetSuvLmRsvRandom(SUV* psuv)
{
    return &psuv->svrb.lmRsvRandom;
}

void SetSuvLmRsvBalance(SUV* psuv, LM lm)
{
    psuv->svrb.lmRsvBalance = lm;
}

void* GetSuvLmRsvBalance(SUV* psuv)
{
    return &psuv->svrb.lmRsvBalance;
}

void CloneSuv(SUV* psuv, SUV* psuvBase)
{
    ClonePo(psuv, psuvBase);

    psuv->sRadiusFrontWheel = psuvBase->sRadiusFrontWheel;
    psuv->sRadiusRearWheel = psuvBase->sRadiusRearWheel;
    psuv->svMax = psuvBase->svMax;
    psuv->dyMax = psuvBase->dyMax;
    psuv->clqTune = psuvBase->clqTune;

    for (int i = 0; i < 2; ++i)
        psuv->asvrb[i] = psuvBase->asvrb[i];

    psuv->svrb = psuvBase->svrb;
    psuv->paloShadow = psuvBase->paloShadow;
    psuv->radTarget = psuvBase->radTarget;
    psuv->svTarget = psuvBase->svTarget;
    psuv->radFront = psuvBase->radFront;
    psuv->xsxp = psuvBase->xsxp;

    for (int i = 0; i < 4; ++i)
        psuv->asxp[i] = psuvBase->asxp[i];

    psuv->pshapeTrack = psuvBase->pshapeTrack;
    psuv->sTrackMax = psuvBase->sTrackMax;
    psuv->uTrack = psuvBase->uTrack;
    psuv->sTrack = psuvBase->sTrack;
    psuv->dyTrack = psuvBase->dyTrack;
    psuv->dyTarget = psuvBase->dyTarget;
    psuv->dsTrackFinish = psuvBase->dsTrackFinish;
    psuv->pshapeLine = psuvBase->pshapeLine;
    psuv->uLine = psuvBase->uLine;
    psuv->tUpright = psuvBase->tUpright;
    psuv->tBoost = psuvBase->tBoost;
    psuv->cBoost = psuvBase->cBoost;
    psuv->rsvBalance = psuvBase->rsvBalance;
    psuv->rsvGoal = psuvBase->rsvGoal;
    psuv->tBalance = psuvBase->tBalance;
    psuv->muSxp = psuvBase->muSxp;
    psuv->tPunched = psuvBase->tPunched;
    psuv->csve = psuvBase->csve;

    for (int i = 0; i < 16; ++i)
        psuv->asve[i] = psuvBase->asve[i];

    psuv->psveCheckFirst = psuvBase->psveCheckFirst;
    psuv->psveCheckCur = psuvBase->psveCheckCur;
    psuv->psveFeatureCur = psuvBase->psveFeatureCur;
    psuv->cLap = psuvBase->cLap;
    psuv->cLapMax = psuvBase->cLapMax;
    psuv->nPlace = psuvBase->nPlace;
    psuv->nPlaceMax = psuvBase->nPlaceMax;
    psuv->tPlace = psuvBase->tPlace;
    psuv->pemitterBoost = psuvBase->pemitterBoost;
    psuv->pexplDirt = psuvBase->pexplDirt;
    psuv->pexplDust = psuvBase->pexplDust;
    psuv->cParticleDirt = psuvBase->cParticleDirt;
    psuv->cParticleDust = psuvBase->cParticleDust;
    psuv->paloFrontAxle = psuvBase->paloFrontAxle;
    psuv->paloRearAxle = psuvBase->paloRearAxle;
    psuv->suvgk = psuvBase->suvgk;
    psuv->suvs = psuvBase->suvs;
    psuv->tSuvs = psuvBase->tSuvs;
    psuv->ppathzone = psuvBase->ppathzone;
    psuv->psoPrizeCur = psuvBase->psoPrizeCur;
    psuv->psmPuncher = psuvBase->psmPuncher;
    psuv->psmaPuncher = psuvBase->psmaPuncher;
    psuv->psoPuncher = psuvBase->psoPuncher;
    psuv->cpsoIgnore = psuvBase->cpsoIgnore;

    for (int i = 0; i < 8; ++i)
        psuv->apsoIgnore[i] = psuvBase->apsoIgnore[i];

    psuv->pxpPuncher = psuvBase->pxpPuncher;
    psuv->pmurray = psuvBase->pmurray;
    psuv->fFreeWheeling = psuvBase->fFreeWheeling;
    psuv->pambSkid = psuvBase->pambSkid;
    psuv->pambRunning = psuvBase->pambRunning;
    psuv->pambIdle = psuvBase->pambIdle;
    psuv->pambBoost = psuvBase->pambBoost;
    psuv->pambTireSlip = psuvBase->pambTireSlip;
    psuv->volBoostTarget = psuvBase->volBoostTarget;
    psuv->volBoostCur = psuvBase->volBoostCur;
    psuv->volTarget = psuvBase->volTarget;
    psuv->volCur = psuvBase->volCur;
    psuv->dvolInc = psuvBase->dvolInc;
    psuv->frqTarget = psuvBase->frqTarget;
    psuv->frqCur = psuvBase->frqCur;
    psuv->dfrqInc = psuvBase->dfrqInc;
    psuv->dfrqTop = psuvBase->dfrqTop;
    psuv->dfrqTireSlip = psuvBase->dfrqTireSlip;
    psuv->dfrqRunning = psuvBase->dfrqRunning;
    psuv->dfrqIdle = psuvBase->dfrqIdle;
    psuv->dfrqExtra = psuvBase->dfrqExtra;
    psuv->uSuspensionLoad = psuvBase->uSuspensionLoad;
    psuv->clqProgressToRsv = psuvBase->clqProgressToRsv;
    psuv->lmRsvBalance = psuvBase->lmRsvBalance;
    psuv->uEngineSpeed = psuvBase->uEngineSpeed;
}

void PostSuvLoad(SUV* psuv)
{
    LO* aplo[16];

    PostAloLoad(psuv);
    SnipAloObjects(psuv, 20, s_asnipSuv);

    psuv->radTarget = atan2f(psuv->xf.mat[0][1], psuv->xf.mat[0][0]);

    if (psuv->psmPuncher != nullptr) {
        psuv->psmaPuncher = PsmaApplySm(psuv->psmPuncher, psuv, OID_Nil, 0);

        int oidGoal = 1156;

        if (psuv->suvgk != SUVGK_Chase)
            oidGoal = 900;

        SetSmaGoal(psuv->psmaPuncher, (OID)oidGoal);
    }

    int sfxRunning = -1;
    int sfxIdle = -1;
    int sfxExtra = -1;

    switch (g_psw->ibnk)
    {
        case 3:
        sfxRunning = psuv->fPlayable ? 466 : 429;
        sfxIdle = 443;
        sfxExtra = 431;
        break;

        case 5:
        sfxRunning = psuv->fPlayable ? 678 : 640;
        sfxIdle = 639;
        sfxExtra = 637;
        break;

        case 6:
        sfxRunning = psuv->fPlayable ? 734 : 732;
        sfxIdle = 731;
        sfxExtra = 733;
        break;
    }

    if (sfxRunning != -1) 
    {
        StartSound((SFXID)sfxRunning, &psuv->pambRunning, psuv, nullptr, 6000.0f, 1000.0f, 1.0f, 0.0f, 0.5f, nullptr, nullptr);
        StartSound((SFXID)sfxIdle, &psuv->pambIdle, psuv, nullptr, 6000.0f, 1000.0f, 1.0f, 0.0f, 0.5f, nullptr, nullptr);
        StartSound((SFXID)sfxExtra, &psuv->pambTireSlip, psuv, nullptr, 6000.0f, 1000.0f, 1.0f, 0.0f, 0.5f, nullptr, nullptr);

        float dfrq = GRandInRange(-0.1f, 0.1f);

        psuv->dfrqTireSlip = dfrq;
        psuv->dfrqRunning = dfrq;
        psuv->dfrqIdle = dfrq;
        psuv->dfrqExtra = dfrq;
    }

    MURRAY* pmurray = (MURRAY*)PloFindSwObject(g_psw, 260, (OID)7, psuv);

    if (pmurray != nullptr) {
        psuv->pzi.paloCollect = (ALO*)pmurray;
        psuv->pzi.sAutoCollect = 200.0f;
        psuv->pmurray = pmurray;
    }

    if (psuv->suvgk == SUVGK_Chase) 
    {
        int csuvPrize = CploFindSwObjects(psuv->psw, 261, (OID)1062, nullptr, 16, aplo);

        for (int i = 0; i < csuvPrize; i++)
            aplo[i]->pvtlo->pfnSubscribeLoObject(aplo[i], psuv);

        g_cCpuCollected = 0;
        g_scores.cCpuCollected = 0;
        g_pscoresCur = &g_scores;
        g_scores.pnActual = &g_scores.cCpuCollected;
    }

    for (int i = 0; i < 4; i++)
        psuv->asxp[i].posCur = psuv->asxp[i].paloWheel->xf.pos;

    ResetSuv(psuv);
}

void PresetSuvAccel(SUV* psuv, float dt)
{
    PresetSoAccel((SO*)psuv, dt);


    if (g_clock.t - psuv->tUpright > DT_SuvUpright && g_clock.t - psuv->tPunched > DT_SuvPunched) {
        glm::mat3 matUpright;
        TiltMatUpright(&psuv->xf.mat, nullptr, &matUpright);
        AccelSoTowardMatSpring(psuv, &matUpright, &s_clqSuvUpright, nullptr, nullptr, dt);
    }

    float gSuspensionTotal = 0.0f;

    for (int i = 0; i < 4; i++) 
    {
        SXP* psxp = &psuv->asxp[i];

        if (psxp->xsxp != psuv->xsxp) {
            continue;
        }

        float sRadiusWheel = i > 1 ? psuv->sRadiusRearWheel : psuv->sRadiusFrontWheel;

        glm::vec3 posTop = psxp->ppntTop->posLocal;
        posTop.z -= sRadiusWheel;
        ConvertAloPos(psuv, nullptr, &posTop, &posTop);

        glm::vec3 posRest = psxp->paloWheel->posOrig;
        posRest.z -= sRadiusWheel;
        ConvertAloPos(psuv, nullptr, &posRest, &posRest);

        glm::vec3 dposSpring = posTop - posRest;
        float sSpring = glm::length(dposSpring);

        if (sSpring <= 0.0001f)
            continue;

        glm::vec3 normalSpring = dposSpring / sSpring;

        glm::vec3 vSuvAtGround;
        CalculateAloMovement(psuv, nullptr, psxp->posGround, &vSuvAtGround, nullptr, nullptr, nullptr);

        glm::vec3 vTangent = vSuvAtGround - psxp->normalGround * glm::dot(vSuvAtGround, psxp->normalGround);
        float svTangent = glm::length(vTangent);

        float cDust = CLQ_DustFromSpeed.g0 + svTangent * (CLQ_DustFromSpeed.g1 + svTangent * CLQ_DustFromSpeed.g2);
        cDust = glm::clamp(cDust, C_DustMin, C_DustMax);

        float uSpring = glm::dot(psxp->posGround - posRest, normalSpring) / sSpring;
        uSpring = glm::clamp(uSpring, 0.0f, 1.0f);

        psxp->cParticleDust += cDust * dt;

        glm::vec3 dvGround = psxp->vGround - vSuvAtGround;
        glm::vec3 dposGround = psxp->posGround - posRest;

        float svSpring = glm::dot(dvGround, normalSpring);
        float sSpringCur = glm::dot(dposGround, normalSpring);

        float gSpring = svSpring * (SuvSpringVelocity.g0 + uSpring * (SuvSpringVelocity.g1 + uSpring * SuvSpringVelocity.g2)) * K_SuvSpringVelocity;
        gSpring += sSpringCur * (CLQ_SuvSpringPos.g0 + uSpring * (CLQ_SuvSpringPos.g1 + uSpring * CLQ_SuvSpringPos.g2)) * K_SuvSpringPos;

        if (gSpring < 0.0f)
            continue;

        glm::vec3 posForce = psxp->posGround - psuv->xf.posWorld;
        glm::vec3 force = normalSpring * (gSpring * psuv->m);
        glm::vec3 accel = force / psuv->m;
        glm::vec3 torque = glm::cross(posForce, force);

        glm::vec3 angularAccel;
        CalculateSoAngularEffectWorld(psuv, &torque, &angularAccel);

        AddSoAcceleration(psuv, &accel);
        AddSoAngularAcceleration(psuv, &angularAccel);

        gSuspensionTotal += glm::dot(force, psuv->xf.mat[2]);
    }

    if (gSuspensionTotal == 0.0f) 
    {
        float uRecover = (g_clock.t - psuv->tPunched) / DT_SuvPunched;
        uRecover = CLQ_AirborneUpright.g0 + uRecover * (CLQ_AirborneUpright.g1 + uRecover * CLQ_AirborneUpright.g2);
        uRecover = glm::clamp(uRecover, 0.0f, 1.0f);

        glm::vec3 wTarget = g_vecZero * uRecover;
        AccelSoTowardMatSpring(psuv, nullptr, nullptr, &wTarget, nullptr, dt);
    }

    psuv->uSuspensionLoad = glm::clamp(gSuspensionTotal / (psuv->m * G_SuvSuspensionLoadMax), 0.0f, 1.0f);

    float mu = psuv->psveFeatureCur != nullptr ? psuv->psveFeatureCur->mu : psuv->muSxp;
    float gTraction = (mu * gSuspensionTotal) / psuv->m;

    float uBoost = (g_clock.t - psuv->tBoost) / DT_SuvBoost;
    float rAccel = CLQ_SuvBoostAccel.g0 + uBoost * (CLQ_SuvBoostAccel.g1 + uBoost * CLQ_SuvBoostAccel.g2);
    rAccel = glm::clamp(rAccel, R_SuvBoostAccelMin, R_SuvBoostAccelMax);

    if (psuv->rsvBalance > 1.0f)
        rAccel *= psuv->rsvBalance * psuv->rsvBalance;

    float svTraction = glm::min(gTraction, SV_SuvTractionMax) * rAccel;

    glm::vec3 vPred = psuv->xf.v + psuv->xf.dv * dt;
    glm::vec3 vLocal = glm::transpose(psuv->xf.mat) * vPred;

    float uTargetSpeed = glm::clamp(psuv->svTarget / psuv->svMax, 0.0f, 1.0f);
    float uSpeed = vLocal.x / psuv->svMax;

    float rSpeed = CLQ_SuvSpeedToDrive.g0 + uSpeed * (CLQ_SuvSpeedToDrive.g1 + uSpeed * CLQ_SuvSpeedToDrive.g2);
    rSpeed = glm::clamp(rSpeed, 0.0f, 1.0f);

    glm::vec3 wLocal = glm::transpose(psuv->xf.mat) * psuv->xf.w;

    float gYawMax = (psuv->m * fabsf(psuv->asxp[0].paloWheel->posOrig.x)) / psuv->momintLocal[2][2];
    const float wheelbase = psuv->asxp[0].paloWheel->posOrig.x - psuv->asxp[2].paloWheel->posOrig.x;
    float dwYaw = (((glm::min(vLocal.x, SV_SuvSteerMax) * psuv->radFront) / wheelbase + uTargetSpeed * rSpeed * psuv->radFront * K_SuvSteer) - wLocal.z) / dt;
    dwYaw = GLimitAbs(dwYaw, gYawMax * svTraction);

    glm::vec3 dwWorld = psuv->xf.mat * glm::vec3(0.0f, 0.0f, dwYaw);
    AddSoAngularAcceleration(psuv, &dwWorld);

    svTraction -= fabsf(dwYaw / gYawMax);

    float rDrive = CLQ_SuvBoostDrive.g0 + uBoost * (CLQ_SuvBoostDrive.g1 + uBoost * CLQ_SuvBoostDrive.g2);
    rDrive = glm::clamp(rDrive, R_SuvDriveMin, R_SuvDriveMax) * psuv->rsvBalance;

    float svDrive = glm::min(rDrive, R_SuvDriveMax) / psuv->rsvBalance;

    glm::vec3 vTarget;
    SetVectorCylind(&vTarget, psuv->radTarget, 1.0f, 0.0f);

    float rFacing = CLQ_SuvFacingDrive.g0 + glm::dot(vTarget, psuv->xf.mat[0]) * (CLQ_SuvFacingDrive.g1 + glm::dot(vTarget, psuv->xf.mat[0]) * CLQ_SuvFacingDrive.g2);
    rFacing = glm::clamp(rFacing, R_SuvFacingDriveMin, R_SuvFacingDriveMax);

    glm::mat3 matSpin;
    LoadRotateMatrixRad((wLocal.z + dwYaw * dt * 0.5f) * dt, &g_normalZ, &matSpin);

    if (svDrive > 1.0f)
        svDrive *= psuv->svMax;
    else
        svDrive *= rFacing * psuv->svTarget;

    glm::vec3 vDrive = matSpin * glm::vec3(svDrive, 0.0f, 0.0f);

    float rAccelLimit = CLQ_SuvAccelLimit.g0 + uBoost * (CLQ_SuvAccelLimit.g1 + uBoost * CLQ_SuvAccelLimit.g2);
    rAccelLimit = glm::clamp(rAccelLimit, R_SuvAccelLimitMin, R_SuvAccelLimitMax);

    glm::vec3 accelDrive = (vDrive - vLocal) / dt;
    accelDrive.z = 0.0f;

    float gAccelMax = glm::min(svTraction, rAccelLimit);
    LimitVectorLength(&accelDrive, gAccelMax, &accelDrive);

    if (psuv->svTarget != 0.0f && psuv->suvgk == SUVGK_Race) {
        float svCur = vLocal.x;

        if (svCur < psuv->svTarget) {
            float svSmooth;
            GSmooth(svCur, psuv->svTarget, dt, &s_smpSuvAccel, &svSmooth);
            accelDrive.x = glm::min(accelDrive.x, glm::min(svSmooth, rAccelLimit));
        }
        else {
            float svSmooth;
            GSmooth(svCur, psuv->svTarget, dt, &s_smpSuvDecel, &svSmooth);
            accelDrive.x = glm::max(accelDrive.x, svSmooth);
        }
    }

    glm::vec3 accelWorld = psuv->xf.mat * accelDrive;
    AddSoAcceleration(psuv, &accelWorld);

    for (int i = 0; i < 4; i++) {
        SXP* psxp = &psuv->asxp[i];

        if (psxp->xsxp != psuv->xsxp)
            continue;

        float sDirt = glm::length(accelDrive) * dt;
        float cDirt = CLQ_DirtFromAccel.g0 + sDirt * (CLQ_DirtFromAccel.g1 + sDirt * CLQ_DirtFromAccel.g2);
        cDirt = glm::clamp(cDirt, C_DirtMin, C_DirtMax);

        psxp->cParticleDirt += cDirt * g_clock.dt;
    }

    glm::vec3 downForce = psuv->xf.mat[2] * G_SuvDownForce;
    glm::vec3 driveForce = accelWorld * psuv->m;
    glm::vec3 driveTorque = glm::cross(downForce, driveForce);

    glm::vec3 angularDrive;
    CalculateSoAngularEffectWorld(psuv, &driveTorque, &angularDrive);

    AddSoAngularAcceleration(psuv, &angularDrive);

}

float GExcludeAlm(int clm, LM* alm, float g)
{
    if (!FCheckAlm(clm, alm, g))
        return g;

    float gBest = FLT_MAX;
    float dgBest = FLT_MAX;

    for (int i = 0; i < clm; i++) {
        float gMin = alm[i].gMin;
        float dgMin = fabsf(g - gMin);

        if (dgMin < dgBest && !FCheckAlm(clm, alm, gMin)) {
            gBest = gMin;
            dgBest = dgMin;
        }

        float gMax = alm[i].gMax;
        float dgMax = fabsf(g - gMax);

        if (dgMax < dgBest && !FCheckAlm(clm, alm, gMax)) {
            gBest = gMax;
            dgBest = dgMax;
        }
    }

    return gBest;
}

void UpdateSuvBalance(SUV* psuv)
{
    SUV* psuvCur = (SUV*)PpoCur();

    if (psuv == psuvCur)
        return;

    psuv->rsvBalance = GSmooth(psuv->rsvBalance, psuv->rsvGoal, g_clock.dt, &s_smpBalance, nullptr);

    if (psuv->tBalance > g_clock.t)
        return;

    float uProgress = (float)((psuv->cLap - 1) * 3 + psuv->psveCheckCur->isveCheck) * 0.1f;
    uProgress = glm::clamp(uProgress, 0.0f, 1.0f);

    float rsvGoal = psuv->clqProgressToRsv.g0 + uProgress * (psuv->clqProgressToRsv.g1 + uProgress * psuv->clqProgressToRsv.g2);
    rsvGoal = GLimitLm(&psuv->lmRsvBalance, rsvGoal);

    psuv->rsvGoal = rsvGoal + GRandInRange(-0.05f, 0.05f);
    psuv->tBalance = g_clock.t + GRandInRange(s_lmDtBalance.gMin, s_lmDtBalance.gMax);
}

float DsGetTrackRelative(float sMax, float s, float sOther)
{
    float ds = s - sOther;

    if (ds > sMax * 0.5f)
        ds -= sMax;
    else if (ds < sMax * -0.5f)
        ds += sMax;

    return ds;
}

int FIsSuvAheadOf(SUV* psuv, SUV* psuvOther)
{
    int dLap = psuv->cLap - psuvOther->cLap;

    if (dLap > 0)
        return 1;

    if (dLap < 0)
        return 0;

    int dCheck = psuv->psveCheckCur->isveCheck - psuvOther->psveCheckCur->isveCheck;

    if (dCheck > 0)
        return 1;

    if (dCheck < 0)
        return 0;

    float dsTrack = DsGetTrackRelative(psuv->sTrackMax, psuv->sTrack, psuvOther->sTrack);

    return dsTrack > 0.0f;
}

void UpdateSuvLine(SUV* psuv, int* pcpsuvInFront)
{
    SUV* apsuv[16];
    LM alm[18];

    int csuvInFront = 0;
    int clmAvoid = 0;

    int csuv = CploFindSwObjectsByClass(psuv->psw, 5, CID_SUV, nullptr, 16, (LO**)apsuv);

    for (int i = 0; i < csuv; i++) {
        SUV* psuvOther = apsuv[i];

        if (psuvOther == psuv)
            continue;

        if (psuvOther->pshapeTrack != psuv->pshapeTrack)
            continue;

        if (FIsSuvAheadOf(psuv, psuvOther))
            continue;

        csuvInFront++;

        float dsTrack = GModPositive(psuvOther->sTrack - psuv->sTrack, psuvOther->sTrackMax);

        float dyAvoid = CLQ_SuvDsToDyAvoid.g0 + dsTrack * (CLQ_SuvDsToDyAvoid.g1 + dsTrack * CLQ_SuvDsToDyAvoid.g2);
        dyAvoid = GLimitLm(&LM_SuvDyAvoid, dyAvoid);

        if (dyAvoid <= 0.0f)
            continue;

        alm[clmAvoid].gMin = psuvOther->dyTrack - dyAvoid;
        alm[clmAvoid].gMax = psuvOther->dyTrack + dyAvoid;
        clmAvoid++;
    }

    *pcpsuvInFront = csuvInFront;

    alm[clmAvoid].gMin = -1000000.0f;
    alm[clmAvoid].gMax = -psuv->dyMax;
    clmAvoid++;

    alm[clmAvoid].gMin = psuv->dyMax;
    alm[clmAvoid].gMax = 1000000.0f;
    clmAvoid++;

    CRV* pcrvTrack = psuv->pshapeTrack->pcrv.get();
    float sHoming = psuv->sTrack + DS_SuvHoming;

    glm::vec3 posHoming;
    glm::vec3 normalHoming;

    float uHoming = pcrvTrack->pvtcrv->pfnUFromCrvS(pcrvTrack, sHoming);

    if (pcrvTrack->pvtcrv->pfnEvaluateCrvFromU != nullptr)
    {
        pcrvTrack->pvtcrv->pfnEvaluateCrvFromU(pcrvTrack, uHoming, &posHoming, &normalHoming);
    }

    glm::vec3 normalLine = glm::cross(g_normalZ, normalHoming);

    if (glm::length(normalLine) > 0.0001f)
        normalLine = glm::normalize(normalLine);

    glm::vec3 pos0 = posHoming + normalLine * LM_SuvTrack.gMin;
    glm::vec3 pos1 = posHoming + normalLine * LM_SuvTrack.gMax;

    glm::vec3 posLine;

    FindCrvClosestPointOnLineSegmentFromU(psuv->pshapeLine->pcrv.get(), &pos0, &pos1, psuv->uLine, &posLine, nullptr, &psuv->uLine, nullptr);

    float dyLine = glm::dot(posLine - posHoming, normalLine);

    float dyTarget = GSmooth(psuv->dyTarget, dyLine, g_clock.dt, &s_smpLine, nullptr);
    float dyFree = GExcludeAlm(clmAvoid, alm, dyTarget);

    if (FCheckLm(&LM_SuvTrack, dyFree)) {
        psuv->dyTarget = dyFree;
        return;
    }

    float dy = dyTarget;

    for (int i = 0; i < clmAvoid; i++) 
    {
        if (dyFree > psuv->dyTrack && alm[i].gMin > psuv->dyTrack && dyFree > alm[i].gMin)
            dy = alm[i].gMin;
        else if (dyFree < psuv->dyTrack && alm[i].gMax < psuv->dyTrack && dyFree < alm[i].gMin)
            dy = alm[i].gMax;
        else
            dy = dyFree;

        dyFree = dy;
    }

    psuv->dyTarget = dy;
}

void UpdateSuvHeading(SUV* psuv)
{
    if (psuv->pshapeTrack == nullptr)
        return;

    CRV* pcrvTrack = psuv->pshapeTrack->pcrv.get();

    glm::vec3 posHoming{};
    glm::vec3 normalHoming{};
    glm::vec3 normalNext{};

    float sHoming = psuv->sTrack + DS_SuvHoming;

    float uHoming = pcrvTrack->pvtcrv->pfnUFromCrvS(pcrvTrack, sHoming);

    if (pcrvTrack->pvtcrv->pfnEvaluateCrvFromU != nullptr) {
        pcrvTrack->pvtcrv->pfnEvaluateCrvFromU(pcrvTrack, uHoming, &posHoming, &normalHoming);
    }

    glm::vec3 normal = glm::cross(g_normalZ, normalHoming);

    if (glm::length(normal) > 0.0001f)
        normal = glm::normalize(normal);

    posHoming += normal * psuv->dyTarget;

    glm::vec3 dpos = posHoming - psuv->xf.pos;
    psuv->radTarget = atan2f(dpos.y, dpos.x);

    float sNext = sHoming + DS_SuvNext;

    float uNext = pcrvTrack->pvtcrv->pfnUFromCrvS(pcrvTrack, sNext);

    if (pcrvTrack->pvtcrv->pfnEvaluateCrvFromU != nullptr) {
        pcrvTrack->pvtcrv->pfnEvaluateCrvFromU(pcrvTrack, uNext, nullptr, &normalNext);
    }

    float radTurn = RadBetweenVectors(&normalHoming, &normalNext, nullptr);

    float svBalance = psuv->rsvBalance * psuv->svMax;
    psuv->svTarget = svBalance;

    if (psuv->psveFeatureCur != nullptr) {
        psuv->svTarget = std::min(psuv->psveFeatureCur->svMax, svBalance);
    }

    if (radTurn > 0.0f) {
        float svTurn = sqrtf(R_SuvTurnDrive * (DS_SuvNext / radTurn) * psuv->muSxp * -psuv->dvGravity.z);

        if (svTurn < psuv->svTarget)
            psuv->svTarget = svTurn;
    }
}

void UpdateSuvWheels(SUV* psuv)
{
    float radCurrent = atan2f(psuv->xf.mat[0][1], psuv->xf.mat[0][0]);
    float radWheelTarget = RadNormalize(psuv->radTarget - radCurrent);

    radWheelTarget = GLimitAbs(radWheelTarget, RAD_SuvWheelMax);
    psuv->radFront = RadSmooth(psuv->radFront, radWheelTarget, g_clock.dt, &s_smpWheelTurn, nullptr);

    for (int i = 0; i < 4; ++i) {
        float wheelRadius = (i < 2) ? psuv->sRadiusFrontWheel : psuv->sRadiusRearWheel;
        psuv->asxp[i].swSpin = psuv->svTarget / wheelRadius;
    }
}

void UpdateSuvExpls(SUV* psuv)
{
    if (psuv->pemitterBoost != nullptr) {
        float dtBoost = (g_clock.t - psuv->tBoost) / DT_SuvBoost;
        float rDensity = CLQ_SuvDtToRDensity.g0 + dtBoost * (CLQ_SuvDtToRDensity.g1 + dtBoost * CLQ_SuvDtToRDensity.g2);

        psuv->pemitterBoost->rDensity = GLimitLm(&g_lmZeroOne, rDensity);

        if (psuv->pemitterBoost->rDensity == 0.0f)
            psuv->pemitterBoost->pvtlo->pfnRemoveLo(psuv->pemitterBoost);
    }

    for (int i = 0; i < 4; ++i) {
        SXP& sxp = psuv->asxp[i];

        if (sxp.cParticleDirt >= 1.0f) {
            int cParticleDirt = static_cast<int>(sxp.cParticleDirt);

            if (psuv->svTarget > 500.0f) {
                EMITOLXF emitolxf{};
                glm::mat3 matDirt{ 1.0f };
                glm::mat3 matSpray{ 1.0f };

                float radStick = GRandInRange(LM_SuvRadStick.gMin, LM_SuvRadStick.gMax);
                LoadRotateMatrixRad(radStick, &g_normalY, &matDirt);

                float radSpray = GRandInRange(LM_SuvRadSpray.gMin, LM_SuvRadSpray.gMax);
                bool fFrontWheel = i < 2;

                if (i == 0)
                    radSpray += psuv->radFront;
                else if (i == 1)
                    radSpray = psuv->radFront - radSpray;
                else if (i == 3)
                    radSpray = -radSpray;

                LoadRotateMatrixRad(radSpray, &g_normalZ, &matSpray);
                matDirt = matSpray * matDirt;

                ConvertAloPos(nullptr, psuv, &sxp.posGround, &emitolxf.posLocal);

                float wheelRadius = fFrontWheel ? psuv->sRadiusFrontWheel : psuv->sRadiusRearWheel;
                glm::vec3 radiusOffset(0.0f, 0.0f, wheelRadius);

                emitolxf.posLocal += radiusOffset - matDirt * radiusOffset;

                glm::vec3 normalGround{};
                ConvertAloVec(nullptr, psuv, &sxp.normalGround, &normalGround);
                LoadLookAtMatrix(&normalGround, &emitolxf.matLocal);

                emitolxf.vLocal = matDirt * glm::vec3(0.0f, -psuv->svTarget, 0.0f);

                EXPL* pexplDirt = psuv->psveFeatureCur != nullptr ? psuv->psveFeatureCur->pexplDirt : psuv->pexplDirt;

                if (pexplDirt != nullptr) 
                {
                    EXPLSO explso{};

                    explso.grfexplso = 192;
                    explso.pemitolxf = &emitolxf;
                    explso.cParticle = cParticleDirt;

                    pexplDirt->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexplDirt, &explso);
                }
            }

            sxp.cParticleDirt -= static_cast<float>(cParticleDirt);
        }

        if (sxp.cParticleDust >= 1.0f) {
            EMITOLXF emitolxf{};
            int cParticleDust = static_cast<int>(sxp.cParticleDust);

            ConvertAloPos(nullptr, psuv, &sxp.posGround, &emitolxf.posLocal);

            glm::vec3 normalGround{};
            ConvertAloVec(nullptr, psuv, &sxp.normalGround, &normalGround);
            LoadLookAtMatrix(&normalGround, &emitolxf.matLocal);

            EXPL* pexplDust = psuv->psveFeatureCur != nullptr ? psuv->psveFeatureCur->pexplDust : psuv->pexplDust;

            if (pexplDust != nullptr) 
            {
                EXPLSO explso{};

                explso.grfexplso = 192;
                explso.pemitolxf = &emitolxf;
                explso.cParticle = cParticleDust;

                pexplDust->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexplDust, &explso);
            }

            sxp.cParticleDust -= static_cast<float>(cParticleDust);
        }
    }
}

void BoostSuv(SUV* psuv)
{
    EMITTER* pemitterBoost = psuv->pemitterBoost;

    psuv->tBoost = g_clock.t;
    --psuv->cBoost;

    if (pemitterBoost != nullptr)
        pemitterBoost->pvtlo->pfnAddLo(pemitterBoost);
}

void HandleSuvRaceLoss(SUV* psuv)
{
    float raceProgress = static_cast<float>((psuv->cLap - 1) * 3 + psuv->psveCheckCur->isveCheck) * 0.1f;
    raceProgress = std::clamp(raceProgress, 0.0f, 1.0f);

    int placeIndex = static_cast<int>(psuv->nPlaceMax) - static_cast<int>(psuv->nPlace);
    bool fUpdateDifficulty = placeIndex <= 2;

    if (placeIndex > 2) {
        int byteIndex = placeIndex >> 3;
        unsigned char bitMask = static_cast<unsigned char>(1u << (placeIndex & 7));

        fUpdateDifficulty = (g_plsCur->abitChkpnt.m_ab[byteIndex] & bitMask) != 0;
    }

    if (fUpdateDifficulty)
        OnDifficultyPlayerDeath(&g_difficulty, raceProgress);

    LS* pls = g_plsCur;

    for (int i = placeIndex; i >= 0; --i) 
    {
        int byteIndex = i >> 3;
        unsigned char bitMask = static_cast<unsigned char>(1u << (i & 7));
        unsigned char& bits = pls->abitChkpnt.m_ab[byteIndex];

        if ((bits & bitMask) == 0) 
        {
            bits |= bitMask;
            ++pls->abitChkpnt.m_cbitSet;
        }
    }
}

void UpdateSuvVolumes(SUV* psuv)
{
    for (int i = 0; i < psuv->csve; ++i) 
    {
        SVE* psve = &psuv->asve[i];
        int fInside = FCheckVolPoint(psve->pvol, &psuv->xf.pos);

        if (fInside == psve->f)
            continue;

        psve->f = fInside;

        if (psve->svek == SVEK_CheckPoint) 
        {
            if (!fInside)
                continue;

            if (psve->psveCheckPrev != psuv->psveCheckCur)
                continue;

            psuv->psveCheckCur = psve;

            if (psve != psuv->psveCheckFirst)
                continue;

            ++psuv->cLap;

            if (psuv->cLap <= psuv->cLapMax)
                continue;

            SM* psmRace = (SM*)PloFindSwObject(psuv->psw, static_cast<GRFFSO>(4), (OID)1068, psuv);
            SMA* psmaRace = psmRace != nullptr ? psmRace->dlSma.psmaFirst : nullptr;

            if (psmaRace == nullptr)
                continue;

            OID oidState = OID_Nil;
            GetSmaGoal(psmaRace, &oidState);

            if (oidState == OID_Nil)
                GetSmaCur(psmaRace, &oidState);

            SUV* psuvPlayer = (SUV*)PpoCur();

            if (psuv == psuvPlayer)
                psuv->cLap = psuv->cLapMax;

            if (oidState == (OID)1071) 
            {
                if (psuv == psuvPlayer) 
                {
                    SetSmaGoal(psmaRace, (OID)1074);
                    OnDifficultyCollectKey(&g_difficulty);
                }
                else 
                    SetSmaGoal(psmaRace, (OID)1072);
            }
            else if (oidState == (OID)1072 && psuv == psuvPlayer) {
                SetSmaGoal(psmaRace, (OID)1073);
                HandleSuvRaceLoss(psuv);
            }
        }
        else if (psve->svek == SVEK_Feature) {
            psuv->psveFeatureCur = fInside ? psve : nullptr;
        }
    }
}

void UpdateSuvSounds(SUV* psuv, float dt)
{
    if (psuv->pambIdle == nullptr)
        return;

    auto EvalQuadratic = [](float x, float g0, float g1, float g2) {
        return g0 + x * (g1 + x * g2);
        };

    float targetEngineSpeed = std::clamp(psuv->svTarget / S_SuvSoundMax, 0.0f, 1.0f);
    psuv->uEngineSpeed = GSmooth(psuv->uEngineSpeed, targetEngineSpeed, dt, &s_smpSuvEngineSpeed, nullptr);

    float measuredEngineSpeed = glm::dot(psuv->xf.v, psuv->xf.matWorld[1]) / S_SuvSoundMax;
    measuredEngineSpeed = std::clamp(measuredEngineSpeed, 0.0f, 1.0f);

    float suspensionBlend = EvalQuadratic(psuv->uSuspensionLoad, CLQ_SuvSuspensionToEngineBlend.g0, CLQ_SuvSuspensionToEngineBlend.g1, CLQ_SuvSuspensionToEngineBlend.g2);
    suspensionBlend = std::clamp(suspensionBlend, LM_SuvEngineBlend.gMin, LM_SuvEngineBlend.gMax);

    float engineInput = (1.0f - suspensionBlend) * measuredEngineSpeed + suspensionBlend * R_SuvTargetEngineSpeed * psuv->uEngineSpeed;

    float idleFrequency = EvalQuadratic(engineInput, CLQ_SuvIdleFrq.g0, CLQ_SuvIdleFrq.g1, CLQ_SuvIdleFrq.g2) + psuv->dfrqIdle;
    SetPambFrq(psuv->pambIdle, idleFrequency);

    float idleVolume = EvalQuadratic(engineInput, CLQ_SuvIdleVol.g0, CLQ_SuvIdleVol.g1, CLQ_SuvIdleVol.g2);
    idleVolume = std::clamp(idleVolume, LM_SuvIdleVol.gMin, LM_SuvIdleVol.gMax);
    SetPambVol(psuv->pambIdle, idleVolume);

    float runningFrequency = EvalQuadratic(engineInput, CLQ_SuvRunningFrq.g0, CLQ_SuvRunningFrq.g1, CLQ_SuvRunningFrq.g2) + psuv->dfrqExtra;
    SetPambFrq(psuv->pambRunning, runningFrequency);

    float runningVolume = EvalQuadratic(engineInput, CLQ_SuvRunningVol.g0, CLQ_SuvRunningVol.g1, CLQ_SuvRunningVol.g2);
    runningVolume = std::clamp(runningVolume, LM_SuvRunningVol.gMin, LM_SuvRunningVol.gMax);
    SetPambVol(psuv->pambRunning, runningVolume);

    glm::vec3 worldVelocity{};
    ConvertAloVec(nullptr, psuv, &psuv->xf.v, &worldVelocity);

    // Retail removes the commanded forward component along local X before
    // measuring lateral tire slip.
    glm::vec3 relativeVelocity = worldVelocity - g_normalX * psuv->svTarget;
    relativeVelocity.x *= R_SuvTireSlipX;

    float unknownSpeed = std::clamp(glm::length(relativeVelocity) / S_SuvTireSlipMax, 0.0f, 1.0f);

    float unknownSuspensionScale = EvalQuadratic(psuv->uSuspensionLoad, CLQ_SuvSuspensionToTireSlip.g0, CLQ_SuvSuspensionToTireSlip.g1, CLQ_SuvSuspensionToTireSlip.g2);
    unknownSuspensionScale = std::clamp(unknownSuspensionScale, LM_SuvTireSlipScale.gMin, LM_SuvTireSlipScale.gMax);

    float unknownInput = unknownSpeed * unknownSuspensionScale;

    float unknownFrequency = EvalQuadratic(unknownInput, CLQ_SuvTireSlipFrq.g0, CLQ_SuvTireSlipFrq.g1, CLQ_SuvTireSlipFrq.g2) + psuv->dfrqTireSlip;
    SetPambFrq(psuv->pambTireSlip, unknownFrequency);

    float unknownVolume = EvalQuadratic(unknownInput, CLQ_SuvTireSlipVol.g0, CLQ_SuvTireSlipVol.g1, CLQ_SuvTireSlipVol.g2);
    // The reconstructed curve carries a 0.5 bias. Feeding that directly to
    // XAudio leaves tire noise audible at rest; retain only the slip-dependent
    // portion so this layer is silent when unknownInput is zero.
    unknownVolume -= CLQ_SuvTireSlipVol.g0;
    unknownVolume = std::clamp(unknownVolume, LM_SuvTireSlipVol.gMin, LM_SuvTireSlipVol.gMax);
    SetPambVol(psuv->pambTireSlip, unknownVolume);
}

void UpdateSuvPuncher(SUV* psuv)
{
    XP* pxp = psuv->pxpPuncher;

    if (pxp == nullptr)
        return;

    while (pxp != nullptr) 
    {
        SUV* psuvFirst = reinterpret_cast<SUV*>(pxp->axpd[0].psoRoot);
        STEPGUARD* pstepguard = reinterpret_cast<STEPGUARD*>(pxp->axpd[psuvFirst == psuv].psoRoot);

        bool fIgnored = false;

        for (int i = 0; i < psuv->cpsoIgnore; ++i) {
            if (psuv->apsoIgnore[i] == pstepguard) {
                fIgnored = true;
                break;
            }
        }

        if (!fIgnored) {
            glm::vec3 velocity{};
            glm::vec3 angularVelocity{};

            ConvertAloVec(psuv, nullptr, &s_vPuncher, &velocity);

            velocity += psuv->xf.v * RV_SuvPuncherSelf;
            angularVelocity = glm::cross(g_normalZ, velocity) * R_SuvPuncherSvToSw;

            pstepguard->pvtalo->pfnSetAloVelocityVec(pstepguard, &velocity);
            pstepguard->pvtalo->pfnSetAloAngularVelocityVec(pstepguard, &angularVelocity);

            if (psuv->cpsoIgnore < 8)
                psuv->apsoIgnore[psuv->cpsoIgnore++] = pstepguard;

            if (FIsBasicDerivedFrom(pstepguard, CID_SUV)) {
                pstepguard->uAsegaPrev = g_clock.t;
            }
            else if (FIsBasicDerivedFrom(pstepguard, CID_STEPGUARD)) {
                SetStepguardSgs(pstepguard, SGS_Stun, nullptr);
            }
        }

        pxp = pxp->pxpNext;
    }

    FreeSwXpList(psuv->psw, psuv->pxpPuncher);
    psuv->pxpPuncher = nullptr;
}

void UpdateSuv(SUV* psuv, float dt)
{
    int csuvInFront = 0;

    UpdatePo(psuv, dt);

    switch (psuv->suvs) 
    {
        case SUVS_Auto:
        UpdateSuvBalance(psuv);
        UpdateSuvLine(psuv, &csuvInFront);
        UpdateSuvHeading(psuv);
        break;

        case SUVS_Stop:
        psuv->svTarget = 0.0f;
        break;

        default:
        break;
    }

    UpdateSuvSounds(psuv, dt);
    UpdateSuvWheels(psuv);
    UpdateSuvExpls(psuv);
    UpdateSuvVolumes(psuv);
    UpdateSuvPuncher(psuv);

    if (psuv->xf.mat[2][2] > 0.5f)
        psuv->tUpright = g_clock.t;

    if (psuv->suvs != SUVS_Stop)
        ResolveAlo(psuv);

}

void UpdateSuvActive(SUV* psuv, JOY* pjoy)
{
    if (psuv->suvs != SUVS_Auto)
    {
        if (pjoy->uDeflect > 0.0f)
            AdjustCmJoy(g_pcm, pjoy, JOYID_Left, &psuv->radTarget);
        else
            psuv->radTarget = atan2f(psuv->xf.mat[0][1], psuv->xf.mat[0][0]);
    }

    if (psuv->suvs == SUVS_Manual)
    {
        psuv->svTarget = psuv->svMax * pjoy->uDeflect;

        if (g_fVehicleSpeedLimited != 0.0f)
            psuv->svTarget *= 0.4f;

        if (pjoy->IsPressed(BTN_SQUARE))
        {
            if (psuv->suvgk == SUVGK_Chase)
            {
                if (psuv->psmPuncher != nullptr)
                {
                    SetSmaGoal(psuv->psmaPuncher, (OID)1157);
                    psuv->cpsoIgnore = 0;
                }
            }
            else if (psuv->suvgk == SUVGK_Race)
            {
                if (psuv->cBoost <= 0 || g_clock.t - psuv->tBoost <= DT_SuvBoostRecharge)
                    StartSound((SFXID)115, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
                else
                {
                    BoostSuv(psuv);
                    StartSound((SFXID)90, nullptr, nullptr, nullptr, 90.0f, 0.0f, 2.0f, -0.25f, 0.0f, nullptr, nullptr);
                }
            }
        }
    }

    HandleDialogButtons(pjoy);

    if (psuv->psveFeatureCur != nullptr)
        psuv->svTarget = std::min(psuv->svTarget, psuv->psveFeatureCur->svMax);

    if (psuv->suvgk == SUVGK_Chase)
    {
        SM* psmChase = (SM*)PloFindSwObject(psuv->psw, static_cast<GRFFSO>(5), (OID)1068, nullptr);

        if (psmChase != nullptr)
        {
            SMA* psmaChase = (SMA*)psmChase->dlSma.pvFirst;
            OID oidState;

            GetSmaGoal(psmaChase, &oidState);

            if (oidState == OID_Nil)
                GetSmaCur(psmaChase, &oidState);

            if (oidState == 1071)
            {
                if (g_cCpuCollected >= N_SuvChaseWin)
                {
                    SetSmaGoal(psmaChase, (OID)1074);
                }
                else if (g_scores.cCpuCollected >= N_SuvChaseWin)
                {
                    SetSmaGoal(psmaChase, (OID)1073);
                    OnDifficultyPlayerDeath(&g_difficulty, 1.0f);
                }
            }
        }
    }
    else if (psuv->suvgk == SUVGK_Race)
    {
        SM* psmRace = (SM*)PloFindSwObject(psuv->psw, static_cast<GRFFSO>(5), (OID)1068, nullptr);

        if (psmRace != nullptr)
        {
            SMA* psmaRace = (SMA*)psmRace->dlSma.pvFirst;
            OID oidState;

            GetSmaGoal(psmaRace, &oidState);

            if (oidState == OID_Nil)
                GetSmaCur(psmaRace, &oidState);

            const bool fRaceRunning = oidState == 1071 || oidState == 1072;
            const bool fCanUpdatePlace = g_clock.t - psuv->tPlace > 0.5f;

            if (fRaceRunning && fCanUpdatePlace && psuv->pshapeTrack != nullptr)
            {
                SUV* apsuv[16]{};
                const int csuv = CploFindSwObjectsByClass(psuv->psw, static_cast<GRFFSO>(5), CID_SUV, nullptr, 16, (LO**)apsuv);

                int nPlace = 1;

                for (int i = 0; i < csuv; ++i)
                {
                    SUV* psuvOther = apsuv[i];

                    if (psuvOther != psuv && FIsSuvAheadOf(psuvOther, psuv))
                        ++nPlace;
                }

                if (nPlace != psuv->nPlace || csuv != psuv->nPlaceMax)
                {
                    psuv->nPlace = nPlace;
                    psuv->nPlaceMax = csuv;
                    psuv->tPlace = g_clock.t;
                }
            }
        }
    }
}

void UpdateSuvBlots(SUV* psuv)
{
    bool fShowRaceBlots = false;
    bool fShowChaseBlots = false;

    if (psuv->suvgk == SUVGK_Race)
    {
        SM* psmRace = static_cast<SM*>(PloFindSwObject(psuv->psw, static_cast<GRFFSO>(5), static_cast<OID>(1068), nullptr));

        if (psmRace != nullptr)
        {
            SMA* psmaRace = psmRace->dlSma.psmaFirst;

            if (psmaRace != nullptr)
            {
                OID oidState;
                GetSmaGoal(psmaRace, &oidState);

                if (oidState == OID_Nil)
                    GetSmaCur(psmaRace, &oidState);

                if (oidState == static_cast<OID>(1071) || oidState == static_cast<OID>(1072))
                    fShowRaceBlots = FCanShowGameplayBlot();
            }
        }
    }
    else if (psuv->suvgk == SUVGK_Chase)
        fShowChaseBlots = FCanShowGameplayBlot();

    if (fShowRaceBlots)
    {
        g_boostctr.pvtblot->pfnShowBlot(&g_boostctr);
        g_lapctr.pvtblot->pfnShowBlot(&g_lapctr);
        g_placectr.pvtblot->pfnShowBlot(&g_placectr);
    }
    else
    {
        g_boostctr.pvtblot->pfnHideBlot(&g_boostctr);
        g_lapctr.pvtblot->pfnHideBlot(&g_lapctr);
        g_placectr.pvtblot->pfnHideBlot(&g_placectr);
    }

    if (fShowChaseBlots)
    {
        g_van.pvtblot->pfnShowBlot(&g_van);
        g_scores.pvtblot->pfnShowBlot(&g_scores);
    }
    else
    {
        g_van.pvtblot->pfnHideBlot(&g_van);
        g_scores.pvtblot->pfnHideBlot(&g_scores);
    }
}

void UpdateSuvInternalXps(SUV* psuv)
{
    psuv->xsxp = psuv->xsxp + 1;
}

void AddSuvCustomXps(SUV* psuv, SO* psoOther, int cbspPruned, BSP* abspPruned, BSP* pbspPruned)
{
    const GRFCID grfcid = psoOther->pvtsuv->grfcid;

    if (!psoOther->fLockedAbove && (grfcid & 0x10) == 0)
    {
        SO* psoPuncher = psuv->psoPuncher;

        if (psoPuncher == nullptr || !FIsLoInWorld(psoPuncher))
            return;

        if (psoOther->fSphere)
            MarkSoContactsSphereBsp(psoOther, &psoOther->xf.posWorld, psoOther->sRadiusSelf, psoPuncher, psoPuncher->bspc.cbsp, psoPuncher->bspc.absp.data(), nullptr, &psuv->pxpPuncher);
        else if (abspPruned != nullptr)
            MarkSoContactsBspBsp(psoPuncher, psoOther, cbspPruned, abspPruned, pbspPruned, &psuv->pxpPuncher);

        return;
    }

    for (int i = 0; i < 4; ++i)
    {
        SXP* psxp = &psuv->asxp[i];

        const bool rearWheel = i >= 2;
        const float wheelRadius = rearWheel ? psuv->sRadiusRearWheel : psuv->sRadiusFrontWheel;

        glm::vec3 topLocal = psxp->ppntTop->posLocal;
        topLocal.z -= wheelRadius;

        glm::vec3 topWorld;
        ConvertAloPos((ALO*)psuv, nullptr, &topLocal, &topWorld);

        glm::vec3 bottomLocal = psxp->paloWheel->posOrig;
        bottomLocal.z -= wheelRadius;

        glm::vec3 bottomWorld;
        ConvertAloPos((ALO*)psuv, nullptr, &bottomLocal, &bottomWorld);

        const glm::vec3 wheelDirection = topWorld - bottomWorld;
        const float directionLength = glm::length(wheelDirection);

        if (directionLength > 0.0001f)
            topWorld += (wheelDirection / directionLength) * wheelRadius;

        LSG lsg{};

        const int contactCount = ClsgClipEdgeToObjectPruned(psoOther, pbspPruned, &topWorld, &bottomWorld, 1, &lsg);

        if (contactCount <= 0 || lsg.au[0] <= 0.0f)
            continue;

        const int contactGeneration = psuv->xsxp + 1;
        bool useContact = psxp->xsxp != contactGeneration;

        if (!useContact)
        {
            const glm::vec3 suvUp = glm::vec3(psuv->xf.mat[2]);
            const float previousHeight = glm::dot(glm::vec3(psxp->posGround), suvUp);
            const float candidateHeight = glm::dot(glm::vec3(lsg.apos[0]), suvUp);

            useContact = previousHeight < candidateHeight;
        }

        if (useContact)
        {
            psxp->normalGround = glm::vec3(lsg.anormal[0]);
            psxp->posGround = glm::vec3(lsg.apos[0]);
            psxp->xsxp = contactGeneration;

            CalculateAloMovement(psoOther, nullptr, psxp->posGround, &psxp->vGround, nullptr, nullptr, nullptr);
        }

        if (psoOther->oid == (OID)1)
            continue;

        if (FIsBasicDerivedFrom((BASIC*)psoOther, CID_BOMB))
        {
            BOMB* pbomb = static_cast<BOMB*>(psoOther);

            if (pbomb->cpbrkp == 0)
                PrimeBomb(pbomb, 0);

            continue;
        }

        if (!FIsBasicDerivedFrom((BASIC*)psoOther, CID_FRAGILE))
            continue;

        FRAGILE* pfragile = static_cast<FRAGILE*>(psoOther);

        if (pfragile->fBreakOnUpdate)
            continue;

        if (!FCheckBrkTouchObject(static_cast<BRK*>(pfragile), static_cast<SO*>(psuv)))
            continue;

        if (pfragile->cnstrImpactForce != CNSTR_Nil)
            SetSoCnstrForce(psoOther, pfragile->cnstrImpactForce);

        if (pfragile->cnstrImpactTorque != CNSTR_Nil)
            SetSoCnstrTorque(psoOther, pfragile->cnstrImpactTorque);

        pfragile->fBreakOnUpdate = true;
        pfragile->psoImpacting = psuv;
        pfragile->m *= pfragile->rImpactMass;
    }
}
void AdjustSuvNewXp(SUV* psuv, XP* pxp, int ixpd)
{
    SO* psoOther = pxp->axpd[1 - ixpd].psoRoot;

    const float normalZ = pxp->normal.z * g_mpixpdr[ixpd];
    const float speed = glm::length(psuv->xf.v);

    if (psoOther == nullptr || !psoOther->fSphere) {
        pxp->mu = 0.15f;

        pxp->elas = 0.5f;
        return;
    }

    if (normalZ > 0.7f && speed > SV_SuvMinFriction) 
    {
        pxp->mu = 1.0f;
        pxp->elas = 0.15f;
        return;
    }

    pxp->mu = 0.0f;
    pxp->elas = 0.0f;
}

void UpdateSuvInfluences(SUV* psuv, RO* pro)
{
    UpdateAloInfluences(psuv, pro);

    for (int i = 0; i < 4; ++i) {
        SXP& sxp = psuv->asxp[i];

        sxp.radSpin += sxp.swSpin * g_clock.dt;

        if (sxp.xsxp == psuv->xsxp) {
            ConvertAloPos(nullptr, psuv, &sxp.posGround, &sxp.posCur);

            float wheelRadius = i < 2 ? psuv->sRadiusFrontWheel : psuv->sRadiusRearWheel;
            sxp.posCur.z += wheelRadius;
        }
        else {
            sxp.posCur = PosSmooth(sxp.posCur, sxp.paloWheel->posOrig, g_clock.dt, &s_smpWheelFree, nullptr);
        }

        glm::mat3 matTurn{ 1.0f };
        glm::mat3 matSpin{ 1.0f };

        if (i < 2)
            LoadRotateMatrixRad(psuv->radFront, &g_normalZ, &matTurn);

        LoadRotateMatrixRad(sxp.radSpin, &g_normalY, &matSpin);

        glm::mat3 matWheel = matTurn * matSpin;

        if (sxp.paloHub != nullptr) {
            glm::vec3 posHub{};

            ConvertAloPos(sxp.paloWheel->paloParent, sxp.paloHub->paloParent, &sxp.posCur, &posHub);
            sxp.paloHub->pvtalo->pfnTranslateAloToPos(sxp.paloHub, &posHub);
        }

        sxp.paloWheel->pvtalo->pfnTranslateAloToPos(sxp.paloWheel, &sxp.posCur);
        sxp.paloWheel->pvtalo->pfnRotateAloToMat(sxp.paloWheel, &matWheel);
    }
}

void RenderSuvSelf(SUV* psuv, CM* pcm, RO* pro)
{
    RenderSoSelf(psuv, pcm, pro);

    if (psuv->paloShadow != nullptr) 
    {
        const glm::mat3& matWorld = psuv->xf.matWorld;

        glm::vec3 scale(
            glm::length(matWorld[0]),
            glm::length(matWorld[1]),
            glm::length(matWorld[2])
        );

        RO roShadow{};
        glm::mat3 matShadow{ 1.0f };

        DupAloRo(psuv, pro, &roShadow);
        TiltMatUpright(&matWorld, nullptr, &matShadow);

        matShadow[0] *= std::min(scale.x, scale.z);
        matShadow[1] *= std::min(scale.y, scale.z);

        glm::vec3 posShadow = psuv->xf.posWorld + matShadow * psuv->paloShadow->xf.pos;

        LoadMatrixFromPosRot(&posShadow, &matShadow, reinterpret_cast<glm::mat4*>(&roShadow));
        psuv->paloShadow->pvtalo->pfnRenderAloGlobset(psuv->paloShadow, pcm, &roShadow);
    }

    if (psuv->paloFrontAxle != nullptr || psuv->paloRearAxle != nullptr) 
    {
        glm::vec3 wheelPositions[4]{};

        for (int i = 0; i < 4; ++i)
            ConvertAloPos(psuv, nullptr, &psuv->asxp[i].posCur, &wheelPositions[i]);

        float alpha = pro != nullptr ? pro->uAlpha : 1.0f;

        if (psuv->paloFrontAxle != nullptr)
            RenderAloLine(psuv->paloFrontAxle, pcm, &wheelPositions[0], &wheelPositions[1], 1.0f, alpha);

        if (psuv->paloRearAxle != nullptr)
            RenderAloLine(psuv->paloRearAxle, pcm, &wheelPositions[2], &wheelPositions[3], 1.0f, alpha);
    }
}

void UpdateSuvBounds(SUV* psuv)
{
    UpdateSoBounds(psuv);

    for (int i = 0; i < 4; ++i) {
        SXP& sxp = psuv->asxp[i];

        if (sxp.paloWheel == nullptr)
            continue;

        glm::vec3 posWheel = sxp.paloWheel->posOrig;

        ConvertAloPos(psuv, nullptr, &posWheel, &posWheel);

        float wheelRadius = i < 2 ? psuv->sRadiusFrontWheel : psuv->sRadiusRearWheel;

        ExtendSoBounds(psuv, &posWheel, wheelRadius);
    }
}

void CollectSuvPrize(SUV* psuv, PCK pck, ALO* paloOther)
{
    if (psuv->pmurray == nullptr) {
        CollectPoPrize(psuv, pck, paloOther);
        return;
    }

    SetStepguardPatrolAnimation(psuv->pmurray, nullptr);
    psuv->pmurray->pvtmurray->pfnCollectMurrayPrize(psuv->pmurray, pck, paloOther);
}

void UpdateSuvShapes(SUV* psuv)
{
    if (psuv->pshapeTrack == nullptr)
        return;

    CRV* pcrvTrack = psuv->pshapeTrack->pcrv.get();

    glm::vec3 posClosest{};
    glm::vec3 normalClosest{};

    if (pcrvTrack->pvtcrv->pfnFindCrvClosestPointFromU != nullptr)
        pcrvTrack->pvtcrv->pfnFindCrvClosestPointFromU(pcrvTrack, &psuv->xf.pos, psuv->uTrack, nullptr, &posClosest, &normalClosest, &psuv->uTrack, nullptr);

    glm::vec3 lateral = glm::cross(g_normalZ, normalClosest);
    float lateralLengthSq = glm::dot(lateral, lateral);

    if (lateralLengthSq > 1.0e-8f)
        lateral *= glm::inversesqrt(lateralLengthSq);
    else
        lateral = glm::vec3(0.0f);

    psuv->sTrack = pcrvTrack->pvtcrv->pfnSFromCrvU(pcrvTrack, psuv->uTrack);
    psuv->dyTrack = glm::dot(psuv->xf.pos - posClosest, lateral);
}

void UpdateSuvXfWorld(SUV* psuv)
{
	UpdateSoXfWorld(psuv);
    UpdateSuvShapes(psuv);
}

void GetSuvCpdefi(SUV* psuv, float dt, CPDEFI* pcpdefi)
{
    GetSoCpdefi(psuv, dt, (CPDEFI*)&pcpdefi->grfcpd);
}

void OnSuvActive(SUV* psuv, int fActive)
{
    OnPoActive(psuv, fActive, nullptr);

    if (fActive == 0) 
    {
        if (psuv->suvs == SUVS_Manual) 
            SetSuvSuvs(psuv, SUVS_Stop);
    }
    else 
    {
        g_lapctr.pnTotal = &psuv->cLapMax;
        g_placectr.pnTotal = &psuv->nPlaceMax;
        g_lapctr.pnActual = &psuv->cLap;
        g_placectr.pnActual = &psuv->nPlace;
        g_boostctr.pnActual = &psuv->cBoost;
        SetSwGameplayHud(psuv->psw, psuv);
    }
}

void HandleSuvMessage(SUV* psuv, MSGID msgid, void* pv)
{
    HandlePoMessage(psuv, msgid, pv);

    if (msgid != MSGID_break_break)
        return;

    FRAGILE* pfragile = static_cast<FRAGILE*>(pv);

    if (pfragile->oid != (OID)1062)
        return;

    if (g_cCpuCollected >= N_SuvChaseWin || g_scores.cCpuCollected >= N_SuvChaseWin)
        return;

    // This nested pointer ultimately identifies the SUV that broke the object.
    if (pfragile->psoImpacting == psuv)
        ++g_cCpuCollected;
    else
        ++g_scores.cCpuCollected;
}

void* GetSuvTrack(SUV* psuv)
{
    return &psuv->pshapeTrack;
}

void SetSuvTrack(SUV* psuv, SHAPE* pshapeTrack)
{
    psuv->pshapeTrack = pshapeTrack;

    CRV* pcrvTrack = pshapeTrack->pcrv.get();

    if (pcrvTrack->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
        pcrvTrack->pvtcrv->pfnFindCrvClosestPointAll(pcrvTrack, &psuv->xf.pos, nullptr, nullptr, nullptr, &psuv->uTrack, nullptr);

    UpdateSuvShapes(psuv);

    psuv->dyTarget = psuv->dyTrack;
    psuv->sTrackMax = pcrvTrack->pvtcrv->pfnSMaxCrv(pcrvTrack);

    for (int i = 0; i < psuv->csve; ++i) {
        SVE& sve = psuv->asve[i];

        if (sve.svek != SVEK_CheckPoint)
            continue;

        glm::vec3 pos{};
        GetXfmPos(sve.pvol, &pos);

        if (pcrvTrack->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
            pcrvTrack->pvtcrv->pfnFindCrvClosestPointAll(pcrvTrack, &pos, nullptr, nullptr, nullptr, nullptr, &sve.sTrack);
    }
}

void* GetSuvLine(SUV* psuv)
{
    return &psuv->pshapeLine;
}

void SetSuvLine(SUV* psuv, SHAPE* pshapeLine)
{
    psuv->pshapeLine = pshapeLine;

    CRV* pcrvLine = pshapeLine->pcrv.get();

    glm::vec3 posLine{};

    if (pcrvLine->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
        pcrvLine->pvtcrv->pfnFindCrvClosestPointAll(pcrvLine, &psuv->xf.pos, nullptr, &posLine, nullptr, &psuv->uLine, nullptr);
}

void AddSuvCheckPoint(SUV* psuv, VOL* pvol)
{
    if (pvol == nullptr || psuv->csve >= 16)
        return;

    SVE& sve = psuv->asve[psuv->csve++];

    sve.svek = SVEK_CheckPoint;
    sve.pvol = pvol;

    if (psuv->psveCheckFirst == nullptr) {
        sve.psveCheckPrev = &sve;
        psuv->psveCheckFirst = &sve;
        psuv->psveCheckCur = &sve;
    }
    else {
        sve.psveCheckPrev = psuv->psveCheckFirst->psveCheckPrev;
        psuv->psveCheckFirst->psveCheckPrev = &sve;
        sve.isveCheck = sve.psveCheckPrev->isveCheck + 1;
    }

    if (psuv->pshapeTrack == nullptr)
        return;

    glm::vec3 pos{};
    GetXfmPos(pvol, &pos);

    CRV* pcrvTrack = psuv->pshapeTrack->pcrv.get();

    if (pcrvTrack->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
        pcrvTrack->pvtcrv->pfnFindCrvClosestPointAll(pcrvTrack, &pos, nullptr, nullptr, nullptr, nullptr, &sve.sTrack);
}

void AddSuvFeature(SUV* psuv, VOL* pvol, float mu, float svMax, EXPL* pexplDirt, EXPL* pexplDust)
{
    if (pvol == nullptr || psuv->csve >= 16)
        return;

    SVE& sve = psuv->asve[psuv->csve++];

    sve.svek = SVEK_Feature;
    sve.pvol = pvol;
    sve.mu = mu;
    sve.svMax = svMax;
    sve.pexplDirt = pexplDirt;
    sve.pexplDust = pexplDust;
}

void* GetSuvSuvs(SUV* psuv)
{
    return &psuv->suvs;
}

void SetSuvSuvs(SUV* psuv, SUVS suvs)
{
    if (psuv->suvs == suvs)
        return;

    psuv->suvs = suvs;
    psuv->tSuvs = g_clock.t;

    if (suvs == SUVS_Manual && PpoCur() != psuv)
        SwitchToPo(psuv);
}

void ResetSuv(SUV* psuv)
{
    psuv->cLap = 1;
    psuv->psveCheckCur = psuv->psveCheckFirst;

    psuv->tPlace = -10.0f;
    psuv->cBoost = 0;
    psuv->tBoost = -10.0f;
    psuv->tBalance = -10.0f;
    psuv->tPunched = -10.0f;

    float difficulty = g_plsCur->uSuck;

    float blend = psuv->clqTune.g0 + difficulty * (psuv->clqTune.g1 + difficulty * psuv->clqTune.g2);
    blend = std::clamp(blend, g_lmZeroOne.gMin, g_lmZeroOne.gMax);

    float inverseBlend = 1.0f - blend;

    psuv->clqProgressToRsv.g0 = inverseBlend * psuv->svrb.clqDuToRsvAhead.g0 + blend * psuv->svrb.lmRsvRandom.gMin;
    psuv->clqProgressToRsv.g1 = inverseBlend * psuv->svrb.clqDuToRsvAhead.g1 + blend * psuv->svrb.lmRsvRandom.gMax;
    psuv->clqProgressToRsv.g2 = inverseBlend * psuv->svrb.clqDuToRsvAhead.g2 + blend * psuv->svrb.lmRsvBalance.gMin;

    psuv->lmRsvBalance.gMin = inverseBlend * psuv->svrb.clqDuToRsvBehind.g0 + blend * psuv->svrb.mpcpsuvuBoost[0];
    psuv->lmRsvBalance.gMax = inverseBlend * psuv->svrb.clqDuToRsvBehind.g1 + blend * psuv->svrb.mpcpsuvuBoost[1];

    float initialBalance = std::clamp(psuv->clqProgressToRsv.g0, psuv->lmRsvBalance.gMin, psuv->lmRsvBalance.gMax);

    psuv->rsvBalance = initialBalance;
    psuv->rsvGoal = initialBalance;
}

void DeleteSuv(SUV* psuv)
{
	delete psuv;
}

void StartupLapCtr(LAPCTR* plapctr)
{
    plapctr->pvtlapctr = &g_vtlapctr;
}

void StartupBoostCtr(BOOSTCTR* pboostctr)
{
    pboostctr->pvtboostctr = &g_vtboostctr;
}

void StartupPlaceCtr(PLACECTR* placectr)
{
    placectr->pvtplacectr = &g_vtplacectr;
}

LAPCTR g_lapctr;
BOOSTCTR g_boostctr;
PLACECTR g_placectr;
float SV_SuvMinFriction = 800.0;
CLQ s_clqTune = {0.0, 1.0, 0.0};

SNIP s_asnipSuv[20] = 
{
    { 0x04, (OID)0x416, offsetof(SUV, asxp[0].paloWheel) },
    { 0x04, (OID)0x417, offsetof(SUV, asxp[1].paloWheel) },
    { 0x04, (OID)0x418, offsetof(SUV, asxp[2].paloWheel) },
    { 0x04, (OID)0x419, offsetof(SUV, asxp[3].paloWheel) },

    { 0x00, (OID)0x41A, offsetof(SUV, asxp[0].ppntTop) },
    { 0x00, (OID)0x41B, offsetof(SUV, asxp[1].ppntTop) },
    { 0x00, (OID)0x41C, offsetof(SUV, asxp[2].ppntTop) },
    { 0x00, (OID)0x41D, offsetof(SUV, asxp[3].ppntTop) },

    { 0x06, (OID)0x41E, offsetof(SUV, asxp[0].paloHub) },
    { 0x06, (OID)0x41F, offsetof(SUV, asxp[1].paloHub) },
    { 0x06, (OID)0x420, offsetof(SUV, asxp[2].paloHub) },
    { 0x06, (OID)0x421, offsetof(SUV, asxp[3].paloHub) },

    { 0x00, (OID)0x422, offsetof(SUV, paloShadow) },
    { 0x02, (OID)0x428, offsetof(SUV, paloFrontAxle) },
    { 0x02, (OID)0x429, offsetof(SUV, paloRearAxle) },
    { 0x02, (OID)0x42A, offsetof(SUV, psmPuncher) },
    { 0x02, (OID)0x42B, offsetof(SUV, psoPuncher) },
    { 0x22, (OID)0x423, offsetof(SUV, pemitterBoost) },
    { 0x20, (OID)0x424, offsetof(SUV, pexplDirt)},
    { 0x20, (OID)0x425, offsetof(SUV, pexplDust)}
};

float DT_SuvUpright = 1.0;
float DT_SuvPunched = 2.0;
CLQ s_clqSuvUpright = {30.0, 3.0, 0.0};
CLQ CLQ_DustFromSpeed = {0.0, 0.0099999998, 0.0};
float C_DustMin = 0.0;
float C_DustMax = 45.0;
CLQ SuvSpringVelocity = {0.0, 0.25, 0.75};
float K_SuvSpringVelocity = 15.0;
CLQ CLQ_SuvSpringPos = {0.0, 0.0, 1.0};
float K_SuvSpringPos = 90.0;
CLQ CLQ_AirborneUpright = {0.0, 0.0, 1.0};
float G_SuvSuspensionLoadMax = 6000.0;
float DT_SuvBoost = 1.25;
CLQ CLQ_SuvBoostAccel = {1.5, 0.0, -0.5};
float R_SuvBoostAccelMin = 1.0;
float R_SuvBoostAccelMax = 1.5;
float SV_SuvTractionMax = 11250.0;
CLQ CLQ_SuvSpeedToDrive = {1.0, -1.0, 0.0};
float SV_SuvSteerMax = 2500.0;
float K_SuvSteer = 7.0;
CLQ CLQ_SuvBoostDrive = {1.35, 0.0, -0.34999999};
float R_SuvDriveMin = 1.0;
float R_SuvDriveMax = 1.35;
CLQ CLQ_SuvFacingDrive = {0.5, 1.0, 0.0};
float R_SuvFacingDriveMin = 0.5;
float R_SuvFacingDriveMax = 1.0;
CLQ CLQ_SuvAccelLimit = {9000, 0.0, -9000};
float R_SuvAccelLimitMin = 4500.0;
float R_SuvAccelLimitMax = 9000.0;
SMP s_smpSuvAccel = {4000.0, 0.0, 2.0};
SMP s_smpSuvDecel = {2000.0, 0.0, 1.0};
CLQ CLQ_DirtFromAccel = {0.0, 0.03333333, 0.0};
float C_DirtMin = 0.0;
float C_DirtMax = 50.0;
float G_SuvDownForce = -10.0;
SMP s_smpBalance = {0.5, 0.0, 0.25};
LM s_lmDtBalance = {1.5, 2.5};
CLQ CLQ_SuvDsToDyAvoid = {-0.5, 0.0, 0.0};
LM LM_SuvDyAvoid = {0.0, 625.0f};
float DS_SuvHoming = 850.0f;
LM LM_SuvTrack = {-1300, 1300};
SMP s_smpLine = {1125.0, 0.0, 0.1};
float DS_SuvNext = 750.0;
float R_SuvTurnDrive = 1.0;
float RAD_SuvWheelMax = 0.69999999;
SMP s_smpWheelTurn = {20.0, 0.0, 0.25};
CLQ CLQ_SuvDtToRDensity = {1.0f, -1.0f, 0.0f};
LM LM_SuvRadStick = {0.4, 1.0};
LM LM_SuvRadSpray = {0.1, 0.4};
float S_SuvSoundMax = 4000.0;
CLQ CLQ_SuvSuspensionToEngineBlend = {1.0, -0.6, 0.0};
LM LM_SuvEngineBlend = {0.25, 1.0};
float R_SuvTargetEngineSpeed = 1.1;
CLQ CLQ_SuvIdleFrq = {-0.5, 0.69999999, 0.0};
CLQ CLQ_SuvIdleVol = {1.0, -0.5, 0.0};
LM LM_SuvIdleVol = {0.0, 1.0};
CLQ CLQ_SuvRunningFrq = {-0.5, 2.0, 0.0};
CLQ CLQ_SuvRunningVol = { 0.5, 4.0, 0.0 };
LM LM_SuvRunningVol = {0.0, 2.0};
float R_SuvTireSlipX = 0.75;
float S_SuvTireSlipMax = 1000.0;
CLQ CLQ_SuvSuspensionToTireSlip = {0.0, 1.0, 0.0};
LM LM_SuvTireSlipScale = {0.0, 1.0};
CLQ CLQ_SuvTireSlipFrq = {0.5, 0.25, 0.0};
CLQ CLQ_SuvTireSlipVol = { 0.5, 0.25, 0.0 };
LM LM_SuvTireSlipVol = {0.0, 1.0};
SMP s_smpSuvEngineSpeed = {10.0, 0.0, 0.1};
glm::vec3 s_vPuncher = {0.0, 2000.0,0.0};
float R_SuvPuncherSvToSw = 0.002;
float RV_SuvPuncherSelf = 0.5;
SMP s_smpWheelFree = {2000.0, 0.0, 0.15};
int N_SuvChaseWin = 60;
int g_fVehicleSpeedLimited = 0;
float DT_SuvBoostRecharge = 0.75;
SCORES* g_pscoresCur;
