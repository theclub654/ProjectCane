#include "button.h"
#include "chkpnt.h"
#include "jt.h"
#include "pnt.h"
#include "sm.h"
#include "dialog.h"
#include "spliceobj.h"

BUTTON* NewButton()
{
	return new BUTTON{};
}

void InitButton(BUTTON* pbutton)
{
	InitSo(pbutton);
    InitBtn(&pbutton->btn);
}

void* GetButtonDtRepush(BUTTON* pbutton)
{
    return &pbutton->btn.dtRepush;
}

void SetButtonDtRepush(BUTTON* pbutton, float dtRepush)
{
    pbutton->btn.dtRepush = dtRepush;
}

void* GetButtonBtnt(BUTTON* pbutton)
{
    return &pbutton->btnt;
}

void SetButtonBtnt(BUTTON* pbutton, BTNT btnt)
{
    pbutton->btnt = btnt;
}

void* GetButtonSvtAnimation(BUTTON* pbutton)
{
    return &pbutton->btn.svtAnimation;
}

void SetButtonSvtAnimation(BUTTON* pbutton, float svtAnimation)
{
    pbutton->btn.svtAnimation = svtAnimation;
}

void* GetButtonFSilent(BUTTON* pbutton)
{
    return &pbutton->btn.fSilent;
}

void SetButtonFSilent(BUTTON* pbutton, int fSilent)
{
    pbutton->btn.fSilent = fSilent;
}

void* GetButtonFReapplyAseg(BUTTON* pbutton)
{
    return &pbutton->btn.fReapplyAseg;
}

void SetButtonFReapplyAseg(BUTTON* pbutton, int fReapplyAseg)
{
    pbutton->btn.fReapplyAseg = fReapplyAseg;
}

void* GetButtonFManualReset(BUTTON* pbutton)
{
    return &pbutton->btn.fManualReset;
}

void SetButtonFManualReset(BUTTON* pbutton, int fManualReset)
{
    pbutton->btn.fManualReset = fManualReset;
}

void* GetButtonFCheckpointed(BUTTON* pbutton)
{
    return &pbutton->btn.fCheckpointed;
}

void SetButtonFCheckpointed(BUTTON* pbutton, int fCheckpointed)
{
    pbutton->btn.fCheckpointed = fCheckpointed;
}

void* GetButtonOidDialog(BUTTON* pbutton)
{
    return &pbutton->btn.oidDialog;
}

void SetButtonOidDialog(BUTTON* pbutton, OID oidDialog)
{
    pbutton->btn.oidDialog = oidDialog;
}

void* GetButtonButtons(BUTTON* pbutton)
{
    return &pbutton->btn.buttons;
}

int GetButtonSize()
{
	return sizeof(BUTTON);
}

void LoadButtonFromBrx(BUTTON* pbutton, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbutton, pbis);
    LoadBtn(&pbutton->btn, pbutton);
}

void InsertButtonPos(BUTTON* pbutton, PNT* ppnt, glm::vec3* ppos)
{
    int iposStop = pbutton->cposStop;

    if (pbutton->cposStop > 0 && FMatchesLoName((LO*)ppnt, (OID)512))
    {
        for (int i = pbutton->cposStop; i > 0; --i)
            pbutton->aposStop[i] = pbutton->aposStop[i - 1];

        iposStop = 0;
    }

    pbutton->aposStop[iposStop] = *ppos;
    ++pbutton->cposStop;
}

void InsertButtonMat(BUTTON* pbutton, XFM* pxfm, glm::mat3* pmat)
{
    int imatStop = pbutton->cmatStop;

    if (pbutton->cmatStop > 0 && FMatchesLoName((LO*)pxfm, (OID)512))
    {
        for (int i = pbutton->cmatStop; i > 0; --i)
            pbutton->amatStop[i] = pbutton->amatStop[i - 1];

        imatStop = 0;
    }

    pbutton->amatStop[imatStop] = *pmat;
    ++pbutton->cmatStop;
}

void CloneButton(BUTTON* pbutton, BUTTON* pbuttonBase)
{
    int ichk = pbutton->btn.ichkPushed;
    CloneSo(pbutton, pbuttonBase);

    pbutton->btnt = pbuttonBase->btnt;
    pbutton->btn = pbuttonBase->btn;
    pbutton->coidPush = pbuttonBase->coidPush;
    for (int i = 0; i < 8; ++i)
        pbutton->aoidPush[i] = pbuttonBase->aoidPush[i];

    pbutton->coidNoPush = pbuttonBase->coidNoPush;
    for (int i = 0; i < 8; ++i)
        pbutton->aoidNoPush[i] = pbuttonBase->aoidNoPush[i];

    pbutton->ccidPush = pbuttonBase->ccidPush;
    for (int i = 0; i < 8; ++i)
        pbutton->acidPush[i] = pbuttonBase->acidPush[i];

    pbutton->ccidNoPush = pbuttonBase->ccidNoPush;
    for (int i = 0; i < 8; ++i)
        pbutton->acidNoPush[i] = pbuttonBase->acidNoPush[i];

    pbutton->istopClosestCur = pbuttonBase->istopClosestCur;
    pbutton->cposStop = pbuttonBase->cposStop;
    for (int i = 0; i < 2; ++i)
        pbutton->aposStop[i] = pbuttonBase->aposStop[i];

    pbutton->cmatStop = pbuttonBase->cmatStop;
    for (int i = 0; i < 2; ++i)
        pbutton->amatStop[i] = pbuttonBase->amatStop[i];

    pbutton->posLever = pbuttonBase->posLever;
    for (int i = 0; i < 2; ++i)
        pbutton->anormLimits[i] = pbuttonBase->anormLimits[i];

    pbutton->phndSwitchHook = pbuttonBase->phndSwitchHook;

    pbutton->btn.ichkPushed = ichk;
    pbutton->btn.paloOwner = pbutton;
}

void PostButtonLoad(BUTTON* pbutton)
{
    DLI dli{};

    dli.m_pdl = &pbutton->dlChild;
    dli.m_ibDle = pbutton->dlChild.ibDle;
    dli.m_pdliNext = s_pdliFirst;
    dli.m_ppv = (void**)dli.m_pdl;
    s_pdliFirst = &dli;

    PostAloLoad(pbutton);
    PostBtnLoad(&pbutton->btn);
    SetButtonButtons(pbutton, BUTTONS_NoContact);

    BTNT btnt = pbutton->btnt;

    while (*dli.m_ppv != nullptr)
    {
        XFM* pxfm = (XFM*)*dli.m_ppv;
        dli.m_ppv = (void**)((uint8_t*)pxfm + dli.m_ibDle);

        if (btnt == BTNT_Push && FIsBasicDerivedFrom((BASIC*)pxfm, CID_PNT))
        {
            glm::vec3 posStop;
            ConvertAloPos(pbutton, pbutton->paloParent, &pxfm->posLocal, &posStop);
            InsertButtonPos(pbutton, (PNT*)pxfm, &posStop);
        }
        else if (btnt == BTNT_Lever && FIsBasicDerivedFrom(pxfm, CID_XFM) && !FIsBasicDerivedFrom(pxfm, CID_TARGET))
        {
            glm::mat3 matStop;
            ConvertAloMat(pbutton, pbutton->paloParent, &pxfm->matLocal, &matStop);
            InsertButtonMat(pbutton, pxfm, &matStop);
            pxfm->pvtlo->pfnRemoveLo(pxfm);
        }

        if (FIsBasicDerivedFrom(pxfm, CID_HND))
            pbutton->phndSwitchHook = (HND*)pxfm;
    }

    if (btnt == BTNT_Lever)
    {
        glm::vec3 normalTorqueLocal;
        ConvertAloVec(pbutton->paloParent, pbutton, &pbutton->constrTorque.normal, &normalTorqueLocal);

        int iLimitAxis;
        int iLeverAxis;
        glm::vec3 normalLeverLocal;

        if (normalTorqueLocal.x > 0.9f)
        {
            iLimitAxis = 2;
            iLeverAxis = 1;
            normalLeverLocal = g_normalY;
        }
        else if (normalTorqueLocal.y > 0.9f)
        {
            iLimitAxis = 0;
            iLeverAxis = 2;
            normalLeverLocal = g_normalZ;
        }
        else
        {
            iLimitAxis = 1;
            iLeverAxis = 0;
            normalLeverLocal = g_normalX;
        }

        int imatLast = pbutton->cmatStop - 1;

        pbutton->anormLimits[0] = pbutton->amatStop[0][iLimitAxis];
        pbutton->anormLimits[1] = pbutton->amatStop[imatLast][iLimitAxis];
        pbutton->posLever = normalLeverLocal * 100.0f;

        glm::vec3 posBase = pbutton->paloParent == nullptr ? pbutton->xf.posWorld : pbutton->xf.pos;
        glm::vec3 posLeverFirst = posBase + pbutton->amatStop[0][iLeverAxis] * 100.0f;
        glm::vec3 posLeverLast  = posBase + pbutton->amatStop[imatLast][iLeverAxis] * 100.0f;

        if (glm::dot(pbutton->anormLimits[1], posLeverLast - posLeverFirst) > 0.0f)
            pbutton->anormLimits[1] = -pbutton->anormLimits[1];
        else
            pbutton->anormLimits[0] = -pbutton->anormLimits[0];
    }
    else if (btnt == BTNT_Push)
    {
        for (int iposStop = 0; iposStop < pbutton->cposStop; ++iposStop)
        {
            glm::vec3 dpos = pbutton->aposStop[iposStop] - pbutton->xf.pos;
            ApplySoConstraintLocal(pbutton, &pbutton->constrForce, &dpos, &dpos, nullptr);
            pbutton->aposStop[iposStop] = pbutton->xf.pos + dpos;
        }

        if (pbutton->cposStop > 1)
        {
            glm::vec3 normalLimit = glm::normalize(pbutton->aposStop[pbutton->cposStop - 1] - pbutton->aposStop[0]);
            pbutton->anormLimits[0] = normalLimit;
            pbutton->anormLimits[1] = -normalLimit;
        }
    }

    s_pdliFirst = dli.m_pdliNext;
}

void SetButtonButtons(BUTTON* pbutton, BUTTONS buttons)
{
    SetBtnButtons(&pbutton->btn, buttons);
}

int FCheckButtonObject(BUTTON* pbutton, SO* psoOther)
{
    if (pbutton->coidPush > 0)
    {
        bool fMatchesPushOid = false;

        for (int ioid = 0; ioid < pbutton->coidPush; ++ioid)
        {
            if (FMatchesLoName(psoOther, pbutton->aoidPush[ioid]))
            {
                fMatchesPushOid = true;
                break;
            }
        }

        if (!fMatchesPushOid)
            return 0;
    }
    else
    {
        for (int ioid = 0; ioid < pbutton->coidNoPush; ++ioid)
        {
            if (FMatchesLoName(psoOther, pbutton->aoidNoPush[ioid]))
                return 0;
        }
    }

    if (pbutton->ccidPush > 0)
    {
        for (int icid = 0; icid < pbutton->ccidPush; ++icid)
        {
            if (FIsBasicDerivedFrom(psoOther, pbutton->acidPush[icid]))
                return 1;
        }

        return 0;
    }

    for (int icid = 0; icid < pbutton->ccidNoPush; ++icid)
    {
        if (FIsBasicDerivedFrom(psoOther, pbutton->acidNoPush[icid]))
            return 0;
    }

    return 1;
}

int IposFindButtonClosest(BUTTON* pbutton, float* psDist)
{
    if (pbutton->cposStop == 0)
        return 0;

    float sDistClosest = std::numeric_limits<float>::max();
    int iposClosest = 0;

    for (int ipos = pbutton->cposStop - 1; ipos >= 0; --ipos)
    {
        float sDist = glm::length(pbutton->aposStop[ipos] - pbutton->xf.pos);

        if (sDist < sDistClosest)
        {
            sDistClosest = sDist;
            iposClosest = ipos;
        }
    }

    if (psDist != nullptr)
        *psDist = sDistClosest;

    return iposClosest;
}

int ImatFindButtonClosest(BUTTON* pbutton, float* prad)
{
    if (pbutton->cmatStop == 0)
        return 0;

    float cosClosest = 0.0f;
    int imatClosest = 0;

    for (int imat = pbutton->cmatStop - 1; imat >= 0; --imat)
    {
        glm::mat3 dmat;
        CalculateDmat(&pbutton->xf.mat, &pbutton->amatStop[imat], &dmat);

        float cosRotation = CosRotateMatrixMagnitude(&dmat);

        if (cosRotation > cosClosest)
        {
            cosClosest = cosRotation;
            imatClosest = imat;
        }
    }

    if (prad != nullptr)
        *prad = acosf(cosClosest);

    return imatClosest;
}

void PresetButtonAccel(BUTTON* pbutton, float dt)
{
    PresetSoAccel(pbutton, dt);

    int istopTarget = pbutton->btn.buttons == BUTTONS_Reset ? 0 : pbutton->istopClosestCur;

    if (pbutton->btnt == BTNT_Push && pbutton->cposStop > 0)
    {
        glm::vec3* pposTarget = &pbutton->aposStop[istopTarget];
        AccelSoTowardPosSpring(pbutton, pposTarget, pbutton->pclqPosSpring.get(), &g_vecZero, pbutton->pclqPosDamping.get(), dt);
    }
    else if (pbutton->btnt == BTNT_Lever && pbutton->cmatStop > 0)
    {
        glm::mat3* pmatTarget = &pbutton->amatStop[istopTarget];
        AccelSoTowardMatSpring(pbutton, pmatTarget, pbutton->pclqRotSpring.get(), &g_vecZero, pbutton->pclqRotDamping.get(), dt);
    }

}

void UpdateButtonInternalXps(BUTTON* pbutton)
{
    if (pbutton->cposStop == 0 && pbutton->cmatStop == 0) {
        return;
    }

    if (pbutton->pxpInternal != nullptr) {
        FreeSwXpList(pbutton->psw, pbutton->pxpInternal);
        pbutton->pxpInternal = nullptr;
    }

    glm::vec3 aposStopWorld[2] = {};
    glm::vec3 anormLimitsWorld[2] = {};

    for (int i = 0; i < 2; i++) {
        if (i < pbutton->cposStop) {
            ConvertAloPos(pbutton->paloParent, nullptr, &pbutton->aposStop[i], &aposStopWorld[i]);
        }

        ConvertAloVec(pbutton->paloParent, nullptr, &pbutton->anormLimits[i], &anormLimitsWorld[i]);
    }

    glm::vec3* ppos;
    float gButton0;
    float gLimit0;
    float gButton1;
    float gLimit1;

    glm::vec3 posLeverWorld;

    if (pbutton->btnt == BTNT_Push) {
        ppos = &pbutton->xf.posWorld;

        gButton0 = glm::dot(pbutton->xf.posWorld, anormLimitsWorld[0]);
        gLimit0 = glm::dot(aposStopWorld[0], anormLimitsWorld[0]);
        gButton1 = -gButton0;
        gLimit1 = glm::dot(aposStopWorld[1], anormLimitsWorld[1]);
    }
    else {
        ConvertAloPos(pbutton, nullptr, &pbutton->posLever, &posLeverWorld);
        ppos = &posLeverWorld;

        gButton0 = glm::dot(posLeverWorld, anormLimitsWorld[0]);
        gLimit0 = glm::dot(pbutton->xf.posWorld, anormLimitsWorld[0]);
        gButton1 = glm::dot(pbutton->xf.posWorld, anormLimitsWorld[1]);
        gLimit1 = glm::dot(posLeverWorld, anormLimitsWorld[1]);
    }

    float gDist = gButton0 - gLimit0;
    glm::vec3* pnormal = &anormLimitsWorld[0];

    if (gDist >= -0.0001f) {
        gDist = pbutton->btnt == BTNT_Push
            ? gButton1 - gLimit1
            : gLimit1 - gButton1;

        if (gDist >= -0.0001f) {
            return;
        }

        pnormal = &anormLimitsWorld[1];
    }

    XP* pxp = PxpAdd(pbutton, nullptr, ppos, pnormal, gDist, XPK_Internal, nullptr, nullptr, &pbutton->pxpInternal);
    pxp->elas = 0.0f;
}

void UpdateButton(BUTTON* pbutton, float dt)
{
    UpdateSo(pbutton, dt);

    BUTTONS buttons = pbutton->btn.buttons;

    switch (buttons)
    {
        case BUTTONS_NoContact:
        case BUTTONS_Contact:
        {
            switch (pbutton->btnt)
            {
                case BTNT_Contact:
                {
                    buttons = BUTTONS_NoContact;

                    SO* psoRoot = (SO*)pbutton->paloRoot;
                    OXA* poxa = psoRoot->poxa;

                    for (OX* pox = poxa != nullptr ? poxa->pox : nullptr; pox != nullptr && buttons == BUTTONS_NoContact; pox = pox->poxNext)
                    {
                        for (XP* pxp = pox->pxp; pxp != nullptr; pxp = pxp->pxpNext)
                        {
                            int ixpdButton = pxp->axpd[0].psoRoot != (SO*)pbutton->paloRoot;

                            if (pxp->axpd[ixpdButton].psoLeaf != (SO*)pbutton)
                                continue;

                            SO* psoOther = pxp->axpd[1 - ixpdButton].psoRoot;

                            if (FCheckButtonObject(pbutton, psoOther))
                            {
                                buttons = BUTTONS_Contact;
                                break;
                            }
                        }
                    }

                    break;
                }

                case BTNT_Hang:
                buttons = BUTTONS_NoContact;

                if (g_pjt != nullptr && g_pjt->jts == JTS_Hang && g_pjt->phndCur == pbutton->phndSwitchHook)
                    buttons = BUTTONS_Contact;

                break;

                case BTNT_Push:
                case BTNT_Lever:
                {
                    float gClosest = std::numeric_limits<float>::max();
                    float gLimit;
                    int istopClosest;

                    if (pbutton->btnt == BTNT_Push)
                    {
                        istopClosest = IposFindButtonClosest(pbutton, &gClosest);
                        gLimit = 2.0f;
                    }
                    else
                    {
                        istopClosest = ImatFindButtonClosest(pbutton, &gClosest);
                        gLimit = 0.1f;
                    }

                    if (gClosest < gLimit)
                    {
                        if (istopClosest == 0)
                            buttons = BUTTONS_NoContact;
                        else if (istopClosest == 1)
                            buttons = BUTTONS_Contact;
                    }

                    if (pbutton->istopClosestCur != istopClosest)
                    {
                        ResolveAlo(pbutton);
                        pbutton->istopClosestCur = istopClosest;
                    }

                    break;
                }

                case BTNT_Whack:
                break;
            }

            SetButtonButtons(pbutton, buttons);
            return;
        }

        case BUTTONS_Pushed:
        if (g_pjt != nullptr && g_pjt->jts == JTS_Hang && g_pjt->phndCur == pbutton->phndSwitchHook)
            pbutton->btn.tButtons = g_clock.t;

        if (pbutton->btn.dtRepush >= 0.0f && pbutton->btn.dtRepush < g_clock.t - pbutton->btn.tButtons)
            SetButtonButtons(pbutton, BUTTONS_Reset);

        return;

        case BUTTONS_Reset:
        switch (pbutton->btnt)
        {
            case BTNT_Push:
            if (IposFindButtonClosest(pbutton, nullptr) == 0)
                SetButtonButtons(pbutton, BUTTONS_Contact);
            break;

            case BTNT_Lever:
            if (ImatFindButtonClosest(pbutton, nullptr) == 0)
                SetButtonButtons(pbutton, BUTTONS_Contact);
            break;

            case BTNT_Contact:
            case BTNT_Hang:
            SetButtonButtons(pbutton, BUTTONS_Contact);
            break;

            case BTNT_Whack:
            SetButtonButtons(pbutton, BUTTONS_NoContact);
            break;
        }

        return;

        default:
        return;
    }
}

int FAbsorbButtonWkr(BUTTON* pbutton, WKR* pwkr)
{
    int fAbsorbed = FAbsorbSoWkr(pbutton, pwkr);

    if (fAbsorbed && (pwkr->grfic & 4U) == 0 && pbutton->btnt == BTNT_Whack && pbutton->btn.buttons == BUTTONS_NoContact)
        SetBtnButtons(&pbutton->btn, BUTTONS_Contact);

    return fAbsorbed;
}

void DeleteButton(BUTTON *pbutton)
{
	delete pbutton;
}

void PostAshLoad(SW* psw, ASH* pash, ALO* paloOwner)
{
    for (int ioid = 0; ioid < pash->coid; ++ioid)
    {
        ASEG* paseg = (ASEG*)PloFindSwObject(psw, 260, pash->aoid[ioid], (LO*)paloOwner);

        if (paseg != nullptr)
        {
            FAddAshAseg(pash, paseg);
            SnipLo(paseg);
        }
    }
}

int FFoundAshAseg(ASH* pash, ASEG* paseg)
{
    for (int ioid = pash->coid - 1; ioid >= 0; --ioid)
    {
        if (FMatchesLoName(paseg, pash->aoid[ioid]))
            return 1;
    }

    return 0;
}

int FAddAshAseg(ASH* pash, ASEG* paseg)
{
    pash->apaseg[pash->cpaseg++] = paseg;
    return pash->cpaseg < 16;
}

int FAddAshOid(ASH* pash, OID oid)
{
    pash->aoid[pash->coid++] = oid;
    return pash->coid < 16;
}

void InitBtn(BTN* pbtn)
{
    pbtn->ichkPushed = IchkAllocChkmgr(&g_chkmgr);
    pbtn->oidDialog = OID_Nil;
}

void LoadBtn(BTN* pbtn, ALO* paloOwner)
{
    DLI dli{};

    dli.m_pdl = &paloOwner->dlChild;
    dli.m_ibDle = paloOwner->dlChild.ibDle;
    dli.m_pdliNext = s_pdliFirst;
    s_pdliFirst = &dli;

    pbtn->paloOwner = paloOwner;

    LO* plo = paloOwner->dlChild.ploFirst;

    while (plo != nullptr)
    {
        dli.m_ppv = (void**)((uint8_t*)plo + dli.m_ibDle);

        if ((plo->pvtlo->grfcid & 0x40U) != 0)
        {
            ASEG* paseg = (ASEG*)plo;

            if (!FFoundAshAseg(&pbtn->aash[1], paseg) && !FFoundAshAseg(&pbtn->aash[0], paseg))
                FAddAshAseg(&pbtn->aash[0], paseg);
        }

        plo = (LO*)*dli.m_ppv;
    }

    s_pdliFirst = dli.m_pdliNext;
}

void PostBtnLoad(BTN* pbtn)
{
    pbtn->buttons = BUTTONS_Nil;

    ALO* paloOwner = pbtn->paloOwner;
    SW* psw = paloOwner->psw;

    for (int iash = 0; iash < 2; ++iash)
        PostAshLoad(psw, &pbtn->aash[iash], paloOwner);

    if (pbtn->fCheckpointed && FGetChkmgrIchk(&g_chkmgr, pbtn->ichkPushed))
        PostSwCallback(psw, (PFNMQ)RestoreBtnFromCheckpointCallback, pbtn, MSGID_callback, nullptr);
}

void RestoreBtnFromCheckpointCallback(BTN* pbtn, MSGID msgid, void* pv)
{
    if (msgid == MSGID_callback)
        PostSwCallback(pbtn->paloOwner->psw, (PFNMQ)RestoreBtnFromCheckpointCallback, pbtn, MSGID_button_trigger, nullptr);
    else 
        TriggerBtn(pbtn, 1, 1);
}

void SetBtnRsmg(BTN* pbtn, int fOnTrigger, OID oidRoot, OID oidSM, OID oidGoal)
{
    FAddRsmg(pbtn->arsmg, 8, &pbtn->crsmg, fOnTrigger, oidRoot, oidSM, oidGoal);
}

void SetBtnButtons(BTN* pbtn, BUTTONS buttons)
{
    BUTTONS buttonsPrev = pbtn->buttons;

    if (buttons == buttonsPrev)
        return;

    pbtn->buttons = buttons;
    pbtn->tButtons = g_clock.t;

    ResolveAlo(pbtn->paloOwner);

    if (buttonsPrev == BUTTONS_NoContact && buttons == BUTTONS_Contact)
        TriggerBtn(pbtn, 0, 0);

    if (buttonsPrev != BUTTONS_Nil && buttons == BUTTONS_NoContact)
        UntriggerBtn(pbtn, 0);
}

int FAddRsmg(RSMG* arsmg, int crsmgMax, int* pcrsmg, int fOnTrigger, OID oidRoot, OID oidSM, OID oidGoal)
{
    for (int irsmg = 0; irsmg < *pcrsmg; ++irsmg)
    {
        RSMG* prsmg = &arsmg[irsmg];

        if (prsmg->oidRoot == oidRoot && (prsmg->oidSM == OID_Nil || prsmg->oidSM == oidSM))
        {
            if (fOnTrigger)
                prsmg->oidTriggerGoal = oidGoal;
            else
                prsmg->oidUntriggerGoal = oidGoal;

            return 1;
        }
    }

    int irsmg = *pcrsmg;

    // This preserves the original's count + 1 < maximum check.
    if (irsmg + 1 >= crsmgMax)
        return 0;

    RSMG* prsmg = &arsmg[irsmg];

    ++*pcrsmg;

    prsmg->oidRoot = oidRoot;
    prsmg->oidSM = oidSM;

    if (fOnTrigger)
    {
        prsmg->oidTriggerGoal = oidGoal;
        prsmg->oidUntriggerGoal = OID_Nil;
    }
    else
    {
        prsmg->oidTriggerGoal = OID_Nil;
        prsmg->oidUntriggerGoal = oidGoal;
    }

    return 1;
}

void TriggerRsmg(SW* psw, int crsmg, RSMG* arsmg, LO* ploContext, int fTrigger)
{
    for (int irsmg = 0; irsmg < crsmg; ++irsmg)
    {
        RSMG* prsmg = &arsmg[irsmg];
        LO* ploRoot = nullptr;

        if (CploFindSwObjects(psw, 772, prsmg->oidRoot, ploContext, 1, &ploRoot) != 1)
            return;

        SMA* psma = PsmaFindAlo((ALO*)ploRoot, prsmg->oidSM);

        if (psma == nullptr)
            continue;

        OID oidGoal = fTrigger ? prsmg->oidTriggerGoal : prsmg->oidUntriggerGoal;

        if (oidGoal != OID_Nil)
            SetSmaGoal(psma, oidGoal);
    }
}

void RunBtnAsegs(BTN* pbtn, IASH asht, int fSeekToEnd, int fChkTrigger)
{
    ASH* pash = &pbtn->aash[asht];

    for (int iaseg = 0; iaseg < pash->cpaseg; ++iaseg)
    {
        ASEG* paseg = pash->apaseg[iaseg];
        ALO* paloRoot = nullptr;

        if (FMatchesLoName(pbtn->paloOwner, (OID)paseg->oidRoot))
            paloRoot = pbtn->paloOwner;
        else if (paseg->oidRoot != OID_Nil)
        {
            paloRoot = (ALO*)PloFindSwNearest(pbtn->paloOwner->psw, (OID)paseg->oidRoot, pbtn->paloOwner);
        }

        if (fSeekToEnd && paseg->segrpt == SEGRPT_Once)
            PasegaEnsureAseg(paseg, paloRoot, SEEK_End, 0.0f, 0.0f, 0);
        else if (pbtn->fReapplyAseg)
        {
            float t = pbtn->svtAnimation < 0.0f ? paseg->tMax : 0.0f;
            PasegaEnsureAseg(paseg, paloRoot, SEEK_Start, t, pbtn->svtAnimation, 0);
        }
        else
            PasegaEnsureAseg(paseg, paloRoot, SEEK_Current, 0.0f, pbtn->svtAnimation, 0);
    }

    TriggerRsmg(pbtn->paloOwner->psw, pbtn->crsmg, pbtn->arsmg, (LO*)pbtn->paloOwner, asht == IASH_On);

    if (pbtn->oidDialog != OID_Nil && !fChkTrigger)
    {
        DIALOG* pdialog = (DIALOG*)PloFindSwNearest(pbtn->paloOwner->psw, (OID)pbtn->oidDialog, (LO*)pbtn->paloOwner);

        if (asht == IASH_On)
            TriggerDialog(pdialog);
        else if (asht == IASH_Off)
            UntriggerDialog(pdialog);
    }
}

void TriggerBtn(BTN* pbtn, int fSeekToEnd, int fChkTrigger)
{
    if (fChkTrigger)
        HandleLoSpliceEvent(pbtn->paloOwner, 22, 0, nullptr);
    else
        HandleLoSpliceEvent(pbtn->paloOwner, 2, 0, nullptr);

    if (!pbtn->fSilent && !fChkTrigger)
        StartSound((SFXID)2, nullptr, pbtn->paloOwner, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

    RunBtnAsegs(pbtn, IASH_On, fSeekToEnd, fChkTrigger);

    if (!pbtn->fManualReset)
        SetBtnButtons(pbtn, BUTTONS_Pushed);

    if (pbtn->fCheckpointed)
        SetChkmgrIchk(&g_chkmgr, pbtn->ichkPushed);
    
    pbtn->paloOwner->pvtlo->pfnSendLoMessage(pbtn->paloOwner, MSGID_button_trigger, pbtn);
}

void UntriggerBtn(BTN* pbtn, int fSeekToEnd)
{
    HandleLoSpliceEvent(pbtn->paloOwner, 3, 0, nullptr);
    RunBtnAsegs(pbtn, IASH_Off, fSeekToEnd, 0);
    pbtn->paloOwner->pvtlo->pfnSendLoMessage(pbtn->paloOwner, MSGID_button_untrigger, pbtn);
}

VOLBTN* NewVolbtn()
{
	return new VOLBTN{};
}

void InitVolbtn(VOLBTN* pvolbtn)
{
    InitSo(pvolbtn);
    InitBtn(&pvolbtn->btn);

    pvolbtn->fNoXpsSelf = 1;
    SetSoConstraints(pvolbtn, CT_Locked, nullptr, CT_Locked, nullptr);
}

void* GetVolbtnDtRepush(VOLBTN* pvolbtn)
{
    return &pvolbtn->btn.dtRepush;
}

void SetVolbtnDtRepush(VOLBTN* pvolbtn, float dtRepush)
{
    pvolbtn->btn.dtRepush = dtRepush;
}

void* GetVolbtnSvtAnimation(VOLBTN* pvolbtn)
{
    return &pvolbtn->btn.svtAnimation;
}

void SetVolbtnSvtAnimation(VOLBTN* pvolbtn, float svtAnimation)
{
    pvolbtn->btn.svtAnimation = svtAnimation;
}

void* GetVolbtnFSilent(VOLBTN* pvolbtn)
{
    return &pvolbtn->btn.fSilent;
}

void SetVolbtnFSilent(VOLBTN* pvolbtn, int fSilent)
{
    pvolbtn->btn.fSilent = fSilent;
}

void* GetVolbtnFReapplyAseg(VOLBTN* pvolbtn)
{
    return &pvolbtn->btn.fReapplyAseg;
}

void SetVolbtnFReapplyAseg(VOLBTN* pvolbtn, int fReapplyAseg)
{
    pvolbtn->btn.fReapplyAseg = fReapplyAseg;
}

void* GetVolbtnFCheckpointed(VOLBTN* pvolbtn)
{
    return &pvolbtn->btn.fCheckpointed;
}

void SetVolbtnFCheckpointed(VOLBTN* pvolbtn, int fCheckpointed)
{
    pvolbtn->btn.fCheckpointed = fCheckpointed;
}

void* GetVolbtnFMusicExit(VOLBTN* pvolbtn)
{
    return &pvolbtn->fMustExit;
}

void SetVolbtnFMusicExit(VOLBTN* pvolbtn, int fMusicExit)
{
    pvolbtn->fMustExit = fMusicExit;
}

void* GetVolbtnFSendChangeEvents(VOLBTN* pvolbtn)
{
    return &pvolbtn->fSendChangeEvents;
}

void SetVolbtnFSendChangeEvents(VOLBTN* pvolbtn, int fSendChangeEvents)
{
    pvolbtn->fSendChangeEvents = fSendChangeEvents;
}

void* GetVolbtnOidDialog(VOLBTN* pvolbtn)
{
    return &pvolbtn->btn.oidDialog;
}

void SetVolbtnOidDialog(VOLBTN* pvolbtn, OID oidDialog)
{
    pvolbtn->btn.oidDialog = oidDialog;
}

void* GetVolbtnButtons(VOLBTN* pvolbtn)
{
    return &pvolbtn->btn.buttons;
}

int GetVolbtnSize()
{
	return sizeof(VOLBTN);
}

void CloneVolbtn(VOLBTN* pvolbtn, VOLBTN* pvolbtnBase)
{
    CloneSo(pvolbtn, pvolbtnBase);

    pvolbtn->coidPush = pvolbtnBase->coidPush;

    for (int i = 0; i < 8; i++)
        pvolbtn->aoidPush[i] = pvolbtnBase->aoidPush[i];

    pvolbtn->cploPush = pvolbtnBase->cploPush;

    for (int i = 0; i < 8; i++)
        pvolbtn->aploPush[i] = pvolbtnBase->aploPush[i];

    pvolbtn->cploPushWithin = pvolbtnBase->cploPushWithin;

    for (int i = 0; i < 8; i++)
        pvolbtn->aploPushWithin[i] = pvolbtnBase->aploPushWithin[i];

    pvolbtn->fPpoPush = pvolbtnBase->fPpoPush;

    int ichkPushed = pvolbtn->btn.ichkPushed;

    pvolbtn->btn = pvolbtnBase->btn;

    pvolbtn->fMustExit = pvolbtnBase->fMustExit;

    pvolbtn->btn.ichkPushed = ichkPushed;
    pvolbtn->btn.paloOwner = pvolbtn;
}

void LoadVolbtnFromBrx(VOLBTN* pvolbtn, CBinaryInputStream* pbis)
{
    LoadSoFromBrx(pvolbtn, pbis);
    LoadBtn(&pvolbtn->btn, pvolbtn);

    if (pvolbtn->fSendChangeEvents != 0) 
        pvolbtn->btn.fManualReset = 1;
}

void PostVolbtnLoad(VOLBTN* pvolbtn)
{
    PostAloLoad(pvolbtn);
    PostBtnLoad(&pvolbtn->btn);
    SetVolbtnButtons(pvolbtn, BUTTONS_NoContact);

    for (int ioid = 0; ioid < pvolbtn->coidPush; ++ioid)
    {
        LO* ploPush = PloFindSwNearest(pvolbtn->psw, pvolbtn->aoidPush[ioid], (LO*)pvolbtn);

        if (ploPush != nullptr)
            pvolbtn->aploPush[pvolbtn->cploPush++] = ploPush;
    }

    if (pvolbtn->coidPush == 0)
        pvolbtn->fPpoPush = 1;
}

void SetVolbtnButtons(VOLBTN* pvolbtn, BUTTONS buttons)
{
    SetBtnButtons(&pvolbtn->btn, buttons);
}

void UpdateVolbtn(VOLBTN* pvolbtn, float dt)
{
    LO* aploPushWithin[8]{};

    UpdateSo(pvolbtn, dt);

    BUTTONS buttons = pvolbtn->btn.buttons;

    if (buttons == BUTTONS_Pushed)
    {
        if (pvolbtn->btn.dtRepush < 0.0f)
            return;

        if (g_clock.t - pvolbtn->btn.tButtons <= pvolbtn->btn.dtRepush)
            return;

        if (pvolbtn->fMustExit)
            SetVolbtnButtons(pvolbtn, BUTTONS_Contact);
        else
            SetVolbtnButtons(pvolbtn, BUTTONS_NoContact);

        return;
    }

    if (buttons != BUTTONS_NoContact && buttons != BUTTONS_Contact)
        return;

    buttons = BUTTONS_NoContact;

    if (pvolbtn->fPpoPush)
    {
        PO* ppoPush = PpoCur();

        if (ppoPush != nullptr)
        {
            pvolbtn->aploPush[0] = (LO*)ppoPush;
            pvolbtn->cploPush = 1;
        }
        else
            pvolbtn->cploPush = 0;
    }

    int cploPushWithin = 0;

    for (int iplo = 0; iplo < pvolbtn->cploPush; ++iplo)
    {
        LO* ploPush = pvolbtn->aploPush[iplo];
        glm::vec3 posWorld{};
        bool fHavePosition = false;

        if ((ploPush->pvtlo->grfcid & 1U) != 0)
        {
            posWorld = ((ALO*)ploPush)->xf.posWorld;
            fHavePosition = true;
        }
        else if (FIsBasicDerivedFrom(ploPush, CID_CM))
        {
            posWorld = ((XFM*)ploPush)->posLocal;
            fHavePosition = true;
        }
        else if (FIsBasicDerivedFrom(ploPush, CID_PNT))
        {
            GetPntPos((PNT*)ploPush, &posWorld);
            fHavePosition = true;
        }
        else if (FIsBasicDerivedFrom(ploPush, CID_XFM))
        {
            GetXfmPos((XFM*)ploPush, &posWorld);
            fHavePosition = true;
        }

        if (!fHavePosition)
            continue;

        bool fWithin = false;

        if (pvolbtn->fSphere)
        {
            float sDistance = glm::length(posWorld - pvolbtn->xf.posWorld);

            if (sDistance < pvolbtn->sRadiusSelf)
                fWithin = true;
        }

        if (!fWithin && pvolbtn->bspc.absp.size() != 0)
        {
            if (PbspPointInBspQuick(&posWorld, pvolbtn->bspc.absp.data()) != nullptr)
                fWithin = true;
        }

        if (!fWithin)
            continue;

        buttons = BUTTONS_Contact;

        if (pvolbtn->fSendChangeEvents && FIsLoInWorld(ploPush))
            aploPushWithin[cploPushWithin++] = ploPush;
    }

    if (pvolbtn->fSendChangeEvents)
    {
        bool fChanged = pvolbtn->cploPushWithin != cploPushWithin;

        if (!fChanged)
        {
            for (int iplo = 0; iplo < cploPushWithin; ++iplo)
            {
                if (pvolbtn->aploPushWithin[iplo] != aploPushWithin[iplo])
                {
                    fChanged = true;
                    break;
                }
            }
        }

        if (fChanged)
        {
            pvolbtn->cploPushWithin = cploPushWithin;

            for (int iplo = 0; iplo < cploPushWithin; ++iplo)
                pvolbtn->aploPushWithin[iplo] = aploPushWithin[iplo];

            HandleLoSpliceEvent(pvolbtn, 12, 0, nullptr);
        }
    }

    SetVolbtnButtons(pvolbtn, buttons);
}

int FGetVolbtnPushObjectsWithinList(VOLBTN* pvolbtn, void* pvstate)
{
    for (int iplo = 0; iplo < pvolbtn->cploPushWithin; ++iplo)
    {
        if (!FAppendSpliceListElement(pvstate, &pvolbtn->aploPushWithin[iplo]))
            return 0;
    }

    return 1;
}

void DeleteVolbtn(VOLBTN* pvolbtn)
{
	delete pvolbtn;
}

void AddBtnAseg(BTN* pbtn, ALO* palo, OID oid)
{
    FAddAshOid(pbtn->aash, oid);
}

void AddButtonAseg(BUTTON* pbutton, int oid)
{
    AddBtnAseg(&pbutton->btn, pbutton, (OID)oid);
}

void AddVolbtnAseg(VOLBTN* pvolbtn, OID oid)
{
    AddBtnAseg(&pvolbtn->btn, pvolbtn, oid);
}

void AddBtnOffAseg(BTN* pbtn, ALO* palo, OID oid)
{
    FAddAshOid(pbtn->aash + 1, oid);
}

void AddButtonOffAseg(BUTTON* pbutton, int oid)
{
    AddBtnOffAseg(&pbutton->btn, pbutton, (OID)oid);
}

void AddVolbtnOffAseg(VOLBTN* pvolbtn, OID oid)
{
    AddBtnOffAseg(&pvolbtn->btn, pvolbtn, oid);
}

void AddButtonPushObject(BUTTON* pbutton, OID oid)
{
    pbutton->aoidPush[pbutton->coidPush++] = oid;
}

void AddButtonNoPushObject(BUTTON* pbutton, OID oid)
{
    pbutton->aoidNoPush[pbutton->coidNoPush++] = oid;
}

void AddButtonPushClass(BUTTON* pbutton, CID cid)
{
    pbutton->acidPush[pbutton->ccidPush++] = cid;
}

void AddButtonNoPushClass(BUTTON* pbutton, CID cid)
{
    pbutton->acidNoPush[pbutton->ccidNoPush++] = cid;
}

void AddVolbtnPushObject(VOLBTN* pvolbtn, OID oid)
{
    pvolbtn->aoidPush[pvolbtn->coidPush++] = oid;
}

void SetButtonRsmg(BUTTON* pbutton, int fOnTrigger, int oidRoot, int oidSm, int oidGoal)
{
    SetBtnRsmg(&pbutton->btn, fOnTrigger, (OID)oidRoot, (OID)oidSm, (OID)oidGoal);
}

void SetVolbtnRsmg(VOLBTN* pvolbtn, int fOnTrigger, OID oidRoot, OID oidSM, OID oidGoal)
{
    SetBtnRsmg(&pvolbtn->btn, fOnTrigger, oidRoot, oidSM, oidGoal);
}
