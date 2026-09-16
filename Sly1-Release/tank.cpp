#include "tank.h"
#include "actadj.h"
#include "sm.h"
#include "jt.h"
#include "bbmark.h"
#include "suv.h"

TANK* NewTank()
{
	return new TANK{};
}

void InitTank(TANK* ptank)
{
    InitStep(ptank);

    ptank->radTarget = atan2f(ptank->xf.mat[0][1], ptank->xf.mat[0][0]);
    LoadRotateMatrixRad(ptank->radTarget, &g_normalZ, &ptank->matTarget);

    ptank->tanks = TANKS_Nil;
    ptank->tFlash = -10.0f;

    ptank->clqDradToURun = s_clqDradToURun;
}

void SetTankTanks(TANK* ptank, TANKS tanks)
{
    TANKS tanksPrev = ptank->tanks;

    if (tanks == tanksPrev) {
        return;
    }

    if (tanksPrev == TANKS_Zap) {
        ptank->tCharm = g_clock.t;
    }
    else if (tanksPrev == TANKS_Dead) {
        ptank->zok = ZOK_Inherit;
        ptank->tCharm = g_clock.t;
    }

    switch (tanks)
    {
        case TANKS_Passive:
        SetSmaGoal(ptank->psma, (OID)849);
        break;

        case TANKS_Active:
        SetSmaGoal(ptank->psma, (OID)850);
        break;

        case TANKS_Zap:
        SeekSma(ptank->psma, (OID)852);
        ptank->tCharm = FLT_MAX;
        break;

        case TANKS_Dead:
        SeekSma(ptank->psma, (OID)853);
        ptank->zok = ZOK_Ignore;
        ptank->tCharm = FLT_MAX;
        break;

        default:
        break;
    }

    if (tanks == TANKS_Passive ||
        (tanks > TANKS_Active && tanks < TANKS_Peek)) {
        ptank->vTarget = g_vecZero;
        LoadRotateMatrixRad(ptank->radTarget, &g_normalZ, &ptank->matTarget);
        ptank->pzi.fCollectEnabled = 0;
    }
    else {
        ptank->pzi.fCollectEnabled = 1;
    }

    ptank->tanks = tanks;
    ptank->tTanks = g_clock.t;
}

void* GetTankTanks(TANK* ptank)
{
    return &ptank->tanks;
}

void SetfAllowEject(TANK* ptank, int fAllowEject)
{
    ptank->fAllowEject = fAllowEject;
}

void* GetfAllowEject(TANK* ptank)
{
    return &ptank->fAllowEject;
}

void PostTankLoad(TANK* ptank)
{
    PostStepLoad(ptank);
    SnipAloObjects(ptank, 9, s_asnipTank);

    ptank->pvtso->pfnCloneSoPhys(ptank, ptank->psoPhys, true);

    CreateAloActadj(ptank->paloGut, 5, &ptank->pactadjGut);
    ptank->pactadjGut->ackPos = ACK_Spring;
    ptank->pactadjGut->ackRot = ACK_Spring;
    ptank->paloGut->pvtalo->pfnRecacheAloActList(ptank->paloGut);

    CreateAloActadj(ptank->paloHead, 5, &ptank->pactadjHead);
    ptank->pactadjHead->ackRot = ACK_Spring;
    ptank->paloHead->pvtalo->pfnRecacheAloActList(ptank->paloHead);

    ptank->psma = PsmaApplySm(ptank->psm, ptank, OID_Nil, true);

    TANKS tanks = TANKS_Active;

    if (ptank->fAllowEject && g_pjt != nullptr) 
        tanks = TANKS_Passive;

    SetTankTanks(ptank, tanks);

    ptank->pzi.paloCollect = ptank;
    ptank->pzi.sAutoCollect = 150.0f;

}

void UpdateTank(TANK* ptank, float dt)
{
    UpdateStep(ptank, dt);

    if (FDrivenAlo(ptank))
        UpdateTankDriveTarget(ptank);

    XP* pxpZap = ptank->pxpZap;
    while (pxpZap != nullptr) 
    {
        TANK* ptankRoot = (TANK*)pxpZap->axpd[0].psoRoot;

        SO* psoSource = pxpZap->axpd[ptankRoot == ptank].psoLeaf;
        while (psoSource != nullptr) {
            ZOK zok = psoSource->zok;

            if (zok != ZOK_Inherit) {
                if (zok == ZOK_Zap || zok == ZOK_ZapAndIgnore) {
                    ZPR zpr;

                    InitZpr(&zpr, ZPK_Blunt, (LO*)psoSource);

                    while (psoSource != nullptr && !psoSource->pvtso->pfnFInflictSoZap(psoSource, pxpZap, &zpr))
                        psoSource = (SO*)psoSource->paloParent;

                    ptank->pvtpo->pfnFTakePoDamage((PO*)ptank, &zpr);
                }

                break;
            }

            psoSource = (SO*)psoSource->paloParent;
        }

        pxpZap = pxpZap->pxpNext;
    }

    if (ptank->pxpZap != nullptr) 
    {
        FreeSwXpList(ptank->psw, ptank->pxpZap);
        ptank->pxpZap = nullptr;
    }

    float dtFlash = g_clock.t - ptank->tFlash;

    float uSpring =
        s_clqDtDamageToUSpring.g0 +
        dtFlash * (s_clqDtDamageToUSpring.g1 +
            dtFlash * s_clqDtDamageToUSpring.g2);

    uSpring = glm::clamp(uSpring, g_lmZeroOne.gMin, g_lmZeroOne.gMax);

    SetAloPositionSpring(ptank->paloGut, uSpring * R_TankGutPosSpring);
    SetAloRotationSpring(ptank->paloGut, uSpring * R_TankGutRotSpring);

    if (ptank->fFlash && ptank->tanks != TANKS_Zap && g_clock.t - ptank->tCharm > 2.0f)
        ptank->fFlash = false;
}

void UpdateTankDriveTarget(TANK* ptank)
{
    float radPanTarget;
    float radTiltTarget;
    float svTarget;

    CalculateAloDrive(ptank, nullptr, nullptr, g_clock.dt, ptank->radTarget, &radPanTarget, &radTiltTarget, &svTarget);
    ptank->radTarget = radPanTarget;

    LoadRotateMatrixPanTilt(radPanTarget, radTiltTarget, &ptank->matTarget);
    ptank->vTarget = glm::vec3(-svTarget, 0.0f, 0.0f);
}

void UseTankCharm(TANK* ptank)
{
    UsePoCharm(ptank);
    SetTankTanks(ptank, TANKS_Active);
}

void UpdateTankActive(TANK* ptank, JOY* pjoy, float dt)
{
    if (FDrivenAlo(ptank)) {
        return;
    }

    int oidGoal = OID_Nil;
    int oidCur = OID_Nil;

    GetSmaCur(ptank->psma, (OID*)&oidCur);
    GetSmaGoal(ptank->psma, (OID*)&oidGoal);

    if (ptank->tanks == TANKS_Zap) {
        if (oidCur == 850) {
            ptank->pvtpo->pfnUsePoCharm(ptank);
        }
        else if (ptank->fCharmEnabled && pjoy->uDeflect2 > 0.7f) {
            ptank->pvtpo->pfnUsePoCharm(ptank);
            SeekSma(ptank->psma, (OID)850);
            GetSmaCur(ptank->psma, (OID*)&oidCur);
            GetSmaGoal(ptank->psma, (OID*)&oidGoal);
        }
    }

    if (oidCur < 850 || oidCur > 851) {
        return;
    }

    if (oidGoal != OID_Nil && oidGoal != 850 && oidGoal != 851) {
        return;
    }

    HandleDialogButtons(pjoy);

    if (FActiveCplcy(&g_pcm->cpman)) {
        SetSmaGoal(ptank->psma, (OID)850);
    }
    else if (pjoy->uDeflect2 > 0.7f) {
        AdjustCmJoy(g_pcm, pjoy, JOYID_Right, &ptank->radTarget);

        float radTank = atan2f(ptank->xf.mat[0][1], ptank->xf.mat[0][0]);
        float dRad = RadNormalize(ptank->radTarget - radTank);

        if (fabsf(dRad) < 0.5f && g_fVehicleSpeedLimited == 0) {
            SetSmaGoal(ptank->psma, (OID)851);
        }
    }
    else {
        SetSmaGoal(ptank->psma, (OID)850);
    }

    float radDrive = 0.0f;
    AdjustCmJoy(g_pcm, pjoy, JOYID_Left, &radDrive);

    float sv = pjoy->uDeflect * pjoy->uDeflect * 800.0f;
    sv = glm::clamp(sv, 0.0f, 800.0f);

    glm::vec3 vDrive;
    SetVectorCylind(&vDrive, radDrive + 3.1415927f, sv, 0.0f);

    glm::vec3 driveDir;
    SetVectorCylind(&driveDir, radDrive, 1.0f, 0.0f);

    glm::vec3 tiltAxis = glm::cross(g_normalZ, driveDir);

    float uTilt = pjoy->uDeflect;
    uTilt = glm::clamp(uTilt, 0.0f, 1.0f);

    glm::mat3 matTilt = glm::rotate(glm::mat4(1.0f), 0.15f * uTilt, tiltAxis);
    glm::mat3 matPan = glm::rotate(glm::mat4(1.0f), ptank->radTarget, g_normalZ);

    ptank->matTarget = matTilt * matPan;
    // vDrive is camera-relative world motion. The original VU code takes
    // row dot-products here, converting it into matTarget-local space; the
    // STEP contact solver transforms it back to world space afterward.
    ptank->vTarget = glm::transpose(ptank->matTarget) * vDrive;

    glm::mat3 matUntilt = glm::rotate(glm::mat4(1.0f), -0.2f * uTilt, tiltAxis);

    ALO* paloGutParent = ptank->paloGut->paloParent;

    glm::mat3 matGutWorld = matUntilt * paloGutParent->xf.matWorld;
    glm::mat3 dmatGoal = glm::transpose(paloGutParent->xf.matWorld) * matGutWorld;

    ptank->pactadjGut->dmatGoal = dmatGoal;
    ptank->pactadjHead->dmatGoal = dmatGoal;

    float speed = glm::length(ptank->xf.v);

    float leanV = 0.0f;
    if (speed > 0.0f) {
        float uSpeed = glm::clamp(speed * 0.00125f, 0.0f, 1.0f);
        leanV = (15.0f * uSpeed) / speed;
    }

    float uLeanJoy = pjoy->uDeflect * (2.0f - pjoy->uDeflect);
    uLeanJoy = glm::clamp(uLeanJoy, 0.0f, 1.0f);
    float leanJoy = 10.0f * uLeanJoy;

    glm::vec3 leanDir;
    SetVectorCylind(&leanDir, radDrive, 1.0f, 0.0f);

    glm::vec3 leanWorld = ptank->xf.v * leanV + leanDir * leanJoy;
    glm::vec3 leanLocal = glm::transpose(paloGutParent->xf.matWorld) * leanWorld;

    ptank->pactadjGut->dposGoal = leanLocal;

    if (ptank->fAllowEject && g_pjt != nullptr && pjoy->IsPressed(BTN_CROSS))
        SetTankTanks(ptank, TANKS_Passive);
}

void OnTankActive(TANK* ptank, int fActive)
{
    OnPoActive(ptank, fActive, nullptr);

    if (fActive == 0) 
        ptank->fFlash = 0;
}

void RenderTankAll(TANK* ptank, CM* pcm, RO* pro)
{
    if (ptank->fHidden) {
        return;
    }

    glm::vec4 cel = g_rgbaCel;

    if (ptank->fFlash) {
        float tCharm = 0.0f;

        if (ptank->tanks != TANKS_Zap) {
            tCharm = (g_clock.t - ptank->tCharm) * 0.5f;
        }

        float flashWave = cosf((g_clock.t - ptank->tFlash) * 10.0f);

        float uZapToCel =
            s_clqUZapToUCel.g0 +
            tCharm * (s_clqUZapToUCel.g1 + tCharm * s_clqUZapToUCel.g2);

        float uFlashToCel =
            s_clqUFlashToUCel.g0 +
            flashWave * (s_clqUFlashToUCel.g1 + flashWave * s_clqUFlashToUCel.g2);

        uZapToCel = glm::clamp(uZapToCel, 0.0f, 1.0f);
        uFlashToCel = glm::clamp(uFlashToCel, 0.0f, 1.0f);

        float uFlash = uZapToCel * uFlashToCel;

        constexpr glm::vec4 tankFlashCel = {0.627451f, 0.0f, 0.0f, 1.0f};

        glm::vec4 flashCel = tankFlashCel;
        flashCel.a = cel.a;

        cel = glm::mix(cel, flashCel, uFlash);
    }

    glm::vec4 overrideCel[4] = {};
    overrideCel[0] = cel;

    SetAloOverrideCel(ptank, overrideCel);
    RenderAloAll(ptank, pcm, pro);
}

void ImpactTank(TANK* ptank, int fParentDirty)
{
    ALO* paloGut = ptank->paloGut;
    ALO* paloParent = paloGut->paloParent;

    glm::vec3 dposWorld = paloParent->xf.matWorld * ptank->dpos;

    paloGut->xf.v -= dposWorld * R_TankImpact.x;

    ImpactSo(ptank, fParentDirty);
}

int FInvulnerableTank(TANK* ptank, ZPK zpk)
{
    if (zpk == ZPK_Pit) {
        return 1;
    }

    if (ptank->tanks == TANKS_Zap ||
        ptank->tanks == TANKS_Dead) {
        return 1;
    }

    if (ptank->tanks == TANKS_Passive) {
        return 1;
    }

    if (ptank->fFlash) {
        return 1;
    }

    if (FSwHandsOff(ptank->psw)) {
        return 1;
    }

    return 0;
}

void ApplyTankThrow(TANK* ptank, PO* ppo)
{
    JT* pjt = static_cast<JT*>(ppo);
    glm::vec3 dir = ptank->xf.v;
    dir.z = 0.0f;

    if (glm::length(dir) >= 0.0001f) {
        dir = glm::normalize(dir);
    }
    else {
        dir = g_normalX;
    }

    glm::vec3 vThrow = dir * R_TankImpact.y;
    vThrow.z = R_TankImpact.z;

    pjt->pvtalo->pfnSetAloVelocityVec(pjt, &vThrow);
    pjt->radTarget = RadNormalize(atan2f(vThrow.y, vThrow.x) + 3.1415927f);

    FixStepAngularVelocity(pjt);
    SetJtJts(pjt, 4, 43);
}

int FTakeTankDamage(TANK* ptank, ZPR* pzpr)
{
    const int charmAvailable = FCharmAvailable();
    const int invulnerable = ptank->pvtpo->pfnFInvulnerablePo(ptank, pzpr->zpk);
    if (invulnerable)
        return 0;

    if (charmAvailable)
        ptank->fFlash = 1;

    ptank->tFlash = g_clock.t;
    InvalidateSwXpForObject(ptank->psw, ptank, 7);

    if (pzpr->zpk == ZPK_Blunt && pzpr->pfnzap != nullptr) {
        pzpr->pfnzap((PO*)pzpr->pv, ptank);

        glm::vec3 tankVel = {ptank->xf.v.x, ptank->xf.v.y, 0.0f};

        glm::vec3 dir = glm::vec3(g_normalX.x, g_normalX.y, 0.0f);
        float speed = glm::length(tankVel);

        if (speed >= 0.0001f) {
            dir = tankVel / speed;
        }

        float tankDamageSpeed = charmAvailable
            ? R_TankImpact.z
            : SV_TankDamageDead;

        glm::vec3 tankThrowVel = dir * tankDamageSpeed;

        glm::vec3 v;
        v.x = tankThrowVel.x;
        v.y = tankThrowVel.y;
        v.z = tankThrowVel.z;
       // v.gUnused = 0.0f;

        ptank->pvtalo->pfnSetAloVelocityVec(ptank, &v);

        glm::vec3 gutVel = dir * VXY_GutDamage;
        gutVel.z = VZ_GutDamage;

        v.x = gutVel.x;
        v.y = gutVel.y;
        v.z = gutVel.z;
        //v.gUnused = 0.0f;

        ConvertAloVec(nullptr, ptank->paloGut->paloParent, &v, &v);
        ptank->paloGut->pvtalo->pfnSetAloVelocityVec(ptank->paloGut, &v);

        // The decompile packs dir.x / dir.y through MMI ops here.
        // This appears to generate a horizontal spin axis perpendicular to the hit direction.
        glm::vec3 gutSpin = {-dir.y * SW_GutDamage, dir.x * SW_GutDamage, 0.0f};

        glm::vec3 w;
        w.x = gutSpin.x;
        w.y = gutSpin.y;
        w.z = gutSpin.z;
        //w.gUnused = 0.0f;

        ConvertAloVec(nullptr, ptank->paloGut->paloParent, &w, &w);
        ptank->paloGut->pvtalo->pfnSetAloAngularVelocityVec(ptank->paloGut, &w);
    }

    LO* source = pzpr->ploSource;

    if (!charmAvailable && g_pjt != nullptr) {
        g_pjt->pvtlo->pfnAddLo(g_pjt);
        g_pjt->pvtalo->pfnMatchAloOtherObject(g_pjt, ptank->paloJt);
        ptank->paloJt->pvtlo->pfnRemoveLo(ptank->paloJt);

        SwitchToPo(g_pjt);
        SetTankTanks(ptank, TANKS_Dead);

        ZPR zpr;
        InitZpr(&zpr, ZPK_Blunt, ptank);
        zpr.pfnzap = (PFNZAP)ApplyTankThrow;

        g_pjt->pvtpo->pfnFTakePoDamage(g_pjt, &zpr);

        ptank->zok = ZOK_Ignore;
    }
    else
    {
        SetTankTanks(ptank, TANKS_Zap);
    }

    ZPR msg = {};
    msg.ploSource = source;
    //msg.zpk = ptank;
    
    ptank->pvtlo->pfnSendLoMessage(ptank, MSGID_damaged, &msg);

    return 1;
}

void AdjustTankXps(TANK* ptank)
{
    AdjustSoZapXps(ptank, &ptank->pxpZap);
    AdjustStepXps(ptank);
}

void AdjustTankNewXp(TANK* ptank, XP* pxp, int ixpd)
{
    if (!ptank->pvtstep->pfnFCheckStepXpBase(ptank, pxp, ixpd)) {
        pxp->elas = 0.0f;
        pxp->mu = 0.0f;
        return;
    }

    float dtDamage = g_clock.t - ptank->tFlash;
    float uMu = s_clqDtDamageToUMu.g0 + dtDamage * (s_clqDtDamageToUMu.g1 + dtDamage * s_clqDtDamageToUMu.g2);

    uMu = glm::clamp(uMu, g_lmZeroOne.gMin, g_lmZeroOne.gMax);

    pxp->elas = 0.0f;
    pxp->mu = uMu * MU_TankInternal;
}

int GetTankSize()
{
	return sizeof(TANK);
}

void CloneTank(TANK* ptank, TANK* ptankBase)
{
    CloneStep(ptank, ptankBase);

    ptank->tanks = ptankBase->tanks;
    ptank->tTanks = ptankBase->tTanks;
    ptank->fFlash = ptankBase->fFlash;
    ptank->tFlash = ptankBase->tFlash;
    ptank->tCharm = ptankBase->tCharm;
    ptank->tCharmPending = ptankBase->tCharmPending;
    ptank->fCharmEnabled = ptankBase->fCharmEnabled;
    ptank->fAllowEject = ptankBase->fAllowEject;
    ptank->pxpZap = ptankBase->pxpZap;
    ptank->paloJt = ptankBase->paloJt;
    ptank->paloGut = ptankBase->paloGut;
    ptank->pactadjGut = ptankBase->pactadjGut;
    ptank->paloHead = ptankBase->paloHead;
    ptank->pactadjHead = ptankBase->pactadjHead;
    ptank->psm = ptankBase->psm;
    ptank->psma = ptankBase->psma;
    ptank->ppntAnchor = ptankBase->ppntAnchor;
}

void HandleTankMessage(TANK* ptank, MSGID msgid, void* pv)
{
    if (msgid == MSGID_label) {
        const ASEGLABELMSG* pmsg = static_cast<const ASEGLABELMSG*>(pv);

        if (ptank->psma != nullptr && pmsg->pasega == ptank->psma->pasegaCur)
        {
            switch (pmsg->label)
            {
                case 350:
                ptank->fCharmEnabled = 1;
                break;

                case 351:
                ptank->fCharmEnabled = 0;
                break;
            }
        }
    }
    else if (msgid == MSGID_sma_transition && pv == ptank->psma) {
        OID oidCur = OID_Nil;
        OID oidGoal = OID_Nil;

        GetSmaCur(static_cast<SMA*>(pv), &oidCur);
        GetSmaGoal(ptank->psma, &oidGoal);
    }

    HandlePoMessage(ptank, msgid, pv);
}

JTHS JthsCurrentTank(TANK* ptank)
{
    return static_cast<JTHS>(ptank->fFlash != 0);
}

void DeleteTank(TANK* ptank)
{
	delete ptank;
}

CLQ s_clqDtDamageToUMu = {2.0, 0.0, 0.0};
float R_TankGutPosSpring = 6.0;
float R_TankGutRotSpring = 3.0;
glm::vec3 R_TankImpact = {0.5f, 700.0f, 650.0f};
float VXY_GutDamage = 400.0;
float VZ_GutDamage = -100.0;
float SV_TankDamageDead = 300.0;
float SW_GutDamage = 5.0;
float MU_TankInternal = 1.5;

SNIP s_asnipTank[9] = 
{
    0, (OID)0x34D, offsetof(TANK, psoPhys),
    0, (OID)0x350, offsetof(TANK, psm),
    4, (OID)0x60,  offsetof(TANK, paloGut),
    4, (OID)0x50,  offsetof(TANK, paloHead),
    4, (OID)0x34E, offsetof(TANK, paloJt),
    2, (OID)0x161, offsetof(TANK, pzi.mpccharmpaloCharm[2]),
    2, (OID)0x162, offsetof(TANK, pzi.mpccharmpaloCharm[3]),
    2, (OID)0x163, offsetof(TANK, pzi.mpccharmpaloCharm[4]),
    4, (OID)0x160, offsetof(TANK, pzi.mpccharmpaloCharm[0]),
};
float g_fTankFireBlocked = 0.0;
