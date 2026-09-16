#include "stephang.h"
#include "jt.h"
#include "hnd.h"
#include "pipe.h"
#include "bbmark.h"
#include "actval.h"

void PostJtLoadSwing(JT* pjt, BL* ablSwing, ASEGBL** ppasegbl)
{
    constexpr int cbl = 3;
    const int cbBl = static_cast<int>(sizeof(BL));

    ablSwing[0].u = 0.0f;
    ablSwing[1].u = 0.0f;
    ablSwing[2].u = 1.0f;

    ASEGBL* pasegbl = nullptr;

    EnsureAsegBlendDynamic(pjt, cbBl, cbl, ablSwing, 0, nullptr, nullptr, &pasegbl);
    ReblendAsegbl(pasegbl, cbBl, cbl, ablSwing);

    if (ppasegbl != nullptr)
        *ppasegbl = pasegbl;
}

void AnticipateJtForce(JT* pjt, SO* psoOther, glm::vec3* pposOther, glm::vec3* pdv, FX* afx)
{
    SO* apso[2] =
    {
        static_cast<SO*>(pjt),
        psoOther
    };

    glm::vec3 apos[2] =
    {
        pjt->xf.posWorld,
        *pposOther
    };

    const glm::vec3 basis[3] =
    {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };

    FX basisEffects[2][3]{};
    glm::vec3 relativeVelocityEffects[3]{};

    for (int axis = 0; axis < 3; ++axis)
    {
        glm::vec3 pointVelocityEffects[2]{};

        for (int body = 0; body < 2; ++body)
        {
            SO* pso = apso[body];
            FX& effect = basisEffects[body][axis];

            const glm::vec3 force = body == 0 ? basis[axis] : -basis[axis];
            const glm::vec3 offset = apos[body] - pso->xf.posWorld;
            glm::vec3 torque = glm::cross(offset, force);
            glm::vec3 dv = force / pso->m;

            glm::vec3 dw{};
            CalculateSoAngularEffectWorld(pso, &torque, &dw);

            ApplySoConstraintWorld(pso, &pso->constrForce, &dv, &effect.dv, nullptr);
            ApplySoConstraintWorld(pso, &pso->constrTorque, &dw, &effect.dw, nullptr);
            CalculateSoEffect(pso, pposOther, &effect, &pointVelocityEffects[body]);
        }

        relativeVelocityEffects[axis] = pointVelocityEffects[0] - pointVelocityEffects[1];
    }

    float responseMatrix[3][3]{};
    float requestedVelocity[3] =
    {
        pdv->x,
        pdv->y,
        pdv->z
    };
    float croutMatrix[3][3]{};
    float forceScale[3]{};

    for (int axis = 0; axis < 3; ++axis)
    {
        responseMatrix[axis][0] = relativeVelocityEffects[axis].x;
        responseMatrix[axis][1] = relativeVelocityEffects[axis].y;
        responseMatrix[axis][2] = relativeVelocityEffects[axis].z;
    }

    CroutDecomp(3, responseMatrix[0], croutMatrix[0]);
    CroutSolve(3, croutMatrix[0], requestedVelocity, forceScale);

    for (int body = 0; body < 2; ++body)
    {
        afx[body].dv = glm::vec3(0.0f);
        afx[body].dw = glm::vec3(0.0f);

        for (int axis = 0; axis < 3; ++axis)
        {
            afx[body].dv += basisEffects[body][axis].dv * forceScale[axis];
            afx[body].dw += basisEffects[body][axis].dw * forceScale[axis];
        }

        ConvertAloVec(nullptr, apso[body]->paloParent, &afx[body].dv, &afx[body].dv);
        ConvertAloVec(nullptr, apso[body]->paloParent, &afx[body].dw, &afx[body].dw);
    }
}

void CalculateJtHangAccel(JT* pjt)
{
    glm::vec3 posHnd{};
    glm::mat3 matHnd{ 1.0f };
    glm::vec3 vHnd{ 0.0f };

    GetXfmPos(static_cast<XFM*>(pjt->phndCur), &posHnd);
    GetXfmMat(static_cast<XFM*>(pjt->phndCur), &matHnd);
    CalculateAloMovement(pjt->phndCur->paloParent, nullptr, posHnd, &vHnd, nullptr, nullptr, nullptr);

    const glm::vec3 dposHnd = pjt->xf.posWorld - posHnd;
    const float radiusCur = glm::length(dposHnd);
    glm::vec3 normalHnd = radiusCur < 0.0001f ? g_normalZ : dposHnd / radiusCur;

    glm::vec3 relativeVelocity = pjt->xf.v - vHnd;

    glm::vec3 vNormal{};
    glm::vec3 vTangent{};
    const float radialVelocity = SProjectVector(&relativeVelocity, &normalHnd, &vNormal, &vTangent);

    glm::vec3 normalW = glm::cross(normalHnd, vTangent);
    const float normalWLength = glm::length(normalW);

    if (normalWLength >= 0.0001f)
        normalW /= normalWLength;
    else
        normalW = g_normalX;

    const glm::vec3 gravityTangent = pjt->dvGravity - normalHnd * glm::dot(pjt->dvGravity, normalHnd);

    glm::vec3 normalDw = glm::cross(normalHnd, gravityTangent);
    const float normalDwLength = glm::length(normalDw);

    if (normalDwLength >= 0.0001f)
        normalDw /= normalDwLength;
    else
        normalDw = g_normalY;

    const glm::vec3 w = normalW * (glm::length(vTangent) / radiusCur);
    const glm::vec3 dw = normalDw * (glm::length(gravityTangent) / radiusCur);

    glm::vec3 wNext = w + dw * g_clock.dt;

    glm::vec3 wAxis{};
    glm::vec3 wTangent{};
    SProjectVector(&wNext, &matHnd[1], &wAxis, &wTangent);

    const bool fActivePump = pjt->jtbs == 50 || pjt->jtbs == 52;
    const float tangentDamping = fActivePump ? R_JtHangDampingActive : R_JtHangDampingNeutral;

    wNext = wAxis * (R_JtHangAxisDamping * g_clock.dt + 1.0f) + wTangent * (tangentDamping * g_clock.dt + 1.0f);

    if (fActivePump && g_clock.t - pjt->tJtbs < DT_JtHangPump)
    {
        float pump = g_clock.dt * SDW_JtHangPump;

        if (pjt->jtbs == 50)
            pump = -pump;

        if (pjt->fHangReverse != 0)
            pump = -pump;

        const float gravityMagnitude = -pjt->dvGravity.z;
        const float tangentSpeed = glm::length(vTangent);
        const float dzPeak = (gravityMagnitude * dposHnd.z + tangentSpeed * tangentSpeed * 0.5f) / gravityMagnitude;
        const float pumpScale = glm::clamp(s_clqDzToU.g0 + dzPeak * (s_clqDzToU.g1 + dzPeak * s_clqDzToU.g2), 0.0f, 1.0f);

        wNext += matHnd[1] * (pump * pumpScale);
    }

    float radialVelocityNext = 0.0f;
    const float radiusNext = GSmoothA(radiusCur, radialVelocity, S_JtHangMax - pjt->phndCur->sRadiusHook, g_clock.dt, &s_smpaHang, &radialVelocityNext);
    const float radiusScale = std::min(S_JtHangMax, radiusCur) / std::max(S_JtHangMin, radiusNext);

    wNext *= radiusScale;

    glm::vec3 vHndNext = vHnd;

    if (pjt->pxaHang != nullptr)
    {
        SO* psoTarget = pjt->pxaHang->psoTarget;
        glm::vec3 positionAcceleration{ 0.0f };
        glm::vec3 angularAcceleration{ 0.0f };

        ACT* pactPos = psoTarget->pactPos;

        if (pactPos != nullptr && pactPos->ackPos == ACK_Spring)
        {
            glm::vec3 posGoal{};
            glm::vec3 vGoal{};

            pactPos->pvtact->pfnGetActPositionGoal(pactPos, 0, &posGoal, &vGoal);
            CalculateAloPositionSpring(static_cast<ALO*>(psoTarget), g_clock.dt, &posGoal, &vGoal, &positionAcceleration);
            ConvertAloVec(psoTarget->paloParent, nullptr, &positionAcceleration, &positionAcceleration);
        }

        ACT* pactRot = psoTarget->pactRot;

        if (pactRot != nullptr && pactRot->ackRot == ACK_Spring)
        {
            glm::mat3 matGoal{ 1.0f };
            glm::vec3 wGoal{ 0.0f };

            pactRot->pvtact->pfnGetActRotationGoal(pactRot, 0, &matGoal, &wGoal);
            CalculateAloRotationSpring(psoTarget, g_clock.dt, &matGoal, &wGoal, &angularAcceleration);
            ConvertAloVec(psoTarget->paloParent, nullptr, &angularAcceleration, &angularAcceleration);
        }

        const glm::vec3 targetOffset = posHnd - psoTarget->xf.posWorld;
        vHndNext = vHnd + positionAcceleration * g_clock.dt + glm::cross(angularAcceleration, targetOffset) * g_clock.dt;
    }

    glm::vec3 rotationStep = (w + wNext) * (g_clock.dt * 0.5f);

    glm::mat3 rotationMatrix{ 1.0f };
    LoadRotateMatrix(&rotationStep, &rotationMatrix);

    const glm::vec3 normalNext = rotationMatrix * normalHnd;
    const glm::vec3 radialOffsetNext = normalNext * radiusNext;
    const glm::vec3 velocityNext = vHndNext + glm::cross(wNext, radialOffsetNext) + normalNext * radialVelocityNext;
    glm::vec3 acceleration = (velocityNext - pjt->xf.v) / g_clock.dt - pjt->dvGravity;

    if (pjt->pxaHang == nullptr)
        pjt->afxHang[0].dv = acceleration;
    else
        AnticipateJtForce(pjt, pjt->pxaHang->psoTarget, &posHnd, &acceleration, pjt->afxHang);
}

void PresetJtAccelHang(JT* pjt)
{
    AddSoAcceleration(pjt, &pjt->afxHang[0].dv);

    glm::vec3 posHnd{};
    GetXfmPos(pjt->phndCur, &posHnd);

    const glm::vec3 dposHnd = posHnd - pjt->xf.posWorld;

    glm::mat3 matTilt{ 1.0f };
    glm::mat3 matSpin{ 1.0f };

    BuildRotateVectorsMatrix(&g_normalZ, &dposHnd, &matTilt);
    LoadRotateMatrixRad(pjt->radTarget, &g_normalZ, &matSpin);

    glm::mat3 mat = matTilt * matSpin;

    AccelSoTowardMatSmooth(pjt, g_clock.dt, &mat, &s_smpMat);
}

void AddJtExternalAccelerations(JT* pjt, XA* pxa, float dt)
{
    AddSoAcceleration(pjt->pxaHang->psoTarget, &pjt->afxHang[1].dv);
    AddSoAngularAcceleration(pjt->pxaHang->psoTarget, &pjt->afxHang[1].dw);
}

void UpdateJtActiveHang(JT* pjt, JOY* pjoy)
{
    bool fForward = false;
    bool fBackward = false;

    GetJtHangHeading(pjt, &pjt->radTarget);

    if (pjt->fHangReverse != 0)
        pjt->radTarget = RadNormalize(pjt->radTarget + 3.1415927f);

    float joyAdjusted[4]{};
    AdjustCmJoy(g_pcm, pjoy, JOYID_Left, joyAdjusted);

    const float radJoyRelative = RadNormalize(joyAdjusted[0] - pjt->radTarget);

    if (pjt->uJoyDeflect > 0.3f)
    {
        if (std::abs(radJoyRelative) < 1.0471976f)
            fForward = true;
        else if (std::abs(radJoyRelative) > 2.0943952f)
            fBackward = true;

        if (fForward || fBackward)
            pjt->tHangDeflectLast = g_clock.t;
    }
    else
    {
        pjt->jtbsHangLatch = (JTBS)-1;
    }

    glm::vec3 posHnd{};
    GetXfmPos(static_cast<XFM*>(pjt->phndCur), &posHnd);

    const glm::vec3 dposHnd = posHnd - pjt->xf.posWorld;
    const float dposLength = glm::length(dposHnd);
    const glm::vec3 normalHnd = dposLength < 0.0001f ? g_normalZ : dposHnd / dposLength;

    glm::mat3 matHndLocal{ 1.0f };
    LoadRotateMatrixRad(pjt->radTarget, &g_normalZ, &matHndLocal);

    const glm::vec3 normalHndLocal = glm::transpose(matHndLocal) * normalHnd;
    const float radSwing = RadNormalize(std::atan2(normalHndLocal.z, normalHndLocal.x) - 1.5707964f);
    const float speed = glm::length(pjt->xf.v);

    int jtbsNext = pjt->jtbs;

    switch (pjt->jtbs)
    {
        case 50:
        if (fBackward)
            jtbsNext = 52;
        else if (fForward)
        {
            if (g_clock.t - pjt->tJtbs > DT_HangForwardRelax)
            {
                pjt->jtbsHangLatch = (JTBS)50;
                jtbsNext = 51;
            }
        }
        else if (g_clock.t - pjt->tHangDeflectLast > DT_HangLastNeutral)
            jtbsNext = 51;
        break;

        case 51:
        if (fForward && pjt->jtbsHangLatch != 50)
        {
            pjt->jtbsHangLatch = (JTBS)-1;
            jtbsNext = 50;
        }
        else if (fBackward && pjt->jtbsHangLatch != 52)
        {
            pjt->jtbsHangLatch = (JTBS)-1;
            jtbsNext = 52;
        }
        else if (std::abs(radSwing) < s_lmRadIdle.gMin && speed < s_lmSvIdle.gMin)
            jtbsNext = 53;
        break;

        case 52:
        if (fForward)
            jtbsNext = 50;
        else if (fBackward)
        {
            if (g_clock.t - pjt->tJtbs > DT_HangBackwardRelax)
            {
                pjt->jtbsHangLatch = (JTBS)52;
                jtbsNext = 51;
            }
        }
        else if (g_clock.t - pjt->tHangDeflectLast > DT_HangLastNeutral)
            jtbsNext = 51;
        break;

        case 53:
        if (fForward && pjt->jtbsHangLatch != 50)
        {
            pjt->jtbsHangLatch = (JTBS)-1;
            jtbsNext = 50;
        }
        else if (fBackward && pjt->jtbsHangLatch != 52)
        {
            pjt->jtbsHangLatch = (JTBS)-1;
            jtbsNext = 52;
        }
        else if (std::abs(radSwing) > s_lmRadIdle.gMax && speed > s_lmSvIdle.gMax)
            jtbsNext = 51;
        break;
    }

    SetJtJts(pjt, 3, jtbsNext);
    CalculateJtHangAccel(pjt);

    if (pjt->pxaHang != nullptr)
        ResolveAlo(pjt->pxaHang->psoTarget);

    if (pjt->jtbs == 53)
        return;

    float uSwingActiveTarget = pjt->uSwingActive;
    float uSwingForwardTarget = pjt->uSwingForward;

    switch (pjt->jtbs)
    {
        case 50:
        uSwingActiveTarget = 1.0f;
        uSwingForwardTarget = 1.0f;
        break;

        case 51:
        uSwingActiveTarget = 0.0f;
        uSwingForwardTarget = pjt->uSwingForward > 0.5f ? 1.0f : 0.0f;
        break;

        case 52:
        uSwingActiveTarget = 1.0f;
        uSwingForwardTarget = 0.0f;
        break;
    }

    pjt->uSwingActive = GSmooth(pjt->uSwingActive, uSwingActiveTarget, g_clock.dt, &s_smpSwingActive, nullptr);
    pjt->uSwingForward = GSmooth(pjt->uSwingForward, uSwingForwardTarget, g_clock.dt, &s_smpSwingForward, nullptr);

    pjt->ablSwing[0].u = pjt->uSwingActive * pjt->uSwingForward;
    pjt->ablSwing[1].u = pjt->uSwingActive * (1.0f - pjt->uSwingForward);
    pjt->ablSwing[2].u = 1.0f - pjt->uSwingActive;

    ReblendAsegbl(pjt->pasegblSwing, static_cast<int>(sizeof(BL)), 3, pjt->ablSwing);

    const float uSwing = glm::clamp(radSwing / RAD_SwingMax, -1.0f, 1.0f);
    const float tTarget = pjt->pasegblSwing->tMax * (uSwing + 1.0f) * 0.5f;
    const float svtTarget = (tTarget - pjt->pasegaCur->tLocal) / g_clock.dt;

    pjt->pasegaCur->svtLocal = GLimitAbs(svtTarget, 3.0);
}

void CheckJtUnhook(JT* pjt)
{
    constexpr float S_UNHOOK_MAX = 50.0f;
    constexpr float S2_UNHOOK_MAX = S_UNHOOK_MAX * S_UNHOOK_MAX;

    if (pjt->phndUnhook != nullptr)
    {
        glm::vec3 posHndWorld{};
        ConvertAloPos(pjt->phndUnhook->paloParent, nullptr, &pjt->phndUnhook->posLocal, &posHndWorld);

        if (glm::distance(pjt->paloHook->xf.posWorld, posHndWorld) > S_UNHOOK_MAX)
        {
            pjt->grfpvaXpValid = 0;
            pjt->phndUnhook = nullptr;
        }

        return;
    }

    if (pjt->ppipeUnhook != nullptr)
    {
        glm::vec3 posHookLocal{};
        glm::vec3 posClosestLocal{};

        ConvertAloPos(nullptr, pjt->ppipeUnhook->paloParent, &pjt->paloHook->xf.posWorld, &posHookLocal);

        CRV* pcrv = pjt->ppipeUnhook->pcrv.get();
        if (pcrv->pvtcrv->pfnFindCrvClosestPointFromS != nullptr)
            pcrv->pvtcrv->pfnFindCrvClosestPointFromS(pcrv, &posHookLocal, pjt->sPipeUnhook, nullptr, &posClosestLocal, nullptr, nullptr, &pjt->sPipeUnhook);

        if (glm::dot(posClosestLocal, posClosestLocal) > S2_UNHOOK_MAX)
            pjt->ppipeUnhook = nullptr;
    }
}

void UpdateJtHookOx(JT* pjt, LO* ploHook, int fHook)
{
    constexpr OXF OXF_Hook = static_cast<OXF>(0x40);

    while (ploHook != nullptr && (ploHook->pvtlo->grfcid & 2U) == 0)
        ploHook = reinterpret_cast<LO*>(static_cast<ALO*>(ploHook)->paloParent);

    if (ploHook != nullptr && FIsLoInWorld(ploHook))
    {
        SO* psoHook = static_cast<SO*>(ploHook);
        SO* psoHookRoot = reinterpret_cast<SO*>(psoHook->paloRoot);

        if (psoHookRoot != nullptr && pjt->poxa != nullptr && psoHookRoot->poxa != nullptr)
        {
            const OXF oxfAdd = fHook != 0 ? OXF_Hook : static_cast<OXF>(0);
            const OXF oxfRemove = fHook != 0 ? static_cast<OXF>(0) : OXF_Hook;

            UpdateSwPox(pjt->psw, pjt->poxa, psoHookRoot->poxa, oxfAdd, oxfRemove);
        }
    }

    pjt->grfpvaXpValid = 0;
}

void AddJtHookXps(JT* pjt, GRFAHX grfahx, LO* ploHook, glm::vec3* pposHook, glm::vec3* pvecHook, glm::vec3* pposHang)
{
    if (!FIsLoInWorld(ploHook))
    {
        SetJtJts(pjt, 2, 2);
        return;
    }

    const bool fEqualityX = (grfahx & 0x01) != 0;
    const bool fEqualityY = (grfahx & 0x02) != 0;
    const bool fEqualityZ = (grfahx & 0x04) != 0;
    const bool fInternalX = (grfahx & 0x08) != 0;
    const bool fInternalY = (grfahx & 0x10) != 0;
    const bool fInternalZ = (grfahx & 0x20) != 0;

    OX* poxDestination = reinterpret_cast<OX*>(&pjt->pxpInternal);
    OX* poxSource = poxDestination;
    SO* psoHook = nullptr;

    for (ALO* paloHook = static_cast<ALO*>(ploHook); paloHook != nullptr; paloHook = paloHook->paloParent)
    {
        if ((paloHook->pvtalo->grfcid & 2) == 0)
            continue;

        psoHook = static_cast<SO*>(paloHook);

        for (poxSource = pjt->poxa->pox; poxSource != nullptr && poxSource->psoOther != psoHook->paloRoot; poxSource = poxSource->poxNext)
        {

        }

        poxDestination = poxSource->poxOther;
        break;
    }

    glm::mat3 matHook(1.0f);

    if (pvecHook != nullptr)
        BuildOrthonormalMatrixZ(pvecHook, &g_normalZ, &matHook);

    const glm::vec3 dposWorld = (*pposHang + pjt->dposPipeXp) - *pposHook;
    const glm::vec3 dposLocal = glm::transpose(matHook) * dposWorld;

    if (fEqualityX || (fInternalX && dposLocal.x < 0.0001f))
    {
        const XPK xpk = fEqualityX ? XPK_Equality : XPK_Internal;
        PxpAdd(pjt, psoHook, pposHang, &matHook[0], dposLocal.x, xpk, nullptr, nullptr, &poxSource->pxp);
    }

    if (fEqualityY || (fInternalY && dposLocal.y < 0.0001f))
    {
        const XPK xpk = fEqualityY ? XPK_Equality : XPK_Internal;
        PxpAdd(pjt, psoHook, pposHang, &matHook[1], dposLocal.y, xpk, nullptr, nullptr, &poxSource->pxp);
    }

    if (fEqualityZ || (fInternalZ && dposLocal.z < 0.0001f))
    {
        const XPK xpk = fEqualityZ ? XPK_Equality : XPK_Internal;
        void* pv0 = fEqualityZ ? nullptr : pjt->pposBase;
        PxpAdd(pjt, psoHook, pposHang, &matHook[2], dposLocal.z, xpk, pv0, nullptr, &poxSource->pxp);
    }

    poxDestination->pxp = poxSource->pxp;
}

void GetJtHangHeading(JT* pjt, float* pradForward)
{
    glm::vec3 normal{};
    ConvertAloVec(pjt->phndCur->paloParent, nullptr, &pjt->phndCur->matLocal[0], &normal);
    *pradForward = std::atan2(normal.y, normal.x);
}

void UpdateJtIkHang(JT* pjt, glm::vec3* pposHand, glm::mat3* pmatHand, glm::mat3* pmatTool)
{
    ALO* paloShoulder = pjt->paloRightHand->palox->ikh.paloShoulder;

    glm::vec3 posHandle;
    glm::vec3 posShoulder;

    ConvertAloPos(pjt->phndCur->paloParent, pjt, &pjt->phndCur->posLocal, &posHandle);
    ConvertAloPos(nullptr, pjt, &paloShoulder->xf.posWorld, &posShoulder);

    const glm::vec3 dposHandle = posHandle - posShoulder;

    glm::mat3 matTilt;
    BuildRotateVectorsMatrix(&g_normalX, &dposHandle, &matTilt);

    const float distance = glm::length(dposHandle);
    const glm::vec3 normalHandle = distance < 0.0001f ? g_vecZero : dposHandle / distance;

    posHandle += normalHandle * pjt->phndCur->sRadiusHook;

    glm::mat3 matWrist;
    LoadRotateMatrixRad(RAD_JtHangWrist, &g_normalX, &matWrist);

    *pmatHand = matTilt * matWrist;

    LoadRotateMatrixRad(RAD_JtHangTool, &g_normalX, pmatTool);

    const glm::vec3 posHookFromTool = pjt->paloTool->xf.pos + (*pmatTool * pjt->paloHook->xf.pos);
    const glm::vec3 posHookFromHand = pjt->paloRightHand->xf.pos + (*pmatHand * posHookFromTool);
    const glm::vec3 dposCorrection = posHandle - posHookFromHand;

    *pposHand = pjt->paloRightHand->xf.pos + dposCorrection;
}

void UpdateJtHangIkGoals(JT* pjt)
{
    glm::vec3 posHangHand;
    glm::mat3 matHangHand;
    glm::mat3 matHangTool;

    UpdateJtIkHang(pjt, &posHangHand, &matHangHand, &matHangTool);

    pjt->dposHangHand = PosSmooth(pjt->dposHangHand, g_vecZero, g_clock.dt, &s_smpHangIkPos, nullptr);

    SmoothMatrix(&pjt->dmatHangHand, &g_matIdentity, &s_smpHangIkRot, g_clock.dt, &pjt->dmatHangHand, nullptr);
    SmoothMatrix(&pjt->dmatHangTool, &g_matIdentity, &s_smpHangIkRot, g_clock.dt, &pjt->dmatHangTool, nullptr);

    pjt->pactvalHangHand->posGoal = posHangHand + pjt->dposHangHand;
    pjt->pactvalHangHand->matGoal = pjt->dmatHangHand * matHangHand;
    pjt->pactvalHangTool->matGoal = pjt->dmatHangTool * matHangTool;

    SnapAct(pjt->pactvalHangHand, 1);
    SnapAct(pjt->pactvalHangTool, 1);
}

void PredictJtRotation(JT* pjt, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    if (pjt->jts == 3)
        PredictAloRotation(pjt, dtOffset, pmat, pw);
    else
        PredictStepRotation(pjt, dtOffset, pmat, pw);
}
