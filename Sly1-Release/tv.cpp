#include "tv.h"
#include "light.h"
#include "binoc.h"
#include "speaker.h"
#include "render.h"

static float TvUiScale()
{
    // TV authoring coordinates are based on the PS2's 640 x 492.8 display.
    // Preserve that size proportionally on the native framebuffer without
    // stretching the curved mask when the window aspect ratio changes.
    const float sx = static_cast<float>(g_gl.width) / 640.0f;
    const float sy = static_cast<float>(g_gl.height) / 492.8f;
    return glm::min(sx, sy);
}

static void SetTvCameraFov(CM* pcm, float radFov)
{
    // The retail PS3 TV pass in the reference capture uses a 1280x720
    // viewport. Keep that portrait-camera aspect when the host window is
    // square; PostTvContext restores the normal scene projection.
    constexpr float tvAspect = 1280.0f / 720.0f;

    SetCmFov(pcm, radFov);

    pcm->yScreenRange = tanf(radFov * 0.5f);
    pcm->xScreenRange = pcm->yScreenRange * tvAspect;
    pcm->sRadiusNearClip = pcm->yScreenRange *
        sqrtf(tvAspect * tvAspect + 1.0f) * pcm->sNearClip + 1.0f;

    BuildProjectionMatrix(radFov, tvAspect, pcm->sNearClip,
        pcm->sFarClip, pcm->matProj);
    UpdateCmMat4(pcm);
}

void ResizeTvForFramebuffer(TV* ptv)
{
    if (ptv == nullptr || ptv->pbloti == nullptr)
        return;

    if (g_binoc.binocs == BINOCS_Confront)
    {
        // Confront TVs are authored as a full-screen 640 x 360 surface.
        // In this mode the TV area follows the complete scene framebuffer.
        ptv->pbloti->y = 0.0f;
        ptv->dx = static_cast<float>(g_gl.width);
        ptv->dy = static_cast<float>(g_gl.height);
    }
    else
    {
        const float uiScale = TvUiScale();
        const float curveRadius = 62.7f * uiScale;

        ptv->pbloti->y = -20.0f * uiScale;
        ptv->dx = 160.0f * uiScale;
        ptv->dy = 125.4f * uiScale;
        ptv->dsOutline = 3.3000002f * uiScale;
        ptv->dsOutlineTarget = 3.3000002f * uiScale;

        if (ptv->blotk == BLOTK_TvLeft)
        {
            ptv->xFlatBase = 0.0f;
            ptv->xCurveCenter = ptv->dx - curveRadius;
        }
        else
        {
            ptv->xFlatBase = ptv->dx;
            ptv->xCurveCenter = curveRadius;
        }
    }
}

void ResizeTvUiForFramebuffer()
{
    ResizeTvForFramebuffer(&g_tvLeft);
    ResizeTvForFramebuffer(&g_tvRight);
}

void StartupTvLeft(TV* ptvleft)
{
    ptvleft->pvttv = &g_vttv;
}

void StartupTvRight(TV* ptvright)
{
    ptvright->pvttv = &g_vttv;
}

void InitTv(TV* ptv, BLOTK blotk)
{
    const int kSegmentCount = 0x39;
    const float uiScale = TvUiScale();
    const float curveRadius = 62.7f * uiScale;

    ptv->dx = 160.0f * uiScale;
    ptv->dy = 125.4f * uiScale;
    ptv->dsOutline = 3.3000002f * uiScale;
    ptv->dsOutlineTarget = 3.3000002f * uiScale;

    InitBlot(ptv, blotk);

    float curveSign;

    if (blotk == BLOTK_TvLeft)
    {
        ptv->xFlatBase = 0.0f;
        ptv->xCurveCenter = ptv->dx - curveRadius;
        curveSign = 1.0f;
    }
    else
    {
        ptv->xFlatBase = ptv->dx;
        ptv->xCurveCenter = curveRadius;
        curveSign = -1.0f;
    }

    for (int segmentIndex = 0; segmentIndex < kSegmentCount; segmentIndex++)
    {
        TVB* segment = &ptv->atvb[segmentIndex];

        float normalizedY = ((float)segmentIndex * (2.0f / (kSegmentCount - 1))) - 1.0f;

        float curveX = curveSign * sqrt(1.0f - normalizedY * normalizedY);

        glm::vec4 stripeColor;

        // GS alpha is normalized against 0x80, not 0xff.  The retail
        // colors are 0x30000000 and 0x30808080 respectively.
        constexpr float gsBandAlpha = 48.0f / 128.0f;

        if (segmentIndex & 1)
            stripeColor = glm::vec4(0.0f, 0.0f, 0.0f, gsBandAlpha);
        else
            stripeColor = glm::vec4(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, gsBandAlpha);

        segment->yTop = normalizedY;
        segment->xCurveTop = curveX;

        segment->rgbaBase = stripeColor;
        segment->rgbaCur = stripeColor;

        segment->uDistort = 0.0f;
        segment->uDistortTarget = 0.0f;

        if (ptv->blotk == BLOTK_TvLeft)
        {
            segment->smpDistort.svFast = GRandInRange(0.6f, 0.8f);
            segment->smpDistort.svSlow = GRandInRange(0.4f, 0.6f);
            segment->smpDistort.dtFast = GRandInRange(0.05f, 0.2f);
        }
        else
        {
            segment->smpDistort.svFast = GRandInRange(0.3f, 0.4f);
            segment->smpDistort.svSlow = GRandInRange(0.2f, 0.3f);
            segment->smpDistort.dtFast = GRandInRange(0.1f, 0.4f);
        }

        if (segmentIndex > 0)
        {
            TVB* previousSegment = &ptv->atvb[segmentIndex - 1];

            previousSegment->yBottom = normalizedY;
            previousSegment->xCurveBottom = curveX;
        }
    }

    ptv->atvb[kSegmentCount - 1].yBottom = 1.0f;
    ptv->atvb[kSegmentCount - 1].xCurveBottom = 0.0f;

    if (ptv->blotk == BLOTK_TvLeft)
    {
        ptv->smpInterfere.svFast = GRandInRange(1.0f, 1.4f);
        ptv->smpInterfere.svSlow = GRandInRange(0.6f, 0.8f);
        ptv->smpInterfere.dtFast = GRandInRange(0.1f, 0.2f);
    }
    else
    {
        ptv->smpInterfere.svFast = GRandInRange(0.5f, 0.7f);
        ptv->smpInterfere.svSlow = GRandInRange(0.2f, 0.3f);
        ptv->smpInterfere.dtFast = GRandInRange(0.3f, 0.5f);
    }
}

void PostTvLoad(TV* ptv)
{
    PostBlotLoad(ptv);

    LIGHT *plight = (LIGHT*)PloNew(CID_LIGHT, g_psw, nullptr, (OID)0x201, -1);
    ptv->plight = plight;
    SetLightKind(plight, LIGHTK_Position);
    SetLightHighlightColor(ptv->plight, g_vecHighlight);
    SetLightHighlightAngle(ptv->plight, g_degHighlight);
    SetLightMidtoneStrength(ptv->plight, g_gMidtone);
    SetLightMidtoneAngle(ptv->plight, g_degMidtone);
    SetLightShadowStrength(ptv->plight, g_gShadow);
    SetLightShadowAngle(ptv->plight, g_degShadow);
    SetLightFallOff(ptv->plight, &s_lmFallOffDefault);
    SetLightDynamic(ptv->plight, 1);
    ptv->plight->pvtlo->pfnRemoveLo(ptv->plight);
}

void GetTvItvbMinMax(TV* ptv, int* pitvbDrawMin, int* pitvbDrawMax)
{
    const int kSegmentCount = 0x39;

    int visibleSegments = (int)(ptv->uUnwind * (float)kSegmentCount);
    int firstSegment = (kSegmentCount - visibleSegments) / 2;

    *pitvbDrawMin = firstSegment;
    *pitvbDrawMax = firstSegment + visibleSegments;
}

void OnTvReset(TV* ptv)
{
    OnBlotReset(ptv);
    ptv->uUnwind = 0.0;
    ptv->tvgs = TVGS_Closed;
    ptv->pspeakerDraw = nullptr;
    ptv->pspeaker = nullptr;
}

void SetTvBlots(TV* ptv, BLOTS blots)
{
    BLOTS oldBlots = ptv->blots;

    SetBlotBlots(ptv, blots);

    if (oldBlots == blots)
        return;

    if (oldBlots == BLOTS_Hidden)
        ResizeTvForFramebuffer(ptv);

    if (blots == BLOTS_Visible && g_binoc.binocs != BINOCS_Confront)
    {
        ptv->tvgs = TVGS_Nil;
        SetTvTvgs(ptv, TVGS_Closed);
    }
}

void SetTvTvgs(TV* ptv, TVGS tvgs)
{
    if (tvgs == ptv->tvgs)
        return;

    if (tvgs == TVGS_Open)
        ptv->uUnwind = 1.0f;
    else if (tvgs == TVGS_Opening)
    {
        for (int i = 0; i < 0x39; i++)
        {
            TVB* tvb = &ptv->atvb[i];

            tvb->uDistort = 1.0f;
            tvb->uDistortTarget = GRandInRange(0.0f, 0.05f);

            tvb->smpDistort.svFast = 1.0f;
            tvb->smpDistort.svSlow = 0.6f;
            tvb->smpDistort.dtFast = 0.2f;
        }

        ptv->tvgs = TVGS_Opening;
        ptv->tTvgs = g_clock.tReal;
        return;
    }
    else if (tvgs == TVGS_Closed)
    {
        ptv->uUnwind = 0.0f;
        AcceptTvSpeaker(ptv);
    }

    ptv->tvgs = tvgs;
    ptv->tTvgs = g_clock.tReal;
}

void SetTvReplace(TV* ptv, ALO* paloReplace)
{
    if (ptv->paloReplace != nullptr)
        ptv->paloReplace->fHidden = 0;

    ptv->paloReplace = paloReplace;

    if (paloReplace != nullptr)
        paloReplace->fHidden = 1;
}

void AcceptTvSpeaker(TV* ptv)
{
    SPEAKER* pspeakerDraw = ptv->pspeakerDraw;

    if (ptv->pspeaker == pspeakerDraw)
        return;

    // In confrontation mode the draw speaker can be the same world object that
    // paloReplace temporarily hides.  The original game leaves that object in
    // the world here; SetTvReplace(nullptr) unhides it immediately afterward.
    if (pspeakerDraw != nullptr && (g_binoc.binocs != BINOCS_Confront || pspeakerDraw != ptv->paloReplace))
        pspeakerDraw->pvtlo->pfnRemoveLo(pspeakerDraw);

    pspeakerDraw = ptv->pspeaker;
    ptv->pspeakerDraw = pspeakerDraw;

    if (pspeakerDraw == nullptr)
        return;

    pspeakerDraw->pvtlo->pfnAddLo(pspeakerDraw);
    SetAloBlotContext(pspeakerDraw, ptv);
    pspeakerDraw->fHidden = 1;
}

void UpdateTv(TV* ptv)
{
    UpdateBlot(ptv);

    BLOTS blots = ptv->blots;
    TVGS tvgs = ptv->tvgs;

    if (blots == BLOTS_Visible)
    {
        if (tvgs == TVGS_Closing)
        {
            ptv->uUnwind -= g_clock.dtReal * 5.0f;

            if (ptv->uUnwind <= 0.0f)
            {
                ptv->uUnwind = 0.0f;
                tvgs = TVGS_Closed;
            }
        }
        else if (tvgs < TVGS_Closed)
        {
            if (tvgs == TVGS_Opening)
            {
                ptv->uUnwind += g_clock.dtReal * 5.0f;

                if (ptv->uUnwind >= 1.0f)
                {
                    ptv->uUnwind = 1.0f;
                    tvgs = TVGS_Open;
                }
            }
        }
        else if (tvgs == TVGS_Closed)
        {
            if (ptv->pspeaker == nullptr)
                blots = BLOTS_Disappearing;
            else
                tvgs = TVGS_Opening;
        }

        if (g_binoc.binocs != BINOCS_Confront)
        {
            ptv->uInterfere = GSmooth(ptv->uInterfere, ptv->uInterfereTarget, g_clock.dtReal, &ptv->smpInterfere, nullptr);

            if (FFloatsNear(ptv->uInterfere, ptv->uInterfereTarget, 0.0001f))
            {
                ptv->uInterfere = 1.0f;

                if (ptv->blotk == BLOTK_TvLeft)
                {
                    ptv->smpInterfere.svFast = GRandInRange(1.0f, 1.4f);
                    ptv->smpInterfere.svSlow = GRandInRange(0.6f, 0.8f);
                    ptv->smpInterfere.dtFast = GRandInRange(0.1f, 0.2f);
                }
                else
                {
                    ptv->smpInterfere.svFast = GRandInRange(0.5f, 0.7f);
                    ptv->smpInterfere.svSlow = GRandInRange(0.2f, 0.3f);
                    ptv->smpInterfere.dtFast = GRandInRange(0.3f, 0.5f);
                }
            }

            const int interfereIndex = std::clamp(static_cast<int>(ptv->uInterfere * 56.999f), 0, 56);
            float addedDistort = GRandInRange(0.1f, 0.2f);

            if (ptv->blotk == BLOTK_TvLeft)
                addedDistort *= 2.0f;

            ptv->atvb[interfereIndex].uDistort = glm::clamp(ptv->atvb[interfereIndex].uDistort + addedDistort, 0.0f, 1.0f);

            for (int i = 0; i < 57; ++i)
            {
                TVB* ptvb = &ptv->atvb[i];

                if (!FFloatsNear(ptvb->uDistort, ptvb->uDistortTarget, 0.0001f))
                    ptvb->uDistort = GSmooth(ptvb->uDistort, ptvb->uDistortTarget, g_clock.dtReal, &ptvb->smpDistort, nullptr);
                else
                {
                    if (ptv->blotk == BLOTK_TvLeft)
                    {
                        ptvb->smpDistort.svFast = GRandInRange(0.6f, 0.8f);
                        ptvb->smpDistort.svSlow = GRandInRange(0.4f, 0.6f);
                        ptvb->smpDistort.dtFast = GRandInRange(0.05f, 0.2f);
                    }
                    else
                    {
                        ptvb->smpDistort.svFast = GRandInRange(0.3f, 0.4f);
                        ptvb->smpDistort.svSlow = GRandInRange(0.2f, 0.3f);
                        ptvb->smpDistort.dtFast = GRandInRange(0.1f, 0.4f);
                    }

                    ptvb->uDistortTarget = GRandInRange(0.0f, 0.05f);
                }
            }
        }
    }
    else if (blots < BLOTS_Disappearing)
    {
        if (blots == BLOTS_Hidden)
        {
            // Retail does not make a cached draw speaker immediately visible.
            // A pending speaker follows the binoc blot into Appearing first;
            // SetTvBlots then initializes the TV gate as Closed, allowing the
            // normal Closed -> Opening -> Open sequence to animate uUnwind.
            if (ptv->pspeaker != nullptr)
                blots = static_cast<BLOTS>(g_binoc.blots != BLOTS_Hidden);
        }
    }
    else if (blots == BLOTS_Disappearing)
    {
        tvgs = TVGS_Closed;
    }

    ptv->pvtblot->pfnSetBlotBlots(ptv, blots);
    SetTvTvgs(ptv, tvgs);
}

void RenderTv(TV* ptv)
{
    if (g_binoc.binocs == BINOCS_Confront)
    {
        if (ptv->tvs != TVS_Talk)
            return;

        if (ptv->tvgs != TVGS_Open)
            return;
    }
    else if (!FCanRenderTv())
        return;

    int itvbMin;
    int itvbMax;
    GetTvItvbMinMax(ptv, &itvbMin, &itvbMax);

    if (itvbMin == itvbMax)
        return;

    float radFovOld;
    float radFovTargetOld;

    if (g_binoc.binocs != BINOCS_Confront)
    {
        radFovOld = g_pcm->radFOV;
        radFovTargetOld = g_pcm->radFOVTarget;
        SetTvCameraFov(g_pcm, ptv->pspeakerDraw->radFov);
    }

    SPEAKER* pspeaker = ptv->pspeakerDraw;

    glm::vec3 posScreen;
    const float xTv = (1.0f - pspeaker->uxTv) * ptv->x + pspeaker->uxTv * (ptv->x + ptv->dx);
    const float yTv = (1.0f - pspeaker->uyTv) * ptv->y + pspeaker->uyTv * (ptv->y + ptv->dy);

    // BLOT anchoring is resolved in native render-target coordinates.
    posScreen.x = xTv * (2.0f / static_cast<float>(g_gl.width)) - 1.0f;
    posScreen.y = 1.0f - yTv * (2.0f / static_cast<float>(g_gl.height));
    posScreen.z = pspeaker->sDistCm;

    glm::vec3 posWorld;
    ConvertCmScreenToWorld(g_pcm, &posScreen, &posWorld);

    float xOffset = 150.0f;
    float zOffset = 20.0f;

    RO ro{};

    if (g_binoc.binocs == BINOCS_Instruct)
    {
        PositionSpeaker:
        if (ptv->blotk == BLOTK_TvLeft)
            xOffset = -xOffset;

        glm::vec3 posLook = g_pcm->pos + g_pcm->mat[2] * zOffset;
        posLook += g_pcm->mat[1] * xOffset;

        glm::vec3 dpos = posLook - posWorld;
        glm::mat3 matRot;
        BuildOrthonormalMatrixZ(&dpos, &g_pcm->mat[2], &matRot);
        LoadMatrixFromPosRot(&posWorld, &matRot, &ro.model);

        glm::vec3 posLight = posLook + g_pcm->mat * pspeaker->dposLight;
        ptv->plight->pvtalo->pfnTranslateAloToPos(ptv->plight, &posLight);
    }
    else if (g_binoc.binocs < BINOCS_Confront)
    {
        if (g_binoc.binocs == BINOCS_Dialog)
        {
            if (ptv->blotk == BLOTK_TvLeft)
            {
                zOffset = glm::abs(g_joy.y) * g_joy.y;
                xOffset = glm::abs(g_joy.x) * g_joy.x * 150.0f;
            }
            else
            {
                zOffset = glm::abs(g_joy.y2) * g_joy.y2;
                xOffset = glm::abs(g_joy.x2) * g_joy.x2 * -150.0f;
            }

            zOffset = zOffset * 150.0f + 20.0f;
            xOffset += 150.0f;
            goto PositionSpeaker;
        }
    }
    else if (g_binoc.binocs == BINOCS_Confront)
    {
        ALO* paloReplace = ptv->paloReplace;

        if (paloReplace == nullptr)
            goto RestoreCamera;

        LoadMatrixFromPosRot(&paloReplace->xf.posWorld, &paloReplace->xf.matWorld, &ro.model);

        glm::vec3 posLight;
        ConvertAloPos(paloReplace, nullptr, &pspeaker->dposLightConfront, &posLight);
        ptv->plight->pvtalo->pfnTranslateAloToPos(ptv->plight, &posLight);

    }

    // The detached portrait light owns a stable slot in the dedicated TV
    // buffer. Refresh only that slot after RenderTv has resolved its current
    // camera-relative transform.
    UpdateTvLightGpu(ptv->plight, ptv == &g_tvLeft ? 0 : 1);

    if (g_binoc.binocs != BINOCS_Confront)
    {
        RPL rplPre{};
        rplPre.PFNDRAWRPL = PreTvContext;
        rplPre.rp = RP_BlotContext;
        rplPre.ptv = ptv;
        g_blotContextPrpl[g_blotContextCount] = rplPre;
        g_blotContextCount++;
        //rplPre.achzName = "PreTvContext";
        //SubmitRpl(&rplPre);
    }

    pspeaker->fHidden = 0;

    ro.uAlpha = 1.0f;
    ro.uAlphaCelBorder = 1.0f;
    pspeaker->pvtalo->pfnRenderAloAll(pspeaker, g_pcm, &ro);

    pspeaker->fHidden = 1;

	if (g_binoc.binocs == BINOCS_Confront)
        return;

    {
        RPL rplPost{};
        rplPost.PFNDRAWRPL = PostTvContext;
        rplPost.rp = RP_BlotContext;
        rplPost.ptv = ptv;
        //rplPost.achzName = "PostTvContext";
        g_blotContextPrpl[g_blotContextCount] = rplPost;
        g_blotContextCount++;
        //SubmitRpl(&rplPost);
    }

    RestoreCamera:
    if (g_binoc.binocs != BINOCS_Confront)
    {
        SetCmFov(g_pcm, radFovOld);
        g_pcm->radFOVTarget = radFovTargetOld;
    }
}

static bool s_fTvCameraSaved = false;
static float s_radTvFovSaved = 0.0f;
static float s_radTvFovTargetSaved = 0.0f;
static float s_svTvFovSaved = 0.0f;
TV* g_ptvDrawContext = nullptr;

void PreTvContext(RPL* prpl)
{
    TV* ptv = prpl->ptv;

    if (!ptv)
        return;


    int itvbMin;
    int itvbMax;
    GetTvItvbMinMax(ptv, &itvbMin, &itvbMax);

    if (itvbMin == itvbMax)
        return;

    // The PS2 pass clamps the TV rectangle to the authored 640 x 492.8
    // surface, installs that rectangle as the GS scissor, then draws a
    // maximum-depth rectangle before it draws the curved TV mask.  Our TV
    // coordinates have already been resized into framebuffer pixels, so use
    // those pixels directly and convert the top-left UI Y axis to OpenGL's
    // bottom-left scissor axis.
    const float framebufferWidth = static_cast<float>(g_gl.width);
    const float framebufferHeight = static_cast<float>(g_gl.height);
	const float rasterScaleX = static_cast<float>(g_gl.renderWidth) / framebufferWidth;
	const float rasterScaleY = static_cast<float>(g_gl.renderHeight) / framebufferHeight;
    const float xMinF = glm::clamp(ptv->x, 0.0f, framebufferWidth);
    const float xMaxF = glm::clamp(ptv->x + ptv->dx, 0.0f, framebufferWidth);
    const float yMinF = glm::clamp(ptv->y, 0.0f, framebufferHeight);
    const float yMaxF = glm::clamp(ptv->y + ptv->dy, 0.0f, framebufferHeight);
    const GLint scissorX = static_cast<GLint>(std::floor(xMinF * rasterScaleX));
    const GLint scissorY = static_cast<GLint>(std::floor((framebufferHeight - yMaxF) * rasterScaleY));
    const GLsizei scissorWidth = static_cast<GLsizei>(
        std::max(0.0f, std::ceil(xMaxF * rasterScaleX) - std::floor(xMinF * rasterScaleX)));
    const GLsizei scissorHeight = static_cast<GLsizei>(
        std::max(0.0f, std::ceil(yMaxF * rasterScaleY) - std::floor(yMinF * rasterScaleY)));

    if (scissorWidth == 0 || scissorHeight == 0)
        return;

    // DrawGlob may select the isolated portrait light only between this
    // callback and the matching PostTvContext callback. pglob->pblot itself
    // is persistent and therefore cannot identify the current render pass.
    g_ptvDrawContext = ptv;

    s_radTvFovSaved = g_pcm->radFOV;
    s_radTvFovTargetSaved = g_pcm->radFOVTarget;
    s_svTvFovSaved = g_pcm->svRadFOV;
    s_fTvCameraSaved = true;

    SetTvCameraFov(g_pcm, ptv->pspeakerDraw->radFov);
    glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CMGL), &g_pcm->matWorldToClip);

    glEnable(GL_SCISSOR_TEST);
    glScissor(scissorX, scissorY, scissorWidth, scissorHeight);

    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);

    glClearStencil(0);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // RenderDoc event 104:
    // ALWAYS, reference 1, replace stencil on passed fragments.
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_KEEP, GL_REPLACE);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // The TV mask is a screen-space triangle strip.  It must be treated as
    // two-sided: depending on whether this is the left or right TV, part of
    // the curved strip has the opposite winding.  World rendering normally
    // leaves face culling enabled, which can otherwise leave stencil at zero
    // and make the subsequently clipped speaker disappear completely.
    const GLboolean cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    // Produces the 116-vertex curved mask.
    DrawTvArea(ptv);

    if (cullFaceWasEnabled)
        glEnable(GL_CULL_FACE);

    // DrawTvArea uses the UI shader; the queued speaker globs use this one.
    glGlobShader.Use();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // The rectangular depth initialization above corresponds to the first
    // PS2 PreTvContext rectangle.  The speaker now renders against that
    // initialized TV-local depth buffer through the curved stencil mask.
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    // Render the speaker only in the mask written for this TV.  Keep the
    // retail NOTEQUAL test: the pass operation increments stencil, so an
    // EQUAL-1 test would reject every later overlapping surface of the model
    // after the first surface changed that pixel from 1 to 2.
    glStencilMask(0xFF);
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

    // This matches the PS3 capture.
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
}

static void DrawTvContextRect(const TV* ptv)
{
    struct TvContextVertex
    {
        glm::vec2 pos;
    };

    static GLuint vao = 0;
    static GLuint vbo = 0;

    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TvContextVertex) * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
            sizeof(TvContextVertex), reinterpret_cast<void*>(offsetof(TvContextVertex, pos)));
        glBindVertexArray(0);
    }

    const float x0 = glm::clamp(ptv->x, 0.0f, static_cast<float>(g_gl.width));
    const float y0 = glm::clamp(ptv->y, 0.0f, static_cast<float>(g_gl.height));
    const float x1 = glm::clamp(ptv->x + ptv->dx, 0.0f, static_cast<float>(g_gl.width));
    const float y1 = glm::clamp(ptv->y + ptv->dy, 0.0f, static_cast<float>(g_gl.height));

    const TvContextVertex vertices[4] =
    {
        { glm::vec2(x0, y0) },
        { glm::vec2(x0, y1) },
        { glm::vec2(x1, y0) },
        { glm::vec2(x1, y1) }
    };

    glBlotShader.Use();
    const glm::mat4 model(1.0f);
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);
    glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
    glUniform4f(blotColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void PostTvContext(RPL* prpl)
{
    TV* ptv = prpl->ptv;

    // End the light domain even if the remainder of the post pass has no TV
    // rectangle to process. This prevents a stale context from affecting
    // ordinary world draws later in the frame or after dialogue finishes.
    if (g_ptvDrawContext == ptv)
        g_ptvDrawContext = nullptr;

    if (!ptv)
        return;

    int itvbMin;
    int itvbMax;
    GetTvItvbMinMax(ptv, &itvbMin, &itvbMax);

    if (itvbMin == itvbMax)
        return;

    // The retail TV context executes before the main world passes.  Stencil 1
    // is untouched TV background, while Bentley's successful fragments have
    // incremented it to 2 or more.  Seal only those speaker pixels at near
    // depth; the later world pass must remain able to fill the TV background.
    // Color writes stay disabled, so this pass changes only depth.
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    // OpenGL compares ref against the stored stencil value. GL_LESS with a
    // reference of 1 therefore accepts only speaker fragments (stored 2+),
    // while rejecting both untouched mask pixels (1) and outside pixels (0).
    glStencilFunc(GL_LESS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    // DrawTvContextRect supplies z = 0 through the blot projection, which
    // normally lands at window depth 0.5.  That is not enough to protect the
    // portrait from nearby world geometry (grass, leaves, etc.) rendered in
    // the later world passes.  Force this depth-only seal to the near plane;
    // stencil still limits it to pixels actually covered by the speaker.
    glDepthRange(0.0, 0.0);

    // The PS2 post pass draws the rectangular TV bounds. Destination alpha
    // performs the coverage test there; in GL the equivalent is the stencil
    // value left by the speaker pass (anything other than the untouched 1).
    const GLboolean cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);
    DrawTvContextRect(ptv);
    if (cullFaceWasEnabled)
        glEnable(GL_CULL_FACE);

    glDepthRange(0.0, 1.0);

    glGlobShader.Use();
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthFunc(GL_LESS);

    // Clear only the currently scissored TV stencil region.
    glStencilMask(0xFF);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);

    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glDisable(GL_BLEND);

    if (s_fTvCameraSaved)
    {
        SetCmFov(g_pcm, s_radTvFovSaved);
        g_pcm->radFOVTarget = s_radTvFovTargetSaved;
        g_pcm->svRadFOV = s_svTvFovSaved;
        glBindBuffer(GL_UNIFORM_BUFFER, cmUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CMGL), &g_pcm->matWorldToClip);
        s_fTvCameraSaved = false;
    }
}

void DrawTv(TV* ptv)
{
    if (FCanDrawTv())
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        DrawTvBands(ptv);
        DrawTvOutline(ptv);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}

void SetTvTvs(TV* ptv, TVS tvs)
{
    if (tvs != ptv->tvs) {
        ptv->tvs = tvs;
        ptv->tTvs = g_clock.tReal;
    }
}

void SetTvSpeaker(TV* ptv, SPEAKER* pspeaker)
{
    if (pspeaker == ptv->pspeaker)
        return;

    if (ptv->pspeaker == nullptr)
        g_binoc.pvtblot->pfnSetBlotAchzDraw(&g_binoc, nullptr);

    ptv->pspeaker = pspeaker;

    if (g_binoc.binocs == BINOCS_Confront)
    {
        AcceptTvSpeaker(ptv);

        if (ptv->pspeaker == nullptr)
        {
            ptv->pvtblot->pfnSetBlotBlots(ptv, BLOTS_Hidden);
            SetTvTvgs(ptv, static_cast<TVGS>(3));
            SetTvTvs(ptv, static_cast<TVS>(0));
            SetTvReplace(ptv, nullptr);
        }
        else
        {
            ptv->pvtblot->pfnSetBlotBlots(ptv, BLOTS_Visible);
            SetTvTvgs(ptv, static_cast<TVGS>(1));
            SetTvTvs(ptv, static_cast<TVS>(0));
        }
    }
    else if (ptv->tvgs > static_cast<TVGS>(-1) && ptv->tvgs < static_cast<TVGS>(2))
        SetTvTvgs(ptv, static_cast<TVGS>(2));
}
void DrawTvArea(TV* ptv)
{
    if (!ptv)
        return;

    struct TvAreaVertex
    {
        glm::vec2 pos;
    };

    static GLuint vao = 0;
    static GLuint vbo = 0;

    constexpr int MAX_VERTICES = 2 + 57 * 2;

    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(TvAreaVertex) * MAX_VERTICES, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TvAreaVertex), reinterpret_cast<void*>(offsetof(TvAreaVertex, pos)));

        glBindVertexArray(0);
    }

    TvAreaVertex vertices[MAX_VERTICES];
    int vertexCount = 0;

    if (g_binoc.binocs == BINOCS_Confront)
    {
        const float x0 = ptv->xOn;
        const float y0 = ptv->yOn;
        const float x1 = x0 + ptv->dx;
        const float y1 = y0 + ptv->dy;

        // Triangle-strip order matching the four PackXYZF calls.
        vertices[vertexCount++] = { glm::vec2(x0, y0) };
        vertices[vertexCount++] = { glm::vec2(x0, y1) };
        vertices[vertexCount++] = { glm::vec2(x1, y0) };
        vertices[vertexCount++] = { glm::vec2(x1, y1) };
    }
    else
    {
        int itvbDrawMin;
        int itvbDrawMax;
        GetTvItvbMinMax(ptv, &itvbDrawMin, &itvbDrawMax);

        if (itvbDrawMin == itvbDrawMax)
            return;

        const float curveScale = ptv->dy * 0.5f;

        const float xLeft = ptv->x + ptv->xFlatBase;

        const float xCurveCenter = ptv->x + ptv->xCurveCenter;

        const float yCenter = ptv->y + curveScale;

        const TVB& first = ptv->atvb[itvbDrawMin];

        const float firstY = yCenter + first.yTop * curveScale;

        const float firstXRight = xCurveCenter + first.xCurveTop * curveScale;

        // Initial top edge.
        vertices[vertexCount++] = {
            glm::vec2(xLeft, firstY)
        };

        vertices[vertexCount++] = {
            glm::vec2(firstXRight, firstY)
        };

        // Every TVB contributes the following bottom edge.
        for (int i = itvbDrawMin; i < itvbDrawMax; ++i)
        {
            const TVB& band = ptv->atvb[i];

            const float yBottom = yCenter + band.yBottom * curveScale;

            const float xBottomRight = xCurveCenter + band.xCurveBottom * curveScale;

            vertices[vertexCount++] = {
                glm::vec2(xLeft, yBottom)
            };

            vertices[vertexCount++] = {
                glm::vec2(xBottomRight, yBottom)
            };
        }
    }

    if (vertexCount < 4)
        return;

    glBlotShader.Use();

    const glm::mat4 model(1.0f);

    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);
    glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
    glUniform4f(blotColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TvAreaVertex) * vertexCount, vertices);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

    glBindVertexArray(0);
}

void DrawTvBands(TV* ptv)
{
    if (!ptv)
        return;

    struct TvBandVertex
    {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    static GLuint vao = 0;
    static GLuint vbo = 0;
    static bool initialized = false;

    constexpr int kMaxDistortionPoints = 57 * 5;

    if (!initialized)
    {
        initialized = true;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(TvBandVertex) * kMaxDistortionPoints, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TvBandVertex), (void*)offsetof(TvBandVertex, pos));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TvBandVertex), (void*)offsetof(TvBandVertex, uv));

        glBindVertexArray(0);
    }

    int itvbDrawMin = 0;
    int itvbDrawMax = 0;
    GetTvItvbMinMax(ptv, &itvbDrawMin, &itvbDrawMax);

    if (itvbDrawMin == itvbDrawMax)
        return;

    const float s = ptv->dy * 0.5f;

    float xCenter = ptv->x + ptv->xCurveCenter;
    float yBase = ptv->y + s;
    float xLeft = ptv->x + ptv->xFlatBase;

    glBlotShader.Use();

    glm::mat4 model(1.0f);
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

    glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);
    glUniform1i(u_useVertexColorLoc, 0);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    TvBandVertex verts[4];

    for (int i = itvbDrawMin; i < itvbDrawMax; i++)
    {
        TVB* band = &ptv->atvb[i];

        float yTop = yBase + band->yTop * s;
        float yBottom = yBase + band->yBottom * s;

        float xTopRight = xCenter + band->xCurveTop * s;
        float xBottomRight = xCenter + band->xCurveBottom * s;

        // Retail blends toward RGBA_Distortion, not the blue outline
        // color.  RGBA_Distortion is the neutral GS 0x80808080 color.
        const glm::vec4 rgbaDistortion(
            128.0f / 255.0f,
            128.0f / 255.0f,
            128.0f / 255.0f,
            1.0f);
        glm::vec4 color = glm::mix(band->rgbaCur, rgbaDistortion, band->uDistort);

        glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));

        verts[0] = { glm::vec2(xLeft,        yTop),    glm::vec2(0.0f, 0.0f) };
        verts[1] = { glm::vec2(xTopRight,    yTop),    glm::vec2(1.0f, 0.0f) };
        verts[2] = { glm::vec2(xLeft,        yBottom), glm::vec2(0.0f, 1.0f) };
        verts[3] = { glm::vec2(xBottomRight, yBottom), glm::vec2(1.0f, 1.0f) };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Retail follows the colored bands with a black point pass.  Distortion
    // controls both the number of points (2..5) and their random placement
    // inside each band's actual curved bounds.
    TvBandVertex distortionPoints[kMaxDistortionPoints];
    int distortionPointCount = 0;

    for (int i = itvbDrawMin; i < itvbDrawMax; ++i)
    {
        const TVB& band = ptv->atvb[i];
        const int pointCount = static_cast<int>(band.uDistort * 3.999f) + 2;
        const float xTop = xCenter + band.xCurveTop * s;
        const float xBottom = xCenter + band.xCurveBottom * s;
        const float xMin = std::min(xLeft, std::min(xTop, xBottom));
        const float xMax = std::max(xLeft, std::max(xTop, xBottom));
        const float yTop = yBase + band.yTop * s;
        const float yBottom = yBase + band.yBottom * s;

        for (int point = 0;
             point < pointCount && distortionPointCount < kMaxDistortionPoints;
             ++point)
        {
            distortionPoints[distortionPointCount++] =
            {
                glm::vec2(GRandInRange(xMin, xMax), GRandInRange(yTop, yBottom)),
                glm::vec2(0.0f)
            };
        }
    }

    if (distortionPointCount > 0)
    {
        const glm::vec4 distortionPointColor(0.0f, 0.0f, 0.0f, 1.0f);
        glUniform4fv(blotColorLoc, 1, glm::value_ptr(distortionPointColor));
        glBufferSubData(GL_ARRAY_BUFFER, 0,
            sizeof(TvBandVertex) * distortionPointCount, distortionPoints);
        // The GS points are one pixel on the authored 640 x 492.8 surface.
        // The TV geometry is resized into framebuffer pixels, so enlarge the
        // point rasterization by the same factor; otherwise the 100+ static
        // flecks remain single native-monitor pixels and nearly disappear.
        GLfloat pointSizePrevious = 1.0f;
        glGetFloatv(GL_POINT_SIZE, &pointSizePrevious);
        glPointSize(std::max(1.0f, TvUiScale()));
        glDrawArrays(GL_POINTS, 0, distortionPointCount);
        glPointSize(pointSizePrevious);
    }

    glBindVertexArray(0);

}

void DrawTvOutline(TV* ptv)
{
    if (!ptv)
        return;

    struct TvOutlineVertex
    {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    static GLuint vao = 0;
    static GLuint vbo = 0;
    static bool initialized = false;

    constexpr int MAX_VERTS = 256;

    if (!initialized)
    {
        initialized = true;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(TvOutlineVertex) * MAX_VERTS, nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TvOutlineVertex), (void*)offsetof(TvOutlineVertex, pos));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TvOutlineVertex), (void*)offsetof(TvOutlineVertex, uv));

        glBindVertexArray(0);
    }

    float uColor = 0.0f;

    if (ptv->tvs == TVS_Listen)
    {
        uColor = (g_clock.tReal - ptv->tTvs) * 5.0f;
        if (uColor < 0.0f) uColor = 0.0f;
        if (uColor > 1.0f) uColor = 1.0f;
    }
    else if (ptv->tvs == TVS_Talk)
    {
        uColor = 1.0f - (g_clock.tReal - ptv->tTvs) * 5.0f;
        if (uColor < 0.0f) uColor = 0.0f;
        if (uColor > 1.0f) uColor = 1.0f;
    }

    glm::vec4 color = glm::mix(RGBA_LightBlue, RGBA_DarkBlue, uColor);

    // PS2 GS alpha is normalized to 0x80. Both authored outline colors use
    // alpha 0x80, which is fully opaque for this blend pass.
    color.a = glm::clamp(color.a * (255.0f / 128.0f), 0.0f, 1.0f);

    int itvbDrawMin = 0;
    int itvbDrawMax = 0;
    GetTvItvbMinMax(ptv, &itvbDrawMin, &itvbDrawMax);

    if (itvbDrawMin == itvbDrawMax)
        return;

    const float s = ptv->dy * 0.5f;

    float xCurveCenter = ptv->x + ptv->xCurveCenter;
    float xBase = ptv->x + ptv->xFlatBase;
    float yBase = ptv->y + s;
    float ds = ptv->dsOutline;

    TvOutlineVertex verts[MAX_VERTS];
    int count = 0;

    TVB* first = &ptv->atvb[itvbDrawMin];

    float firstYTop = yBase + first->yTop * s;
    float firstXInner = xCurveCenter + first->xCurveTop * s;
    float firstXOuter = xCurveCenter + first->xCurveTop * (s + ds);

    verts[count++] = { glm::vec2(xBase,       firstYTop),      glm::vec2(0.0f, 0.0f) };
    verts[count++] = { glm::vec2(xBase,       firstYTop - ds), glm::vec2(0.0f, 1.0f) };
    verts[count++] = { glm::vec2(firstXInner, firstYTop),      glm::vec2(1.0f, 0.0f) };
    verts[count++] = { glm::vec2(firstXOuter, firstYTop - ds), glm::vec2(1.0f, 1.0f) };

    TVB* last = first;

    for (int i = itvbDrawMin; i < itvbDrawMax && count + 2 < MAX_VERTS; i++)
    {
        TVB* band = &ptv->atvb[i];
        last = band;

        float yInner = yBase + band->yTop * s;
        float xInner = xCurveCenter + band->xCurveTop * s;

        float yOuter = yBase + band->yTop * (s + ds);
        float xOuter = xCurveCenter + band->xCurveTop * (s + ds);

        verts[count++] = { glm::vec2(xInner, yInner), glm::vec2(0.0f, 0.0f) };
        verts[count++] = { glm::vec2(xOuter, yOuter), glm::vec2(1.0f, 1.0f) };
    }

    if (count + 4 < MAX_VERTS)
    {
        float yBottom = yBase + last->yBottom * s;

        float xBottomInner = xCurveCenter + last->xCurveBottom * s;
        float xBottomOuter = xCurveCenter + last->xCurveBottom * (s + ds);

        verts[count++] = { glm::vec2(xBottomInner, yBottom),      glm::vec2(0.0f, 0.0f) };
        verts[count++] = { glm::vec2(xBottomOuter, yBottom + ds), glm::vec2(1.0f, 1.0f) };
        verts[count++] = { glm::vec2(xBase,        yBottom),      glm::vec2(0.0f, 0.0f) };
        verts[count++] = { glm::vec2(xBase,        yBottom + ds), glm::vec2(1.0f, 1.0f) };
    }

    glBlotShader.Use();

    glm::mat4 model(1.0f);
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

    glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);
    glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));
    glUniform1i(u_useVertexColorLoc, 0);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TvOutlineVertex) * count, verts);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, count);

    glBindVertexArray(0);
}

TV g_tvRight;
TV g_tvLeft;
