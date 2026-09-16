#include "turret.h"
#include "sm.h"
#include "jt.h"
#include "murray.h"
#include "rwm.h"
#include "binoc.h"
#include "alarm.h"

TURRET* NewTurret()
{
	return new TURRET{};
}

int GetTurretSize()
{
	return sizeof(TURRET);
}

void InitTurret(TURRET* pturret)
{
    InitPo(pturret);

    pturret->lookk = LOOKK_Sniper;

    pturret->apsoTarget.resize(16);
    pturret->aoidTarget.resize(16);

    pturret->lmTilt.gMin = -0.7;
    pturret->lmTilt.gMax = 0.7;

    pturret->lmPan.gMin = -1.5;
    pturret->lmPan.gMax = 1.5;
}

void AddTurretTargetOid(TURRET* pturret, int oidTarget)
{
    if (pturret->aoidTarget.size() < 16) 
    {
        pturret->aoidTarget.push_back(oidTarget);
        pturret->cpsoTarget = (int)pturret->aoidTarget.size();
    }
}

void* GetTurretPsmaTurret(TURRET* pturret)
{
    return &pturret->psmaTurret;
}

void SetTurretPsmaTurret(TURRET* pturret, SMA* psmaTurret)
{
    pturret->psmaTurret = psmaTurret;
}

void SetTurretfSetCameraMatrix(TURRET* pturret, int fSetCameraMatrix)
{
    pturret->fSetCameraMatrix = fSetCameraMatrix;
}

void* GetTurretfSetCameraMatrix(TURRET* pturret)
{
    return &pturret->fSetCameraMatrix;
}

void SetTurretlookk(TURRET* pturret, int lookk)
{
    pturret->lookk = (LOOKK)lookk;
}

void* GetTurretlookk(TURRET* pturret)
{
    return &pturret->lookk;
}

void SetTurretLmTilt(TURRET* pturret, LM lmTilt)
{
    pturret->lmTilt = lmTilt;
}

void* GetTurretLmTilt(TURRET* pturret)
{
    return &pturret->lmTilt;
}

void SetTurretLmPan(TURRET* pturret, LM lmPan)
{
    pturret->lmPan = lmPan;
}

void* GetTurretLmPan(TURRET* pturret)
{
    return &pturret->lmPan;
}

void CloneTurret(TURRET* pturret, TURRET* pturretBase)
{
    ClonePo(pturret, pturretBase);

    pturret->psmTurret = pturretBase->psmTurret;
    pturret->psmaTurret = pturretBase->psmaTurret;
    pturret->prwm = pturretBase->prwm;
    pturret->ppntAnchor = pturretBase->ppntAnchor;
    pturret->pmurray = pturretBase->pmurray;
    pturret->fCharmEnabled = pturretBase->fCharmEnabled;
    pturret->fSetCameraMatrix = pturretBase->fSetCameraMatrix;
    pturret->field_0x630 = pturretBase->field_0x630;
    pturret->fTrack = pturretBase->fTrack;
}

void PostTurretLoad(TURRET* pturret)
{
    PostAloLoad(pturret);
    SnipAloObjects(pturret, 3, s_asnipTurret);

    pturret->psmaTurret = PsmaApplySm(pturret->psmTurret, (ALO*)pturret, OID_Nil, 1);

    MURRAY* pmurray = (MURRAY*)PloFindSwNearest(g_psw, (OID)7, pturret);
    if (pmurray == nullptr) 
        pturret->pjt = g_pjt;
    else 
    {
        pturret->pjt = (JT*)pmurray;
        pturret->pmurray = pmurray;
        pmurray->pturret = pturret;

        pturret->pzi.paloCollect = (ALO*)pmurray;
        for (int i = 0; i < 4; ++i)
            pturret->pzi.mpccharmpaloCharm[i] = (ALO*)PloFindSwObject(pturret->psw, 1, (OID)(352 + i), (LO*)pmurray);

        pturret->pzi.sAutoCollect = 300.0f;
        pmurray->pvtlo->pfnSubscribeLoObject(pmurray, pturret);
    }

    int i = 0;
    for (; i < pturret->cpsoTarget; i++) 
    {
        ALO* paloTarget = (ALO*)PloFindSwNearest(pturret->psw, (OID)pturret->aoidTarget[i], pturret);

        if (paloTarget == nullptr) {
            break;
        }

        pturret->apsoTarget[i] = (SO*)paloTarget;
    }

    pturret->cpsoTarget = i;

    DecomposeRotateMatrixPanTilt(&pturret->xf.matWorld, &pturret->radPan, &pturret->radTilt);
}

void UpdateTurret(TURRET* pturret, float dt)
{
    UpdatePo(pturret, dt);

    MURRAY* pmurray = pturret->pmurray;

    if (pmurray == nullptr)
        return;

    int hitReactionState = pmurray->pvtmurray->pfnJthsCurrentMurray(pmurray);

    if (hitReactionState == 0)
    {
        pturret->pzi.fCollectEnabled = 1;
        return;
    }

    STEPGUARD* apstepguard[8];

    int cstepguard = CploFindSwObjectsByClass(pturret->psw, 5, CID_STEPGUARD, nullptr, 8, reinterpret_cast<LO**>(apstepguard));

    for (int istepguard = 0; istepguard < cstepguard; ++istepguard)
    {
        STEPGUARD* pstepguard = apstepguard[istepguard];

        if (pstepguard != static_cast<STEPGUARD*>(pmurray) && pstepguard->sgs != SGS_Dying)
            SetStepguardSgs(pstepguard, SGS_Taunt, nullptr);
    }

    pturret->pzi.fCollectEnabled = 0;
}

void UpdateTurretActive(TURRET* pturret, JOY* pjoy, float dt)
{
    UpdateTurretAim(pturret);

    if (pjoy->IsPressed(BTN_SQUARE) || pjoy->IsPressed(BTN_CROSS))
        FireTurret(pturret);

    if (g_pjt != nullptr && g_pjt->pjsgCur != nullptr && FIsLoInWorld(g_pjt))
    {
        JOY joyEmpty = {};

        if (g_pjt->pvtjt->pfnUpdateJtActive != nullptr)
            g_pjt->pvtjt->pfnUpdateJtActive(g_pjt, &joyEmpty, dt);
    }
}

void OnTurretActive(TURRET* pturret, int fActive)
{
    OnPoActive(pturret, fActive, nullptr);

    if (!fActive) 
    {
        g_pcm->cplook.rZoomMax = 10.0f;

        PopUiActiveBlot(&g_ui);

        RevokeCmPolicy(g_pcm, 15, CPP_LookAround, &g_pcm->cplook, pturret, nullptr);
        LookkPopCplook(&g_pcm->cplook);

        SetSmaGoal(pturret->psmaTurret, (OID)856);
        return;
    }

    g_pcm->cplook.rZoomMax = 100.0f;

    PushUiActiveBlot(&g_ui, &g_binoc);
    SetBinocBinocs(&g_binoc, BINOCS_Sniper);

    g_pcm->cplook.ppntAnchor = pturret->ppntAnchor;

    if (pturret->fSetCameraMatrix) 
    {
        glm::mat4 matCamera(1.0f);

        matCamera[0] = glm::vec4(pturret->xf.matWorld[0], 0.0f);
        matCamera[1] = glm::vec4(pturret->xf.matWorld[1], 0.0f);
        matCamera[2] = glm::vec4(pturret->xf.matWorld[2], 0.0f);
        matCamera[3] = glm::vec4(pturret->xf.posWorld, 1.0f);

        g_pcm->mat = matCamera;
    }

    SetCmPolicy(g_pcm,CPP_LookAround, &g_pcm->cplook, pturret, nullptr);
    PushCplookLookk(&g_pcm->cplook, pturret->lookk);
    SetSmaGoal(pturret->psmaTurret, (OID)857);
}

int FFilterTurret(TURRET* pturret, SO* psoOther)
{
    if (psoOther->fNoXpsSelf) {
        return false;
    }

    if (psoOther->paloRoot == (SO*)pturret) {
        return false;
    }

    if (FIsRwmAmmo(pturret->prwm, psoOther)) {
        return false;
    }

    SO* psoRoot = (SO*)psoOther->paloRoot;
    SO* psoAmmo = pturret->prwm->psoAmmo;

    return !psoRoot->pvtso->pfnFIgnoreSoIntersection(psoRoot, psoAmmo) &&
        !psoAmmo->pvtso->pfnFIgnoreSoIntersection(psoAmmo, psoRoot);
}

void UpdateTurretAim(TURRET* pturret)
{
    RWM* prwm = pturret->prwm;

    if (!FEnsureRwmLoaded(prwm))
        return;

    float xFocus;
    float yFocus;
    GetBinocReticleFocus(&g_binoc, &xFocus, &yFocus);

    glm::vec3 posScreenClose(xFocus * (1.0f / 320.0f) - 1.0f, 1.0f - yFocus * 0.0040584416f, 10.0f);
    glm::vec3 posScreenFar(posScreenClose.x, posScreenClose.y, 65536.0f);

    glm::vec3 posWorldClose;
    glm::vec3 posWorldFar;

    ConvertCmScreenToWorld(g_pcm, &posScreenClose, &posWorldClose);
    ConvertCmScreenToWorld(g_pcm, &posScreenFar, &posWorldFar);

    std::vector <SO*> found;
    IntersectSwBoundingBox(pturret->psw, nullptr, &posWorldClose, &posWorldFar, (PFNFILTER)FFilterTurret, pturret, found);

    LSG lsg;
    SO* psoHit = PsoHitTestLineObjects(0, &posWorldClose, &posWorldFar, found, &lsg);

    glm::vec3 posHit;

    if (psoHit == nullptr || psoHit->oid == OID__MERGED_STATICS) 
    {
        prwm->rwti.palo = nullptr;
        prwm->rwti.pos = posWorldFar;
        posHit = posWorldFar;
        g_binoc.fTargeting = 0;
    }
    else 
    {
        prwm->rwti.palo = (ALO*)psoHit;
        ConvertAloPos(nullptr, (ALO*)psoHit, &lsg.apos[0], &prwm->rwti.pos);

        posHit = lsg.apos[0];

        ALO* paloRoot = psoHit->paloRoot != nullptr ? psoHit->paloRoot : psoHit;

        if (FIsBasicDerivedFrom((BASIC*)paloRoot, CID_MURRAY) ||
            FIsBasicDerivedFrom((BASIC*)paloRoot, CID_JT))
        {
            g_binoc.fTargeting = 2;
        }
        else if (FIsBasicDerivedFrom((BASIC*)paloRoot, CID_STEPGUARD))
        {
            g_binoc.fTargeting = 1;
        }
        else
        {
            g_binoc.fTargeting = 0;

            for (ALO* palo = psoHit; palo != nullptr && g_binoc.fTargeting == 0; palo = palo->paloParent)
            {
                for (int i = 0; i < pturret->cpsoTarget; ++i)
                {
                    if (pturret->apsoTarget[i] == palo)
                    {
                        g_binoc.fTargeting = 1;
                        break;
                    }
                }
            }
        }
    }

    glm::vec3 posFire;
    glm::mat3 matFireFake;
    GetRwfiPosMat(&prwm->rwfi, &posFire, &matFireFake, nullptr);

    glm::vec3 dpos = posHit - posFire;

    prwm->rwac.radPan  = atan2f(dpos.y, dpos.x);
    prwm->rwac.radTilt = atan2f(dpos.z, glm::length(glm::vec2(dpos.x, dpos.y)));
}

void FireTurret(TURRET* pturret)
{
    OID oidGoal;
    OID oidCur;

    GetSmaGoal(pturret->psmaTurret, &oidGoal);
    GetSmaCur(pturret->psmaTurret, &oidCur);

    if (oidGoal == (OID)857 && oidCur == (OID)857 && FEnsureRwmLoaded(pturret->prwm))
        SetSmaGoal(pturret->psmaTurret, (OID)858);
}

void HandleTurretMessage(TURRET* pturret, MSGID msgid, void* pv)
{
    HandlePoMessage(pturret, msgid, pv);

    if (msgid != MSGID_label)
        return;

    LABELMSG* pmsg = static_cast<LABELMSG*>(pv);

    if (pmsg->pasega == pturret->psmaTurret->pasegaCur)
    {
        if (pmsg->oidLabel == 859)
            FFireRwm(pturret->prwm, 1);

        return;
    }

    if (pturret->pmurray == nullptr)
        return;

    ASEGA* pasegaCharm = pturret->pmurray->pasegaSgs;

    if (pmsg->pasega != pasegaCharm)
        return;

    if (pmsg->oidLabel == 350)
        pturret->fCharmEnabled = 1;
    else if (pmsg->oidLabel == 351)
        pturret->fCharmEnabled = 0;
}

int FIgnoreTurretIntersection(TURRET* pturret, SO* psoOther)
{
    if (FIgnoreSoIntersection((SO*)pturret, psoOther)) {
        return 1;
    }

    if (pturret->prwm != nullptr && FIsRwmAmmo(pturret->prwm, psoOther)) {
        return 1;
    }

    return 0;
}

void CollectTurretPrize(TURRET* pturret, PCK pck, ALO* paloOther)
{
    if (pturret->pmurray == nullptr) 
        CollectPoPrize(pturret, pck, paloOther);
    else 
        pturret->pmurray->pvtpo->pfnCollectPoPrize(pturret->pmurray, pck, paloOther);
}

void GetTurretDiapi(TURRET* pturret, DIALOG* pdialog, DIAPI* pdiapi)
{
    pdiapi->fPlayable = 0;
    pdiapi->fCallable = 0;
}

ALO* PloGetTurretFocus(TURRET* pturret)
{
    if (pturret->pjt != nullptr)
        return pturret->pjt;

    return pturret;
}

void DeleteTurret(TURRET *pturret)
{
	delete pturret;
}

SNIP s_asnipTurret[3] = 
{
    0, (OID)0x356, offsetof(TURRET, prwm),
    0, (OID)0x357, offsetof(TURRET, psmTurret),
    2, (OID)0x35,  offsetof(TURRET, ppntAnchor)
};
