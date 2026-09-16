#include "act.h"
#include "so.h"
#include "bez.h"
#include "jt.h"
#include "actseg.h"
#include "asega.h"
#include <cfloat>
#include <cstdio>

std::shared_ptr <ACT> PactNew(SW* psw, ALO* palo, VTACT* pvtact)
{
    auto pact = std::make_shared<ACT>();

    pact->pvtact = pvtact;
    pact->pvtact->pfnInitAct(pact.get(), palo);

    psw->actOwners.push_back(pact);

    return pact;
}

std::shared_ptr <ACT> PactNewClone(ACT* pactBase, SW* psw, ALO* palo)
{
    auto pact = PactNew(psw, palo, &g_vtact);
    pact->pvtact->pfnCloneAct(pact.get(), pactBase);

    return pact;
}

void InitAct(ACT* pact, ALO* palo)
{
    pact->palo = palo;
    pact->ackPos = ACK_Nil;
    pact->ackRot = ACK_Nil;
    pact->ackScale = ACK_Nil;
    pact->ackPose = ACK_Nil;
}

void CloneAct(ACT *pact, ACT *pactBase)
{
    // Control modes are part of the ACT state.  In particular, cloned
    // restore actions must retain ACK_Spring or the clone is left with the
    // ACK_Nil values installed by InitAct and will never restore its pose.
    pact->ackPos = pactBase->ackPos;
    pact->ackRot = pactBase->ackRot;
    pact->ackScale = pactBase->ackScale;
    pact->ackPose = pactBase->ackPose;
    pact->nPriority = pactBase->nPriority;
    pact->tMatch = pactBase->tMatch;
}

void RetractAct(ACT* pact, GRFRA grfra)
{
    if (pact == nullptr || pact->palo == nullptr)
        return;

    SO* pso = static_cast<SO*>(pact->palo);
    SW* psw = pso->psw;

    ACT* pactPosOld = pso->pactPos;
    ACT* pactRotOld = pso->pactRot;

    RemoveDlEntry(&pso->dlAct, pact);
    pso->pvtalo->pfnRecacheAloActList(pso);

    // If the retracted action controlled position and no replacement
    // position action was selected, optionally preserve/clear velocity.
    if (pact == pactPosOld && pso->pactPos == nullptr && (grfra & 1U) != 0)
    {
        if ((pso->pvtlo->grfcid & 2U) == 0)
            SetAloVelocityVec(pso, &g_vecZero);
        else
        {
            glm::vec3 velocity{};

            ApplySoConstraintLocal(pso, &pso->constrForce, &pso->xf.v, &velocity, nullptr);
            pso->pvtalo->pfnSetAloVelocityVec(pso, &velocity);
        }
    }

    // If the retracted action controlled rotation and no replacement
    // rotation action was selected, optionally preserve/clear angular velocity.
    if (pact == pactRotOld && pso->pactRot == nullptr && (grfra & 2U) != 0)
    {
        if ((pso->pvtlo->grfcid & 2U) == 0)
            SetAloAngularVelocityVec(static_cast<ALO*>(pso), &g_vecZero);
        else
        {
            glm::vec3 angularVelocity{};

            ApplySoConstraintLocal(pso, &pso->constrTorque, &pso->xf.w, &angularVelocity, nullptr);
            pso->pvtalo->pfnSetAloAngularVelocityVec(pso, &angularVelocity);
        }
    }

    // Equivalent to FreeSlotheapPv(...), using your shared ownership.
    std::erase_if(psw->actOwners, [pact](const std::shared_ptr<ACT>& owner)
    {
        return owner.get() == pact;
    });
}

void GetActPositionGoal(ACT* pact, float dtOffset, glm::vec3* ppos, glm::vec3* pv)
{
    ALO* palo = pact->palo;

    *ppos = palo->posOrig;
    *pv = g_vecZero;

    if (palo->pvtalo->pfnAdjustAloPosition != nullptr)
        palo->pvtalo->pfnAdjustAloPosition(palo, ppos, pv);
}

void GetActRotationGoal(ACT* pact, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    ALO* palo = pact->palo;

    *pmat = palo->matOrig;
    *pw = glm::vec3(0.0f);

    palo->pvtalo->pfnAdjustAloRotation(palo, pmat, nullptr);
}

void GetActTwistGoal(ACT* pact, float* pradTwist, float* pdradTwist)
{
    *pradTwist = pact->palo->palox->ikh.radTwistOrig;
    *pdradTwist = 0.0;
}

void GetActScale(ACT* pact, glm::mat3* pmat)
{
    *pmat = glm::mat3(1.0f);
}

float GetActPoseGoal(ACT* pact, int ipose)
{
    return pact->palo->globset.agPosesOrig[ipose];
}

void CalculateActDefaultAck(ACT* pact)
{
    ALO* palo = pact->palo;
    SO* pso = nullptr;

    if ((palo->pvtlo->grfcid & 2U) != 0)
        pso = static_cast<SO*>(palo);

    if (palo->ackPos != ACK_Nil)
        pact->ackPos = palo->ackPos;
    else if (pso != nullptr && pso->constrForce.ct != CT_Locked)
        pact->ackPos = ACK_Smooth;
    else
        pact->ackPos = ACK_Velocity;

    if (palo->ackRot != ACK_Nil)
        pact->ackRot = palo->ackRot;
    else if (pso != nullptr && pso->constrTorque.ct != CT_Locked)
        pact->ackRot = ACK_Smooth;
    else
        pact->ackRot = ACK_Velocity;
}

void SnapAct(ACT* pact, int fForce)
{
    ALO* palo = pact->palo;

    if (pact == palo->pactPos && (fForce || pact->ackPos == ACK_Velocity))
    {
        glm::vec3 posGoal{};
        glm::vec3 velocity{};

        pact->pvtact->pfnGetActPositionGoal(pact, 0.0f, &posGoal, &velocity);

        palo->pvtalo->pfnTranslateAloToPos(palo, &posGoal);
        palo->pvtalo->pfnSetAloVelocityVec(palo, &velocity);
    }

    if (pact == palo->pactRot && (fForce || pact->ackRot == ACK_Velocity))
    {
        glm::mat3 matGoal{ 1.0f };
        glm::vec3 angularVelocity{};

        pact->pvtact->pfnGetActRotationGoal(pact, 0.0f, &matGoal, &angularVelocity);

        palo->pvtalo->pfnRotateAloToMat(palo, &matGoal);
        palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &angularVelocity);
    }
}

void CalculateAloPositionSpring(ALO* palo, float dt, const glm::vec3* pposGoal, const glm::vec3* pvGoal, glm::vec3* pdv)
{
    glm::vec3 dv{ 0.0f };

    CLQ* pclq = palo->pclqPosSpring.get();

    if (pclq != nullptr)
    {
        glm::vec3 dpos = *pposGoal - palo->xf.pos;
        float dist = glm::length(dpos);

        if (dist > 0.0001f)
        {
            float scale =
                pclq->g2 * dist +
                pclq->g1 +
                pclq->g0 / dist;

            dv = dpos * scale;
        }
    }

    pclq = palo->pclqPosDamping.get();

    if (pclq != nullptr)
    {
        glm::vec3 dvGoal = *pvGoal - palo->xf.v;
        float dist = glm::length(dvGoal);

        if (dist > 0.0001f)
        {
            float scale =
                pclq->g2 * dist +
                pclq->g1 +
                pclq->g0 / dist;

            scale = glm::min(scale, 1.0f / dt);

            dv += dvGoal * scale;
        }
    }

    *pdv = dv;
}

void ProjectActPosition(ACT* pact)
{
    ALO* palo = pact->palo;

    const float dt = palo->fRealClock ? g_clock.dtReal : g_clock.dt;
    const float t = palo->fRealClock ? g_clock.tReal : g_clock.t;

    glm::vec3 posGoal(0.0f);
    glm::vec3 vGoal(0.0f);

	pact->pvtact->pfnGetActPositionGoal(pact, 0.0f, &posGoal, &vGoal);

    switch (pact->ackPos)
    {
        case ACK_Spring:
        {
            glm::vec3 springAcceleration(0.0f);
            CalculateAloPositionSpring(palo, dt, &posGoal, &vGoal, &springAcceleration);

            const glm::vec3 acceleration = palo->xf.dv + springAcceleration;

            palo->xf.pos += palo->xf.v * dt + acceleration * (0.5f * dt * dt);
            palo->xf.v += acceleration * dt;
            break;
        }

        case ACK_Velocity:
        case ACK_SmoothLock:
        {
            palo->xf.pos = posGoal;
            palo->xf.v = vGoal;
            break;
        }

        case ACK_Smooth:
        case ACK_SmoothForce:
        case ACK_SmoothNoLock:
        {
            const glm::vec3 dpos = posGoal - palo->xf.pos;
            const float distance = glm::length(dpos);

            glm::vec3 direction = g_normalX;

            if (distance >= 0.0001f)
                direction = dpos / distance;

            glm::vec3 vTangential(0.0f);
            const float speedProjected = SProjectVector(&vGoal, &direction, nullptr, &vTangential);

            SMP smp{};
            smp.svFast = palo->psmpaPos->svFast + speedProjected;
            smp.svSlow = palo->psmpaPos->svSlow + speedProjected;
            smp.dtFast = palo->psmpaPos->dtFast;

            float speedNext = 0.0f;
            float distanceStep = GSmooth(0.0f, distance, dt, &smp, &speedNext);

            if (pact->ackPos == ACK_SmoothForce)
            {
                float distanceForced = distance;

                if (t < pact->tMatch)
                    distanceForced = (distance * dt) / ((pact->tMatch - t) + dt);

                if (distanceStep < distanceForced)
                {
                    speedNext = distanceForced / dt;
                    distanceStep = distanceForced;
                }
            }

            if (distance <= distanceStep)
            {
                palo->xf.pos = posGoal;
                palo->xf.v = vGoal;

                if (pact->ackPos != ACK_SmoothNoLock)
                    pact->ackPos = ACK_SmoothLock;
            }
            else
            {
                palo->xf.pos += direction * distanceStep;
                palo->xf.v = vTangential + direction * speedNext;
            }

            break;
        }

        case ACK_Spline:
        {
            const float dtMatch = pact->tMatch - t;

            if (dtMatch < dt)
            {
                palo->xf.pos = posGoal + vGoal * ((t + dt) - pact->tMatch);
                palo->xf.v = vGoal;
            }
            else
            {
                glm::vec3 posNext(0.0f);
                glm::vec3 vNext(0.0f);

                EvaluateBezierPos(dtMatch, dt, 1.0f, &palo->xf.pos, &palo->xf.v, &posGoal, &vGoal, &posNext, &vNext, nullptr);

                palo->xf.pos = posNext;
                palo->xf.v = vNext;
            }

            break;
        }

        default:
        break;
    }

}

void CalculateAloRotationSpring(ALO* palo, float dt, glm::mat3* pmatGoal, glm::vec3* pwGoal, glm::vec3* pdw)
{
    glm::vec3 dw(0.0f);

    if (palo->pclqRotSpring)
    {
        glm::mat3 dmat(1.0f);
        CalculateDmat(&palo->xf.mat, pmatGoal, &dmat);

        float rad = 0.0f;
        glm::vec3 normal(0.0f);

        DecomposeRotateMatrixRad(&dmat, &rad, &normal);

        if (rad > 0.0001f)
        {
            CLQ* pclq = palo->pclqRotSpring.get();

            const float spring = pclq->g2 * rad * rad + pclq->g1 * rad + pclq->g0;
            dw = normal * spring;
        }
    }

    if (palo->pclqRotDamping)
    {
        CLQ* pclq = palo->pclqRotDamping.get();

        glm::vec3 dwGoal = *pwGoal - palo->xf.w;
        float mag = glm::length(dwGoal);

        if (mag > 0.0001f)
        {
            float damping =
                pclq->g2 * mag +
                pclq->g1 +
                pclq->g0 / mag;

            const float maxDamping = 1.0f / dt;

            if (damping > maxDamping)
                damping = maxDamping;

            dw += dwGoal * damping;
        }
    }

    *pdw = dw;
}

void ProjectActRotation(ACT* pact)
{
    if (pact == nullptr || pact->palo == nullptr || pact->pvtact == nullptr || pact->pvtact->pfnGetActRotationGoal == nullptr)
        return;

    ALO* palo = pact->palo;

    const float dt = palo->fRealClock ? g_clock.dtReal : g_clock.dt;
    const float t = palo->fRealClock ? g_clock.tReal : g_clock.t;

    glm::mat3 matGoal(1.0f);
    glm::vec3 wGoal(0.0f);

    pact->pvtact->pfnGetActRotationGoal(pact, 0.0f, &matGoal, &wGoal);

    const bool hasTwist = palo->palox != nullptr && (palo->palox->grfalox & 0x20U) != 0;

    float radTwistGoal = 0.0f;
    float dradTwistGoal = 0.0f;

    if (hasTwist && pact->pvtact->pfnGetActTwistGoal != nullptr)
        pact->pvtact->pfnGetActTwistGoal(pact, &radTwistGoal, &dradTwistGoal);

    switch (pact->ackRot)
    {
    case ACK_Spring:
    {
        glm::vec3 dwSpring(0.0f);

        CalculateAloRotationSpring(palo, dt, &matGoal, &wGoal, &dwSpring);
        dwSpring += palo->xf.dw;

        glm::vec3 rotStep = palo->xf.w * dt + dwSpring * (0.5f * dt * dt);

        glm::mat3 matStep(1.0f);
        LoadRotateMatrix(&rotStep, &matStep);

        palo->xf.mat = matStep * palo->xf.mat;
        NormalizeRotateMatrix3(&palo->xf.mat);

        palo->xf.w += dwSpring * dt;

        if (hasTwist)
            palo->palox->ikh.radTwist = GSmooth(palo->palox->ikh.radTwist, radTwistGoal, dt, (SMP*)palo->psmpaRot.get(), &palo->palox->ikh.dradTwist);

        break;
    }

    case ACK_Velocity:
    case ACK_SmoothLock:
    {
        palo->xf.mat = matGoal;
        palo->xf.w = wGoal;

        if (hasTwist)
        {
            palo->palox->ikh.radTwist = radTwistGoal;
            palo->palox->ikh.dradTwist = dradTwistGoal;
        }

        break;
    }

    case ACK_Smooth:
    case ACK_SmoothForce:
    case ACK_SmoothNoLock:
    {
        glm::mat3 dmat(1.0f);
        CalculateDmat(&palo->xf.mat, &matGoal, &dmat);

        float rad = 0.0f;
        glm::vec3 axis(0.0f);
        DecomposeRotateMatrixRad(&dmat, &rad, &axis);

        glm::vec3 wTangential(0.0f);
        const float swProjected = SProjectVector(&wGoal, &axis, nullptr, &wTangential);

        SMP smp{};
        smp.svFast = palo->psmpaRot->svFast + swProjected;
        smp.svSlow = palo->psmpaRot->svSlow + swProjected;
        smp.dtFast = palo->psmpaRot->dtFast;

        float swNext = 0.0f;
        float radStep = GSmooth(0.0f, rad, dt, &smp, &swNext);

        if (pact->ackRot == ACK_SmoothForce)
        {
            float radForce = rad;

            if (t < pact->tMatch)
                radForce = (rad * dt) / ((pact->tMatch - t) + dt);

            if (radStep < radForce)
            {
                radStep = radForce;
                swNext = radForce / dt;
            }
        }

        bool shouldLock = false;

        if (rad <= radStep)
        {
            palo->xf.mat = matGoal;
            palo->xf.w = wGoal;
            shouldLock = pact->ackRot != ACK_SmoothNoLock;
        }
        else
        {
            glm::mat3 matStep(1.0f);
            LoadRotateMatrixRad(radStep, &axis, &matStep);

            palo->xf.mat = matStep * palo->xf.mat;
            NormalizeRotateMatrix3(&palo->xf.mat);

            palo->xf.w = wTangential + axis * swNext;
        }

        if (hasTwist)
        {
            palo->palox->ikh.radTwist = GSmooth(palo->palox->ikh.radTwist, radTwistGoal, dt, (SMP*)palo->psmpaRot.get(), &palo->palox->ikh.dradTwist);

            if (palo->palox->ikh.radTwist != radTwistGoal)
                shouldLock = false;
        }

        if (shouldLock)
            pact->ackRot = ACK_SmoothLock;

        break;
    }

    case ACK_Spline:
    {
        const float dtMatch = pact->tMatch - t;

        if (dtMatch <= dt)
        {
            if (glm::all(glm::lessThan(glm::abs(wGoal), glm::vec3(0.0001f))))
            {
                palo->xf.mat = matGoal;
                palo->xf.w = glm::vec3(0.0f);
            }
            else
            {
                glm::vec3 rotStep = wGoal * ((t + dt) - pact->tMatch);

                glm::mat3 matStep(1.0f);
                LoadRotateMatrix(&rotStep, &matStep);

                palo->xf.mat = matStep * matGoal;
                palo->xf.w = wGoal;
            }

            if (hasTwist)
            {
                palo->palox->ikh.radTwist = radTwistGoal;
                palo->palox->ikh.dradTwist = 0.0f;
            }
        }
        else
        {
            glm::mat3 matOut(1.0f);
            glm::vec3 wOut(0.0f);

            EvaluateBezierMat(dtMatch, dt, 1.0f, palo->xf.mat, palo->xf.w, matGoal, wGoal, &matOut, &wOut, nullptr);

            palo->xf.mat = matOut;
            palo->xf.w = wOut;

            if (hasTwist)
                EvaluateBezierFloat(dtMatch, dt, 1.0f, palo->palox->ikh.radTwist, palo->palox->ikh.dradTwist, radTwistGoal, dradTwistGoal, &palo->palox->ikh.radTwist, &palo->palox->ikh.dradTwist, nullptr);
        }

        break;
    }

    default:
        break;
    }

}
void ProjectActPose(ACT* pact, int ipose)
{
    float goal = pact->pvtact->pfnGetActPoseGoal(pact, ipose);
    float& pose = pact->palo->globset.agPoses[ipose];

    switch (pact->ackPose)
    {
        case ACK_Velocity:
        pose = goal;
        break;

        case ACK_Smooth:
        {
            float dt = pact->palo->fRealClock ? g_clock.dtReal : g_clock.dt;
            pose = GSmooth(pose, goal, dt, &s_smpPose, nullptr);
            break;
        }

        default:
        break;
    }
}

void PredictAloPosition(ALO* palo, float dtOffset, glm::vec3* ppos, glm::vec3* pv)
{
    if (dtOffset == 0.0f)
    {
        *ppos = palo->xf.pos;
        *pv = palo->xf.v;
        return;
    }

    if (palo->pactPos == nullptr)
    {
        *pv = palo->xf.v;
        *ppos = palo->xf.pos + palo->xf.v * dtOffset;
        return;
    }

    ACT* pact = palo->pactPos;
    pact->pvtact->pfnGetActPositionGoal(pact, dtOffset, ppos, pv);
}

void PredictAloRotation(ALO* palo, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    if (dtOffset == 0.0f)
    {
        *pmat = palo->xf.mat;
        *pw = palo->xf.w;
        return;
    }

    if (palo->pactRot == nullptr)
    {
        *pw = palo->xf.w;

        glm::vec3 rotationOffset = palo->xf.w * dtOffset;
        glm::mat3 rotationMatrix{ 1.0f };

        LoadRotateMatrix(&rotationOffset, &rotationMatrix);
        *pmat = rotationMatrix * palo->xf.mat;
        return;
    }

    ACT* pact = palo->pactRot;
    pact->pvtact->pfnGetActRotationGoal(pact, dtOffset, pmat, pw);
}

void AdaptAct(ACT* pact)
{
    if (pact->ackPos == ACK_SmoothLock)
        pact->ackPos = ACK_Smooth;

    if (pact->ackRot == ACK_SmoothLock)
        pact->ackRot = ACK_Smooth;
}

SMP s_smpPose{3.0, 1.0, 0.1};
