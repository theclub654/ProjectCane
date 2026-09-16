#include "trail.h"

void InitTrail(TRAIL* ptrail, glm::vec3* ppos, float scale, SO* psoTouch)
{
    InitRip(ptrail, ppos, scale, psoTouch);

    ptrail->dtLifetime = 0.35f;
    ptrail->paloRender = static_cast<ALO*>(g_psw->aploStock[7]);
    ptrail->emitdv.rvDamping = -15.0f;
    ptrail->clqAlpha = CLQ{0.5, -0.5, 0.0};

    ptrail->clqScale.g0 = g_clqTrailScale.g0 * scale;
    ptrail->clqScale.g1 = g_clqTrailScale.g1 * scale;
    ptrail->clqScale.g2 = g_clqTrailScale.g2 * scale;
    ptrail->clqScale.gUnused = g_clqTrailScale.gUnused * scale;
}

void OnTrailRemove(TRAIL* ptrail)
{
	SetTrailTrls(ptrail, TRLS_None, nullptr);
}

void SetTrailTrls(TRAIL* ptrail, TRLS trls, void* pv)
{
    if (ptrail == nullptr || ptrail->trls == trls)
        return;

    const TRLS trlsOld = ptrail->trls;

    if (trlsOld == TRLS_TrackingRip)
    {
        RIP* pripOld = ptrail->pripTrack;

        if (pripOld != nullptr)
            UnsubscribeRipStruct(pripOld, (PFNMQ)HandleTrailMessage, ptrail);

        if (trls == TRLS_None && pripOld != nullptr)
            ptrail->posPrev = pripOld->pos;
    }
    else if (trlsOld == TRLS_TrackingAlo)
    {
        ALO* paloOld = ptrail->paloTrack;

        if (paloOld != nullptr && paloOld->pvtlo->pfnUnsubscribeLoStruct != nullptr)
            paloOld->pvtlo->pfnUnsubscribeLoStruct(paloOld, HandleTrailMessage, ptrail);

        if (trls == TRLS_None)
        {
            if (pv != nullptr)
                ptrail->posPrev = *static_cast<glm::vec3*>(pv);
            else if (paloOld != nullptr)
                ptrail->posPrev = paloOld->xf.posWorld;
        }
    }

    ptrail->trls = trls;

    switch (trls)
    {
        case TRLS_None:
        {
            ptrail->v = glm::vec3(0.0f);
            ptrail->pripTrack = nullptr;
            break;
        }

        case TRLS_TrackingRip:
        {
            RIP* pripTrack = static_cast<RIP*>(pv);

            if (pripTrack == nullptr)
            {
                ptrail->trls = TRLS_None;
                ptrail->v = glm::vec3(0.0f);
                ptrail->pripTrack = nullptr;
                break;
            }

            ptrail->pripTrack = pripTrack;
            ptrail->pos = pripTrack->pos;
            ptrail->v = pripTrack->v;

            SubscribeRipStruct(pripTrack, (PFNMQ)HandleTrailMessage, ptrail);
            break;
        }

        case TRLS_TrackingAlo:
        {
            ALO* paloTrack = static_cast<ALO*>(pv);

            if (paloTrack == nullptr)
            {
                ptrail->trls = TRLS_None;
                ptrail->v = glm::vec3(0.0f);
                ptrail->paloTrack = nullptr;
                break;
            }

            ptrail->paloTrack = paloTrack;
            ptrail->pos = paloTrack->xf.posWorld;
            ptrail->v = paloTrack->xf.v;

            if (paloTrack->pvtlo->pfnSubscribeLoStruct != nullptr)
                paloTrack->pvtlo->pfnSubscribeLoStruct(paloTrack, HandleTrailMessage, ptrail);

            break;
        }

        case TRLS_ChainingRip:
        {
            ptrail->pripTrack = static_cast<RIP*>(pv);
            break;
        }
    }
}

void DetachTrail(TRAIL* ptrail)
{
    if (ptrail == nullptr)
        return;

    TRAIL* ptrailDetached = static_cast<TRAIL*>(PripNewRipg(RIPT_Trail, nullptr));

    if (ptrailDetached == nullptr)
    {
        SetTrailTrls(ptrail, TRLS_None, nullptr);
        return;
    }

    RIP* pripDetached = static_cast<RIP*>(ptrailDetached);

    if (pripDetached->pvtrip->pfnInitRip != nullptr)
        pripDetached->pvtrip->pfnInitRip(pripDetached, &ptrail->pos, 0.0f, nullptr);

    ptrailDetached->paloRender = ptrail->paloRender;
    ptrailDetached->emitdv.rvDamping = ptrail->emitdv.rvDamping;
    ptrailDetached->clqAlpha = ptrail->clqAlpha;
    ptrailDetached->dtLifetime = ptrail->dtLifetime;

    void* pvTrack = nullptr;

    if (ptrail->trls == TRLS_TrackingRip && ptrail->pripTrack != nullptr)
    {
        RIP* pripTrack = ptrail->pripTrack;

        float scale = ptrail->clqScale.g0;

        if (pripTrack->dtLifetime > 0.0001f)
        {
            const float ratioCreated = (ptrail->tCreated - pripTrack->tCreated) / pripTrack->dtLifetime;
            const float ratioCurrent = (g_clock.t - pripTrack->tCreated) / pripTrack->dtLifetime;

            const float scaleCreated = pripTrack->clqScale.g0 + ratioCreated * (pripTrack->clqScale.g1 + ratioCreated * pripTrack->clqScale.g2);
            const float scaleCurrent = pripTrack->clqScale.g0 + ratioCurrent * (pripTrack->clqScale.g1 + ratioCurrent * pripTrack->clqScale.g2);

            if (glm::abs(scaleCreated) > 0.0001f)
                scale = (ptrail->clqScale.g0 / scaleCreated) * scaleCurrent;
        }

        ptrailDetached->clqScale.g0 = scale;
        ptrailDetached->clqScale.g1 = -scale;
        ptrailDetached->clqScale.g2 = 0.0f;
        ptrailDetached->clqScale.gUnused = 0.0f;

        pvTrack = pripTrack;
    }
    else
    {
        ptrailDetached->clqScale = ptrail->clqScale;
        pvTrack = ptrail->paloTrack;
    }

    SetTrailTrls(ptrailDetached, ptrail->trls, pvTrack);
    SetTrailTrls(ptrail, TRLS_ChainingRip, ptrailDetached);
}

void ProjectTrailTransform(TRAIL* ptrail, float dt)
{
    if (ptrail == nullptr)
        return;

    ptrail->v = ptrail->v * (1.0f + ptrail->emitdv.rvDamping * dt) + ptrail->emitdv.dv * dt;
    ptrail->pos += ptrail->v * dt;

    glm::vec3* pposTrack = nullptr;
    glm::vec3* pvTrack = nullptr;

    switch (ptrail->trls)
    {
    case TRLS_None:
    {
        ptrail->posPrev += ptrail->v * dt;
        return;
    }

    case TRLS_TrackingRip:
    {
        if (ptrail->pripTrack == nullptr)
        {
            SetTrailTrls(ptrail, TRLS_None, nullptr);
            return;
        }

        pposTrack = &ptrail->pripTrack->pos;
        pvTrack = &ptrail->pripTrack->v;
        break;
    }

    case TRLS_TrackingAlo:
    {
        if (ptrail->paloTrack == nullptr)
        {
            SetTrailTrls(ptrail, TRLS_None, nullptr);
            return;
        }

        if (ptrail->paloTrack->fWater != 0)
        {
            SetTrailTrls(ptrail, TRLS_None, nullptr);
            return;
        }

        pposTrack = &ptrail->paloTrack->xf.posWorld;
        pvTrack = &ptrail->paloTrack->xf.v;
        break;
    }

    case TRLS_ChainingRip:
    default:
        return;
    }

    const float trackSpeedSquared = glm::dot(*pvTrack, *pvTrack);

    if (trackSpeedSquared < 40000.0f)
    {
        SetTrailTrls(ptrail, TRLS_None, nullptr);
        return;
    }

    const glm::vec3 separation = ptrail->pos - *pposTrack;
    const float separationSquared = glm::dot(separation, separation);

    if (separationSquared > 625.0f)
        DetachTrail(ptrail);
}

void RenderTrail(TRAIL* ptrail, CM* pcm)
{
    if (ptrail == nullptr || pcm == nullptr)
        return;

    ALO* paloRender = ptrail->paloRender;

    if (paloRender == nullptr)
    {
        RemoveRip(static_cast<RIP*>(ptrail));
        return;
    }

    const float radius = paloRender->sRadiusRenderAll;

    if (!SphereInFrustum(pcm->frustum, ptrail->pos, radius))
        return;

    const glm::vec3 dpos = ptrail->pos - pcm->pos;
    float uAlphaCm = 1.0f;

    if (!FInsideCmMrd(pcm, dpos, radius, paloRender->sMRD, uAlphaCm))
        return;

    if (ptrail->dtLifetime <= 0.0f)
    {
        RemoveRip(static_cast<RIP*>(ptrail));
        return;
    }

    const float lifetimeRatio = (g_clock.t - ptrail->tCreated) / ptrail->dtLifetime;

    const float alphaValue = ptrail->clqAlpha.g0 + lifetimeRatio * (ptrail->clqAlpha.g1 + lifetimeRatio * ptrail->clqAlpha.g2);
    const float alpha = glm::clamp(alphaValue, 0.0f, 1.0f);

    const float scale = ptrail->clqScale.g0 + lifetimeRatio * (ptrail->clqScale.g1 + lifetimeRatio * ptrail->clqScale.g2);

    if (alpha <= 0.0001f || scale <= 0.0001f)
    {
        RemoveRip(static_cast<RIP*>(ptrail));
        return;
    }

    glm::vec3* pposEnd = nullptr;

    switch (ptrail->trls)
    {
        case TRLS_None:
        pposEnd = &ptrail->posPrev;
        break;

        case TRLS_TrackingRip:
        case TRLS_ChainingRip:
        if (ptrail->pripTrack != nullptr)
            pposEnd = &ptrail->pripTrack->pos;
        break;

        case TRLS_TrackingAlo:
        if (ptrail->paloTrack != nullptr)
            pposEnd = &ptrail->paloTrack->xf.posWorld;
        break;
    }

    if (pposEnd == nullptr)
        return;

    RenderAloLine(ptrail->paloRender, pcm, &ptrail->pos, pposEnd, scale * 100.0f, alpha * uAlphaCm);
}

void HandleTrailMessage(TRAIL* ptrail, MSGID msgid, void* pv)
{
    if (ptrail == nullptr)
        return;

    switch (msgid)
    {
        case MSGID_translated:
        {
            glm::vec3* pposTranslated = nullptr;

            if (pv != nullptr)
                pposTranslated = reinterpret_cast<glm::vec3*>(static_cast<std::byte*>(pv) + 16);

            SetTrailTrls(ptrail, TRLS_None, pposTranslated);
            break;
        }

        case MSGID_rip_removed:
        {
            // ReleaseRip has already detached the source RIP's subscriber list
            // before sending this notification. Finalize the state directly;
            // calling SetTrailTrls here would re-enter UnsubscribeRipStruct on
            // a RIP whose removal is already in progress.
            RIP* pripRemoved = static_cast<RIP*>(pv);
            if (pripRemoved != nullptr)
                ptrail->posPrev = pripRemoved->pos;

            ptrail->trls = TRLS_None;
            ptrail->v = glm::vec3(0.0f);
            ptrail->pripTrack = nullptr;
            break;
        }

        case MSGID_removed:
        case MSGID_water_entered:
            SetTrailTrls(ptrail, TRLS_None, nullptr);
            break;

        default:
        break;
    }
}

CLQ g_clqTrailScale = {1.0, -1.0, 0.0 };
VTTRAIL g_vttrail;
