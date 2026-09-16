#include "rose.h"

void InitRose(ROSE* prose, glm::vec3* ppos, float gScale, SO* psoTouch)
{
    InitRip(prose, ppos, gScale, psoTouch);

    prose->paloRender = static_cast<ALO*>(g_psw->aploStock[16]);
    prose->c = -1;
    prose->uSpin = 0.0f;
    prose->xhpKey = -1;
    prose->roses = ROSES_Nil;
}

void ProjectRoseTransform(RIP* prip, float dt)
{
    // The original function is an intentional no-op. Roses are screen-space
    // finder outlines whose hull is rebuilt by their PFNROSE callback, so
    // they must not receive the normal moving-particle transform integration.
    (void)prip;
    (void)dt;
}

void UpdateRose(ROSE* prose, float dt)
{
    if (prose->dtLifetime <= 0.0f)
        return;

    const float u = (g_clock.t - prose->tCreated) / prose->dtLifetime;

    float duSpin = 0.0f;

    switch (prose->roses)
    {
        case ROSES_Closing:
        duSpin = s_clqUToDuClosing.g0 + u * (s_clqUToDuClosing.g1 + u * s_clqUToDuClosing.g2);
        break;

        case ROSES_Spinning:
        case ROSES_Opening:
        duSpin = s_clqUToDuOpening.g0 + u * (s_clqUToDuOpening.g1 + u * s_clqUToDuOpening.g2);
        break;

        default:
        break;
    }

    prose->uSpin += duSpin * dt;
    prose->uSpin -= std::floor(prose->uSpin);

    if (u <= 1.0f)
        return;

    switch (prose->roses)
    {
        case ROSES_Closing:
        SetRoseRoses(prose, ROSES_Spinning);
        break;

        case ROSES_Spinning:
        SetRoseRoses(prose, ROSES_Opening);
        break;

        case ROSES_Opening:
        RemoveRip(prose);
        break;

        default:
        break;
    }
}

void RenderRose(ROSE* prose, CM* pcm)
{
    if (prose == nullptr || pcm == nullptr || prose->paloRender == nullptr || prose->pfnrose == nullptr || prose->dtLifetime <= 0.0f)
        return;

    HP ahp[64]{};

    const int chp = prose->pfnrose(prose->pv, 63, ahp);

    if (chp <= 0 || chp > 63)
        return;

    ahp[chp] = ahp[0];

    const float u = glm::clamp((g_clock.t - prose->tCreated) / prose->dtLifetime, 0.0f, 1.0f);

    float sOpening = 0.0f;

    switch (prose->roses)
    {
        case ROSES_Closing:
        sOpening = s_clqUToSClosing.g0 + u * (s_clqUToSClosing.g1 + u * s_clqUToSClosing.g2);
        break;

        case ROSES_Spinning:
        sOpening = s_clqUToSOpening.g0;
        break;

        case ROSES_Opening:
        sOpening = s_clqUToSOpening.g0 + u * (s_clqUToSOpening.g1 + u * s_clqUToSOpening.g2);
        break;

        default:
        return;
    }

    const float lifetimeAlpha = glm::clamp(prose->clqAlpha.g0 + u * (prose->clqAlpha.g1 + u * prose->clqAlpha.g2), 0.0f, 1.0f);
    const float baseScale = prose->clqScale.g0 + u * (prose->clqScale.g1 + u * prose->clqScale.g2);

    float totalLength = 0.0f;
    float renderScale = std::numeric_limits<float>::max();
    float renderAlpha = 0.0f;

    for (int i = 0; i < chp; ++i)
    {
        const glm::vec3 dposHull = glm::vec3(ahp[i + 1].posHull - ahp[i].posHull);

        ahp[i].radHull = std::atan2(dposHull.y, dposHull.x) + glm::pi<float>();
        ahp[i].sEdge = glm::length(glm::vec2(dposHull.x, dposHull.y));

        const float depth = glm::max(ahp[i].posHull.z, pcm->sNearClip);

        const float depthAlphaRaw = s_clqSToUAlpha.g0 + depth * (s_clqSToUAlpha.g1 + depth * s_clqSToUAlpha.g2);
        const float depthAlpha = glm::clamp(depthAlphaRaw, s_lmUAlphaRose.gMin, s_lmUAlphaRose.gMax);

        const float depthScaleRaw = s_clqSToRScale.g0 + depth * (s_clqSToRScale.g1 + depth * s_clqSToRScale.g2);
        const float depthScale = glm::clamp(depthScaleRaw, s_lmRScale.gMin, s_lmRScale.gMax);

        renderAlpha = glm::max(renderAlpha, lifetimeAlpha * depthAlpha);
        renderScale = glm::min(renderScale, baseScale * depthScale);
        totalLength += ahp[i].sEdge;
    }

    if (renderAlpha <= 0.0001f || renderScale <= 0.0001f || !std::isfinite(renderScale))
        return;

    for (int i = 0; i < chp; ++i)
    {
        float drad = ahp[i + 1].radHull - ahp[i].radHull;

        if (drad <= 0.0f)
            drad += glm::two_pi<float>();

        const float radius = glm::max(ahp[i + 1].posHull.z, 0.0001f);

        ahp[i].sCorner = drad * s_clqUToSOpening.g0 / radius;
        totalLength += ahp[i].sCorner;
    }

    if (totalLength <= 0.0001f)
        return;

    if (prose->c < 0)
    {
        const float countRaw = s_clqSToC.g0 + totalLength * (s_clqSToC.g1 + totalLength * s_clqSToC.g2);
        prose->c = glm::max(1, static_cast<int>(glm::clamp(countRaw, s_lmC.gMin, s_lmC.gMax)));
    }

    if (prose->c <= 0)
        return;

    int ihpStart = 0;

    if (prose->xhpKey >= 0)
    {
        for (int i = 0; i < chp; ++i)
        {
            if (ahp[i].xhpKey == prose->xhpKey)
            {
                ihpStart = i;
                break;
            }
        }
    }

    prose->xhpKey = ahp[ihpStart].xhpKey;

    const float spacing = totalLength / static_cast<float>(prose->c);
    float distanceToRose = prose->uSpin * spacing;

    RO ro{};
    ro.uAlpha = renderAlpha;
    ro.uAlphaCelBorder = 1.0f;

    glm::vec3 scale(renderScale);

    auto RenderInstance = [&](const glm::vec3& posBase, float radHull)
    {
        // The original rotates around the engine's forward axis. Sly's world
        // convention uses +X as forward, so this spreads the rose pieces in
        // the camera-facing Y/Z plane.
        glm::mat3 matSpin = glm::mat3(glm::rotate(glm::mat4(1.0f), radHull, glm::vec3(1.0f, 0.0f, 0.0f)));
        glm::mat3 mat = pcm->matRotateTiltToCam * matSpin;
        glm::vec3 pos = posBase + mat[2] * (sOpening * renderScale);

        LoadMatrixFromPosRotScale(&pos, &mat, &scale, &ro.model);
        prose->paloRender->pvtalo->pfnRenderAloGlobset(prose->paloRender, pcm, &ro);
    };

    for (int n = 0; n < chp; ++n)
    {
        const int i = (ihpStart + n) % chp;
        const int next = (i + 1) % chp;

        while (distanceToRose < ahp[i].sEdge)
        {
            const float t = ahp[i].sEdge > 0.0001f ? distanceToRose / ahp[i].sEdge : 0.0f;
            RenderInstance(glm::vec3(glm::mix(ahp[i].pos, ahp[next].pos, t)), ahp[i].radHull);
            distanceToRose += spacing;
        }

        distanceToRose -= ahp[i].sEdge;

        float drad = ahp[next].radHull - ahp[i].radHull;

        if (drad <= 0.0f)
            drad += glm::two_pi<float>();

        while (distanceToRose < ahp[i].sCorner)
        {
            const float t = ahp[i].sCorner > 0.0001f ? distanceToRose / ahp[i].sCorner : 0.0f;
            RenderInstance(glm::vec3(ahp[next].pos), ahp[i].radHull + drad * t);
            distanceToRose += spacing;
        }

        distanceToRose -= ahp[i].sCorner;
    }
}

void SetRoseRoses(ROSE* prose, ROSES roses)
{
    if (prose == nullptr || prose->roses == roses)
        return;

    float uPrevious = 0.0f;

    if (prose->dtLifetime > 0.0f)
        uPrevious = glm::clamp((g_clock.t - prose->tCreated) / prose->dtLifetime, 0.0f, 1.0f);

    switch (roses)
    {
        case ROSES_Closing:
            prose->dtLifetime = 0.25f;
            prose->clqAlpha = CLQ{0.0f, 1.0f, 0.0f};
            break;

        case ROSES_Spinning:
        prose->dtLifetime = std::numeric_limits<float>::max();
        prose->clqAlpha = CLQ{1.0, 0.0, 0.0};
        break;

        case ROSES_Opening:
            prose->dtLifetime = 0.5f;
            prose->clqAlpha = CLQ{1.0f, -1.0f, 0.0f};
            break;

        default:
        break;
    }

    const bool wasTransitioning = prose->roses == ROSES_Closing || prose->roses == ROSES_Opening;
    const bool isTransitioning = roses == ROSES_Closing || roses == ROSES_Opening;

    // Preserve the current alpha when reversing between opening and closing.
    const float uNew = wasTransitioning && isTransitioning ? 1.0f - uPrevious : 0.0f;

    prose->roses = roses;
    prose->tCreated = g_clock.t - uNew * prose->dtLifetime;
}

int SgnCmpHp(const void* pv0, const void* pv1)
{
    const HP* hp0 = static_cast<const HP*>(pv0);
    const HP* hp1 = static_cast<const HP*>(pv1);

    return hp0->radHull < hp1->radHull ? -1 : 1;
}

int ChpBuildConvexHullScreen(const glm::vec3* pposCenter, int chp, HP* ahp)
{
    if (!pposCenter || !ahp || chp <= 0)
        return 0;

    glm::vec3 forward = *pposCenter - glm::vec3(g_pcm->pos);

    if (glm::dot(forward, forward) <= 0.000001f)
        return 0;

    forward = glm::normalize(forward);

    glm::vec3 worldUp(0.0f, 0.0f, 1.0f);

    if (std::abs(glm::dot(forward, worldUp)) > 0.999f)
        worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // This is the original matRotate * matOffset basis: X is the
    // target-relative horizontal axis, Y is its vertical axis, and Z is
    // positive distance from the camera toward the target.
    const glm::vec3 horizontal = glm::normalize(glm::cross(forward, worldUp));
    const glm::vec3 vertical = glm::cross(horizontal, forward);

    for (int i = 0; i < chp; ++i)
    {
        const glm::vec3 delta = glm::vec3(ahp[i].pos) - glm::vec3(g_pcm->pos);
        const float depth = glm::dot(delta, forward);

        if (std::abs(depth) <= 0.000001f)
            return 0;

        ahp[i].posHull = glm::vec4(
            glm::dot(delta, horizontal) / depth,
            glm::dot(delta, vertical) / depth,
            depth,
            1.0f);
    }

    if (chp == 1)
        return 1;

    const auto pivot = std::min_element(ahp, ahp + chp, [](const HP& a, const HP& b)
    {
        return a.posHull.y < b.posHull.y;
    });

    std::iter_swap(ahp, pivot);
    const glm::vec2 pivotPos(ahp[0].posHull);

    for (int i = 1; i < chp; ++i)
    {
        const glm::vec2 delta = glm::vec2(ahp[i].posHull) - pivotPos;
        ahp[i].radHull = std::atan2(delta.y, delta.x);
    }

    std::sort(ahp + 1, ahp + chp, [](const HP& a, const HP& b)
    {
        return a.radHull < b.radHull;
    });

    int hullCount = 0;

    for (int i = 0; i < chp; ++i)
    {
        while (hullCount >= 2)
        {
            const glm::vec2 a(ahp[hullCount - 2].posHull);
            const glm::vec2 b(ahp[hullCount - 1].posHull);
            const glm::vec2 c(ahp[i].posHull);
            const glm::vec2 ab = b - a;
            const glm::vec2 ac = c - a;

            if (ab.x * ac.y - ab.y * ac.x < 0.0f)
                break;

            --hullCount;
        }

        ahp[hullCount++] = ahp[i];
    }

    return hullCount;
}

int ChpBuildConvexHullXY(const glm::mat4* pmat, int chp, HP* ahp)
{
    if (pmat == nullptr || ahp == nullptr || chp <= 0)
        return 0;

    // Transform and project every point into camera-relative XY space.
    for (int i = 0; i < chp; ++i)
    {
        const glm::vec4 transformed = *pmat * ahp[i].pos;

        ahp[i].posHull = transformed;

        if (std::abs(transformed.z) > 0.00001f)
        {
            ahp[i].posHull.x /= transformed.z;
            ahp[i].posHull.y /= transformed.z;
        }
    }

    // Use the lowest projected point as the hull pivot.
    const auto pivot = std::min_element(ahp, ahp + chp, [](const HP& a, const HP& b)
    {
        return a.posHull.y < b.posHull.y;
    });

    std::iter_swap(ahp, pivot);

    const glm::vec2 pivotPos(ahp[0].posHull);

    // Calculate the polar angle of every point around the pivot.
    for (int i = 1; i < chp; ++i)
    {
        const glm::vec2 delta = glm::vec2(ahp[i].posHull) - pivotPos;
        ahp[i].radHull = std::atan2(delta.y, delta.x);
    }

    // Equivalent to the original SgnCmpHp sorting.
    std::sort(ahp + 1, ahp + chp, [](const HP& a, const HP& b)
    {
        return a.radHull < b.radHull;
    });

    // Perform the in-place Graham scan.
    int hullCount = 0;

    for (int i = 0; i < chp; ++i)
    {
        while (hullCount >= 2)
        {
            const glm::vec2 a(ahp[hullCount - 2].posHull);
            const glm::vec2 b(ahp[hullCount - 1].posHull);
            const glm::vec2 c(ahp[i].posHull);

            const glm::vec2 ab = b - a;
            const glm::vec2 ac = c - a;
            const float cross = ab.x * ac.y - ab.y * ac.x;

            if (cross < 0.0f)
                break;

            --hullCount;
        }

        ahp[hullCount++] = ahp[i];
    }

    return hullCount;
}

CLQ s_clqUToDuOpening{ 1.0f, -0.7f, 0.0f };
CLQ s_clqUToDuClosing{ 0.0f, 0.0f, 1.0f };

CLQ s_clqUToSOpening{50, 170, -85};
CLQ s_clqUToSClosing{ 135, 0.0, -85 };

CLQ s_clqSToUAlpha{1.0f, -0.00019999999f, 0.0f};
CLQ s_clqSToRScale{ 1.0, 0.0005, 0.0};
LM s_lmRScale{ 1.0, 10 };
CLQ s_clqSToC{ 0.0, 6.0, 0.0};
LM s_lmC{5.0, 16.0};
LM s_lmUAlphaRose{0.65f, 1.0f};
VTROSE g_vtrose;
