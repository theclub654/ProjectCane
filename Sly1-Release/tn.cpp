#include "tn.h"
#include "jt.h"

TN* NewTn()
{
    return new TN{};
}

TNFN* PtnfnFromTn(TN* ptn)
{
    if (ptn != nullptr)
        return &ptn->tnfn;

    return &g_tnfnDefault;
}

void GetTnfnNose(TNFN* ptnfn, CPDEFI* pcpdefi, glm::vec3* pposNose, TN* ptnAdjust)
{
    *pposNose = glm::mix(pcpdefi->posBase, pcpdefi->posAdjust, ptnfn->uAdjust);

    if (ptnAdjust == nullptr)
        return;

    CLQ aclq[3]{};

    aclq[0].g1 = ptnfn->vecFocus.x;
    aclq[1].g1 = ptnfn->vecFocus.y;
    aclq[2].g1 = ptnfn->vecFocus.z;

    glm::vec3 dposNose(0.0f);
    glm::vec3 vecNose(0.0f);

    const int fCylinderPrev = ptnAdjust->fCylinder;
    ptnAdjust->fCylinder = 0;

    CalculateTnCrv(ptnAdjust, pposNose, &dposNose, &vecNose);
    CalculateTnPos(ptnAdjust, &dposNose, ptnAdjust->sClosest, 0.0f, aclq, g_tnfnDefault.almCam, FTND_Forward, pposNose);

    ptnAdjust->fCylinder = fCylinderPrev;
}

void InitTn(TN* ptn)
{
    InitAlo(ptn);

    ptn->tnfn = g_tnfnDefault;
	ptn->tnsOverride = TNS_Nil;

    ptn->fNoFreeze = 1;

    ptn->cfk = CFK_Nil;
    ptn->fUseVolume = 1;

    ptn->dtPanOnEntry = 2.0f;
    ptn->rswPanOnEntry = 1.0f;

    ptn->dtPanOnIdle = 2.0f;
    ptn->rswPanOnIdle = 1.0f;
}

void* GetTnDsCam(TN* ptn)
{
    return &ptn->tnfn.dsCam;
}

void SetTnDsCam(TN* ptn, float dsCam)
{
    ptn->tnfn.dsCam = dsCam;
}

void* GetTnAclqCam0(TN* ptn)
{
    return &ptn->tnfn.aclqCam[0];
}

void SetTnAclqCam0(TN* ptn, CLQ clqCam)
{
    ptn->tnfn.aclqCam[0] = clqCam;
}

void* GetTnAlmCam0(TN* ptn)
{
    return &ptn->tnfn.almCam[0];
}

void SetTnAlmCam0(TN* ptn, LM lmCam)
{
    ptn->tnfn.almCam[0] = lmCam;
}

void* GetTnAclqCam1(TN* ptn)
{
    return &ptn->tnfn.aclqCam[1];
}

void SetTnAclqCam1(TN* ptn, CLQ clqCam)
{
    ptn->tnfn.aclqCam[1] = clqCam;
}

void* GetTnAlmCam1(TN* ptn)
{
    return &ptn->tnfn.almCam[1];
}

void SetTnAlmCam1(TN* ptn, LM lmCam)
{
    ptn->tnfn.almCam[1] = lmCam;
}

void* GetTnAclqCam2(TN* ptn)
{
    return &ptn->tnfn.aclqCam[2];
}

void SetTnAclqCam2(TN* ptn, CLQ clqCam)
{
    ptn->tnfn.aclqCam[2] = clqCam;
}

void* GetTnAlmCam2(TN* ptn)
{
    return &ptn->tnfn.almCam[2];
}

void SetTnAlmCam2(TN* ptn, LM lmCam)
{
    ptn->tnfn.almCam[2] = lmCam;
}

void* GetTnDradSlack(TN* ptn)
{
    return &ptn->dradSlack;
}

void SetTnDradSlack(TN* ptn, float dradSlack)
{
    ptn->dradSlack = dradSlack;
}

void* GetTnXScreenPref(TN* ptn)
{
    return &ptn->tnfn.xScreenPref;
}

void SetTnXScreenPref(TN* ptn, float xScreenPref)
{
    ptn->tnfn.xScreenPref = xScreenPref;
}

void* GetTnYScreenPref(TN* ptn)
{
    return &ptn->tnfn.yScreenPref;
}

void SetTnYScreenPref(TN* ptn, float yScreenPref)
{
    ptn->tnfn.yScreenPref = yScreenPref;
}

void* GetTnUAdjust(TN* ptn)
{
    return &ptn->tnfn.uAdjust;
}

void SetTnUAdjust(TN* ptn, float uAdjust)
{
    ptn->tnfn.uAdjust = uAdjust;
}

void* GetTnUFollow(TN* ptn)
{
    return &ptn->tnfn.uFollow;
}

void SetTnUFollow(TN* ptn, float uFollow)
{
    ptn->tnfn.uFollow = uFollow;
}

void* GetTnULockOn(TN* ptn)
{
    return &ptn->tnfn.uLockOn;
}

void SetTnULockOn(TN* ptn, float uLockOn)
{
    ptn->tnfn.uLockOn = uLockOn;
}

void* GetTnFocusRatio(TN* ptn)
{
    return &ptn->tnfn.vecFocus.x;
}

void* GetTnVecFocus(TN* ptn)
{
    return &ptn->tnfn.vecFocus;
}

void SetTnVecFocus(TN* ptn, glm::vec3 vecFocus)
{
    ptn->tnfn.vecFocus = vecFocus;
}

void* GetTnFCylinder(TN* ptn)
{
    return &ptn->fCylinder;
}

void SetTnFCylinder(TN* ptn, int fCylinder)
{
    ptn->fCylinder = fCylinder;
}

void* GetTnGrftnd(TN* ptn)
{
    return &ptn->grftnd;
}

void SetTnGrftnd(TN* ptn, int grftnd)
{
    ptn->grftnd = grftnd;
}

void* GetTnFPanOnEntry(TN* ptn)
{
    return &ptn->fPanOnEntry;
}

void SetTnFPanOnEntry(TN* ptn, int fPanOnEntry)
{
    ptn->fPanOnEntry = fPanOnEntry;
}

void* GetTnFPanOnIdle(TN* ptn)
{
    return &ptn->fPanOnIdle;
}

void SetTnFPanOnIdle(TN* ptn, int fPanOnIdle)
{
    ptn->fPanOnIdle = fPanOnIdle;
}

void* GetTnPriCamera(TN* ptn)
{
    return &ptn->priCamera;
}

void SetTnPriCamera(TN* ptn, int priCamera)
{
    ptn->priCamera = priCamera;
}

void* GetTnFSwitchInAir(TN* ptn)
{
    return &ptn->fSwitchInAir;
}

void SetTnFSwitchInAir(TN* ptn, int fSwitchInAir)
{
    ptn->fSwitchInAir = fSwitchInAir;
}

void* GetTnCfk(TN* ptn)
{
    return &ptn->cfk;
}

void SetTnCfk(TN* ptn, CFK cfk)
{
    ptn->cfk = cfk;
}

void* GetTnDtPanOnEntry(TN* ptn)
{
    return &ptn->dtPanOnEntry;
}

void SetTnDtPanOnEntry(TN* ptn, float dtPanOnEntry)
{
    ptn->dtPanOnEntry = dtPanOnEntry;
}

void* GetTnRswPanOnEntry(TN* ptn)
{
    return &ptn->rswPanOnEntry;
}

void SetTnRswPanOnEntry(TN* ptn, float rswPanOnEntry)
{
    ptn->rswPanOnEntry = rswPanOnEntry;
}

void* GetTnDtPanOnIdle(TN* ptn)
{
    return &ptn->dtPanOnIdle;
}

void SetTnDtPanOnIdle(TN* ptn, float dtPanOnIdle)
{
    ptn->dtPanOnIdle = dtPanOnIdle;
}

void* GetTnRswPanOnIdle(TN* ptn)
{
    return &ptn->rswPanOnIdle;
}

void SetTnRswPanOnIdle(TN* ptn, float rswPanOnIdle)
{
    ptn->rswPanOnIdle = rswPanOnIdle;
}

void* GetTnFCutOnActivate(TN* ptn)
{
    return &ptn->fCutOnActivate;
}

void SetTnFCutOnActivate(TN* ptn, int fCutOnActivate)
{
    ptn->fCutOnActivate = fCutOnActivate;
}

void* GetTnRadFOVOverride(TN* ptn)
{
    return &ptn->radFOVOverride;
}

void SetTnRadFOVOverride(TN* ptn, float radFOVOverride)
{
    ptn->radFOVOverride = radFOVOverride;
}

void* GetTnFNoSquish(TN* ptn)
{
    return &ptn->fNoSquish;
}

void SetTnFNoSquish(TN* ptn, int fNoSquish)
{
    ptn->fNoSquish = fNoSquish;
}

void* GetTnPaloTarget(TN* ptn)
{
    return &ptn->paloTarget;
}

void SetTnPaloTarget(TN* ptn, ALO* paloTarget)
{
    ptn->paloTarget = paloTarget;
}

void* GetTnClqFollowTarget(TN* ptn)
{
    return &ptn->clqFollowTarget;
}

void SetTnClqFollowTarget(TN* ptn, CLQ clqFollowTarget)
{
    ptn->clqFollowTarget = clqFollowTarget;
}

void* GetTnLmFollowTarget(TN* ptn)
{
    return &ptn->lmFollowTarget;
}

void SetTnLmFollowTarget(TN* ptn, LM lmFollowTarget)
{
    ptn->lmFollowTarget = lmFollowTarget;
}

void* GetTnTns(TN* ptn)
{
    return &ptn->tns;
}

void* GetTnTnsOverride(TN* ptn)
{
    return &ptn->tnsOverride;
}

void SetTnTnsOverrideUser(TN* ptn, TNS tnsOverride)
{
    ptn->tnsOverride = tnsOverride;
}

void* GetTnSpringStrength(TN* ptn)
{
    return &ptn->tnfn.springStrength;
}

void SetTnSpringStrength(TN* ptn, float springStrength)
{
    ptn->tnfn.springStrength = springStrength;
}

void* GetTnSpringDamping(TN* ptn)
{
    return &ptn->tnfn.springDamping;
}

void SetTnSpringDamping(TN* ptn, float springDamping)
{
    ptn->tnfn.springDamping = springDamping;
}

int GetTnSize()
{
    return sizeof(TN);
}

void OnTnRemove(TN* ptn)
{
    OnAloRemove(ptn);
    SetTnTns(ptn, TNS_Out);
    ClearSwCallbacks(ptn->psw, 2, nullptr, ptn, MSGID_Nil, nullptr);
}

void LoadTnFromBrx(TN *ptn, CBinaryInputStream *pbis)
{
    ptn->xf.mat = pbis->ReadMatrix();
    ptn->xf.pos = pbis->ReadVector();

    ptn->pvtalo->pfnUpdateAloXfWorld(ptn);

    LoadTbspFromBrx(&ptn->ctsurf, ptn->atsurf, &ptn->ctbsp, ptn->atbsp, pbis);

    int8_t crvk = pbis->S8Read();

    if (crvk == CRVK_Nil) 
    {
        ptn->matXfm = pbis->ReadMatrix();
        ptn->posXfm = pbis->ReadVector();

        ptn->pcrv = nullptr;
        ptn->sTotal = 0.0f;
    }
    else 
    {
        ptn->pcrv = PcrvNew((CRVK)crvk);

        if (ptn->pcrv->pvtcrv->pfnLoadCrvFromBrx != nullptr) {
            ptn->pcrv->pvtcrv->pfnLoadCrvFromBrx(ptn->pcrv.get(), pbis);
        }

        // Total curve length / final cumulative distance
        ptn->sTotal = ptn->pcrv->mpicvs[ptn->pcrv->ccv - 1];
    }

    // Bitfield flags:
    // 5 = base tunnel flags
    // 7 = base flags plus curve-present behavior
    ptn->grftnd = (ptn->pcrv != nullptr) ? 7 : 5;

    LoadOptionsFromBrx(ptn, pbis);
}

void CloneTn(TN* ptn, TN* ptnBase)
{
    CloneAlo(ptn, ptnBase);

    ptn->ppo = ptnBase->ppo;
    ptn->ctsurf = ptnBase->ctsurf;
    ptn->atsurf = ptnBase->atsurf;
    ptn->ctbsp = ptnBase->ctbsp;
    ptn->atbsp = ptnBase->atbsp;

    // TBSP nodes contain pointers into the owning TN's vectors. A vector copy
    // preserves the pointer values, so rebase every copied pointer onto this
    // clone's storage.
    for (size_t i = 0; i < ptn->atbsp.size(); ++i)
    {
        const TBSP& tbspBase = ptnBase->atbsp[i];
        TBSP& tbsp = ptn->atbsp[i];

        tbsp.ptsurf = tbspBase.ptsurf == nullptr
            ? nullptr
            : ptn->atsurf.data() + (tbspBase.ptsurf - ptnBase->atsurf.data());
        tbsp.ptbspNeg = tbspBase.ptbspNeg == nullptr
            ? nullptr
            : ptn->atbsp.data() + (tbspBase.ptbspNeg - ptnBase->atbsp.data());
        tbsp.ptbspPos = tbspBase.ptbspPos == nullptr
            ? nullptr
            : ptn->atbsp.data() + (tbspBase.ptbspPos - ptnBase->atbsp.data());
    }

    ptn->pcrv = ptnBase->pcrv;
    ptn->tnfn = ptnBase->tnfn;
    ptn->fCylinder = ptnBase->fCylinder;
    ptn->fSwitchInAir = ptnBase->fSwitchInAir;
    ptn->priCamera = ptnBase->priCamera;
    ptn->fFakeCylinder = ptnBase->fFakeCylinder;
    ptn->dradSlack = ptnBase->dradSlack;
    ptn->tns = ptnBase->tns;
    ptn->tTns = ptnBase->tTns;
    ptn->sClosest = ptnBase->sClosest;
    ptn->matXfm = ptnBase->matXfm;
    ptn->posXfm = ptnBase->posXfm;
    ptn->sTotal = ptnBase->sTotal;
    ptn->posFake = ptnBase->posFake;
    ptn->sRadFake = ptnBase->sRadFake;
    ptn->grftnd = ptnBase->grftnd;
    ptn->fPanOnEntry = ptnBase->fPanOnEntry;
    ptn->fPanOnIdle = ptnBase->fPanOnIdle;
    ptn->dtPanOnEntry = ptnBase->dtPanOnEntry;
    ptn->rswPanOnEntry = ptnBase->rswPanOnEntry;
    ptn->dtPanOnIdle = ptnBase->dtPanOnIdle;
    ptn->rswPanOnIdle = ptnBase->rswPanOnIdle;
    ptn->fUseVolume = ptnBase->fUseVolume;
    ptn->cfk = ptnBase->cfk;
    ptn->fCutOnEntry = ptnBase->fCutOnEntry;
    ptn->radFOV = ptnBase->radFOV;
    ptn->fNoSquish = ptnBase->fNoSquish;
    ptn->tnsOverride = ptnBase->tnsOverride;
    ptn->fCutOnActivate = ptnBase->fCutOnActivate;
    ptn->radFOVOverride = ptnBase->radFOVOverride;
    ptn->radFOVPrev = ptnBase->radFOVPrev;
    ptn->paloTarget = ptnBase->paloTarget;
    ptn->clqFollowTarget = ptnBase->clqFollowTarget;
    ptn->lmFollowTarget = ptnBase->lmFollowTarget;
}

void PostTnLoad(TN* ptn)
{
    PostAloLoad(ptn);
}

void SetTnTns(TN* ptn, TNS tns)
{
    if (ptn->tns == tns)
        return;

    switch (tns)
    {
        case TNS_Out:
        RevokeCmPolicy(g_pcm, 8, CPP_Nil, nullptr, nullptr, ptn);
        HandleLoSpliceEvent(ptn, 3, 0, nullptr);
        break;

        case TNS_In:
        {
            const CPP cpp = static_cast<CPP>(ptn->priCamera + static_cast<int>(CPP_Tunnel_Pri0));
            SetCmPolicy(g_pcm, cpp, &g_pcm->cptn, ptn->ppo, ptn);
            HandleLoSpliceEvent(ptn, 2, 0, nullptr);
            break;
        }

        default:
        break;
    }

    ptn->tns = tns;
    ptn->tTns = g_clock.t;
}

void SetTnTnsOverride(TN* ptn, TNS tnsOverride)
{
    if (ptn->tnsOverride == tnsOverride)
        return;

    ptn->tnsOverride = tnsOverride;
    UpdateTnCallback(ptn, MSGID_callback, nullptr);
}

void UpdateTnCallback(TN* ptn, MSGID msgid, void* pv)
{
    JT* pjtFocus = reinterpret_cast<JT*>(PpoCur());

    if (pjtFocus == nullptr)
        return;

    if (pjtFocus != ptn->ppo)
    {
        SetTnTns(ptn, TNS_Out);
        ptn->ppo = pjtFocus;
    }

    TNS tns = ptn->tnsOverride;

    if (tns == TNS_Nil)
    {
        glm::vec3 posLocal;
        ConvertAloPos(nullptr, ptn, &pjtFocus->xf.posWorld, &posLocal);
        tns = FCheckTbspPoint(ptn->atbsp.data(), &posLocal) != 0 ? TNS_In : TNS_Out;
    }

    const bool fCanSwitch = ptn->fSwitchInAir != 0 || pjtFocus != g_pjt || pjtFocus->jts != JTS_Jump || g_pcm->fCut != 0 || ptn->tnsOverride != TNS_Nil;

    if (fCanSwitch)
        SetTnTns(ptn, tns);
}

void UpdateTn(TN* ptn, float dt)
{
    UpdateAlo(ptn, dt);

    if (ptn->tnsOverride != TNS_Out) 
        PostSwCallback(ptn->psw, (PFNMQ)UpdateTnCallback, ptn, MSGID_callback, nullptr);
}

void RenderTnSelf(TN* ptn, CM* pcm, RO* pro)
{
    RenderAloSelf(ptn, pcm, pro);
}

void FreezeTn(TN* ptn, int fFreeze)
{
    FreezeAlo(ptn, fFreeze);

    if (fFreeze != 0)
        SetTnTns(ptn, TNS_Out);
}

void SetTnFocusRatio(TN* ptn, float u)
{
    ptn->tnfn.vecFocus.x = u;
    ptn->tnfn.vecFocus.z = 1.0;
    ptn->tnfn.vecFocus.y = u;
}

void CalculateTnCrv(TN* ptn, glm::vec3* ppos, glm::vec3* pdposCrv, glm::vec3* pvecClosest)
{
    glm::vec3 posLocal;
    ConvertAloPos(nullptr, ptn, ppos, &posLocal);

    if (ptn->pcrv == nullptr)
    {
        if (ptn->fFakeCylinder == 0)
        {
            *pvecClosest = g_normalX;
            *pdposCrv = glm::transpose(ptn->matXfm) * (posLocal - ptn->posXfm);
            return;
        }

        glm::vec3 posRadial;
        SetVectorCylind(&posRadial, ptn->sClosest / ptn->sRadFake, ptn->sRadFake, 0.0f);

        const glm::vec3 posCrv = ptn->posFake + posRadial;
        *pvecClosest = glm::normalize(glm::cross(g_normalZ, posRadial));

        glm::mat3 matCrv;
        BuildOrthonormalMatrixZ(pvecClosest, &g_normalZ, &matCrv);

        *pdposCrv = glm::transpose(matCrv) * (posLocal - posCrv);
        return;
    }

    glm::vec3 posCrv(0.0f);

    if (ptn->pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
        ptn->pcrv->pvtcrv->pfnEvaluateCrvFromS(ptn->pcrv.get(), ptn->sClosest, &posCrv, pvecClosest);

    glm::mat3 matCrv;
    BuildOrthonormalMatrixZ(pvecClosest, &g_normalZ, &matCrv);

    *pdposCrv = glm::transpose(matCrv) * (posLocal - posCrv);
}

void CalculateTnPos(TN* ptn, glm::vec3* pdposCrv, float sBase, float dsOffset, CLQ* aclq, LM* alm, FTND ftnd, glm::vec3* pposOther)
{
    glm::vec3 dposOtherCrv(0.0f);

    for (int i = 0; i < 3; ++i)
    {
        const float component = (*pdposCrv)[i];
        const float value = aclq[i].g0 + component * (aclq[i].g1 + component * aclq[i].g2);
        dposOtherCrv[i] = glm::clamp(value, alm[i].gMin, alm[i].gMax);
    }

    if (ftnd == FTND_Reverse)
        dposOtherCrv.x = -dposOtherCrv.x;

    if (ptn->fCylinder != 0)
        SetVectorCylind(&dposOtherCrv, dposOtherCrv.x, dposOtherCrv.y, dposOtherCrv.z);

    if (ptn->pcrv == nullptr && ptn->fFakeCylinder == 0)
    {
        dposOtherCrv.x += dsOffset;

        glm::vec3 posOtherLocal = ptn->posXfm + ptn->matXfm * dposOtherCrv;
        ConvertAloPos(ptn, nullptr, &posOtherLocal, pposOther);
        return;
    }

    glm::vec3 posOtherLocal(0.0f);
    glm::vec3 vecTangentOther(0.0f);

    const float sOther = ftnd == FTND_Forward ? sBase + dsOffset : sBase - dsOffset;

    if (ptn->pcrv == nullptr)
    {
        const float sCircumference = ptn->sRadFake * glm::two_pi<float>();
        const float sWrapped = std::fmod(sOther, sCircumference);

        glm::vec3 posRadial;
        SetVectorCylind(&posRadial, sWrapped / ptn->sRadFake, ptn->sRadFake, 0.0f);

        posOtherLocal = ptn->posFake + posRadial;
        vecTangentOther = glm::normalize(glm::cross(g_normalZ, posRadial));
    }
    else if (ptn->pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
        ptn->pcrv->pvtcrv->pfnEvaluateCrvFromS(ptn->pcrv.get(), sOther, &posOtherLocal, &vecTangentOther);

    glm::mat3 matOther;
    BuildOrthonormalMatrixZ(&vecTangentOther, &g_normalZ, &matOther);

    posOtherLocal += matOther * dposOtherCrv;

    ConvertAloPos(ptn, nullptr, &posOtherLocal, pposOther);
}

void LoadTbspFromBrx(int* pctsurf, std::vector <TSURF>& atsurf, int* pctbsp, std::vector <TBSP>& atbsp, CBinaryInputStream* pbis)
{
    *pctsurf = pbis->U16Read();
    atsurf.resize(*pctsurf);

    for (int i = 0; i < *pctsurf; i++)
    {
        TSURF& tsurf = atsurf[i];

        tsurf.gDot = pbis->F32Read();
        tsurf.normal = pbis->ReadVector();
    }

    *pctbsp = pbis->U16Read();
    atbsp.resize(*pctbsp);

    for (int i = 0; i < *pctbsp; i++)
    {
        TBSP& tbsp = atbsp[i];

        u16 surfIndex = pbis->U16Read();
        tbsp.ptsurf = atsurf.data() + surfIndex;

        s16 negIndex = pbis->S16Read();
        tbsp.ptbspNeg = negIndex == -1 ? nullptr : atbsp.data() + negIndex;

        s16 posIndex = pbis->S16Read();
        tbsp.ptbspPos = posIndex == -1 ? nullptr : atbsp.data() + posIndex;
    }
}

int FCheckTbspPoint(TBSP* atbsp, glm::vec3* pposLocal)
{
    if (atbsp == nullptr)
        return 0;

    while (atbsp != nullptr)
    {
        float dot = glm::dot(atbsp->ptsurf->normal, *pposLocal);

        if (dot < atbsp->ptsurf->gDot)
        {
            atbsp = atbsp->ptbspNeg;

            if (atbsp == nullptr)
                return 1;
        }
        else
            atbsp = atbsp->ptbspPos;
    }

    return 0;
}

void DeleteTn(TN *ptn)
{
    delete ptn;
}

TNFN g_tnfnDefault =
{
    -450.0f, // dsCam

    // aclqCam
    {
        { 0.0f,   1.0f, 0.0f, 0.0f },
        { 0.0f,   1.0f, 0.0f, 0.0f },
        { 125.0f, 1.0f, 0.0f, 0.0f }
    },

    // almCam
    {
        { -FLT_MAX, FLT_MAX },
        { -FLT_MAX, FLT_MAX },
        { -FLT_MAX, FLT_MAX }
    },

    0.0f,            // xScreenPref
    -0.1f,           // yScreenPref
    1.0f,            // uAdjust
    1.0f,            // uFollow
    1.0f,            // uLockOn
    glm::vec3(1.0f), // vecFocus
    1.0f,            // springStrength
    1.0f             // springDamping
};
