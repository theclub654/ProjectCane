#include "dart.h"
#include "jt.h"
#include "expl.h"

DART* NewDart()
{
	return new DART{};
}

void InitSwDartFreeDl(SW* psw)
{
	InitDl(&psw->dlDartFree, offsetof(DART, dleDartFree));
}

void InitDart(DART* pdart)
{
	InitSo(pdart);
    SetDartDarts(pdart, DARTS_Nil);
}

void* GetDartDarts(DART* pdart)
{
    return &pdart->darts;
}

void SetDartSvDart(DART* pdart, float svDart)
{
    pdart->svDart = svDart;
}

void* GetDartSvDart(DART* pdart)
{
    return &pdart->svDart;
}

void SetDartSRadiusFireCloud(DART* pdart, float sRadiusFireCloud)
{
    pdart->sRadiusFireCloud = sRadiusFireCloud;
}

void* GetDartSRadiusFireCloud(DART* pdart)
{
    return &pdart->sRadiusFireCloud;
}

void SetDartSRadiusStrikeCloud(DART* pdart, float sRadiusStrikeCloud)
{
    pdart->sRadiusStrikeCloud = sRadiusStrikeCloud;
}

void* GetDartSRadiusStrikeCloud(DART* pdart)
{
    return &pdart->sRadiusStrikeCloud;
}

void SetDartDtFade(DART* pdart, float dtFade)
{
    pdart->dtFade = dtFade;
}

void* GetDartDtFade(DART* pdart)
{
    return &pdart->dtFade;
}

void SetDartDtMaxStuck(DART* pdart, float dtMaxStuck)
{
    pdart->dtMaxStuck = dtMaxStuck;
}

void* GetDartDtMaxStuck(DART* pdart)
{
    return &pdart->dtMaxStuck;
}

int GetDartSize()
{
	return sizeof(DART);
}

void OnDartAdd(DART* pdart)
{
    OnSoAdd(pdart);

    if (FFindDlEntry(&pdart->psw->dlDartFree, pdart))
        RemoveDlEntry(&pdart->psw->dlDartFree, pdart);

    pdart->fNoXpsSelf = pdart->paloParent != nullptr && FIsBasicDerivedFrom(pdart->paloParent, CID_HBSK);
}

void RemoveDart(DART* pdart)
{
	RemoveLo(pdart);
	AppendDlEntry(&pdart->psw->dlDartFree, pdart);
    SetDartDarts(pdart, DARTS_AvailToFire);
}

void CloneDart(DART* pdart, DART* pdartBase)
{
    DLE dleDartFree = pdart->dleDartFree;

    CloneSo(pdart, pdartBase);

    pdart->darts = pdartBase->darts;
    pdart->tDarts = pdartBase->tDarts;
    pdart->svDart = pdartBase->svDart;
    pdart->sRadiusFireCloud = pdartBase->sRadiusFireCloud;
    pdart->sRadiusStrikeCloud = pdartBase->sRadiusStrikeCloud;
    pdart->dtFade = pdartBase->dtFade;
    pdart->dtMaxStuck = pdartBase->dtMaxStuck;

    pdart->dleDartFree = pdartBase->dleDartFree;
    pdart->pasegSticking = pdartBase->pasegSticking;
    pdart->pasegaSticking = pdartBase->pasegaSticking;
    pdart->paloTarget = pdartBase->paloTarget;

    pdart->dtLaunchToTarget = pdartBase->dtLaunchToTarget;
    pdart->dzTarget = pdartBase->dzTarget;

    pdart->pdartgunFiredFrom = pdartBase->pdartgunFiredFrom;
    pdart->pexpl = pdartBase->pexpl;

    pdart->dleDartFree = dleDartFree;
}

void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pdart, pbis);
    InferExpl(&pdart->pexpl, pdart);
}

void HandleDartMessage(DART* pdart, MSGID msgid, void* pv)
{
    HandleAloMessage(pdart, msgid, pv);

    if ((msgid == MSGID_asega_retracted) && (pv == pdart->pasegaSticking))
    {
        pdart->pasegaSticking = nullptr;
        SetDartDarts(pdart, DARTS_Stuck);
    }
}

void PostDartLoad(DART* pdart)
{
    PostAloLoad(pdart);
    SnipAloObjects(pdart, 1, &s_asnipDartLoad);
}

void UpdateDart(DART* pdart, float dt)
{
    UpdateSo(pdart, dt);

    switch (pdart->darts) 
    {
        case DARTS_Airborne:
        UpdateDartAirborne(pdart, dt);
        break;

        case DARTS_Stuck:
        if (g_clock.t - pdart->tDarts > pdart->dtMaxStuck && (g_pjt == nullptr || pdart->paloParent != (ALO*)g_pjt->phbsk)) 
            SetDartDarts(pdart, DARTS_Fading);
        break;

        default:
        break;
    }
}

void SetDartDarts(DART* pdart, DARTS darts)
{
    if (pdart->darts == darts)
        return;

    if (pdart->darts == DARTS_Airborne) 
    {
        pdart->paloTarget = nullptr;
        pdart->dtLaunchToTarget = 0.0f;
        pdart->dzTarget = 0.0f;
        pdart->pdartgunFiredFrom = nullptr;
    }

    pdart->darts = darts;
    pdart->tDarts = g_clock.t;

    if (darts == DARTS_Fading)
        FadeAloOut(pdart, pdart->dtFade);
}

void ApplyDartThrow(DART* pdart, PO* ppo)
{
    glm::vec3 normal(pdart->xf.v.x, pdart->xf.v.y, 0.0f);

    if (glm::length(normal) < 0.0001f)
        normal = glm::vec3(g_normalX);
    else
        normal = glm::normalize(normal);

    glm::vec3 velocity = normal * 500.0f + glm::vec3(g_normalZ) * 800.0f;

    ppo->pvtalo->pfnSetAloVelocityVec(ppo, &velocity);

    if (FIsBasicDerivedFrom(ppo, CID_JT)) {
        SetJtJts((JT*)ppo, JTS_Zap, (JTBS)43);
    }
}

void UpdateDartAirborne(DART* pdart, float dt)
{
    SO* psoTouch = PsoFirstSoTouching((SO*)pdart);

    if (psoTouch == nullptr) {
        if (g_pjt != nullptr && g_pjt->jts == JTS_Zap) {
            return;
        }

        glm::vec3 posTarget = glm::vec3(pdart->paloTarget->xf.posWorld);
        posTarget.z += pdart->dzTarget;

        float dtOffset = glm::clamp(glm::distance(posTarget, glm::vec3(pdart->xf.posWorld)) / pdart->svDart, 0.0f, 0.25f);

        PredictAloTransform(pdart->paloTarget, nullptr, dtOffset, &posTarget, nullptr, nullptr, nullptr);
        posTarget.z += pdart->dzTarget;

        glm::vec3 dpos = posTarget - glm::vec3(pdart->xf.posWorld);
        glm::vec3 velocity = glm::length(dpos) < 0.0001f ? glm::vec3(0.0f) : glm::normalize(dpos) * pdart->svDart;
        glm::mat3 mat;

        pdart->pvtalo->pfnSetAloVelocityVec(pdart, &velocity);
        LoadLookAtMatrix(&velocity, &mat);
        pdart->pvtalo->pfnRotateAloToMat(pdart, &mat);
        return;
    }

    bool fImpact = true;

    if (psoTouch == (SO*)g_pjt && (g_pjt->jts != JTS_Hide || g_pjt->jthk != JTHK_Basket)) {
        ZPR zpr{};

        InitZpr(&zpr, ZPK_Blunt, (LO*)pdart);
        zpr.pfnzap = (PFNZAP)ApplyDartThrow;
        g_pjt->pvtpo->pfnFTakePoDamage(g_pjt, &zpr);
    }
    else if (FIsBasicDerivedFrom((BASIC*)psoTouch, CID_HBSK)) {
        HBSK* phbsk = (HBSK*)psoTouch;

        StartSound((SFXID)52, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

        /*if (g_pjt != nullptr && g_pjt->phbsk == phbsk) {
            TriggerJoyRumbleRumk(&g_joy, RUMK_Blunt, 0.25f);
        }*/

        if (CdartStuckHbsk(phbsk) >= phbsk->cMaxDartsStuck) {
            DART* pdartOldest = PdartOldestHbskStuck(phbsk);

            if (pdartOldest != nullptr) {
                SetDartDarts(pdartOldest, DARTS_Fading);
            }
        }

        glm::vec3 zero(0.0f);

        pdart->pvtlo->pfnSetLoParent(pdart, phbsk);
        pdart->pvtalo->pfnSetAloVelocityVec(pdart, &zero);
        pdart->pvtalo->pfnSetAloAngularVelocityVec(pdart, &zero);
        SetSoConstraints(pdart, CT_Locked, nullptr, CT_Locked, nullptr);
        SetDartDarts(pdart, DARTS_Stuck);

        if (pdart->pasegSticking != nullptr) {
            ApplyAseg(pdart->pasegSticking, pdart, 0.0f, 1.0f, 0, &pdart->pasegaSticking);

            if (pdart->pasegaSticking != nullptr) {
                SetDartDarts(pdart, DARTS_Sticking);
            }
        }

        fImpact = false;
    }
    else if (FIsBasicDerivedFrom((BASIC*)psoTouch, CID_DART)) {
        DART* pdartHit = (DART*)psoTouch;

        if (pdartHit->darts >= DARTS_Sticking && pdartHit->darts < DARTS_Max) {
            fImpact = false;
            pdartHit->pvtlo->pfnSetLoParent(pdartHit, nullptr);
            pdartHit->pvtlo->pfnRemoveLo(pdartHit);
        }
    }
    else {
        WKR wkr{};

        wkr.ploTarget = (LO*)psoTouch;
        wkr.ploSource = (LO*)pdart;
        wkr.pos = pdart->xf.posWorld;
        wkr.v = pdart->xf.v;

        PaloAbsorbWkr(&wkr, 0, nullptr);
    }

    if (!fImpact) {
        return;
    }

    StartSound((SFXID)51, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

    if (pdart->pexpl != nullptr) 
    {
        EXPLSO explso{};

        explso.grfexplso = 4;
        explso.posOrigin = pdart->xf.posWorld;

        pdart->pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pdart->pexpl, &explso);
    }
    else 
        StandardSmokeCloud(&pdart->xf.posWorld, pdart->sRadiusStrikeCloud);

    pdart->pvtlo->pfnSetLoParent(pdart, nullptr);
    pdart->pvtlo->pfnRemoveLo(pdart);
}

void DeleteDart(DART *pdart)
{
	delete pdart;
}

SNIP s_asnipDartLoad = 
{
    2, (OID)0x2BC, offsetof(DART, pasegSticking)
};
