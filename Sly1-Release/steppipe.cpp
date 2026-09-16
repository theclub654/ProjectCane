#include "steppipe.h"
#include "jt.h"
#include "pipe.h"

void PostJtLoadPipe(JT* pjt)
{
    for (int i = 0; i < 2; ++i)
    {
        glm::vec3 v(0.0f);

        PredictAsegEffect(pjt->ablcl[i].paseg, 0.0f, 1.0f, pjt->paloRightFoot, pjt, 1, nullptr, nullptr, &v, nullptr);

        pjt->ablcl[i].sv = v.z;
    }

    EnsureAsegBlendDynamic(pjt, static_cast<int>(sizeof(BLCL)), static_cast<int>(std::size(pjt->ablcl)), pjt->ablcl, 0, nullptr, nullptr, &pjt->pasegblClimb);

    PredictAsegEffect(pjt->pasegPipeStay, 0.0f, 1.0f, pjt->paloLeftHand,  pjt, 1, &pjt->posPipeHandLocal, nullptr, nullptr, nullptr);
    PredictAsegEffect(pjt->pasegPipeStay, 0.0f, 1.0f, pjt->paloRightFoot, pjt, 1, &pjt->posPipeFootLocal, nullptr, nullptr, nullptr);

    const float xCenter = (pjt->posPipeHandLocal.x + pjt->posPipeFootLocal.x) * 0.5f;

    pjt->posPipeHandLocal.x = xCenter;
    pjt->posPipeHandLocal.y = 0.0f;

    pjt->posPipeFootLocal.x = xCenter;
    pjt->posPipeFootLocal.y = 0.0f;
}

void GetJtPipeGoalMat(JT* pjt, PIPE* ppipe, float s, glm::mat3* pmat)
{
    constexpr float DS_JtPipeNormalGap = 50.0f;
    constexpr float DS_JtPipeNormalSearch = 25.0f;

    CRV* pcrv = ppipe->pcrv.get();
    const float sMax = pcrv->pvtcrv->pfnSMaxCrv(pcrv);

    auto NormalizeOr = [](const glm::vec3& v, const glm::vec3& fallback)
    {
        const float length = glm::length(v);
        return length < 0.0001f ? fallback : v / length;
    };

    glm::vec3 vecOrig(0.0f);

    if (pcrv->pvtcrv->pfnEvaluateCrvFromU != nullptr)
        pcrv->pvtcrv->pfnEvaluateCrvFromU(pcrv, pcrv->mpicvu[0], nullptr, &vecOrig);

    vecOrig = NormalizeOr(vecOrig, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::vec3 vecCur(0.0f);
    glm::vec3 vecRotate(0.0f);

    for (uint32_t i = 0;; ++i)
    {
        float sSearch;

        if ((i & 1) == 0)
            sSearch = s - static_cast<float>(i) * DS_JtPipeNormalSearch;
        else
            sSearch = s + static_cast<float>(i + 1) * DS_JtPipeNormalSearch;

        if (pcrv->fClosed)
            sSearch = GModPositive(sSearch, sMax);
        else if (sSearch < pcrv->mpicvs[0] || sSearch > sMax)
            continue;

        glm::vec3 posTop(0.0f);
        glm::vec3 posBottom(0.0f);

        if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
        {
            pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, sSearch + DS_JtPipeNormalGap, &posTop, nullptr);
            pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, sSearch - DS_JtPipeNormalGap, &posBottom, nullptr);
        }

        vecRotate = NormalizeOr(posTop - posBottom, glm::vec3(0.0f, 0.0f, 1.0f));

        if (i == 0)
            vecCur = vecRotate;

        if (glm::dot(vecOrig, vecRotate) > -0.9f)
            break;
    }

    glm::mat3 dmat(1.0f);
    BuildRotateVectorsMatrix(&vecOrig, &vecRotate, &dmat);

    glm::vec3 vecFace = dmat * ppipe->vecFace;

    ConvertAloVec(ppipe->paloParent, nullptr, &vecCur, &vecCur);
    ConvertAloVec(ppipe->paloParent, nullptr, &vecFace, &vecFace);

    glm::vec3 axisZ = NormalizeOr(vecCur, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec3 axisY = NormalizeOr(glm::cross(axisZ, vecFace), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 axisX = glm::cross(axisY, axisZ);

    if (pjt->fPipeReverse)
    {
        axisY = -axisY;
        axisZ = -axisZ;
    }

    if (pmat != nullptr)
    {
        (*pmat)[0] = axisX;
        (*pmat)[1] = axisY;
        (*pmat)[2] = axisZ;
    }
}

void PresetJtAccelPipe(JT* pjt)
{
    glm::mat3 matGoal(1.0f);
    glm::mat3 matNext(1.0f);

    GetJtPipeGoalMat(pjt, pjt->ppipeCur, pjt->sPipeHand, &matGoal);

    if (pjt->jtbs == 23 || pjt->jtbs == 27)
    {
        const float dtRemain = pjt->pasegaCur->paseg->tMax - pjt->pasegaCur->tLocal;
        const float dtPredict = std::min(dtRemain, g_clock.dt);

        EvaluateBezierMat(dtPredict, g_clock.dt, 1.0f, pjt->xf.mat, pjt->xf.w, matGoal, g_vecZero, &matNext, nullptr, nullptr);
    }
    else
        matNext = matGoal;

    glm::mat3 dmat(1.0f);
    glm::vec3 axis(0.0f);
    float rad = 0.0f;

    CalculateDmat(&pjt->xf.mat, &matNext, &dmat);
    DecomposeRotateMatrixRad(&dmat, &rad, &axis);

    pjt->xf.w = axis * (rad / g_clock.dt);
}

void UpdateJtActivePipe(JT* pjt, JOY* pjoy)
{
    constexpr int JTS_Pipe = 7;

    constexpr int JTBS_Pipe_Init = 23;
    constexpr int JTBS_Pipe_Stay = 24;
    constexpr int JTBS_Pipe_Up = 25;
    constexpr int JTBS_Pipe_Down = 26;
    constexpr int JTBS_Pipe_Spin = 27;

    constexpr int JTPDK_Nil = -1;
    constexpr int JTPDK_Up = 0;
    constexpr int JTPDK_Down = 1;

    constexpr float DS_JtPipeLimitHand = 35.0f;
    constexpr float DS_JtPipeLimitFoot = 100.0f;
    constexpr float DS_JtPipeNormalGap = 50.0f;
    constexpr float Z_JtPipeVerticalThreshold = 0.3f;

    PIPE* ppipe = pjt->ppipeCur;
    CRV* pcrv = ppipe->pcrv.get();

    pjt->dposPipeXp = PosSmooth(pjt->dposPipeXp, glm::vec3(0.0f), g_clock.dt, &s_smpPipeXp, nullptr);

    if (pjt->jtbs == JTBS_Pipe_Init || pjt->jtbs == JTBS_Pipe_Spin)
    {
        pjt->vTarget = glm::vec3(0.0f);
        return;
    }

    glm::vec3 posTopLocal(0.0f);
    glm::vec3 posBottomLocal(0.0f);

    if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
    {
        pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeHand + DS_JtPipeNormalGap, &posTopLocal, nullptr);
        pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeHand - DS_JtPipeNormalGap, &posBottomLocal, nullptr);
    }

    glm::vec3 posTop(0.0f);
    glm::vec3 posBottom(0.0f);

    ConvertAloPos(ppipe->paloParent, nullptr, &posTopLocal, &posTop);
    ConvertAloPos(ppipe->paloParent, nullptr, &posBottomLocal, &posBottom);

    const glm::vec3 pipeDirection = glm::length(posTop - posBottom) < 0.0001f
        ? glm::vec3(0.0f, 0.0f, 1.0f)
        : glm::normalize(posTop - posBottom);

    /*
     * Automatically spin when the pipe direction and the current facing
     * direction disagree. Explicit JTPDK input suppresses this test.
     */
    if (pjt->jtpdk == JTPDK_Nil)
    {
        bool spin = false;

        if (std::abs(pipeDirection.z) > Z_JtPipeVerticalThreshold)
        {
            if (pipeDirection.z > 0.0f)
                spin = pjt->fPipeReverse != 0;
            else
                spin = pjt->fPipeReverse == 0;
        }

        if (!spin && pjt->uPipeClimbSmooth > 0.4 && pjt->fPipeReverse && pipeDirection.z > -0.3)
            spin = true;

        if (!spin && pjt->uPipeClimbSmooth < -0.4 && !pjt->fPipeReverse && pipeDirection.z < 0.3)
            spin = true;

        if (spin)
        {
            SetJtJts(pjt, JTS_Pipe, JTBS_Pipe_Spin);
            return;
        }
    }

    /*
     * Determine the pipe's apparent direction on screen. When a control
     * rope exists, its curve is used instead of the pipe curve.
     */
    glm::vec3 posTopScreen(0.0f);
    glm::vec3 posBottomScreen(0.0f);

    if (ppipe->pshape == nullptr)
    {
        ConvertCmWorldToScreen(g_pcm, &posTop, &posTopScreen);
        ConvertCmWorldToScreen(g_pcm, &posBottom, &posBottomScreen);
    }
    else
    {
        SHAPE* pshapeRope = ppipe->pshape;
        CRV* pcrvRope = pshapeRope->pcrv.get();

        glm::vec3 posJtLocal(0.0f);
        glm::vec3 posRopeLocal(0.0f);
        float sRope = 0.0f;

        ConvertAloPos(nullptr, pshapeRope->paloParent, &pjt->xf.posWorld, &posJtLocal);

        if (pcrvRope->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
            pcrvRope->pvtcrv->pfnFindCrvClosestPointAll(pcrvRope, &posJtLocal, nullptr, nullptr, nullptr, nullptr, &sRope);

        const float sMaxRope = pcrvRope->pvtcrv->pfnSMaxCrv(pcrvRope);
        sRope = std::clamp(sRope, DS_JtPipeNormalGap, sMaxRope - DS_JtPipeNormalGap);

        if (pcrvRope->pvtcrv->pfnEvaluateCrvFromS != nullptr)
            pcrvRope->pvtcrv->pfnEvaluateCrvFromS(pcrvRope, sRope + DS_JtPipeNormalGap, &posRopeLocal, nullptr);

        ConvertAloPos(pshapeRope->paloParent, nullptr, &posRopeLocal, &posTop);
        ConvertCmWorldToScreen(g_pcm, &posTop, &posTopScreen);

        if (pcrvRope->pvtcrv->pfnEvaluateCrvFromS != nullptr)
            pcrvRope->pvtcrv->pfnEvaluateCrvFromS(pcrvRope, sRope - DS_JtPipeNormalGap, &posRopeLocal, nullptr);

        ConvertAloPos(pshapeRope->paloParent, nullptr, &posRopeLocal, &posBottom);
        ConvertCmWorldToScreen(g_pcm, &posBottom, &posBottomScreen);
    }

    glm::vec3 screenDirection = posTopScreen - posBottomScreen;
    screenDirection.z = 0.0f;

    if (glm::length(screenDirection) < 0.0001f)
        screenDirection = glm::vec3(0.0f, 1.0f, 0.0f);
    else
        screenDirection = glm::normalize(screenDirection);

    int jtbsNext = pjt->jtbs;
    float uClimb = 0.0f;
    int fPipeReverse = pjt->fPipeReverse;
    const bool fKeyboardUp = pjoy->IsHeld(BTN_UP) && !pjoy->IsHeld(BTN_DOWN);
    const bool fKeyboardDown = pjoy->IsHeld(BTN_DOWN) && !pjoy->IsHeld(BTN_UP);

    if (pjt->jtpdk == JTPDK_Up)
    {
        jtbsNext = JTBS_Pipe_Up;
        uClimb = 1.0f;
    }
    else if (pjt->jtpdk == JTPDK_Down)
    {
        jtbsNext = JTBS_Pipe_Down;
        uClimb = 1.0f;
    }
    else if (fKeyboardUp)
    {
        jtbsNext = JTBS_Pipe_Up;
        uClimb = 1.0f;
        pjt->jtMoveDir = 0;
    }
    else if (fKeyboardDown)
    {
        jtbsNext = JTBS_Pipe_Down;
        uClimb = 1.0f;
        pjt->jtMoveDir = 1;
    }
    else
    {
        const glm::vec2 joy(pjoy->x, pjoy->y);
        const float joyDeflect = glm::length(joy);
        const glm::vec2 joyDirection = joyDeflect < 0.0001f ? glm::vec2(0.0f) : joy / joyDeflect;
        const float dotJoy = glm::dot(glm::vec2(screenDirection), joyDirection);

        if (pjt->jtMoveDir == 0)
        {
            if (joyDeflect < 0.25f)
                pjt->jtMoveDir = -1;
            else if (dotJoy < -0.7f)
                pjt->jtMoveDir = 1;
        }
        else if (pjt->jtMoveDir == -1)
        {
            if (joyDeflect > 0.35f)
                pjt->jtMoveDir = dotJoy < 0.0f ? 1 : 0;
        }
        else if (pjt->jtMoveDir == 1)
        {
            if (joyDeflect < 0.25f)
                pjt->jtMoveDir = -1;
            else if (dotJoy > 0.7f)
                pjt->jtMoveDir = 0;
        }

        if (pjt->jtMoveDir == 0)
        {
            jtbsNext = fPipeReverse ? JTBS_Pipe_Down : JTBS_Pipe_Up;
            uClimb = joyDeflect;
        }
        else if (pjt->jtMoveDir == 1)
        {
            jtbsNext = fPipeReverse ? JTBS_Pipe_Up : JTBS_Pipe_Down;
            uClimb = joyDeflect;
        }
        else
        {
            jtbsNext = JTBS_Pipe_Stay;
            uClimb = 0.0f;
        }
    }

    float svPipe = 0.0f;
    const float svFast = pjt->ablcl[1].sv;

    if (uClimb > 0.01f)
    {
        svPipe = (1.0f - uClimb) * pjt->ablcl[0].sv + uClimb * svFast;

        if (jtbsNext == JTBS_Pipe_Down)
            svPipe = -svPipe;
    }
    else
    {
        jtbsNext = JTBS_Pipe_Stay;
    }

    if (fPipeReverse)
        svPipe = -svPipe;

    const float uSmooth = 1.0 * g_clock.dt;
    pjt->uPipeClimbSmooth = uSmooth * (-svPipe / std::abs(svFast)) + (1.0f - uSmooth) * pjt->uPipeClimbSmooth;

    if (!pcrv->fClosed)
    {
        SMP smpPipe
        {
            std::abs(svFast),
            0.0f,
            0.2f
        };

        const float sMin = pcrv->mpicvs[0];
        const float sMax = pcrv->mpicvs[pcrv->ccv - 1];

        const float dsStart = fPipeReverse ? DS_JtPipeLimitHand : DS_JtPipeLimitFoot;
        const float dsEnd = fPipeReverse ? DS_JtPipeLimitFoot : DS_JtPipeLimitHand;

        float svMin = 0.0f;
        float svMax = 0.0f;

        GSmooth(pjt->sPipeHand, sMin + dsStart, g_clock.dt, &smpPipe, &svMin);
        GSmooth(pjt->sPipeHand, sMax - dsEnd, g_clock.dt, &smpPipe, &svMax);

        svPipe = std::clamp(svPipe, -svMax, -svMin);
    }

    pjt->dsPipeHand = -svPipe;
    pjt->sPipeHand -= svPipe * g_clock.dt;

    if (pcrv->fClosed)
        pjt->sPipeHand = GModPositive(pjt->sPipeHand, pcrv->pvtcrv->pfnSMaxCrv(pcrv));

    if (std::abs(svPipe) < 0.01f)
    {
        jtbsNext = JTBS_Pipe_Stay;
        SetJtJtpdk(pjt, JTPDK_Nil);
    }
    else
    {
        const float svSlow = std::abs(pjt->ablcl[0].sv);
        const float uBlend = std::clamp((std::abs(svPipe) - svSlow) / (std::abs(pjt->ablcl[1].sv) - svSlow), 0.0f, 1.0f);

        pjt->ablcl[0].u = 1.0f - uBlend;
        pjt->ablcl[1].u = uBlend;

        ReblendAsegbl(
            pjt->pasegblClimb,
            static_cast<int>(sizeof(BLCL)),
            static_cast<int>(std::size(pjt->ablcl)),
            pjt->ablcl);
    }

    SetJtJts(pjt, JTS_Pipe, jtbsNext);
}

void UpdateJtInternalXpsPipe(JT* pjt)
{
    PIPE* ppipe = pjt->ppipeCur;
    ALO* paloPipe = ppipe->paloParent;

    /*
     * Use the JT's internal constraint list by default. If the pipe belongs
     * to a dynamic object, locate the OX connecting JT to that object.
     */
    OX* pox = reinterpret_cast<OX*>(&pjt->pxpInternal);
    OX* poxOther = pox;

    if (paloPipe != nullptr && (paloPipe->pvtlo->grfcid & 2) != 0)
    {
        for (pox = pjt->poxa->pox; pox != nullptr && pox->psoOther != reinterpret_cast<SO*>(paloPipe->paloRoot); pox = pox->poxNext)
        {
        }

        if (pox == nullptr)
            return;

        poxOther = pox->poxOther;
    }

    CRV* pcrv = ppipe->pcrv.get();

    glm::vec3 posPipeLocal(0.0f);
    glm::vec3 tangentPipeLocal(0.0f);

    if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
        pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeHand, &posPipeLocal, &tangentPipeLocal);

    glm::vec3 posPipe(0.0f);
    glm::vec3 tangentPipe(0.0f);

    ConvertAloPos(paloPipe, nullptr, &posPipeLocal, &posPipe);
    ConvertAloVec(paloPipe, nullptr, &tangentPipeLocal, &tangentPipe);

    if (glm::length(tangentPipe) < 0.0001f)
        tangentPipe = glm::vec3(0.0f, 0.0f, 1.0f);
    else
        tangentPipe = glm::normalize(tangentPipe);

    glm::vec3 normalPipe[2]{};

    GetNormalVectors(&tangentPipe, normalPipe, &g_normalX, &g_normalZ);

    glm::vec3 posHand(0.0f);
    ConvertAloPos(pjt, nullptr, &pjt->posPipeHandLocal, &posHand);

    const glm::vec3 dpos = posHand + pjt->dposPipeXp - posPipe;

    PxpAdd(pjt, reinterpret_cast<SO*>(paloPipe), &posHand, &normalPipe[0], glm::dot(dpos, normalPipe[0]), XPK_Equality, nullptr, nullptr, &pox->pxp);
    PxpAdd(pjt, reinterpret_cast<SO*>(paloPipe), &posHand, &normalPipe[1], glm::dot(dpos, normalPipe[1]), XPK_Equality, nullptr, nullptr, &pox->pxp);
    PxpAdd(pjt, reinterpret_cast<SO*>(paloPipe), &posHand, &tangentPipe,   glm::dot(dpos, tangentPipe),   XPK_Equality, nullptr, nullptr, &pox->pxp);

    poxOther->pxp = pox->pxp;
}

void SetJtJtpdk(JT* pjt, int jtpdk)
{
    if (pjt->jtpdk != jtpdk) {
        if (pjt->jtpdk == -1) {
            IncrementSwHandsOff(pjt->psw);
            AcquireSwLetterbox(pjt->psw);
        }
        pjt->jtpdk = (JTPDK)jtpdk;
        if (jtpdk == -1) {
            DecrementSwHandsOff(pjt->psw);
            ReleaseSwLetterbox(pjt->psw);
        }
    }
}

void PlaceJtOnPipe(JT* pjt, PIPE* ppipe)
{
    constexpr int JTS_Pipe = 7;
    constexpr int JTBS_Pipe_Init = 23;

    if (pjt->ppipeCur != ppipe)
    {
        if (pjt->jts == JTS_Pipe)
        {
            HandleLoSpliceEvent(pjt->ppipeCur, 3, 0, nullptr);
            pjt->ppipeCur->pvtlo->pfnUnsubscribeLoObject(pjt->ppipeCur, pjt);

            HandleLoSpliceEvent(ppipe, 2, 0, nullptr);
            ppipe->pvtlo->pfnSubscribeLoObject(ppipe, pjt);
        }

        pjt->ppipeCur = ppipe;
        pjt->phndCur = nullptr;
        pjt->ptargetCur = nullptr;

        CRV* pcrv = ppipe->pcrv.get();
        glm::vec3 posJtPipeLocal(0.0f);

        ConvertAloPos(nullptr, ppipe->paloParent, &pjt->xf.posWorld, &posJtPipeLocal);

        if (pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
            pcrv->pvtcrv->pfnFindCrvClosestPointAll(pcrv, &posJtPipeLocal, nullptr, nullptr, nullptr, nullptr, &pjt->sPipeCur);
    }

    CRV* pcrv = ppipe->pcrv.get();
    glm::mat3 matGoal(1.0f);

    GetJtPipeGoalMat(pjt, ppipe, pjt->sPipeCur, &matGoal);
    pjt->pvtalo->pfnRotateAloToMat(pjt, &matGoal);

    glm::vec3 posPipeLocal(0.0f);

    if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
        pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, pjt->sPipeCur, &posPipeLocal, nullptr);

    glm::vec3 posPipeJt(0.0f);
    ConvertAloPos(ppipe->paloParent, pjt, &posPipeLocal, &posPipeJt);

    posPipeJt -= pjt->posPipeHandLocal;

    glm::vec3 posJtWorld(0.0f);
    ConvertAloPos(pjt, nullptr, &posPipeJt, &posJtWorld);

    pjt->pvtalo->pfnTranslateAloToPos(pjt, &posJtWorld);

    SetJtJts(pjt, JTS_Pipe, JTBS_Pipe_Init);
}
