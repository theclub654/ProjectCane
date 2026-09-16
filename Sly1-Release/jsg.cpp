#include "jsg.h"
#include "jt.h"
#include "tn.h"
#include "xform.h"
#include "pipe.h"
#include "rail.h"
#include "jump.h"
#include "stephide.h"
#include "stepcane.h"

JSG* NewJsg()
{
	return new JSG{};
}

void InitJsg(JSG* pjsg)
{
	InitLo(pjsg);
    pjsg->ajsge.resize(200);
    pjsg->fHandsOff = 1;
}

void* GetJsgFHandsOff(JSG* pjsg)
{
    return &pjsg->fHandsOff;
}

void SetJsgFHandsOff(JSG* pjsg, int fHandsOff)
{
    pjsg->fHandsOff = fHandsOff;
}

void CloneJsg(JSG* pjsg, JSG* pjsgBase)
{
    CloneLo(pjsg, pjsgBase);

    // Clone the members
    pjsg->cjsge = pjsgBase->cjsge;

    // Clone the ajsge array (assuming simple array copy)
    pjsg->ajsge = pjsgBase->ajsge;

    // Clone the pjt pointer
    pjsg->pjt = pjsgBase->pjt;

    // Clone the ijsgeCur and tCur
    pjsg->ijsgeCur = pjsgBase->ijsgeCur;
    pjsg->tCur = pjsgBase->tCur;

    // Clone the pjsgeJoy pointer
    pjsg->pjsgeJoy = pjsgBase->pjsgeJoy;

    // Clone the ploContext pointer
    pjsg->ploContext = pjsgBase->ploContext;

    // Clone the pasegaCur pointer
    pjsg->pasegaCur = pjsgBase->pasegaCur;

    // Clone the ploFocus pointer
    pjsg->ploFocus = pjsgBase->ploFocus;

    // Clone the ptnCur pointer
    pjsg->ptnCur = pjsgBase->ptnCur;

    // Clone the fHideTn flag
    pjsg->fHideTn = pjsgBase->fHideTn;

    pjsg->uRail = pjsgBase->uRail;
    pjsg->sPipe = pjsgBase->sPipe;
    pjsg->fHandsOff = pjsgBase->fHandsOff;
}

void GetJsgLocation(JSG* pjsg, LO* plo, glm::vec3* ppos, float* prad)
{
    JT* pjt = pjsg->pjt;

    glm::vec3 pos = pjt->xf.pos;
    float rad = pjt->radTarget;

    pos.z -= 75.0f;

    if (plo != nullptr)
    {
        if (FIsBasicDerivedFrom(plo, CID_PNT))
            GetPntPos(static_cast<PNT*>(plo), &pos);
        else if (FIsBasicDerivedFrom(plo, CID_XFM))
        {
            XFM* pxfm = static_cast<XFM*>(plo);
            glm::mat3 mat;

            GetXfmPos(pxfm, &pos);
            GetXfmMat(pxfm, &mat);

            rad = std::atan2(mat[0].y, mat[0].x);
        }
        else if (FIsBasicDerivedFrom(plo, CID_SHAPE))
        {
            SHAPE* pshape = static_cast<SHAPE*>(plo);
            CRV* pcrv = pshape->pcrv.get();

            glm::vec3 posLocal;
            glm::vec3 normalLocal = g_normalX;
            glm::vec3 normal;

            ConvertAloPos(nullptr, pshape->paloParent, &pjt->xf.pos, &posLocal);

            if (pcrv != nullptr && pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
                pcrv->pvtcrv->pfnFindCrvClosestPointAll(pcrv, &posLocal, nullptr, &posLocal, &normalLocal, nullptr, nullptr);

            ConvertAloPos(pshape->paloParent, nullptr, &posLocal, &pos);
            ConvertAloVec(pshape->paloParent, nullptr, &normalLocal, &normal);

            rad = std::atan2(normal.y, normal.x);
        }
        else if (FIsBasicDerivedFrom(plo, CID_ALO))
        {
            ALO* palo = static_cast<ALO*>(plo);

            pos = palo->xf.posWorld;
            rad = std::atan2(palo->xf.matWorld[0].y, palo->xf.matWorld[0].x);
        }
    }

    if (ppos != nullptr)
        *ppos = pos;

    if (prad != nullptr)
        *prad = rad;
}

void SetJsgFocus(JSG* jsg, LO* focus) 
{
    JT* jt = jsg->pjt;

    if (jsg->ploFocus) {
        jt->dposSmoothCpdefi = jt->posBasePrev - jt->xf.pos;
        jt->dposSmoothCpdefi.z = 0.0f;
        jt->fLockBase = false;
        jsg->ploFocus = nullptr;
    }

    if (focus) {
        jt->fLockBase = true;
        GetJsgLocation(jsg, (ALO*)focus, &jt->posBasePrev, nullptr);
        jt->posBasePrev.z += 125.0f;
        jt->dposSmoothCpdefi = glm::vec3(0.0f);
        jsg->ploFocus = focus;
    }
}

void SetJsgTn(JSG* pjsg, TN* ptn)
{
    if (pjsg->ptnCur != nullptr)
    {
        TN* ptnCur = pjsg->ptnCur;

        SetTnTnsOverride(ptnCur, TNS_Nil);
        ptnCur->ppo = nullptr;

        if (pjsg->fHideTn)
            ptnCur->pvtlo->pfnRemoveLo(ptnCur);

        pjsg->ptnCur = nullptr;
    }

    if (ptn != nullptr)
    {
        pjsg->ptnCur = ptn;
        pjsg->fHideTn = !FIsLoInWorld(ptn);

        if (pjsg->fHideTn)
            ptn->pvtlo->pfnAddLo(ptn);

        ptn->ppo = static_cast<PO*>(pjsg->pjt);
        SetTnTnsOverride(ptn, TNS_In);
    }
}

void NextJsgJsge(JSG* pjsg)
{
    ++pjsg->ijsgeCur;

    if (pjsg->ijsgeCur >= pjsg->cjsge)
        return;

    JT* pjt = pjsg->pjt;
    JSGE* pjsge = &pjsg->ajsge[pjsg->ijsgeCur];
    LO* plo = PloFindSwObject(pjsg->psw, 260, (OID)pjsge->oid, pjsg->ploContext);

    switch (pjsge->jsgek)
    {
        case JSGEK_Context:
        pjsg->ploContext = plo;
        break;

        case JSGEK_Cut:
        if (plo == nullptr || !FIsBasicDerivedFrom(plo, CID_WARP))
        {
            glm::vec3 pos;
            float rad;

            GetJsgLocation(pjsg, plo, &pos, &rad);

            pjt->posBaseXp = pos;
            pjt->posBasePrev = pos;
            pjt->posBasePrev.z += 75.0f;

            pjt->pvtalo->pfnTranslateAloToPos(pjt, &pos);

            pjt->radTarget = rad;

            glm::mat3 mat;
            LoadRotateMatrixRad(rad, &g_normalZ, &mat);

            pjt->pvtalo->pfnRotateAloToMat(pjt, &mat);
            pjt->pvtalo->pfnSetAloVelocityVec(pjt, &g_vecZero);
            pjt->pvtalo->pfnSetAloAngularVelocityVec(pjt, &g_vecZero);

            SetJtJts(pjt, JTS_Jump, JTBS_Jump_Fall);

            pjt->fBaseXp = 0;
            pjsg->pjsgeJoy = nullptr;
        }
        else
            TriggerWarp(static_cast<WARP*>(plo));
        break;

        case JSGEK_Label:
        {
            void* pvLabel = &pjsge->data;
            HandleLoSpliceEvent(static_cast<LO*>(pjsg), 24, 1, &pvLabel);
            break;
        }

        case JSGEK_Tunnel:
        {
            TN* ptn = nullptr;

            if (plo != nullptr && FIsBasicDerivedFrom(plo, CID_TN))
                ptn = static_cast<TN*>(plo);

            SetJsgTn(pjsg, ptn);
            break;
        }

        case JSGEK_Pause:
        if (plo != nullptr)
            pjsg->pjsgeJoy = pjsge;
        break;

        case JSGEK_Jump:
        {
            pjsg->pjsgeJoy = pjsge;

            int jtbs = pjsge->data.jump.jsgjk == JSGJK_Fall ? JTBS_Jump_Fall : JTBS_Jump_Init;

            if (plo == nullptr)
            {
                SetJtJts(pjt, JTS_Jump, jtbs);
                break;
            }

            glm::vec3 posJump = pjt->xf.pos;
            glm::vec3 vJump(0.0f);
            float dtDoubleJump = 0.0f;

            pjt->jtjk = JTJK_Standing;

            switch (pjt->jts)
            {
                case JTS_Run:
                pjt->jtjk = JTJK_Running;
                break;

                case JTS_Hang:
                case JTS_Pipe:
                pjt->jtjk = JTJK_Swinging;
                break;

                case JTS_Edge:
                pjt->jtjk = JTJK_Edge;
                break;
            }

            switch (pjsge->data.jump.jsgjk)
            {
                case JSGJK_Fall:
                vJump = pjt->xf.v;
                break;

                case JSGJK_Jump:
                GetJtJumpVelocity(pjt, &vJump);
                break;

                case JSGJK_DoubleJump:
                {
                    GetJtJumpVelocity(pjt, &vJump);

                    dtDoubleJump = -vJump.z / pjt->dvGravity.z;
                    glm::vec3 vAtApex = vJump + pjt->dvGravity * dtDoubleJump;
                    posJump += (vJump + vAtApex) * (dtDoubleJump * 0.5f);

                    glm::vec3 vPrev = pjt->xf.v;
                    pjt->xf.v = vAtApex;

                    GetJtJumpBoostVelocity(pjt, &vJump);

                    pjt->xf.v = vPrev;
                    break;
                }
            }

            glm::vec3 posTarget;
            glm::vec3 vTarget;

            GetJsgLocation(pjsg, plo, &posTarget, nullptr);
            CalculateAloMovement(static_cast<ALO*>(plo)->paloParent, nullptr, posTarget, &vTarget, nullptr, nullptr, nullptr);

            posTarget += vTarget * dtDoubleJump;

            if (FIsBasicDerivedFrom(plo, CID_HND))
                posTarget.z += DZ_JsgHndTarget;

            float dtJump;
            const int fTrajectory =
                FTimeJtJumpToTarget(pjt, &posJump, &vJump, &posTarget, &vTarget, &dtJump);

            if (fTrajectory)
            {
                SetJtJts(pjt, JTS_Jump, jtbs);

                glm::vec3 posTargetFinal = posTarget + vTarget * dtJump;
                glm::vec3 v = (posTargetFinal - pjt->xf.pos) / (dtDoubleJump + dtJump);

                v.z = pjt->xf.v.z;

                pjt->pvtalo->pfnSetAloVelocityVec(pjt, &v);

                glm::vec3 dpos = posTargetFinal - pjt->xf.pos;
                float radTarget = std::atan2(dpos.y, dpos.x);

                pjt->radJoy = radTarget;
                pjt->radTarget = radTarget;
            }

            pjt->jsglk = pjsge->data.jump.jsglk;
            break;
        }

        case JSGEK_Run:
        pjsg->pjsgeJoy = pjsge;

        if (plo != nullptr)
        {
            bool fHideTarget =
                FIsBasicDerivedFrom(plo, CID_HPNT) ||
                FIsBasicDerivedFrom(plo, CID_HSHAPE) ||
                FIsBasicDerivedFrom(plo, CID_HBSK) ||
                FIsBasicDerivedFrom(plo, CID_PIPE) ||
                FIsBasicDerivedFrom(plo, CID_HND);

            if (fHideTarget)
            {
                JTBS jtbs = (JTBS)JtbsChooseJtHide(pjt, plo, &pjt->jthk);

                if (jtbs != JTBS_Nil)
                    SetJtJts(pjt, JTS_Hide, jtbs);
            }

            if (pjt->jts == JTS_Hide && pjt->jthk == JTHK_Rail)
            {
                glm::vec3 pos;

                GetJsgLocation(pjsg, plo, &pos, nullptr);
                ConvertAloPos(nullptr, pjt->prailCur->paloParent, &pos, &pos);

                CRV* pcrv = pjt->prailCur->pcrv.get();

                if (pcrv != nullptr && pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
                    pcrv->pvtcrv->pfnFindCrvClosestPointAll(pcrv, &pos, nullptr, nullptr, nullptr, &pjsg->uRail, nullptr);
            }
        }
        break;

        case JSGEK_Climb:
        {
            pjsg->pjsgeJoy = pjsge;

            glm::vec3 pos;

            GetJsgLocation(pjsg, plo, &pos, nullptr);
            ConvertAloPos(nullptr, pjt->ppipeCur->paloParent, &pos, &pos);

            CRV* pcrv = pjt->ppipeCur->pcrv.get();

            if (pcrv != nullptr && pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
                pcrv->pvtcrv->pfnFindCrvClosestPointAll(pcrv, &pos, nullptr, nullptr, nullptr, nullptr, &pjsg->sPipe);

            break;
        }

        case JSGEK_Attack:
        switch (pjsge->data.attack.ftak)
        {
            case FTAK_Sweep:
            SetJtJtcs(pjt, JTCS_SweepInit);
            break;

            case FTAK_Smash:
            SetJtJts(pjt, JTS_Jump, JTBS_Jump_Smash);
            break;

            case FTAK_Rush:
            SetJtJts(pjt, JTS_Rush, JTBS_Rush_Attack);
            break;
        }
        break;

        case JSGEK_Aseg:
        if (plo != nullptr && FIsBasicDerivedFrom(plo, CID_ASEG))
        {
            ASEGA* pasega = nullptr;

            ApplyAseg(static_cast<ASEG*>(plo), nullptr, 0.0f, 1.0f, 0, &pasega);

            if (!pjsge->fAsync)
                pjsg->pasegaCur = pasega;
        }
        break;

        case JSGEK_Focus:
        SetJsgFocus(pjsg, plo);
        break;

        case JSGEK_Clock:
        SetClockRate(pjsge->data.clock.rdt);
        break;
    }

    pjsg->tCur = g_clock.t;
}

bool FIsJsgJsgeComplete(JSG* pjsg, JSGE* pjsge)
{
    JT* pjt = pjsg->pjt;
    LO* plo = PloFindSwObject(pjt->psw, 260, (OID)pjsge->oid, pjsg->ploContext);

    switch (pjsge->jsgek)
    {
        case JSGEK_Pause:
        {
            const float elapsed = g_clock.t - pjsg->tCur;
            const bool complete = elapsed > pjsge->data.pause.dtPause;
            return complete;
        }

        case JSGEK_Run:
        {
            if (plo == nullptr)
                return true;

            glm::vec3 posTarget;
            GetJsgLocation(pjsg, plo, &posTarget, nullptr);

            const glm::vec3 dpos = pjt->xf.posWorld - posTarget;
            const float distance = glm::length(glm::vec2(dpos));
            const bool complete = distance < SD_JsgRunComplete;

            return complete;
        }

        case JSGEK_Climb:
        return std::abs(pjt->sPipeHand - pjsg->sPipe) < DS_JsgClimbComplete;

        case JSGEK_Attack:
        switch (pjsge->data.attack.ftak)
        {
            case FTAK_Sweep:
            return pjt->jtcs == JTCS_Nil;

            case FTAK_Smash:
            return pjt->jts != JTS_Jump;

            case FTAK_Rush:
            return pjt->jts != JTS_Rush;

            default:
            return true;
        }

        case JSGEK_Jump:
        {
            const bool complete = pjt->jts != JTS_Jump;

            return complete;
        }

        case JSGEK_Aseg:
        {
            ASEGA* pasega = pjsg->pasegaCur;

            if (pasega != nullptr && pasega->svtLocal != 0.0f)
                return false;

            if (pasega != nullptr)
            {
                RetractAsega(pasega);
                pjsg->pasegaCur = nullptr;
            }

            return true;
        }

        case JSGEK_Hide:
        if (pjt->jts == JTS_Jump && static_cast<unsigned int>(pjt->jtbs - JTBS_Jump_Hshape) < 2U)
            return false;

        if (pjt->jts == JTS_Hide && static_cast<unsigned int>(pjt->jthk) < 2U)
            SetJtJts(pjt, JTS_Stand, JTBS_Nil);

        return true;

        default:
        return true;
    }
}

void UpdateJsgJsge(JSG* pjsg)
{
    if (pjsg->ijsgeCur < 0 && pjsg->cjsge > 0)
        NextJsgJsge(pjsg);

    while (pjsg->ijsgeCur < pjsg->cjsge)
    {
        JSGE* pjsge = &pjsg->ajsge[pjsg->ijsgeCur];

        if (!pjsge->fAsync && !FIsJsgJsgeComplete(pjsg, pjsge))
            break;

        NextJsgJsge(pjsg);
    }

    if (pjsg->pjsgeJoy != nullptr && FIsJsgJsgeComplete(pjsg, pjsg->pjsgeJoy))
        pjsg->pjsgeJoy = nullptr;
}

void ReadJsgJoystick(JSG* pjsg, JOY* pjoy)
{
    JSGE* pjsge = pjsg->pjsgeJoy;
    JT* pjt = pjsg->pjt;

    if (pjsge == nullptr)
    {
        pjt->uJoyDeflect = 0.0f;

        if (pjt->jts == JTS_Hide)
            pjoy->forcedHeld[BTN_CIRCLE] = true;

        return;
    }

    LO* plo = PloFindSwObject(pjt->psw, 260, (OID)pjsge->oid, pjsg->ploContext);

    switch (pjsge->jsgek)
    {
        case JSGEK_Pause:
        if (plo != nullptr)
        {
            float rad;
            GetJsgLocation(pjsg, plo, nullptr, &rad);

            pjt->radJoy = rad;
            pjt->radTarget = rad;
        }

        pjt->uJoyDeflect = 0.0f;

        if (pjt->jts == JTS_Hide)
            pjoy->forcedHeld[BTN_CIRCLE] = true;

        return;

        case JSGEK_Jump:
        {
            if (pjt->jts == JTS_Jump && plo != nullptr)
            {
                bool fChooseLanding =
                    (pjt->jtbs == JTBS_Jump_Fall && g_clock.t - pjt->tJtbs > DT_JsgTargetFall) ||
                    (pjt->jtbs == JTBS_Jump_Boost && g_clock.t - pjt->tJtbs > DT_JsgTargetBoost);

                if (fChooseLanding)
                {
                    bool fLandingTarget =
                        FIsBasicDerivedFrom(plo, CID_SPIRE) ||
                        FIsBasicDerivedFrom(plo, CID_RAIL) ||
                        FIsBasicDerivedFrom(plo, CID_HSHAPE) ||
                        FIsBasicDerivedFrom(plo, CID_HPNT) ||
                        FIsBasicDerivedFrom(plo, CID_PIPE) ||
                        FIsBasicDerivedFrom(plo, CID_HND);

                    if (fLandingTarget)
                    {
                        JTBS jtbs = (JTBS)JtbsChooseJtLanding(pjt, plo);
                        SetJtJts(pjt, JTS_Jump, jtbs);
                    }
                }
            }

            if (pjsge->data.jump.jsgjk == JSGJK_DoubleJump && pjt->jtbs == JTBS_Jump_Fall && pjt->fCanBoost)
                SetJtJts(pjt, JTS_Jump, JTBS_Jump_Boost);

            pjt->uJoyDeflect = 0.0f;
            return;
        }

        case JSGEK_Run:
        {
            float uRun = pjsge->data.run.uRun;

            if (plo != nullptr)
            {
                if (pjt->jts == JTS_Hide && pjt->jthk == JTHK_Rail)
                {
                    CRV* pcrv = pjt->prailCur->pcrv.get();
                    ALO* paloRail = pjt->prailCur->paloParent;

                    glm::vec3 posRail;
                    glm::vec3 vecRail;

                    if (pcrv->pvtcrv->pfnEvaluateCrvFromU != nullptr)
                        pcrv->pvtcrv->pfnEvaluateCrvFromU(pcrv, pjt->uRailCur, &posRail, &vecRail);

                    ConvertAloPos(paloRail, nullptr, &posRail, &posRail);
                    ConvertAloVec(paloRail, nullptr, &vecRail, &vecRail);

                    vecRail.z = 0.0f;

                    if (glm::length(vecRail) < 0.0001f)
                        vecRail = g_normalX;
                    else
                        vecRail = glm::normalize(vecRail);

                    pjt->radJoy = std::atan2(vecRail.y, vecRail.x);

                    bool fReverse;

                    if (!pcrv->fClosed)
                    {
                        fReverse = pjsg->uRail < pjt->uRailCur;
                    }
                    else if (pjsg->uRail < pjt->uRailCur)
                    {
                        float duDirect = pjt->uRailCur - pjsg->uRail;
                        float duWrapped = pcrv->pvtcrv->pfnUMaxCrv(pcrv) + pjsg->uRail - pjt->uRailCur;
                        fReverse = duDirect < duWrapped;
                    }
                    else
                    {
                        float duDirect = pjsg->uRail - pjt->uRailCur;
                        float duWrapped = pcrv->pvtcrv->pfnUMaxCrv(pcrv) + pjt->uRailCur - pjsg->uRail;
                        fReverse = duWrapped < duDirect;
                    }

                    if (fReverse)
                        pjt->radJoy = RadNormalize(pjt->radJoy + glm::pi<float>());
                }
                else
                {
                    glm::vec3 posTarget;
                    GetJsgLocation(pjsg, plo, &posTarget, nullptr);

                    glm::vec3 dpos = posTarget - pjt->xf.posWorld;
                    pjt->radJoy = std::atan2(dpos.y, dpos.x);
                }
            }

            pjt->uJoyDeflect = std::abs(uRun);

            if (pjt->jts == JTS_Hide && pjt->jthk >= 0 && pjt->jthk < 2)
                pjoy->forcedHeld[BTN_CIRCLE] = true;

            return;
        }

        case JSGEK_Climb:
        {
            PIPE* ppipe = pjt->ppipeCur;
            CRV* pcrv = ppipe->pcrv.get();

            glm::vec3 posAhead;
            glm::vec3 posBehind;

            if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
            {
                pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeHand + DS_JsgClimbSample, &posAhead, nullptr);
                pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeHand - DS_JsgClimbSample, &posBehind, nullptr);
            }

            ConvertAloPos(ppipe->paloParent, nullptr, &posAhead, &posAhead);
            ConvertAloPos(ppipe->paloParent, nullptr, &posBehind, &posBehind);

            glm::vec3 screenAhead;
            glm::vec3 screenBehind;

            ConvertCmWorldToScreen(g_pcm, &posAhead, &screenAhead);
            ConvertCmWorldToScreen(g_pcm, &posBehind, &screenBehind);

            glm::vec3 vecScreen = screenAhead - screenBehind;
            vecScreen.z = 0.0f;

            if (glm::length(vecScreen) < 0.0001f)
                vecScreen = g_normalY;
            else
                vecScreen = glm::normalize(vecScreen);

            pjt->radJoy = std::atan2(vecScreen.y, vecScreen.x);
            pjoy->x = vecScreen.x;
            pjoy->y = vecScreen.y;

            bool fReverse;

            if (!pcrv->fClosed)
            {
                fReverse = pjsg->sPipe < pjt->sPipeHand;
            }
            else if (pjsg->sPipe < pjt->sPipeHand)
            {
                float dsDirect = pjt->sPipeHand - pjsg->sPipe;
                float dsWrapped = pcrv->pvtcrv->pfnSMaxCrv(pcrv) + pjsg->sPipe - pjt->sPipeHand;
                fReverse = dsDirect < dsWrapped;
            }
            else
            {
                float dsDirect = pjsg->sPipe - pjt->sPipeHand;
                float dsWrapped = pcrv->pvtcrv->pfnSMaxCrv(pcrv) + pjt->sPipeHand - pjsg->sPipe;
                fReverse = dsWrapped < dsDirect;
            }

            if (fReverse)
            {
                pjt->radJoy = RadNormalize(pjt->radJoy + glm::pi<float>());
                pjoy->x = -pjoy->x;
                pjoy->y = -pjoy->y;
            }

            pjt->uJoyDeflect = 1.0f;
            return;
        }

        default:
        return;
    }
}

bool FIsJsgActive(JSG* pjsg)
{
    UpdateJsgJsge(pjsg);
    return pjsg->cjsge > 0 && pjsg->ijsgeCur < pjsg->cjsge;
}

void ClearJsg(JSG* pjsg)
{
    if (pjsg->pjt == nullptr)
        pjsg->cjsge = 0;
    else 
    {
        RetractJsg(pjsg);
        pjsg->cjsge = 0;
    }
    pjsg->ijsgeCur = 0;
}

void ApplyJsg(JSG* pjsg, JT* pjt)
{
	pjt->pjsgCur = pjsg;
    pjsg->pjt = pjt;
    pjsg->ijsgeCur = -1;

    HandleLoSpliceEvent(pjsg, 23, 0, nullptr);

    if (pjsg->fHandsOff != 0)
    {
        AddGrfusr(4);
        IncrementSwHandsOff(pjsg->psw);
    }
}

void RetractJsg(JSG* pjsg)
{
    if (pjsg->pjt == nullptr)
        return;

    SetJsgTn(pjsg, nullptr);
    SetJsgFocus(pjsg, nullptr);

    if (pjsg->pasegaCur != nullptr)
    {
        RetractAsega(pjsg->pasegaCur);
        pjsg->pasegaCur = nullptr;
    }

    SetClockRate(1.0f);

    pjsg->ijsgeCur = pjsg->cjsge;
    pjsg->ploContext = nullptr;
    pjsg->pjsgeJoy = nullptr;

    pjsg->pjt->pjsgCur = nullptr;
    pjsg->pjt = nullptr;

    HandleLoSpliceEvent(pjsg, 25, 0, nullptr);

    if (pjsg->fHandsOff)
    {
        RemoveGrfusr(4);
        DecrementSwHandsOff(pjsg->psw);
    }
}

JSGE* PjsgeAllocJsg(JSG* pjsg, JSGEK jsgek, OID oid, int fAsync)
{
    if (pjsg->cjsge >= 128)
        return s_ajsge;

    JSGE* pjsge = &pjsg->ajsge[pjsg->cjsge++];

    std::memset(pjsge, 0, sizeof(JSGE));

    pjsge->jsgek = jsgek;
    pjsge->oid = oid;
    pjsge->fAsync = fAsync;

    return pjsge;
}

void AddJsgContext(JSG* pjsg, OID oid)
{
    PjsgeAllocJsg(pjsg, JSGEK_Context, oid, true);
}

void AddJsgCut(JSG* pjsg, OID oid)
{
    PjsgeAllocJsg(pjsg, JSGEK_Cut, oid, true);
}

void AddJsgFocus(JSG* pjsg, OID oid)
{
    PjsgeAllocJsg(pjsg, JSGEK_Focus, oid, true);
}

void AddJsgLabel(JSG* pjsg, OID oid)
{
    JSGE* pjsge = PjsgeAllocJsg(pjsg, JSGEK_Label, OID_Nil, true);
    pjsge->data.label.oid = oid;
}

void AddJsgTunnel(JSG* pjsg, OID oid)
{
    PjsgeAllocJsg(pjsg, JSGEK_Tunnel, oid, true);
}

void AddJsgPause(JSG* pjsg, OID oid, float dtPause)
{
    JSGE* pjsge = PjsgeAllocJsg(pjsg, JSGEK_Pause, oid, false);
    pjsge->data.pause.dtPause = dtPause;
}

void AddJsgAseg(JSG* pjsg, OID oid, int fAsync)
{
	PjsgeAllocJsg(pjsg, JSGEK_Aseg, oid, fAsync);
}

void AddJsgJump(JSG* pjsg, OID oid, JSGJK jsgjk, JSGLK jsglk, int fAsync)
{
    JSGE* pjsge = PjsgeAllocJsg(pjsg, JSGEK_Jump, oid, fAsync);
    pjsge->data.jump.jsgjk = jsgjk;
    pjsge->data.jump.jsglk = jsglk;

}

void AddJsgRun(JSG* pjsg, OID oid, float uRun, int fAsync)
{
    JSGE* pjsge = PjsgeAllocJsg(pjsg, JSGEK_Run, oid, fAsync);
    pjsge->data.run.uRun = uRun;
}

void AddJsgClimb(JSG* pjsg, OID oid, float uClimb, int fAsync)
{
    JSGE* pjsge = PjsgeAllocJsg(pjsg, JSGEK_Climb, oid, fAsync);
    pjsge->data.climb.uClimb = uClimb;
}

void AddJsgAttack(JSG* pjsg, OID oid, FTAK ftak, int fAsync)
{
    JSGE* pjsge = PjsgeAllocJsg(pjsg, JSGEK_Attack, oid, fAsync);
    pjsge->data.attack.ftak = ftak;
}

void AddJsgClock(JSG* pjsg, float rdt)
{
    JSGE* pjsge = PjsgeAllocJsg(pjsg, JSGEK_Clock, OID_Nil, false);
    pjsge->data.clock.rdt = rdt;
}

void AddJsgHide(JSG* pjsg)
{
    PjsgeAllocJsg(pjsg, JSGEK_Hide, OID_Nil, false);
}

int GetJsgSize()
{
	return sizeof(JSG);
}

void DeleteJsg(JSG* pjsg)
{
	delete pjsg;
}
