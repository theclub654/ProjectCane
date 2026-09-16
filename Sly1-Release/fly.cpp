#include "fly.h"
#include "emitter.h"
#include "lgn.h"
#include "target.h"
#include "landing.h"
#include "expl.h"
#include "expls.h"

FLY* NewFly()
{
	return new FLY{};
}

void InitFly(FLY* pfly)
{
	InitSo(pfly);

    pfly->flys = FLYS_Nil;

    pfly->lmSCylinderRadius = s_lmSCylinderRadius;
    pfly->lmSCylinderHeight = s_lmSCylinderHeight;
    pfly->lmSvxyFlight = s_lmSvxyFlight;
    pfly->lmSvzFlight = s_lmSvzFlight;

    pfly->oidSoar = OID_Nil;

    pfly->lmDtFlying = s_lmDtFlying;
    pfly->lmDtGround = s_lmDtGround;
    pfly->lmDtGroundFrame = s_lmDtGroundFrame;
    pfly->lmDtGroundFramePause = s_lmDtGroundFramePause;
    pfly->lmDtDroppings = s_lmDtDroppings;

    pfly->diRender = 1;
    pfly->ccoin = -1;

	AppendDlEntry(&pfly->psw->dlFly, pfly);
}

void SetFlyFlyk(FLY* pfly, FLYK flyk)
{
    pfly->flyk = flyk;
}

void* GetFlyFlyk(FLY* pfly)
{
    return &pfly->flyk;
}

void SetFlyFlysInit(FLY* pfly, FLYS flysInit)
{
    pfly->flysInit = flysInit;
}

void* GetFlyFlysInit(FLY* pfly)
{
    return &pfly->flysInit;
}

void* GetFlyFlys(FLY* pfly)
{
    return &pfly->flys;
}

void SetFlyLmSCylinderRadius(FLY* pfly, LM lmSCylinderRadius)
{
    pfly->lmSCylinderRadius = lmSCylinderRadius;
}

void* GetFlyLmSCylinderRadius(FLY* pfly)
{
    return &pfly->lmSCylinderRadius;
}

void SetFlyLmSCylinderHeight(FLY* pfly, LM lmSCylinderHeight)
{
    pfly->lmSCylinderHeight = lmSCylinderHeight;
}

void* GetFlyLmSCylinderHeight(FLY* pfly)
{
    return &pfly->lmSCylinderHeight;
}

void SetFlyLmSvxyFlight(FLY* pfly, LM lmSvxyFlight)
{
    pfly->lmSvxyFlight = lmSvxyFlight;
}

void* GetFlyLmSvxyFlight(FLY* pfly)
{
    return &pfly->lmSvxyFlight;
}

void SetFlyLmSvzFlight(FLY* pfly, LM lmSvzFlight)
{
    pfly->lmSvzFlight = lmSvzFlight;
}

void* GetFlyLmSvzFlight(FLY* pfly)
{
    return &pfly->lmSvzFlight;
}

void* GetFlyOidSoar(FLY* pfly)
{
    return &pfly->oidSoar;
}

void SetFlyOidSoar(FLY* pfly, OID oidSoar)
{
    pfly->oidSoar = oidSoar;
}

void SetFlyUSoar(FLY* pfly, float uSoar)
{
    pfly->uSoar = uSoar;
}

void* GetFlyUSoar(FLY* pfly)
{
    return &pfly->uSoar;
}

void SetFlyLmDtFlying(FLY* pfly, LM lmDtFlying)
{
    pfly->lmDtFlying = lmDtFlying;
}

void* GetFlyLmDtFlying(FLY* pfly)
{
    return &pfly->lmDtFlying;
}

void SetFlyDtFlyingFrame(FLY* pfly, float dtFlyingFrame)
{
    pfly->dtFlyingFrame = dtFlyingFrame;
}

void* GetFlyDtFlyingFrame(FLY* pfly)
{
    return &pfly->dtFlyingFrame;
}

void SetFlyLmDtGround(FLY* pfly, LM lmDtGround)
{
    pfly->lmDtGround = lmDtGround;
}

void* GetFlyLmDtGround(FLY* pfly)
{
    return &pfly->lmDtGround;
}

void SetFlyLmDtGroundFrame(FLY* pfly, LM lmDtGroundFrame)
{
    pfly->lmDtGroundFrame = lmDtGroundFrame;
}

void* GetFlyLmDtGroundFrame(FLY* pfly)
{
    return &pfly->lmDtGroundFrame;
}

void SetFlyLmDtGroundFramePause(FLY* pfly, LM lmDtGroundFramePause)
{
    pfly->lmDtGroundFramePause = lmDtGroundFramePause;
}

void* GetFlyLmDtGroundFramePause(FLY* pfly)
{
    return &pfly->lmDtGroundFramePause;
}

void SetFlyLmDtDroppings(FLY* pfly, LM lmDtDroppings)
{
    pfly->lmDtDroppings = lmDtDroppings;
}

void* GetFlyLmDtDroppings(FLY* pfly)
{
    return &pfly->lmDtDroppings;
}

void SetFlySFlee(FLY* pfly, float sFlee)
{
    pfly->sFlee = sFlee;
}

void* GetFlySFlee(FLY* pfly)
{
    return &pfly->sFlee;
}

void SetFlyCcoin(FLY* pfly, int ccoin)
{
    pfly->ccoin = ccoin;
}

void* GetFlyCcoin(FLY* pfly)
{
    return &pfly->ccoin;
}

void SetFlyFSuppressDifficultyUpdate(FLY* pfly, int fSuppressDifficultyUpdate)
{
    pfly->fSuppressDifficultyUpdate = fSuppressDifficultyUpdate;
}

void* GetFlyFSuppressDifficultyUpdate(FLY* pfly)
{
    return &pfly->fSuppressDifficultyUpdate;
}

int GetFlySize()
{
	return sizeof(FLY);
}

void LoadFlyFromBrx(FLY* pfly, CBinaryInputStream* pbis)
{
    constexpr int oidRenderFirst = 572;
    constexpr int oidGroundRenderFirst = 589;

    LoadSoFromBrx(pfly, pbis);
    SnipAloObjects(pfly, 2, s_asnipFly);
    InferExpl(&pfly->pexpl, pfly);

    for (int irender = 0; irender < 16; ++irender)
    {
        OID oidRender = static_cast<OID>(oidRenderFirst + irender);
        ALO* paloRender = static_cast<ALO*>(PloFindSwChild(pfly->psw, oidRender, pfly));

        if (paloRender == nullptr)
            continue;

        SnipLo(paloRender);

        if (pfly->flyk == FLYK_Pigeon && pfly->oidSoar == oidRender)
            pfly->ipaloRenderSoar = pfly->cpaloRender;

        pfly->apaloRender[pfly->cpaloRender++] = paloRender;
    }

    if (pfly->flyk == FLYK_Pigeon)
    {
        for (int irender = 0; irender < 8; ++irender)
        {
            OID oidRender = static_cast<OID>(oidGroundRenderFirst + irender);
            ALO* paloRender = static_cast<ALO*>(PloFindSwChild(pfly->psw, oidRender, pfly));

            if (paloRender == nullptr)
                continue;

            SnipLo(paloRender);
            pfly->apaloRenderGround[pfly->cpaloRenderGround++] = paloRender;
        }
    }

    if (pfly->flyk != FLYK_Insect && FFindDlEntry(&pfly->psw->dlFly, pfly))
        RemoveDlEntry(&pfly->psw->dlFly, pfly);
}

void CloneFly(FLY* pfly, FLY* pflyBase)
{
    DLE dleFly = pfly->dleFly;

    CloneSo(pfly, pflyBase);

    pfly->flyk = pflyBase->flyk;
    pfly->flys = pflyBase->flys;
    pfly->tFlys = pflyBase->tFlys;
    pfly->tFlysNext = pflyBase->tFlysNext;
    pfly->flysInit = pflyBase->flysInit;
    pfly->cpaloRender = pflyBase->cpaloRender;
    pfly->ipaloRender = pflyBase->ipaloRender;
    std::memcpy(pfly->apaloRender, pflyBase->apaloRender, sizeof(pflyBase->apaloRender));
    pfly->cpaloRenderGround = pflyBase->cpaloRenderGround;
    std::memcpy(pfly->apaloRenderGround, pflyBase->apaloRenderGround, sizeof(pflyBase->apaloRenderGround));
    pfly->paloRenderDead = pflyBase->paloRenderDead;
    pfly->pexpl = pflyBase->pexpl;
    pfly->ptarget = pflyBase->ptarget;
    pfly->dleFly = pflyBase->dleFly;
    pfly->pambBuzz = pflyBase->pambBuzz;
    pfly->posWanderNext = pflyBase->posWanderNext;
    pfly->vWanderNext = pflyBase->vWanderNext;
    pfly->lmSCylinderRadius = pflyBase->lmSCylinderRadius;
    pfly->lmSCylinderHeight = pflyBase->lmSCylinderHeight;
    pfly->lmSvxyFlight = pflyBase->lmSvxyFlight;
    pfly->lmSvzFlight = pflyBase->lmSvzFlight;
    pfly->oidSoar = pflyBase->oidSoar;
    pfly->ipaloRenderSoar = pflyBase->ipaloRenderSoar;
    pfly->uSoar = pflyBase->uSoar;
    pfly->lmDtFlying = pflyBase->lmDtFlying;
    pfly->dtFlyingFrame = pflyBase->dtFlyingFrame;
    pfly->tFlyingFrameNext = pflyBase->tFlyingFrameNext;
    pfly->sGroundMax = pflyBase->sGroundMax;
    pfly->lmDtGround = pflyBase->lmDtGround;
    pfly->lmDtGroundFrame = pflyBase->lmDtGroundFrame;
    pfly->lmDtGroundFramePause = pflyBase->lmDtGroundFramePause;
    pfly->tGroundFrameNext = pflyBase->tGroundFrameNext;
    pfly->tGroundTestLast = pflyBase->tGroundTestLast;
    pfly->lmDtDroppings = pflyBase->lmDtDroppings;
    pfly->pexplsDroppings = pflyBase->pexplsDroppings;
    pfly->ppntDroppings = pflyBase->ppntDroppings;
    pfly->tDroppingsNext = pflyBase->tDroppingsNext;
    pfly->sFlee = pflyBase->sFlee;
    pfly->tLandNext = pflyBase->tLandNext;
    pfly->diRender = pflyBase->diRender;
    pfly->ccoin = pflyBase->ccoin;

    pfly->dleFly = dleFly;

    if (pfly->flyk == FLYK_Insect)
        StartSound((SFXID)65, &pfly->pambBuzz, pfly, nullptr, 700.0f, 10.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
    else
    {
        if (FFindDlEntry(&pfly->psw->dlFly, pfly))
            RemoveDlEntry(&pfly->psw->dlFly, pfly);
    }
}

void FreezeFly(FLY* pfly, int fFreeze)
{
    if (pfly->flyk == FLYK_Insect) {
        if (fFreeze == 0)
            StartSound(SFXID(65), &pfly->pambBuzz, (ALO*)pfly, nullptr, 700.0, 10.0, 1.0, 0.0, 0.0, nullptr, nullptr);
        else
            StopSound(pfly->pambBuzz, 0);
    }
}

void PostFlyLoad(FLY* pfly)
{
    SnipAloObjects(pfly, 2, s_asnipPostLoadFly);

    if (pfly->flyk == FLYK_Pigeon)
    {
        pfly->tLandNext = g_clock.t + GRandInRange(pfly->lmDtFlying.gMin, pfly->lmDtFlying.gMax);
        pfly->tDroppingsNext = g_clock.t + GRandInRange(pfly->lmDtDroppings.gMin, pfly->lmDtDroppings.gMax);
    }

    SetFlyFlys(pfly, pfly->flysInit);
    PostAloLoad(pfly);

    pfly->sGroundMax = pfly->sRadiusSelf * 2.0f;
}

void PresetFlyAccel(FLY* pfly, float dt)
{
    switch (pfly->flys)
    {
        case FLYS_Wander:
        case FLYS_Attract:
        case FLYS_Land:
        {
            glm::vec3 pos;
            glm::vec3 v;
            glm::vec3 w;
            glm::mat3 matTarget;

            EvaluateBezierPos((pfly->tFlysNext + dt) - g_clock.t, dt, 1.0f, &pfly->xf.pos, &pfly->xf.v, &pfly->posWanderNext, &pfly->vWanderNext, &pos, &v, nullptr);

            glm::vec3 dv = (v - pfly->xf.v) / dt;

            LimitVectorLength(&dv, 3000.0f, &dv);
            AddSoAcceleration(pfly, &dv);

            dv -= pfly->dvGravity;

            BuildOrthonormalMatrixZ(v, dv, matTarget);
            SmoothMatrix(&pfly->xf.mat, &matTarget, &s_smpMat, dt, nullptr, &w);
            pfly->pvtalo->pfnSetAloAngularVelocityVec(pfly, &w);
            break;
        }

        case FLYS_Dying:
        AddSoAcceleration(pfly, &pfly->dvGravity);
        AccelSoTowardPosSpring(pfly, nullptr, nullptr, &g_vecZero, &s_clqFlyDampV, dt);
        AccelSoTowardMatSpring(pfly, nullptr, nullptr, &g_vecZero, &s_clqFlyDampW, dt);
        break;

        case FLYS_Ground:
        PresetSoAccel(pfly, dt);
        break;

        default:
        break;
    }
}

void UpdateFly(FLY* pfly, float dt)
{
    UpdateSo(pfly, dt);

    if (pfly->tFlysNext <= g_clock.t)
    {
        FLYS flysNext = pfly->flys;

        switch (pfly->flys)
        {
            case FLYS_Wander:
            case FLYS_Attract:
            SetFlyFlys(pfly, FLYS_Nil);
            flysNext = FLYS_Wander;
            break;

            case FLYS_Dying:
            flysNext = FLYS_Dead;
            break;

            case FLYS_Land:
            flysNext = FlysAttemptedFlyLanding(pfly);
            break;

            case FLYS_Ground:
            flysNext = FLYS_Wander;
            break;

            default:
            break;
        }

        SetFlyFlys(pfly, flysNext);
    }

    const bool fAirborne = pfly->flys == FLYS_Wander || pfly->flys == FLYS_Attract || pfly->flys == FLYS_Land;

    if (fAirborne)
    {
        if (pfly->flyk == FLYK_Insect)
        {
            if (pfly->pambBuzz == nullptr)
                StartSound((SFXID)65, &pfly->pambBuzz, pfly, nullptr, 700.0f, 10.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

            if (pfly->cpaloRender > 0)
                pfly->ipaloRender = (pfly->ipaloRender + 1) % pfly->cpaloRender;
        }
        else if (pfly->flyk == FLYK_Pigeon)
        {
            if (pfly->flys == FLYS_Wander && pfly->tLandNext <= g_clock.t)
            {
                glm::vec3 posLanding;

                if (FFindFlyClosestLandingPos(pfly, &posLanding))
                {
                    pfly->posWanderNext = posLanding;
                    SetFlyFlys(pfly, FLYS_Land);
                }
                else
                {
                    pfly->tLandNext = g_clock.t + GRandInRange(pfly->lmDtFlying.gMin, pfly->lmDtFlying.gMax);
                }
            }

            if (pfly->tFlyingFrameNext <= g_clock.t && pfly->cpaloRender > 1)
            {
                pfly->ipaloRender += pfly->diRender;

                if (pfly->ipaloRender >= pfly->cpaloRender)
                {
                    pfly->ipaloRender = pfly->cpaloRender - 2;
                    pfly->diRender = -1;
                }
                else if (pfly->ipaloRender < 0)
                {
                    pfly->ipaloRender = 1;
                    pfly->diRender = 1;
                }

                pfly->tFlyingFrameNext = g_clock.t + pfly->dtFlyingFrame;
            }
        }

        if (g_clock.t - pfly->tFlys > 0.15f)
        {
            for (OX* pox = pfly->poxa->pox; pox != nullptr; pox = pox->poxNext)
            {
                if (pox->pxp != nullptr)
                {
                    pfly->tFlysNext = g_clock.t;
                    break;
                }
            }
        }
    }
    else if (pfly->flys == FLYS_Ground)
    {
        if (FCheckFlyOpenSpaceBelow(pfly) || FShouldFlyFlee(pfly))
            SetFlyFlys(pfly, FLYS_Wander);
        else if (pfly->tGroundFrameNext <= g_clock.t && pfly->cpaloRenderGround > 0)
        {
            pfly->ipaloRender = (pfly->ipaloRender + 1) % pfly->cpaloRenderGround;

            const LM& lmFrame = pfly->ipaloRender == 0 ? pfly->lmDtGroundFramePause : pfly->lmDtGroundFrame;
            pfly->tGroundFrameNext = g_clock.t + GRandInRange(lmFrame.gMin, lmFrame.gMax);
        }
    }

    if (pfly->flyk == FLYK_Pigeon && (pfly->flys == FLYS_Wander || pfly->flys == FLYS_Land))
        EmitFlyDroppings(pfly);
}

void RenderFlySelf(FLY* pfly, CM* pcm, RO* pro)
{
    RO ro;
    DupAloRo(pfly, pro, &ro);

    if (pfly->flys == FLYS_Attract)
    {
        const float u = (g_clock.t - pfly->tFlys) / (pfly->tFlysNext - pfly->tFlys);
        const float alpha = s_clqArriveAlpha.g0 + u * (s_clqArriveAlpha.g1 + u * s_clqArriveAlpha.g2);

        ro.uAlpha *= GLimitLm(&g_lmZeroOne, alpha);

        ALO* paloRender = pfly->apaloRender[pfly->ipaloRender];
        paloRender->pvtalo->pfnRenderAloAll(paloRender, pcm, &ro);
        return;
    }

    if (pfly->flys == FLYS_Dying)
    {
        const float u = (g_clock.t - pfly->tFlys) / (pfly->tFlysNext - pfly->tFlys);
        const float alpha = s_clqDyingAlpha.g0 + u * (s_clqDyingAlpha.g1 + u * s_clqDyingAlpha.g2);

        ro.uAlpha *= GLimitLm(&g_lmZeroOne, alpha);

        ALO* paloRender = pfly->paloRenderDead;
        paloRender->pvtalo->pfnRenderAloAll(paloRender, pcm, &ro);
        return;
    }

    if (pfly->flys == FLYS_Ground)
    {
        ALO* paloRender = pfly->apaloRenderGround[pfly->ipaloRender];
        paloRender->pvtalo->pfnRenderAloAll(paloRender, pcm, &ro);
        return;
    }

    int ipaloRender = pfly->ipaloRender;

    if (pfly->flyk == FLYK_Pigeon && pfly->oidSoar != OID_Nil)
    {
        const float u = (g_clock.t - pfly->tFlys) / (pfly->tFlysNext - pfly->tFlys);

        if (u > 1.0f - pfly->uSoar)
            ipaloRender = pfly->ipaloRenderSoar;
    }

    ALO* paloRender = pfly->apaloRender[ipaloRender];
    paloRender->pvtalo->pfnRenderAloAll(paloRender, pcm, &ro);
}

int FAbsorbFlyWkr(FLY* pfly, WKR* pwkr)
{
    if (pfly->flys == FLYS_Dying)
        return 1;

    SetFlyFlys(pfly, FLYS_Dying);

    if ((pwkr->grfic & 8U) != 0)
        ApplySoImpulse(pfly, &pwkr->pos, &pwkr->v, pwkr->sftMax);

    if (pfly->pexpl != nullptr)
    {
        EXPLSO explso{};
        explso.posOrigin = pwkr->pos;
        explso.vec = pwkr->v;
        explso.grfexplso = 6;

        pfly->pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pfly->pexpl, &explso);
    }

    return 1;
}

void SetFlyFlys(FLY* pfly, FLYS flys)
{
    if (pfly->flys == flys)
        return;

    switch (pfly->flys)
    {
        case FLYS_Wander:
        case FLYS_Attract:
        case FLYS_Land:
        pfly->mtlk = 0;
        break;

        case FLYS_Dead:
        pfly->pvtlo->pfnAddLo(pfly);
        break;

        case FLYS_Ground:
        SetSoConstraints(pfly, CT_Free, nullptr, CT_Free, nullptr);
        pfly->ipaloRender = 0;
        pfly->tDroppingsNext = g_clock.t + GRandInRange(pfly->lmDtDroppings.gMin, pfly->lmDtDroppings.gMax);
        break;

        default:
        break;
    }

    pfly->ptarget->grftak = 14;
    pfly->tFlysNext = g_clock.t;

    switch (flys)
    {
        case FLYS_Wander:
        case FLYS_Attract:
        case FLYS_Land:
        {
            // The release version uses 11 here, while the prototype uses 10.
            pfly->mtlk = 11;

            if (flys == FLYS_Land)
                pfly->vWanderNext = glm::normalize(pfly->posWanderNext - pfly->xf.pos) * 50.0f;
            else
            {
                const float radPosition = GRandInRange(-3.1415927f, 3.1415927f);
                const float radius = GRandInRange(pfly->lmSCylinderRadius.gMin, pfly->lmSCylinderRadius.gMax);
                const float height = GRandInRange(pfly->lmSCylinderHeight.gMin, pfly->lmSCylinderHeight.gMax);

                SetVectorCylind(&pfly->posWanderNext, radPosition, radius, height);
                pfly->posWanderNext += pfly->posOrig;

                const float radVelocity = GRandInRange(-3.1415927f, 3.1415927f);
                const float svxy = GRandInRange(pfly->lmSvxyFlight.gMin, pfly->lmSvxyFlight.gMax);
                const float svz = GRandInRange(pfly->lmSvzFlight.gMin, pfly->lmSvzFlight.gMax);

                SetVectorCylind(&pfly->vWanderNext, radVelocity, svxy, svz);
            }

            float xValues[4] =
            {
                pfly->xf.pos.x,
                pfly->xf.v.x,
                pfly->posWanderNext.x,
                pfly->vWanderNext.x
            };

            float yValues[4] =
            {
                pfly->xf.pos.y,
                pfly->xf.v.y,
                pfly->posWanderNext.y,
                pfly->vWanderNext.y
            };

            float zValues[4] =
            {
                pfly->xf.pos.z,
                pfly->xf.v.z,
                pfly->posWanderNext.z,
                pfly->vWanderNext.z
            };

            float dtMin;
            LimitBezierMulti(3, xValues, 1500.0, yValues, 1500.0, zValues, 1500.0, &dtMin);

            dtMin = (static_cast<int>(dtMin * 60.0f) + 1) * (1.0f / 60.0f);
            pfly->tFlysNext += dtMin;

            if (pfly->flys == FLYS_Ground)
            {
                if (flys != FLYS_Wander)
                {
                    pfly->flys = flys;
                    pfly->tFlys = g_clock.t;
                    return;
                }

                SetAloVelocityXYZ(pfly, pfly->vWanderNext.x, pfly->vWanderNext.y, 400.0f);
                pfly->tLandNext = g_clock.t + GRandInRange(pfly->lmDtFlying.gMin, pfly->lmDtFlying.gMax);
            }

            break;
        }

        case FLYS_Dying:
        StartSound(SFXID(58), nullptr, nullptr, &pfly->xf.posWorld, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        OnDifficultyBreak(&g_difficulty, &pfly->xf.posWorld, pfly->ccoin, pfly->fSuppressDifficultyUpdate);
        pfly->tFlysNext += 2.0f;
        break;

        case FLYS_Dead:
        pfly->pvtlo->pfnRemoveLo(pfly);
        pfly->flys = flys;
        pfly->tFlys = g_clock.t;
        return;

        case FLYS_Ground:
        {
            glm::mat3 matFlyUpright;
            TiltMatUpright(&pfly->xf.matWorld, nullptr, &matFlyUpright);

            pfly->pvtalo->pfnRotateAloToMat(pfly, &matFlyUpright);
            pfly->pvtalo->pfnSetAloVelocityVec(pfly, &g_vecZero);
            pfly->pvtalo->pfnSetAloAngularVelocityVec(pfly, &g_vecZero);

            SetSoConstraints(pfly, CT_Locked, nullptr, CT_Locked, nullptr);

            pfly->ipaloRender = 0;
            pfly->tGroundFrameNext = g_clock.t + GRandInRange(pfly->lmDtGroundFrame.gMin, pfly->lmDtGroundFrame.gMax);
            pfly->tGroundTestLast = g_clock.t;
            pfly->tFlysNext += GRandInRange(pfly->lmDtGround.gMin, pfly->lmDtGround.gMax);
            break;
        }

        default:
        break;
    }

    pfly->flys = flys;
    pfly->tFlys = g_clock.t;
}

void EmitFlyDroppings(FLY* pfly)
{
    if (pfly->pexplsDroppings == nullptr || g_clock.t < pfly->tDroppingsNext)
        return;

    EXPLSO explso{};
    explso.grfexplso = 6;

    if (pfly->ppntDroppings != nullptr)
        GetPntPos(pfly->ppntDroppings, &explso.posOrigin);
    else
        explso.posOrigin = pfly->xf.posWorld;

    explso.vec = -pfly->xf.v;
    explso.vec.z = 0.0f;

    pfly->pexplsDroppings->pvtexpl->pfnExplodeExplExplso(pfly->pexplsDroppings, &explso);

    pfly->tDroppingsNext = g_clock.t + GRandInRange(pfly->lmDtDroppings.gMin, pfly->lmDtDroppings.gMax);
}

int FFindFlyClosestLandingPos(FLY* pfly, glm::vec3* ppos)
{
    float distanceBest = FLT_MAX;
    glm::vec3 posBest(0.0f);

    for (LANDING* planding = pfly->psw->dlLanding.plandingFirst; planding != nullptr; planding = planding->dleLanding.plandingNext)
    {
        if (planding->pcrv == nullptr || planding->pcrv->pvtcrv->pfnFindCrvClosestPointAll == nullptr)
            continue;

        glm::vec3 posFlyLocal;
        glm::vec3 posClosest;

        ConvertAloPos(nullptr, planding->paloParent, &pfly->xf.posWorld, &posFlyLocal);
        planding->pcrv->pvtcrv->pfnFindCrvClosestPointAll(planding->pcrv.get(), &posFlyLocal, nullptr, &posClosest, nullptr, nullptr, nullptr);

        const float distance = glm::length(posFlyLocal - posClosest);

        if (distance < distanceBest)
        {
            ConvertAloPos(planding->paloParent, nullptr, &posClosest, &posBest);
            distanceBest = distance;
        }
    }

    if (distanceBest == FLT_MAX)
        return 0;

    std::vector <SO*> apso;
    IntersectSwBoundingSphere(pfly->psw, pfly, &posBest, 500.0f, nullptr, nullptr, apso);

    glm::vec3 posBestBelow = posBest;
    posBestBelow.z -= 500.0f;

    LSG lsg{};
    SO* psoGround = PsoHitTestLineObjects(0, &posBest, &posBestBelow, apso, &lsg);

    if (psoGround == nullptr)
        return 0;

    *ppos = lsg.apos[0];
    return 1;
}

FLYS FlysAttemptedFlyLanding(FLY* pfly)
{
    bool fContact = false;

    for (OX* pox = pfly->poxa->pox; pox != nullptr; pox = pox->poxNext)
    {
        if (pox->pxp != nullptr)
        {
            fContact = true;
            break;
        }
    }

    const float distanceToLanding = glm::length(pfly->xf.posWorld - pfly->posWanderNext);

    if (distanceToLanding < pfly->sGroundMax)
        return FLYS_Ground;

    if (fContact)
    {
        pfly->tLandNext = g_clock.t + GRandInRange(pfly->lmDtFlying.gMin, pfly->lmDtFlying.gMax);
        return FLYS_Wander;
    }

    SetFlyFlys(pfly, FLYS_Nil);
    return FLYS_Land;
}

int FShouldFlyFlee(FLY* pfly)
{
    PO* ppo = PpoCur();

    if (ppo == nullptr)
        return 0;

    const glm::vec3 posPoSoon = ppo->xf.posWorld + ppo->xf.v * 0.5f;
    const glm::vec3 dpos = posPoSoon - pfly->xf.posWorld;

    return glm::dot(dpos, dpos) < pfly->sFlee * pfly->sFlee;
}

int FFilterFly(FLY* pfly, SO* pso) 
{
    return !pso->fNoXpsSelf && pso->paloRoot != pfly->paloRoot && !pso->fHidden;
}

int FCheckFlyOpenSpaceBelow(FLY* pfly)
{
    if (g_clock.t - pfly->tGroundTestLast < 0.05f)
        return 0;

    std::vector<SO*> apso;
    IntersectSwBoundingSphere(pfly->psw, nullptr, &pfly->xf.posWorld, pfly->sGroundMax, reinterpret_cast<PFNFILTER>(FFilterFly), pfly, apso);

    glm::vec3 posBelow = pfly->xf.posWorld;
    posBelow.z -= pfly->sGroundMax;

    SO* psoBelow = PsoHitTestLineObjects(0, &pfly->xf.posWorld, &posBelow, apso, nullptr);

    pfly->tGroundTestLast = g_clock.t;

    return psoBelow == nullptr;
}

void DeleteFly(FLY *pfly)
{
	delete pfly;
}

CLQ s_clqArriveAlpha{0.0, 2.0, 0.0, 0.0};
CLQ s_clqDyingAlpha{1.0, 1.5, -2.5, 0.0};
LM s_lmSCylinderRadius = {200, 200};
LM s_lmSCylinderHeight = {100, 100};
LM s_lmSvxyFlight = {500, 500};
LM s_lmSvzFlight = {-300, 300};
LM s_lmDtFlying = {15, 60};
LM s_lmDtGround = {30, 120};
LM s_lmDtGroundFrame = {0.05, 0.2};
LM s_lmDtGroundFramePause = {0.1, 0.8};
LM s_lmDtDroppings = {5.0, 15.0};
SNIP s_asnipFly[2] = 
{
    0, (OID)0x256, offsetof(FLY, paloRenderDead),
    2, (OID)0x259, offsetof(FLY, pexplsDroppings)
};
SNIP s_asnipPostLoadFly[2] = 
{
    4, (OID)0x258, offsetof(FLY, ptarget),
    2, (OID)0x25A, offsetof(FLY, ppntDroppings)
};

CLQ s_clqFlyDampV = {0.0, 0.25, 0.00025};
CLQ s_clqFlyDampW = { 0.25, 0.025, 0.0 };
