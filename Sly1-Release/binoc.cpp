#include "binoc.h"
#include "aseg.h"
#include "totals.h"
#include "pzo.h"
#include "tv.h"

void StartupBinoc(BINOC* pbinoc)
{
    g_teBinoc.m_ch = '-';
    g_teBinoc.m_rgba = glm::vec4(0.0f, 0.2941f, 0.4902f, 1.0f);
    g_teBinoc.m_dxExtra = 2.0;
    g_teBinoc.m_ryScaling = 0.3;
    g_teBinoc.m_rxScaling = 0.3;

    pbinoc->pvtbinoc = &g_vtbinoc;
}

void InitBinoc(BINOC* pbinoc, BLOTK blotk)
{
    pbinoc->dx = g_gl.width;
    pbinoc->dy = g_gl.height;
    pbinoc->svch = 15.0;
    pbinoc->rgbaText = glm::vec4(1.0f, 1.0f, 1.0f, 128.0f / 255.0f);

    InitBlot(pbinoc, blotk);
}

void PostBinocLoad(BINOC* pbinoc)
{
    PostBlotLoad(pbinoc);
    ResetBinoc(pbinoc);

    // Clone and configure compass font
    pbinoc->pfontCompass = pbinoc->pfont->PfontClone(0.7f, 0.8f);
    pbinoc->pfontCompass->m_fGstest = 1;
    pbinoc->pfontCompass->m_gstest = 0x3f001;

    // Clone and assign base font with custom scale
    pbinoc->pfont = pbinoc->pfont->PfontClone(0.75f, 0.8f);

    if (FFontLoaded(2))
    {
        pbinoc->pte = &g_teBinoc;
        pbinoc->pte->m_pfont = PfontFromFont(2);
    }

    //Calculate max width of point labels
    g_dxPointsMax = pbinoc->pfontCompass->DxFromPchz((char*)g_aachzPoints[0]);

    for (int i = 1; i < 8; i++)
    {
        float dx = pbinoc->pfontCompass->DxFromPchz((char*)&g_aachzPoints[i]);
        if (dx > g_dxPointsMax)
            g_dxPointsMax = dx;
    }

    InitBei(&s_beiUpper, &s_clqUpper, 0.171875, 40.0, 0x18);
    InitBei(&s_beiLower, &s_clqLower, 0.171875, -60.0, 0x18);
    InitBei(&s_beiReticle, &s_clqReticle, 0.2857143, 8.0, 0x18);

    BuildBinocBackGround(pbinoc);
    BuildBinocOutline(pbinoc);

    float triangleVerts[] = {
        // x, y,    u, v
        0.0f, 0.0f, 0.0f, 0.0f, // bottom-left
        1.0f, 0.0f, 1.0f, 0.0f, // bottom-right
        0.5f, 1.0f, 0.5f, 1.0f  // top-center (middle of base)
    };

    glGenVertexArrays(1, &pbinoc->triangleBinocVAO);
    glGenBuffers(1, &pbinoc->triangleBinocVBO);

    glBindVertexArray(pbinoc->triangleBinocVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pbinoc->triangleBinocVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVerts), triangleVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // a_position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // a_texcoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    glGenVertexArrays(1, &pbinoc->binocIndicatorVAO);
    glGenBuffers(1, &pbinoc->binocIndicatorVBO);

    glBindVertexArray(pbinoc->binocIndicatorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pbinoc->binocIndicatorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

    glDisableVertexAttribArray(1);
    glVertexAttrib2f(1, 0.0f, 0.0f);

    glDisableVertexAttribArray(2);
    glVertexAttrib4f(2, 1.0f, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(0);

    // Dynamic position/color quad used by the reticle strips and ruler ticks.
    glGenVertexArrays(1, &pbinoc->reticleVAO);
    glGenBuffers(1, &pbinoc->reticleVBO);

    glBindVertexArray(pbinoc->reticleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pbinoc->reticleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);

    glDisableVertexAttribArray(1);
    glVertexAttrib2f(1, 0.0f, 0.0f);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
        reinterpret_cast<void*>(2 * sizeof(float)));

    glBindVertexArray(0);
}

void ResetBinoc(BINOC* pbinoc)
{
    pbinoc->pvtblot->pfnSetBlotAchzDraw(pbinoc, 0);
    SetBinocLookat(pbinoc, nullptr);
    pbinoc->uZoom = 0.0;
    pbinoc->uCompassBarOffset = 0.4;
    pbinoc->dxReticle = 0.0;
    pbinoc->dyReticle = 0.0;
}

void InitBei(BEI* pbei, const CLQ* pclq, float duWidth, float dgHeight, int cseg)
{
    float fcseg = static_cast<float>(cseg);
    int midSeg = cseg / 2;

    // Copy CLQ data into BEI
    pbei->cseg = fcseg;
    pbei->clq.g0 = pclq->g0;
    pbei->clq.g1 = pclq->g1;
    pbei->clq.g2 = pclq->g2;
    pbei->clq.gUnused = pclq->gUnused;

    // Segment range for the notch
    pbei->isegNotchMid = midSeg;

    float segWidth = 1.0f / fcseg;
    int halfNotchSegs = static_cast<int>(duWidth / segWidth) / 2;

    pbei->isegNotchFirst = midSeg - halfNotchSegs;
    pbei->isegNotchLast = midSeg + halfNotchSegs;
    pbei->csegNotchHalf = static_cast<float>(halfNotchSegs);

    float delta = static_cast<float>(pbei->isegNotchFirst) / fcseg;

    // Evaluate the quadratic curve at delta (g(t) = g0 + g1 * t + g2 * t^2)
    pbei->gNotchEdge = pclq->g0 + delta * (pclq->g1 + delta * pclq->g2);

    // Evaluate gNotchCenter using midpoint formula (g1 + g2 * 0.5) * 0.5 + g0 + dgHeight
    pbei->gNotchCenter = pclq->g0 + ((pclq->g1 + pclq->g2 * 0.5f) * 0.5f) + dgHeight;
}

float GEvaluateBei(const BEI& bei, int iseg)
{
    if (iseg < bei.isegNotchFirst || iseg > bei.isegNotchLast) {
        float t = static_cast<float>(iseg) / static_cast<float>(bei.cseg);
        return bei.clq.g0 + t * (bei.clq.g1 + t * bei.clq.g2);
    }

    if (iseg == bei.isegNotchFirst || iseg == bei.isegNotchLast) {
        return bei.gNotchEdge;
    }

    if (iseg < bei.isegNotchMid) {
        float t = static_cast<float>(iseg - bei.isegNotchFirst) / bei.csegNotchHalf;
        return (1.0f - t) * bei.gNotchEdge + t * bei.gNotchCenter;
    }

    if (iseg == bei.isegNotchMid) {
        return bei.gNotchCenter;
    }

    float t = static_cast<float>(iseg - bei.isegNotchMid) / bei.csegNotchHalf;
    return (1.0f - t) * bei.gNotchCenter + t * bei.gNotchEdge;
}

void GetBinocReticleFocus(BINOC* pbinoc, float* px, float* py)
{
    // Reticle geometry is authored in the original 640 x 492.8 UI space.
    // DrawBinocReticle performs the conversion to framebuffer pixels.
    *px = pbinoc->dxReticle + 320.0f;
    *py = pbinoc->dyReticle + 180.40001f;
}

void OnBinocPush(BINOC* pbinoc)
{
    DIALOG* pdialog = pbinoc->pdialogPlaying;

    if (pdialog == nullptr)
    {
        SetBinocBinocs(pbinoc, BINOCS_Peek);
        return;
    }

    switch (pdialog->dialogk)
    {
        case DIALOGK_Binoc:
        SetBinocBinocs(pbinoc, BINOCS_Dialog);
        break;

        case DIALOGK_Instruct:
        SetBinocBinocs(pbinoc, BINOCS_Instruct);
        break;

        case DIALOGK_Confront:
        SetBinocBinocs(pbinoc, BINOCS_Confront);
        break;

        default:
        break;
    }

    SetDialogDialogs(pdialog, DIALOGS_Playing);
}

void OnBinocPop(BINOC* pbinoc)
{
    if (pbinoc->pdialogPlaying == nullptr)
        SetBinocBinocs(pbinoc, BINOCS_None);
    else
        SetDialogDialogs(pbinoc->pdialogPlaying, DIALOGS_Calling);
}

void SetBinocBinocs(BINOC* pbinoc, BINOCS binocs)
{
    if (binocs == pbinoc->binocs)
        return;

    const BINOCS binocsPrev = pbinoc->binocs;

    if (binocsPrev == BINOCS_Peek)
        StartSound((SFXID)143, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
    else if (binocsPrev == BINOCS_None)
        ResetBinoc(pbinoc);

    if (binocs == BINOCS_Peek)
        StartSound((SFXID)142, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
    else if (binocs == BINOCS_Confront)
    {
        g_totals.fDrawOverLetterbox = 0;
        g_totals.pvtblot->pfnHideBlot(&g_totals);
    }

    pbinoc->binocs = binocs;
    pbinoc->tBinocs = g_clock.t;
}

void SetBinocBlots(BINOC* pbinoc, BLOTS blots)
{
    if (blots == pbinoc->blots)
        return;

    if (pbinoc->blots == BLOTS_Hidden)
    {
        ASEG* pasegCredits = reinterpret_cast<ASEG*>(PloFindSwObject(g_psw, 261, (OID)1173, nullptr));

        if (pasegCredits)
        {
            DLI dli;
            dli.m_pdl = &pasegCredits->dlAsega;
            dli.m_ibDle = pasegCredits->dlAsega.ibDle;
            dli.m_pdliNext = s_pdliFirst;

            ASEGA* pasega = reinterpret_cast<ASEGA*>(pasegCredits->dlAsega.paloFirst);

            while (pasega)
            {
                s_pdliFirst = &dli;

                dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(pasega) + dli.m_ibDle);
                ASEGA* pasegaNext = static_cast<ASEGA*>(*dli.m_ppv);

                SetAsegaSpeed(pasega, 0.0f);
                pasega = pasegaNext;
            }

            s_pdliFirst = dli.m_pdliNext;
        }
    }

    if (blots == BLOTS_Hidden)
    {
        ASEG* pasegCredits = reinterpret_cast<ASEG*>(PloFindSwObject(g_psw, 261, (OID)1173, nullptr));

        if (pasegCredits)
        {
            DLI dli;
            dli.m_pdl = &pasegCredits->dlAsega;
            dli.m_ibDle = pasegCredits->dlAsega.ibDle;
            dli.m_pdliNext = s_pdliFirst;

            ASEGA* pasega = reinterpret_cast<ASEGA*>(pasegCredits->dlAsega.paloFirst);

            while (pasega)
            {
                s_pdliFirst = &dli;

                dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(pasega) + dli.m_ibDle);
                ASEGA* pasegaNext = static_cast<ASEGA*>(*dli.m_ppv);

                SetAsegaSpeed(pasega, 1.0f);
                pasega = pasegaNext;
            }

            s_pdliFirst = dli.m_pdliNext;
        }
    }

    SetBlotBlots(pbinoc, blots);
}

int NCmpSlo(const SLO* pslo1, const SLO* pslo2)
{
    const bool fInWorld1 = FIsLoInWorld(pslo1->plo) != 0;
    const bool fInWorld2 = FIsLoInWorld(pslo2->plo) != 0;

    if (fInWorld1 != fInWorld2)
        return fInWorld1 ? -1 : 1;

    if (pslo1->s < pslo2->s)
        return -1;

    if (pslo1->s > pslo2->s)
        return 1;

    return 0;
}

int NCmpClueSlo(const SLO* pslo1, const SLO* pslo2)
{
    uint32_t fCollected1 = 0;
    uint32_t fCollected2 = 0;

    GetClueCollected(reinterpret_cast<CLUE*>(pslo1->plo), &fCollected1);
    GetClueCollected(reinterpret_cast<CLUE*>(pslo2->plo), &fCollected2);

    if (fCollected1 != fCollected2)
        return fCollected1 ? 1 : -1;

    if (!fCollected1 && !fCollected2)
        return pslo1->s < pslo2->s ? -1 : 1;

    return 1;
}

void UpdateFilterRose(BINOC* pbinoc, CID cid, PFNSPHERE pfnSphere, PFNROSE pfnRose, int cploMax, float sMax, ALO* paloRender)
{
    SLO aslo[128];
    LO* aplo[128];

    const int cplo = CploFindSwObjectsByClass(g_psw, 261, cid, nullptr, 128, aplo);

    for (int i = 0; i < cplo; ++i)
    {
        SLO& slo = aslo[i];

        slo.plo = aplo[i];
        pfnSphere(slo.plo, &slo.pos, &slo.sRadius);
        slo.s = glm::distance(slo.pos, g_pcm->pos) - slo.sRadius;
    }

    if (cid == 46)
        std::sort(aslo, aslo + cplo, [](const SLO& a, const SLO& b) { return NCmpClueSlo(&a, &b) < 0; });
    else
        std::sort(aslo, aslo + cplo, [](const SLO& a, const SLO& b) { return NCmpSlo(&a, &b) < 0; });

    std::memset(pbinoc->mpinormalf, 0, sizeof(pbinoc->mpinormalf));

    bool fCanCreateRose = g_clock.t - pbinoc->tLastRose > DT_BinocRoseInterval;

    for (int i = 0; i < cplo; ++i)
    {
        SLO& slo = aslo[i];

        bool fCandidate = false;

        if (pbinoc->binocs == BINOCS_Peek && i < cploMax && slo.s < sMax)
        {
            if (cid == 46)
            {
                uint32_t fCollected = 0;
                GetClueCollected(reinterpret_cast<CLUE*>(slo.plo), &fCollected);
                fCandidate = fCollected == 0;
            }
            else
                fCandidate = FIsLoInWorld(slo.plo) != 0;
        }

        const glm::vec3 dpos = slo.pos - g_pcm->pos;
        const bool fInsideFrustum = SphereInFrustum(g_pcm->frustum, dpos, slo.sRadius);

        if (fCandidate && !fInsideFrustum)
        {
            const float radPan = std::atan2(dpos.y, dpos.x);
            const float sXY = std::sqrt(dpos.x * dpos.x + dpos.y * dpos.y);
            const float radTilt = std::atan2(dpos.z, sXY);

            const float dradPan = RadNormalize(radPan - g_pcm->cplook.radPan);
            const float dradTilt = RadNormalize(radTilt - g_pcm->cplook.radTilt);
            const float radFOV = g_pcm->radFOV;

            if (dradPan > radFOV * g_pcm->rAspect * 0.45f)
                pbinoc->mpinormalf[0] = 1;
            else if (dradPan < radFOV * g_pcm->rAspect * -0.45f)
                pbinoc->mpinormalf[1] = 1;
            else if (dradTilt > radFOV * 0.45f)
                pbinoc->mpinormalf[2] = 1;
            else if (dradTilt < radFOV * -0.45f)
                pbinoc->mpinormalf[3] = 1;
        }

        ROSE* prose = nullptr;
        int ibre = 0;

        for (; ibre < pbinoc->cbre; ++ibre)
        {
            if (pbinoc->abre[ibre].plo == slo.plo)
            {
                prose = pbinoc->abre[ibre].prose;
                break;
            }
        }

        if (!prose)
        {
            if (fCandidate && fCanCreateRose && fInsideFrustum)
            {
                prose = reinterpret_cast<ROSE*>(PripNewRipg(RIPT_Rose, nullptr));

                if (prose)
                {
                    fCanCreateRose = false;

                    prose->pvtrip->pfnInitRip(prose, &slo.pos, 1.0f, nullptr);
                    prose->paloRender = paloRender;
                    prose->pfnrose = pfnRose;
                    prose->pv = slo.plo;

                    SetRoseRoses(prose, ROSES_Closing);

                    pbinoc->tLastRose = g_clock.t;
                    pbinoc->abre[pbinoc->cbre].plo = slo.plo;
                    pbinoc->abre[pbinoc->cbre].prose = prose;
                    ++pbinoc->cbre;
                }
            }
        }
        else if (!fCandidate)
        {
            SetRoseRoses(prose, ROSES_Opening);

            std::memmove(&pbinoc->abre[ibre], &pbinoc->abre[ibre + 1], static_cast<size_t>(pbinoc->cbre - ibre - 1) * sizeof(pbinoc->abre[0]));
            --pbinoc->cbre;
        }
    }
}

void GetSoSphere(LO* plo, glm::vec3* ppos, float* psRadius)
{
    SO* pso = static_cast<SO*>(plo);

    *ppos = pso->xf.posWorld;
    *psRadius = pso->sRadiusAll;
}

int ChpFillAlo(void* pv, int chpMax, HP* ahp)
{
    ALO* palo = static_cast<ALO*>(pv);

    if (!palo || !ahp || chpMax < 1)
        return 0;

    ahp[0].pos = glm::vec4(glm::vec3(palo->xf.posWorld), 1.0f);

    return ChpBuildConvexHullScreen(&palo->xf.posWorld, 1, ahp);
}

void SetBinocBfk(BINOC* pbinoc, BFK bfk, int fImmediate)
{
    if (bfk == pbinoc->bfk)
        return;

    for (int ibre = 0; ibre < pbinoc->cbre; ++ibre)
    {
        ROSE* prose = pbinoc->abre[ibre].prose;

        if (fImmediate)
            RemoveRip(prose);
        else
            SetRoseRoses(prose, ROSES_Opening);
    }

    if (fImmediate && pbinoc->cbre > 0)
        std::memset(pbinoc->mpinormalu, 0, sizeof(pbinoc->mpinormalu));

    pbinoc->cbre = 0;
    pbinoc->tLastRose = 0.0f;
    std::memset(pbinoc->mpinormalf, 0, sizeof(pbinoc->mpinormalf));
    pbinoc->bfk = bfk;
}

void UpdateBinocActiveFilter(BINOC* pbinoc, JOY* pjoy)
{
    GRFVAULT grfvault[4]{};
    GetBlueprintInfo(grfvault, 0);

    const GRFVAULT grfvaultAvailable = GetAvailableVaultFlags();

    if ((grfvaultAvailable & grfvault[0]) == 0)
    {
        SetBinocBfk(pbinoc, static_cast<BFK>(0), 1);
        return;
    }

    SetBinocBfk(pbinoc, BFK_Clue, 0);

    if (pbinoc->bfk == BFK_Clue)
    {
        const float sMax = FLT_MAX;

        UpdateFilterRose(pbinoc, static_cast<CID>(46), GetSoSphere, ChpFillAlo, 5, sMax, (ALO*)g_psw->aploStock[23]);
        UpdateFilterRose(pbinoc, static_cast<CID>(33), GetSoSphere, ChpFillAlo, 5, sMax, (ALO*)g_psw->aploStock[27]);
    }
}

void UpdateBinocActive(BINOC* pbinoc, JOY* pjoy)
{
    if (pbinoc->binocs == BINOCS_Peek)
    {
        SMP smpReticle =
        {
            80.0f,
            50.0f,
            0.5f
        };

        SMP smpReticleFast =
        {
            250.0f,
            200.0f,
            0.1f
        };

        float xReticle = std::clamp(pjoy->x + pjoy->x2, -1.0f, 1.0f);
        float yReticle = std::clamp(pjoy->y, -1.0f, 1.0f);

        if ((g_pgsCur->grfgs & 512U) != 0)
            yReticle = -yReticle;

        glm::vec3 posCur(pbinoc->dxReticle, pbinoc->dyReticle, 0.0f);
        glm::vec3 posTarget(xReticle * 50.0f, yReticle * 50.0f, 0.0f);

        SMP* psmp = pjoy->uDeflect + pjoy->uDeflect2 > 0.0001f ? &smpReticle : &smpReticleFast;

        glm::vec3 posNew;
        PosSmooth(posCur, posTarget, g_clock.dt, psmp, &posNew);

        pbinoc->dxReticle = posNew.x;
        pbinoc->dyReticle = posNew.y;
        pbinoc->uCompassBarOffset = GModPositive(pbinoc->uCompassBarOffset - xReticle * 0.08f, 1.0f);
    }
    else if (pbinoc->binocs == BINOCS_Sniper)
        pbinoc->dyReticle = 50.0f;

    UpdateBinocActiveFilter(pbinoc, pjoy);
}

void OnBinocActive(BINOC* pbinoc, int fActive)
{
    const bool fWasActive = pbinoc->fActive != 0;
    const bool fActivate = fActive != 0;

    if (fActivate == fWasActive)
        return;

    if (pbinoc->pdialogPlaying && pbinoc->pdialogPlaying->dialogs == DIALOGS_Playing)
    {
        if (fActivate)
        {
            ContinueVag();
        }
        else
        {
            PauseVag();
        }
    }

    pbinoc->fActive = fActive;
    pbinoc->fTargeting = 0;

    if (!fActivate)
    {
        SetBinocBlots(pbinoc, BLOTS_Hidden);

        pbinoc->bfkPrev = pbinoc->bfk;
        SetBinocBfk(pbinoc, BFK_None, 1);
        return;
    }

    SetBinocBlots(pbinoc, BLOTS_Visible);

    GRFVAULT grfvault[4]{};
    GetBlueprintInfo(grfvault, 0);

    const GRFVAULT grfvaultAvailable = GetAvailableVaultFlags();

    if ((grfvaultAvailable & grfvault[0]) != 0 && (grfvaultAvailable & s_mpbfkgrfvault[pbinoc->bfkPrev]) != 0)
        SetBinocBfk(pbinoc, pbinoc->bfkPrev, 1);

    if ((grfvaultAvailable & 0x800) != 0)
    {
        SCAN* apscan[256];

        int cpscan = CploFindSwObjectsByClass(g_psw, 517, CID_SCAN, nullptr, 256, reinterpret_cast<LO**>(apscan));
        cpscan = std::min(cpscan, 256);

        for (int i = 0; i < cpscan; ++i)
            InitializeScanDisplay(apscan[i]);
    }
}

void OnBinocReset(BINOC* pbinoc)
{
    OnBlotReset(pbinoc);

    if (pbinoc->pdialogPlaying != nullptr) 
        SetDialogDialogs(pbinoc->pdialogPlaying, DIALOGS_Unavailable);
}

void SetBinocAchzDraw(BINOC* pbinoc, char* pchz)
{
    pbinoc->cichLR = 0;  // Reset line break counter

    if (pchz && *pchz != '\0') {
        strcpy(pbinoc->achzDraw, pchz);

        // Choose scaling based on Binoc mode
        float scale = (pbinoc->binocs == BINOCS_Confront) ? 0.8f : 1.0f;
        pbinoc->pfont->PushScaling(scale, scale);

        // Wrap rich text for this string
        CRichText rt(pbinoc->achzDraw, pbinoc->pfont);

        // Choose wrap width based on Binoc mode
        float wrapWidth = (pbinoc->binocs == BINOCS_Confront) ? 380.0f : 280.0f;
        rt.ClineWrap(wrapWidth);
        pbinoc->pfont->PopScaling();

        // Reset parser and extract line break indices
        rt.Reset();
        int glyphIndex = 0;
        char ch;
        while ((ch = rt.ChNext()) != '\0') {
            if (ch == '\n') {
                pbinoc->aichLR[pbinoc->cichLR++] = glyphIndex;
            }
            glyphIndex++;
        }
    }
    else {
        pbinoc->achzDraw[0] = '\0'; // Empty string fallback
    }

    // Timestamp when text was set (used for animations/fades?)
    pbinoc->tAchzSet = g_clock.t;
}

int FDoneBinocAchz(BINOC* pbinoc)
{
    if (pbinoc->achzDraw[0] == '\0')
        return true;

    CRichText rt(pbinoc->achzDraw, pbinoc->pfont);
    return static_cast<int>((g_clock.t - pbinoc->tAchzSet) * pbinoc->svch) >= rt.Cch();
}

void SetBinocLookat(BINOC* pbinoc, ALO* paloLookat)
{
    pbinoc->paloLookat = paloLookat;
}

void SetBinocZoom(BINOC* pbinoc, float zoomPercent)
{
    pbinoc->uZoom = glm::clamp(zoomPercent * 0.01f, 0.0f, 1.0f);
}

float DtAppearBinoc(BINOC* pbinoc)
{
    float dt;

    switch (pbinoc->binocs)
    {
        case BINOCS_Peek:
        case BINOCS_Dialog:
        case BINOCS_Sniper:
        dt = 0.0;
        break;
        default:
        dt = DtAppearBlot(pbinoc);
    }
    return dt;
}

float DtDisappearBinoc(BINOC* pbinoc)
{
    float dt;

    switch (pbinoc->binocs)
    {
        case BINOCS_Peek:
        case BINOCS_Dialog:
        case BINOCS_Sniper:
        dt = 0.0;
        break;
        default:
        dt = DtDisappearBlot(pbinoc);
    }
    return dt;
}

void BuildBinocBackGround(BINOC* pbinoc)
{
    if (pbinoc->backGroundBinocVAO != 0)
    {
        glDeleteVertexArrays(1, &pbinoc->backGroundBinocVAO);
        glDeleteBuffers(1, &pbinoc->backGroundBinocVBO);
        glDeleteBuffers(1, &pbinoc->backGroundBinocEBO);
        pbinoc->backGroundBinocVAO = 0;
        pbinoc->backGroundBinocVBO = 0;
        pbinoc->backGroundBinocEBO = 0;
    }

    // ---- Virtual space (matches original authoring) ----
    const float Vw = 640.0f;
    const float Vh = 492.8f;

    // Original uses 1/24 steps across width -> 25 columns (0..24)
    const int columns = 25;          // 0..24
    const int segments = columns - 1;

    // Tunable "flat edges" in VIRTUAL Y.
    // You should adjust these to match your look if GEvaluateBei returns a different baseline.
    const float upperFlatY = 0.0f;    // top flat edge of upper band (virtual)
    const float lowerFlatY = Vh;      // bottom flat edge of lower band (virtual)

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    std::vector<Vertex> vertices;
    std::vector<uint16_t>& indices = pbinoc->backGroundBinocIndices;
    vertices.clear();
    indices.clear();

    vertices.reserve(segments * 4 * 2);
    indices.reserve(segments * 6 * 2);

    auto pushQuad = [&](uint16_t base) {
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        };

    // Evaluator wrapper to emulate original: interior indices are 1..23.
    // If your GEvaluateBei already supports 0..24, this still works.
    auto evalBei = [&](const BEI& bei, int colIndex) -> float 
    {
        // Clamp to [0..24]
        if (colIndex < 0) colIndex = 0;
        if (colIndex > 24) colIndex = 24;

        // Original loop used i=1..23; endpoints were explicit.
        // If your evaluator expects 1..24, this still clamps safely.
        return GEvaluateBei(bei, colIndex);
    };

    // ---------- Upper band ----------
    for (int s = 0; s < segments; ++s)
    {
        int c0 = s;
        int c1 = s + 1;

        float t0 = float(c0) / 24.0f;
        float t1 = float(c1) / 24.0f;

        float x0 = t0 * Vw;
        float x1 = t1 * Vw;

        float y0Curve = evalBei(s_beiUpper, c0);
        float y1Curve = evalBei(s_beiUpper, c1);

        uint16_t base = (uint16_t)vertices.size();

        // Flat edge (top), then curve edge
        vertices.push_back({ {x0, upperFlatY}, {t0, 0.0f} });
        vertices.push_back({ {x1, upperFlatY}, {t1, 0.0f} });
        vertices.push_back({ {x1, y1Curve},    {t1, 1.0f} });
        vertices.push_back({ {x0, y0Curve},    {t0, 1.0f} });

        pushQuad(base);
    }

    // ---------- Lower band ----------
    for (int s = 0; s < segments; ++s)
    {
        int c0 = s;
        int c1 = s + 1;

        float t0 = float(c0) / 24.0f;
        float t1 = float(c1) / 24.0f;

        float x0 = t0 * Vw;
        float x1 = t1 * Vw;

        float y0Curve = evalBei(s_beiLower, c0);
        float y1Curve = evalBei(s_beiLower, c1);

        uint16_t base = (uint16_t)vertices.size();

        // Curve edge, then flat edge (bottom)
        vertices.push_back({ {x0, y0Curve}, {t0, 1.0f} });
        vertices.push_back({ {x1, y1Curve}, {t1, 1.0f} });
        vertices.push_back({ {x1, lowerFlatY}, {t1, 0.0f} });
        vertices.push_back({ {x0, lowerFlatY}, {t0, 0.0f} });

        pushQuad(base);
    }

    // ---- Upload ----
    glGenVertexArrays(1, &pbinoc->backGroundBinocVAO);
    glGenBuffers(1, &pbinoc->backGroundBinocVBO);
    glGenBuffers(1, &pbinoc->backGroundBinocEBO);

    glBindVertexArray(pbinoc->backGroundBinocVAO);

    glBindBuffer(GL_ARRAY_BUFFER, pbinoc->backGroundBinocVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pbinoc->backGroundBinocEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    glBindVertexArray(0);
}

void BuildBinocOutline(BINOC* pbinoc)
{
    if (pbinoc->outlineVAO != 0)
    {
        glDeleteVertexArrays(1, &pbinoc->outlineVAO);
        glDeleteBuffers(1, &pbinoc->outlineVBO);
        glDeleteBuffers(1, &pbinoc->outlineColorVBO);
        glDeleteBuffers(1, &pbinoc->outlineEBO);
        pbinoc->outlineVAO = 0;
        pbinoc->outlineVBO = 0;
        pbinoc->outlineColorVBO = 0;
        pbinoc->outlineEBO = 0;
    }

    const float Vw = 640.0f;
    const float Vh = 492.8f;

    const int columns = 25;      // 0..24
    const int segments = columns - 1;

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    std::vector<Vertex> vertices;
    std::vector<uint16_t>& indices = pbinoc->outlineIndices;

    vertices.clear();
    indices.clear();

    auto pushQuad = [&](uint16_t base)
    {
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    };

    auto evalBei = [&](const BEI& bei, int colIndex) -> float 
    {
        if (colIndex < 0) colIndex = 0;
        if (colIndex > 24) colIndex = 24;
        return GEvaluateBei(bei, colIndex);
    };

    auto emitBand = [&](const BEI& bei, float direction, float offset0, float offset1)
    {
        for (int s = 0; s < segments; ++s)
        {
            int c0 = s;
            int c1 = s + 1;

            float t0 = float(c0) / 24.0f;
            float t1 = float(c1) / 24.0f;

            float x0 = t0 * Vw;
            float x1 = t1 * Vw;

            float y0Curve = evalBei(bei, c0);
            float y1Curve = evalBei(bei, c1);

            float y0a = y0Curve + offset0 * direction;
            float y1a = y1Curve + offset0 * direction;

            float y0b = y0Curve + offset1 * direction;
            float y1b = y1Curve + offset1 * direction;

            uint16_t base = (uint16_t)vertices.size();

            vertices.push_back({ {x0, y0a}, {t0, 0.0f} });
            vertices.push_back({ {x1, y1a}, {t1, 0.0f} });
            vertices.push_back({ {x1, y1b}, {t1, 1.0f} });
            vertices.push_back({ {x0, y0b}, {t0, 1.0f} });

            pushQuad(base);
        }
    };

    // Original emits two four-pixel strips on each edge. Colors are supplied
    // dynamically so each strip interpolates between the animated phases.
    emitBand(s_beiUpper, +1.0f, 0.0f, 4.0f);
    emitBand(s_beiUpper, +1.0f, 4.0f, 8.0f);

    emitBand(s_beiLower, -1.0f, 0.0f, 4.0f);
    emitBand(s_beiLower, -1.0f, 4.0f, 8.0f);

    glGenVertexArrays(1, &pbinoc->outlineVAO);
    glGenBuffers(1, &pbinoc->outlineVBO);
    glGenBuffers(1, &pbinoc->outlineColorVBO);
    glGenBuffers(1, &pbinoc->outlineEBO);

    glBindVertexArray(pbinoc->outlineVAO);

    glBindBuffer(GL_ARRAY_BUFFER, pbinoc->outlineVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pbinoc->outlineEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    std::vector<glm::vec4> colors(vertices.size(), glm::vec4(1.0f));
    glBindBuffer(GL_ARRAY_BUFFER, pbinoc->outlineColorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);

    glBindVertexArray(0);
}

void DrawBinocReticle(BINOC* pbinoc)
{
    JOY* pjoy = (g_grfjoyt & 2U) != 0 ? &g_joy : &g_joyZero;

    float spinSpeed = 3.0f;
    if (pjoy->y2 > 0.0f)
        spinSpeed = 6.0f;
    else if (pjoy->y2 < 0.0f)
        spinSpeed = -6.0f;

    pbinoc->radReticle = RadNormalize(pbinoc->radReticle + g_clock.dt * spinSpeed);

    const float uZoom = g_pcm->cplook.uZoom;
    const float scale = uZoom + 1.0f;
    const float alpha = glm::clamp(48.0f + uZoom * 24.999f, 0.0f, 255.0f) / 255.0f;

    const glm::vec4 darkBlue(RGBA_DarkBlue.r, RGBA_DarkBlue.g, RGBA_DarkBlue.b, alpha);
    const glm::vec4 lightBlue(RGBA_LightBlue.r, RGBA_LightBlue.g, RGBA_LightBlue.b, alpha);
    const glm::vec4 darkRed(RGBA_DarkRed.r, RGBA_DarkRed.g, RGBA_DarkRed.b, alpha);
    const glm::vec4 lightRed(RGBA_LightRed.r, RGBA_LightRed.g, RGBA_LightRed.b, alpha);
    const glm::vec4 rgbaDark = pbinoc->fTargeting != 0 ? darkRed : darkBlue;
    const glm::vec4 rgbaLight = pbinoc->fTargeting != 0 ? lightRed : lightBlue;

    const float outerPhase = 0.5f - 0.5f * std::cos(pbinoc->radReticle - 0.19634955f);
    const float innerPhase = 0.5f - 0.5f * std::cos(pbinoc->radReticle - 0.44178647f);
    const glm::vec4 rgbaOuter = glm::mix(rgbaDark, rgbaLight, outerPhase);
    const glm::vec4 rgbaInner = glm::mix(rgbaDark, rgbaLight, innerPhase);

    float xFocus;
    float yFocus;
    GetBinocReticleFocus(pbinoc, &xFocus, &yFocus);

    constexpr float kUiWidth = 640.0f;
    constexpr float kUiHeight = 492.80002f;
    const float targetWidth = static_cast<float>(g_gl.width);
    const float targetHeight = static_cast<float>(g_gl.height);
    const float uiScale = glm::min(targetWidth / kUiWidth, targetHeight / kUiHeight);
    const float uiOriginX = (targetWidth - kUiWidth * uiScale) * 0.5f;
    const float uiOriginY = (targetHeight - kUiHeight * uiScale) * 0.5f;

    struct ReticleVertex
    {
        float x;
        float y;
        float r;
        float g;
        float b;
        float a;
    };

    std::vector<ReticleVertex> reticleVertices;
    reticleVertices.reserve((24 * 2 + 64 * 2 + 32 * 2) * 6);

    auto vertex = [&](const glm::vec2& pos, const glm::vec4& color)
    {
        return ReticleVertex{
            uiOriginX + pos.x * uiScale,
            uiOriginY + pos.y * uiScale,
            color.r, color.g, color.b, color.a
        };
    };

    glBlotShader.Use();
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    const glm::mat4 model(1.0f);
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);
	// The fragment shader multiplies blotColor by the per-vertex reticle
	// colors. DrawBinocBackground leaves this set to RGBA_Overlay, so reset it
	// here or the reticle inherits the dark overlay tint.
	glUniform4f(blotColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    glUniform1i(u_useVertexColorLoc, 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    auto drawQuad = [&](const glm::vec2& p0, const glm::vec2& p1,
                        const glm::vec2& p2, const glm::vec2& p3,
                        const glm::vec4& c0, const glm::vec4& c1,
                        const glm::vec4& c2, const glm::vec4& c3)
    {
        reticleVertices.push_back(vertex(p0, c0));
        reticleVertices.push_back(vertex(p1, c1));
        reticleVertices.push_back(vertex(p2, c2));
        reticleVertices.push_back(vertex(p0, c0));
        reticleVertices.push_back(vertex(p2, c2));
        reticleVertices.push_back(vertex(p3, c3));
    };

    // The center reticle is two 24-segment curved strips. Each strip fades
    // between the two independently phased colors across its thickness.
    const float xLeft = xFocus - scale * 35.0f;
    glm::vec2 upperInnerPrev(xLeft, yFocus - scale * 8.0f);
    glm::vec2 upperOuterPrev(xLeft, yFocus - scale * 3.0f);
    glm::vec2 lowerInnerPrev(xLeft, yFocus + scale * 8.0f);
    glm::vec2 lowerOuterPrev(xLeft, yFocus + scale * 3.0f);

    for (int i = 1; i <= 24; ++i)
    {
        const float x = xLeft + scale * (static_cast<float>(i) / 24.0f) * 70.0f;
        const float curve = scale * GEvaluateBei(s_beiReticle, i);
        const float edge = (i == 24) ? 4.0f : 5.0f;
        const glm::vec2 upperInner(x, yFocus + curve);
        const glm::vec2 upperOuter(x, yFocus + curve + scale * edge);
        const glm::vec2 lowerInner(x, yFocus - curve);
        const glm::vec2 lowerOuter(x, yFocus - curve - scale * edge);

        drawQuad(upperInnerPrev, upperOuterPrev, upperOuter, upperInner,
            rgbaInner, rgbaOuter, rgbaOuter, rgbaInner);
        drawQuad(lowerInnerPrev, lowerOuterPrev, lowerOuter, lowerInner,
            rgbaInner, rgbaOuter, rgbaOuter, rgbaInner);

        upperInnerPrev = upperInner;
        upperOuterPrev = upperOuter;
        lowerInnerPrev = lowerInner;
        lowerOuterPrev = lowerOuter;
    }

    if (pbinoc->binocs != BINOCS_Sniper)
    {
        // Original fixed horizontal ruler: 64 mirrored pairs, dx 41..545.
        for (int i = 0; i < 64; ++i)
        {
            const float dx = 41.0f + static_cast<float>(i) * 8.0f;
            const float phase = 0.5f - 0.5f * std::cos(
                pbinoc->radReticle + (dx / 256.0f) * glm::pi<float>());
            const glm::vec4 color = glm::mix(rgbaDark, rgbaLight, phase);
            const float y = yFocus - scale * 5.0f;

            glm::vec2 p0(xFocus + scale * dx, y);
            glm::vec2 p1 = p0 + glm::vec2(scale * 2.0f, 0.0f);
            glm::vec2 p2 = p0 + glm::vec2(scale * 2.0f, scale * 10.0f);
            glm::vec2 p3 = p0 + glm::vec2(0.0f, scale * 10.0f);
            drawQuad(p0, p1, p2, p3, color, color, color, color);

            p0 = glm::vec2(xFocus - scale * dx, y);
            p1 = p0 - glm::vec2(scale * 2.0f, 0.0f);
            p2 = p1 + glm::vec2(0.0f, scale * 10.0f);
            p3 = p0 + glm::vec2(0.0f, scale * 10.0f);
            drawQuad(p0, p1, p2, p3, color, color, color, color);
        }

        // The GS clips the ruler to the curved BINOC aperture. Determine the
        // aperture at the ruler's current X so the horizontal tick marks do
        // not continue into the black/dialog region below the lens.
        const int focusSegment = std::clamp(
            static_cast<int>(std::round((xFocus / 640.0f) * 24.0f)), 0, 24);
        const float apertureY0 = GEvaluateBei(s_beiUpper, focusSegment);
        const float apertureY1 = GEvaluateBei(s_beiLower, focusSegment);
        const float apertureTop = std::min(apertureY0, apertureY1);
        const float apertureBottom = std::max(apertureY0, apertureY1);

        // Original fixed vertical ruler: 32 mirrored pairs, dy 24..272.
        for (int i = 0; i < 32; ++i)
        {
            const float dy = 24.0f + static_cast<float>(i) * 8.0f;
            const float phase = 0.5f - 0.5f * std::cos(
                pbinoc->radReticle + (dy / 128.0f) * glm::pi<float>());
            const glm::vec4 color = glm::mix(rgbaDark, rgbaLight, phase);
            const float x = xFocus - scale * 5.0f;

            const float yPositive = yFocus + scale * dy;
            if (yPositive >= apertureTop && yPositive + scale * 2.0f <= apertureBottom)
            {
                glm::vec2 p0(x, yPositive);
                glm::vec2 p1 = p0 + glm::vec2(scale * 10.0f, 0.0f);
                glm::vec2 p2 = p0 + glm::vec2(scale * 10.0f, scale * 2.0f);
                glm::vec2 p3 = p0 + glm::vec2(0.0f, scale * 2.0f);
                drawQuad(p0, p1, p2, p3, color, color, color, color);
            }

            const float yNegative = yFocus - scale * dy;
            if (yNegative - scale * 2.0f >= apertureTop && yNegative <= apertureBottom)
            {
                glm::vec2 p0(x, yNegative);
                glm::vec2 p1 = p0 + glm::vec2(scale * 10.0f, 0.0f);
                glm::vec2 p2 = p1 - glm::vec2(0.0f, scale * 2.0f);
                glm::vec2 p3 = p0 - glm::vec2(0.0f, scale * 2.0f);
                drawQuad(p0, p1, p2, p3, color, color, color, color);
            }
        }
    }

    // Retail submits the complete reticle in one GIF packet. Keep the same
    // ownership model here: upload only this frame's completed geometry, then
    // issue one draw. Repeatedly overwriting a six-vertex streaming buffer
    // between queued draws can leave more than one reticle position visible
    // while the focus is moving.
    if (!reticleVertices.empty())
    {
        glBindVertexArray(pbinoc->reticleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pbinoc->reticleVBO);
        glBufferData(GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(reticleVertices.size() * sizeof(ReticleVertex)),
            reticleVertices.data(), GL_STREAM_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(reticleVertices.size()));
    }

    glBindVertexArray(0);
    glUniform1i(u_useVertexColorLoc, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void DrawBinocBackground(BINOC* pbinoc)
{
    GLint previousDepthFunction = GL_LESS;
    GLboolean previousDepthMask = GL_TRUE;
    const GLboolean depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST);

    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunction);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthMask);

    glBlotShader.Use();

    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

    // Scale from virtual 640x492.8 -> screen pixels
    const float sx = g_gl.width / 640.0f;
    const float sy = g_gl.height / 492.8f;

    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(sx, sy, 1.0f));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniform4f(uvRectLoc, 0, 0, 1, 1);
    glUniform4fv(blotColorLoc, 1, glm::value_ptr(RGBA_Overlay));
    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // The original BINOC overlay is submitted after the TV speaker, but
    // PostTvContext seals successful speaker pixels at the overlay depth.
    // Keeping the depth test enabled makes this tint cover the world while
    // rejecting those protected TV pixels. Disabling depth here caused the
    // overlay to darken the speaker portraits as well.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE);

    glBindVertexArray(pbinoc->backGroundBinocVAO);
    glDrawElements(GL_TRIANGLES, (GLsizei)pbinoc->backGroundBinocIndices.size(), GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glDepthMask(previousDepthMask);
    glDepthFunc(previousDepthFunction);

    if (depthTestWasEnabled)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

inline float FadeChannel(float base, float target, float t)
{
    if (base == target) return base;
    return (base < target) ? base + (target - base) * t : base - (base - target) * (1.0f - t);
}

void DrawBinocCompass(BINOC* pbinoc)
{
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_STENCIL_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // RECTANGLE
    float width = (g_gl.width / 640.0f) * 94.0f;
    float height = ((g_gl.height) / 492.8f) * 35.0f;

    float x = (g_gl.width - width) * 0.5f;
    float y = ((g_gl.height) - height) * 0.02f;

    glm::vec2 pos = glm::vec2(x, y);
    glm::vec2 size = glm::vec2(width, height);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    glBlotShader.Use();

    glStencilFunc(GL_ALWAYS, 0, 255);
    glStencilOp(GL_KEEP, GL_KEEP, GL_NONE);
    glColorMask(0, 0, 0, 0);

    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);
    glUniform4f(blotColorLoc, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
    glBindVertexArray(g_gl.gao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    // TRIANGLE
    model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(width, height, 1.0f));
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glBindVertexArray(pbinoc->triangleBinocVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(g_gl.gao);

    // ===== UI scale (virtual 640x492.8 -> your pixel projection) =====
    const float uiScaleX = g_gl.width / 640.0f;
    const float uiScaleY = (g_gl.height) / 492.8f;

    // TICKS
    const float baseX = 273.0f;
    const float baseY = 9.0f;

    const float tickWidth = 7.52f * uiScaleX;
    const float tickHeight = 35.5f * uiScaleY;
    y = baseY * uiScaleY;

    glStencilFunc(GL_NOTEQUAL, 0, 255);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glColorMask(1, 1, 1, 1);

    glUniform4f(blotColorLoc, RGBA_DarkBlue.r, RGBA_DarkBlue.g, RGBA_DarkBlue.b, RGBA_DarkBlue.a);

    for (int i = 0; i < 11; i++) {
        float xOffset = (baseX + (i - pbinoc->uCompassBarOffset) * 0.1f * 94.0f) * uiScaleX;

        model = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, y, 0.0f));
        model = glm::scale(model, glm::vec3(tickWidth, tickHeight, 1.0f));

        glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }

    // --- COMPASS TEXT LAYOUT (match original) ---
    const float spacingV = g_dxPointsMax + 4.0f;

    // Use the orientation that was actually applied to the camera.  During the
    // smooth binoc transition cplook.radPan is the controller target and can
    // lag behind (or remain unchanged while another look mode drives pcm).
    float cameraPan = 0.0f;
    float cameraTilt = 0.0f;
    DecomposeRotateMatrixPanTilt(&g_pcm->mat, &cameraPan, &cameraTilt);

    float yaw = GModPositive(cameraPan + 0.3926991f, 6.283185f);
    int   idx = (int)(yaw / 0.7853982f) & 7;
    float interp = GModPositive(yaw, 0.7853982f) / 0.7853982f;

    // IMPORTANT: scale the FONT for this draw so DxFromPchz / DxDrawCh return PIXELS
    CFontBrx* font = pbinoc->pfontCompass;
    const float oldRx = font->m_rxScale;
    const float oldRy = font->m_ryScale;
    font->m_rxScale = oldRx * uiScaleX;
    font->m_ryScale = oldRy * uiScaleY;

    // Now these widths are in SCREEN PIXELS
    const float dxCenterPx = font->DxFromPchz((char*)g_aachzPoints[idx]);
    const float dxRightPx = font->DxFromPchz((char*)g_aachzPoints[(idx + 1) & 7]);
    const float dxLeftPx = font->DxFromPchz((char*)g_aachzPoints[(idx + 7) & 7]);

    const float spacingPx = spacingV * uiScaleX;
    const float slidePx = interp * spacingPx;
    const float centerPx = g_gl.width * 0.5f;

    // Pixel X positions (same algebra as original, just in pixels)
    const float xLeft = (centerPx - spacingPx) - (slidePx - (spacingPx - dxLeftPx) * 0.5f);
    const float xRight = (centerPx + spacingPx) - (slidePx - (spacingPx - dxRightPx) * 0.5f);
    const float xCenter = centerPx - (slidePx - (spacingPx - dxCenterPx) * 0.5f);

    CTextBox tbx;
    tbx.SetPos(0, 0);
    tbx.SetSize((float)g_gl.width, (float)(g_gl.height));

    glm::vec4 clqBright;
    tbx.SetTextColor(&clqBright);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    tbx.m_dx = g_dxPointsMax * uiScaleX;                          // textbox width in pixels
    tbx.m_dy = (float)font->m_dyUnscaled * font->m_ryScale;        // already includes uiScaleY
    tbx.m_y = baseY * uiScaleY;                                  // pixel Y
    tbx.m_rgba = RGBA_LightBlue;

    tbx.m_x = xLeft;
    font->DrawPchz((char*)g_aachzPoints[(idx + 7) & 7], &tbx);

    tbx.m_x = xRight;
    font->DrawPchz((char*)g_aachzPoints[(idx + 1) & 7], &tbx);

    // Center text fade effect (same curve)
    float fade = interp * (interp * -4.0f + 4.0f);
    glm::vec4 fadeColor = RGBA_LightBlue;
    fadeColor.r = FadeChannel(fadeColor.r, 0.19607843f, fade);
    fadeColor.g = FadeChannel(fadeColor.g, 0.51764706f, fade);
    fadeColor.b = FadeChannel(fadeColor.b, 0.69019608f, fade);
    fadeColor.a = FadeChannel(fadeColor.a, 0.50196078f, fade);

    tbx.m_rgba = fadeColor;
    tbx.m_x = xCenter;
    font->DrawPchz((char*)g_aachzPoints[idx], &tbx);

    // restore font scales
    font->m_rxScale = oldRx;
    font->m_ryScale = oldRy;

    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDepthFunc(GL_LESS);
}

void DrawBinocZoom(BINOC* pbinoc)
{
    const glm::mat4 model(1.0f);

    glBlotShader.Use();
    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
    glUniform1i(u_useVertexColorLoc, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    const float zoom = g_pcm->cplook.uZoom;

    // Virtual UI (original authored around this)
    const float Vw = 640.0f;
    const float Vh = 492.8f;

    const float sx = g_gl.width / Vw;
    const float sy = g_gl.height / Vh;

    const float xCenter = 320.0f;

    for (int i = 0; i < 8; ++i)
    {
        float t0 = i * (1.0f / 9.0f);
        float t1 = t0 + 0.06666667f;
        float mid = 0.5f * (t0 + t1);

        glm::vec4 color = (zoom < mid) ? RGBA_DarkBlue : RGBA_LightBlue;

        float halfTop = (1.0f - t0) * 55.0f - 9.0f;
        float halfBottom = (1.0f - t1) * 55.0f + 1.0f - 9.0f;
        float yTop = 372.8f - t0 * 60.0f;
        float yBottom = 372.8f - t1 * 60.0f;

        const float xTopLeft = (xCenter - halfTop) * sx;
        const float xTopRight = (xCenter + halfTop) * sx;
        const float xBottomLeft = (xCenter - halfBottom) * sx;
        const float xBottomRight = (xCenter + halfBottom) * sx;
        const float yTopScreen = yTop * sy;
        const float yBottomScreen = yBottom * sy;

        // Original is a four-vertex strip. Expand it to two triangles while
        // retaining the independently authored top and bottom widths.
        const float vertices[] =
        {
            xTopLeft,     yTopScreen,
            xTopRight,    yTopScreen,
            xBottomRight, yBottomScreen,

            xTopLeft,     yTopScreen,
            xBottomRight, yBottomScreen,
            xBottomLeft,  yBottomScreen
        };

        glBindVertexArray(pbinoc->binocIndicatorVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pbinoc->binocIndicatorVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    if (pbinoc->binocs == BINOCS_Peek)
    {
        JOY* pjoy = (g_grfjoyt & 2U) != 0 ? &g_joy : &g_joyZero;
        CFontBrx* joyFont = g_pfontJoy != nullptr ? g_pfontJoy : PfontFromFont(1);

        if (joyFont != nullptr)
        {
            CTextBox tbx;
            tbx.SetPos(0.0f, 0.0f);
            tbx.SetSize(static_cast<float>(g_gl.width), static_cast<float>(g_gl.height));

            glm::vec4 color(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);
            tbx.SetTextColor(&color);
            tbx.SetHorizontalJust(JH_Center);
            tbx.SetVerticalJust(JV_Center);
            tbx.m_dx = 0.0f;
            tbx.m_dy = 0.0f;

            auto drawStick = [&](char glyph, float xVirtual, float deflection)
            {
                const float scale = ((pbinoc->fActive != 0) && deflection != 0.0f) ? 0.5f : 0.4f;
                const char text[2] = { glyph, '\0' };

                joyFont->PushScaling(scale * sy, scale * sy);
                tbx.m_x = xVirtual * sx;
                tbx.m_y = 390.80002f * sy;
                joyFont->DrawPchz(const_cast<char*>(text), &tbx);
                joyFont->PopScaling();
            };

            drawStick('(', 265.0f, pjoy->uDeflect);
            drawStick(')', 375.0f, pjoy->uDeflect2);
        }
    }
}

void DrawBinocOutline(BINOC* pbinoc)
{
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // --- PS2 color phases ---
    float rad = pbinoc->radReticle;

    float tInner = 0.5f - 0.5f * cosf(rad + 0.0f);
    float tMid = 0.5f - 0.5f * cosf(rad + glm::half_pi<float>());
    float tOuter = 0.5f - 0.5f * cosf(rad + glm::pi<float>());

    glm::vec4 rgbaInner = glm::mix(RGBA_DarkBlue, RGBA_LightBlue, tInner);
    glm::vec4 rgbaMid = glm::mix(RGBA_DarkBlue, RGBA_LightBlue, tMid);
    glm::vec4 rgbaOuter = glm::mix(RGBA_DarkBlue, RGBA_LightBlue, tOuter);

    // --- scale virtual 640x492.8 -> screen ---
    const float sx = g_gl.width / 640.0f;
    const float sy = g_gl.height / 492.8f;

    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(sx, sy, 1.0f));

    glBlotShader.Use();
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(uvRectLoc, 0, 0, 1, 1);

    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);
    glBindVertexArray(pbinoc->outlineVAO);

    constexpr int segments = 24;
    constexpr int verticesPerBand = segments * 4;
    constexpr int bandCount = 4;

    std::vector<glm::vec4> colors(verticesPerBand * bandCount);
    auto colorBand = [&](int band, const glm::vec4& edge0, const glm::vec4& edge1)
    {
        const int first = band * verticesPerBand;
        for (int segment = 0; segment < segments; ++segment)
        {
            const int vertex = first + segment * 4;
            colors[vertex + 0] = edge0;
            colors[vertex + 1] = edge0;
            colors[vertex + 2] = edge1;
            colors[vertex + 3] = edge1;
        }
    };

    colorBand(0, rgbaOuter, rgbaMid);
    colorBand(1, rgbaMid, rgbaInner);
    colorBand(2, rgbaOuter, rgbaMid);
    colorBand(3, rgbaMid, rgbaInner);

    glBindBuffer(GL_ARRAY_BUFFER, pbinoc->outlineColorVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, colors.size() * sizeof(glm::vec4), colors.data());

    glUniform1i(u_useVertexColorLoc, 1);
    glUniform4f(blotColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(pbinoc->outlineIndices.size()), GL_UNSIGNED_SHORT, nullptr);
    glUniform1i(u_useVertexColorLoc, 0);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
}

int NCmpScanDistance(const void* pv0, const void* pv1)
{
    const SCAN* pscan0 = *static_cast<SCAN* const*>(pv0);
    const SCAN* pscan1 = *static_cast<SCAN* const*>(pv1);

    glm::vec3 pos0;
    glm::vec3 pos1;

    GetPntPos(reinterpret_cast<PNT*>(const_cast<SCAN*>(pscan0)), &pos0);
    GetPntPos(reinterpret_cast<PNT*>(const_cast<SCAN*>(pscan1)), &pos1);

    const glm::vec3 cameraPos(g_pcm->pos);

    const float distanceSq0 = glm::distance2(cameraPos, pos0);
    const float distanceSq1 = glm::distance2(cameraPos, pos1);

    return distanceSq0 < distanceSq1 ? 1 : -1;
}

void DrawBinocScan(BINOC* pbinoc)
{
    constexpr int kScanMax = 32;
    constexpr float kScanDistanceMax = 4000.0f;
    constexpr float kScreenWidth = 640.0f;
    constexpr float kScreenHeight = 492.80002f;

    SCAN* scans[kScanMax];

    int scanCount = CploFindSwObjectsByClass(g_psw, 5, CID_SCAN, nullptr, kScanMax, reinterpret_cast<LO**>(scans));

    scanCount = std::clamp(scanCount, 0, kScanMax);

    const glm::vec3 cameraPosition(g_pcm->pos.x, g_pcm->pos.y, g_pcm->pos.z);

    // Original comparator at 0x00134AA0 sorts farthest to nearest.
    std::sort(scans, scans + scanCount, [&](SCAN* scan0, SCAN* scan1)
    {
        glm::vec3 pos0;
        glm::vec3 pos1;

        GetPntPos(reinterpret_cast<PNT*>(scan0), &pos0);
        GetPntPos(reinterpret_cast<PNT*>(scan1), &pos1);

        const glm::vec3 p0(pos0.x, pos0.y, pos0.z);
        const glm::vec3 p1(pos1.x, pos1.y, pos1.z);

        return glm::distance2(cameraPosition, p0) > glm::distance2(cameraPosition, p1);
    });

    int visibleCount = 0;
    SCAN* selectedScan = nullptr;
    float selectedDistance = s_scanSelectionDistanceMax;

    // Filter scans by distance and camera frustum.
    for (int i = 0; i < scanCount; ++i)
    {
        SCAN* scan = scans[i];

        glm::vec3 posWorld;
        GetPntPos(reinterpret_cast<PNT*>(scan), &posWorld);

        const glm::vec3 worldPosition(posWorld.x, posWorld.y, posWorld.z);

        if (glm::distance2(cameraPosition, worldPosition) > kScanDistanceMax * kScanDistanceMax)
            continue;

        // The original frustum test uses a radius of zero.
        if (!SphereInFrustum(g_pcm->frustum, worldPosition, 0.0f))
        {
            InitializeScanDisplay(scan);
            continue;
        }

        // Compact the visible scans into the beginning of the array.
        scans[visibleCount++] = scan;

        glm::vec3 posScreen;
        ConvertCmWorldToScreen(g_pcm, &posWorld, &posScreen);

        const glm::vec2 screenPosition(posScreen.x, posScreen.y);

        const float distanceFromSelection =
            glm::distance(screenPosition, s_scanSelectionCenter);

        if (distanceFromSelection < selectedDistance)
        {
            selectedDistance = distanceFromSelection;
            selectedScan = scan;
        }
    }

    if (visibleCount == 0)
        return;

    for (int i = 0; i < visibleCount; ++i)
    {
        SCAN* scan = scans[i];

        glm::vec3 posWorld;
        glm::vec3 posScreen;

        GetPntPos(reinterpret_cast<PNT*>(scan), &posWorld);
        ConvertCmWorldToScreen(g_pcm, &posWorld, &posScreen);

        char scanText[256];

        int writeIndex = 0;

        if (scan->pchzScan.c_str() != nullptr)
        {
            while (scan->pchzScan[writeIndex] != '\0' &&
                writeIndex < static_cast<int>(sizeof(scanText)) - 1)
            {
                const char ch = scan->pchzScan[writeIndex];
                scanText[writeIndex] = ch == '_' ? ' ' : ch;
                ++writeIndex;
            }
        }

        scanText[writeIndex] = '\0';

        /*
         * posScreen is in the original normalized camera-screen space.
         *
         * X: [-1, 1] -> [0, 640]
         * Y: [-1, 1] -> [492.8, 0]
         */
        const glm::vec2 anchorPosition(posScreen.x* (kScreenWidth * 0.5f) + (kScreenWidth * 0.5f), posScreen.y * -(kScreenHeight * 0.5f) + (kScreenHeight * 0.5f));
        // DrawBinocScan at 0x00134e1c uses vmul.xy followed by vaddy.x,
        // so scan visibility is based only on normalized screen X/Y.  Including
        // camera-space Z makes the fade distance enormous and hides the label.
        const float screenRadius = glm::length(glm::vec2(posScreen.x, posScreen.y));

        float alpha = s_scanAlphaBase + screenRadius * (s_scanAlphaLinear + screenRadius * s_scanAlphaQuadratic);
        alpha = glm::clamp(alpha, 0.0f, 1.0f);

        if (scan != selectedScan)
            alpha *= s_scanInactiveAlpha;

        scan->uScan = GSmooth(scan->uScan, alpha, g_clock.dtReal, &s_smpScanAlpha, nullptr);

        const float targetScale = scan == selectedScan ? s_scanSelectedScale : s_scanInactiveScale;
        scan->uScanTarget = GSmooth(scan->uScanTarget, targetScale, g_clock.dtReal, &s_smpScanScale, nullptr);

        const float scale = scan->uScanTarget;

        pbinoc->pfont->PushScaling(scale, scale);

        const float textWidth = s_scanTextWidth * scale;
        const float textHeight = pbinoc->pfont->DyWrapPchz(scanText, textWidth);

        // Everything above is measured in the original 640 x 492.8 space.
        pbinoc->pfont->PopScaling();

        const glm::vec2 textPosition(anchorPosition.x + s_scanTextOffsetX * scale, anchorPosition.y + s_scanTextOffsetY * scale - textHeight);
        const auto UiPosition = [](const glm::vec2& position)
        {
            return g_gl.uiOrigin + position * g_gl.uiScale;
        };

        const glm::vec2 drawTextPosition = UiPosition(textPosition);

        CTextBox textBox;
        textBox.SetPos(drawTextPosition.x, drawTextPosition.y);
        textBox.SetSize(textWidth * g_gl.uiScale, textHeight * g_gl.uiScale);

        glm::vec4 textColor(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, scan->uScan);

        textBox.SetTextColor(&textColor);
        textBox.SetHorizontalJust(JH_Left);
        textBox.SetVerticalJust(JV_Top);

        pbinoc->pfont->PushScaling(scale * g_gl.uiScale, scale * g_gl.uiScale);

        // The retail scan label uses its own edge descriptor initialized by
        // FUN_00136fe8.  It is deliberately much roomier than g_teBinoc;
        // reusing the ordinary Binocucom text edge makes the dossier collapse
        // into a tiny box around the glyphs.
        CFontBrx* pfontScanEdge = PfontFromFont(2);

        if (pfontScanEdge != nullptr)
        {
            CTextEdge teScan{};
            teScan.m_pfont = pfontScanEdge;
            teScan.m_ch = '-';
            teScan.m_dxExtra = 32.0f;
            teScan.m_dyExtra = 16.0f;
            teScan.m_rxScaling = 0.3f * g_gl.uiScale;
            teScan.m_ryScaling = 0.3f * g_gl.uiScale;
            teScan.m_rgba = glm::vec4(
                0.0f,
                75.0f / 255.0f,
                125.0f / 255.0f,
                scan->uScan);

            pfontScanEdge->EdgeRect(&teScan, &textBox);
        }

        pbinoc->pfont->DrawPchz(scanText, &textBox);

        /*
         * Draw the line from the scan label to its screen-space anchor.
         * This uses the same position-only dynamic VBO as the filter arrows.
         */
        const glm::vec2 lineStart = UiPosition(glm::vec2(
            textPosition.x + s_scanLineOffsetX * scale,
            textPosition.y + textHeight + s_scanLineOffsetY * scale));
        const glm::vec2 lineEnd = UiPosition(anchorPosition);

        const float lineVertices[] =
        {
            lineStart.x, lineStart.y,
            lineEnd.x,   lineEnd.y
        };

        glm::vec4 lineColor = pbinoc->pte != nullptr
            ? pbinoc->pte->m_rgba
            : glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

        lineColor.a *= scan->uScan;

        glBlotShader.Use();

        const glm::mat4 model(1.0f);

        glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
        glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);

        glUniform1i(u_useVertexColorLoc, 0);
        glUniform4fv(blotColorLoc, 1, glm::value_ptr(lineColor));

        // Bindless resident 1x1 white texture.
        glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);

        glBindVertexArray(pbinoc->binocIndicatorVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pbinoc->binocIndicatorVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineVertices), lineVertices);
        glDrawArrays(GL_LINES, 0, 2);
        glBindVertexArray(0);

        pbinoc->pfont->PopScaling();
    }
}

void DrawBinocFilter(BINOC* pbinoc)
{
    if (pbinoc->binocs != BINOCS_Peek)
        return;

    const GRFVAULT grfvaultAvailable = GetAvailableVaultFlags();

    if ((grfvaultAvailable & 0x800) != 0)
        DrawBinocScan(pbinoc);

    GRFVAULT grfvaultBlueprint = 0;
    GetBlueprintInfo(&grfvaultBlueprint, 0);

    if ((grfvaultAvailable & grfvaultBlueprint) == 0)
        return;

    const glm::vec4 colorActive(128.0f / 255.0f, 128.0f / 255.0f, 64.0f / 255.0f, 1.0f);
    const glm::vec4 colorActiveDim(96.0f / 255.0f, 96.0f / 255.0f, 64.0f / 255.0f, 1.0f);
    const glm::vec4 colorInactive(64.0f / 255.0f, 64.0f / 255.0f, 64.0f / 255.0f, 1.0f);

    const glm::vec2 center(320.0f, 415.80002f);
    const float pulse = std::cos(g_clock.t * s_loFinderPulse) * 0.5f + 0.5f;

    // Finder geometry is authored in the same 640x492.8 virtual space as
    // the Binocucom zoom ladder and analog-stick prompts.
    const float sx = static_cast<float>(g_gl.width) / 640.0f;
    const float sy = static_cast<float>(g_gl.height) / 492.8f;
    
    glBlotShader.Use();

    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(sx, sy, 1.0f));

    // Draw this as a 2D overlay regardless of the state left by the outline.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
    glUniform1i(u_useVertexColorLoc, 0);

    glUniformHandleui64ARB(u_fontTexLoc, whiteHandle);

    glBindVertexArray(pbinoc->binocIndicatorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pbinoc->binocIndicatorVBO);

    for (int i = 0; i < 4; ++i)
    {
        const float target = pbinoc->mpinormalf[i] != 0 ? 1.0f : 0.0f;

        pbinoc->mpinormalu[i] = GSmooth(pbinoc->mpinormalu[i], target, g_clock.dt, &s_smpLoFinderAlpha, nullptr);

        const float active = pbinoc->mpinormalu[i];

        glm::vec4 color =
            colorInactive * (1.0f - active) +
            colorActiveDim * (active * pulse) +
            colorActive * (active * (1.0f - pulse));

        color.a *= 0.5f;

        const glm::vec2 v0 = center + s_aBinocDirectionVertices[i][0];
        const glm::vec2 v1 = center + s_aBinocDirectionVertices[i][1];
        const glm::vec2 v2 = center + s_aBinocDirectionVertices[i][2];
        const glm::vec2 v3 = center + s_aBinocDirectionVertices[i][3];

        // The original primitive is a triangle fan:
        // v0-v1-v2 and v0-v2-v3.
        const float vertices[] =
        {
            v0.x, v0.y,
            v1.x, v1.y,
            v2.x, v2.y,

            v0.x, v0.y,
            v2.x, v2.y,
            v3.x, v3.y
        };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
}

void DrawBinoc(BINOC* pbinoc)
{
    if (!FCanDrawBinoc())
        return;

    const BINOCS state = pbinoc->binocs;

    if (state > BINOCS_None && state < BINOCS_Instruct)
    {
        DrawBinocBackground(pbinoc);
        DrawBinocReticle(pbinoc);
        DrawBinocCompass(pbinoc);
        DrawBinocZoom(pbinoc);
        DrawBinocOutline(pbinoc);
        DrawBinocFilter(pbinoc);
    }
    else if (state == BINOCS_Sniper)
    {
        DrawBinocReticle(pbinoc);
    }

    if (state == BINOCS_Instruct && pbinoc->achzDraw[0] == '\0')
        return;

    constexpr float kUiWidth = 640.0f;
    constexpr float kUiHeight = 492.80002f;

    const float targetWidth = static_cast<float>(g_gl.width);
    const float targetHeight = static_cast<float>(g_gl.height);
    // Keep the BINOC text in the same 640 x 492.8 virtual space as the
    // resized TV panels.  Height-only scaling lets the text grow underneath
    // the TVs on narrow window aspect ratios.
    const float uiScale = glm::min(targetWidth / kUiWidth, targetHeight / kUiHeight);
    const float uiOriginX = (targetWidth - kUiWidth * uiScale) * 0.5f;
    const float uiOriginY = targetHeight - kUiHeight * uiScale;

    const auto UiX = [uiOriginX, uiScale](float x) { return uiOriginX + x * uiScale; };
    const auto UiY = [uiOriginY, uiScale](float y) { return uiOriginY + y * uiScale; };
    const auto UiSize = [uiScale](float value) { return value * uiScale; };

    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float boxWidthVirtual = 280.0f;
    int visibleLineCount = 3;

    if (state == BINOCS_Instruct)
    {
        offsetY = -8.0f;
    }
    else if (state == BINOCS_Confront)
    {
        offsetX = -50.0f;
        offsetY = 40.0f;
        boxWidthVirtual = 380.0f;
        visibleLineCount = 2;
    }

    const float baseFontScale = state == BINOCS_Confront ? 0.8f : 1.0f;
    const float fontScale = baseFontScale * uiScale;

    pbinoc->pfont->PushScaling(fontScale, fontScale);

    glm::vec4 colorText = pbinoc->rgbaText;
    glm::vec4 colorEdge = pbinoc->pte != nullptr ? pbinoc->pte->m_rgba : glm::vec4(0.0f);

    const float textHeight = static_cast<float>(pbinoc->pfont->m_dyUnscaled) * pbinoc->pfont->m_ryScale;
    const float boxWidth = UiSize(boxWidthVirtual);
    const float boxHeight = textHeight * static_cast<float>(visibleLineCount);
    const float clipX = UiX(180.0f + offsetX);
    const float clipY = UiY(386.80002f + offsetY);

    CTextBox tbxClip;
    tbxClip.SetPos(clipX, clipY);
    tbxClip.SetSize(boxWidth, boxHeight);

    glm::vec4 colorClip(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);

    tbxClip.SetTextColor(&colorClip);
    tbxClip.SetHorizontalJust(JH_Left);
    tbxClip.SetVerticalJust(JV_Top);

    float uText = 1.0f;

    if (g_tvLeft.pspeaker != nullptr)
        uText = std::min(uText, g_tvLeft.uOn);

    if (g_tvRight.pspeaker != nullptr)
        uText = std::min(uText, g_tvRight.uOn);

    if (uText < 1.0f)
    {
        colorText.a *= uText;
        colorEdge.a *= uText;
    }

    if (state == BINOCS_Instruct && pbinoc->pte != nullptr && pbinoc->pte->m_pfont != nullptr)
    {
        const glm::vec4 colorEdgePrevious = pbinoc->pte->m_rgba;

        pbinoc->pte->m_rgba = colorEdge;
        pbinoc->pte->m_pfont->EdgeRect(pbinoc->pte, &tbxClip);
        pbinoc->pte->m_rgba = colorEdgePrevious;
    }

    if (pbinoc->achzDraw[0] != '\0')
    {
        CRichText rt(pbinoc->achzDraw, pbinoc->pfont);

        const int totalChars = rt.Cch();
        int visibleChars = static_cast<int>((g_clock.t - pbinoc->tAchzSet) * pbinoc->svch);
        visibleChars = std::clamp(visibleChars, 0, totalChars);

        int scrollSegment = 0;

        if (pbinoc->cichLR > 0 && pbinoc->aichLR[0] <= visibleChars)
        {
            scrollSegment = 1;

            while (scrollSegment < pbinoc->cichLR && pbinoc->aichLR[scrollSegment] <= visibleChars)
                ++scrollSegment;
        }

        float scrollInterpolation = 0.0f;

        if (scrollSegment > 0 && scrollSegment < pbinoc->cichLR && pbinoc->svch > 0.0f)
        {
            const float t0 = static_cast<float>(pbinoc->aichLR[scrollSegment - 1]) / pbinoc->svch;
            const float t1 = static_cast<float>(pbinoc->aichLR[scrollSegment]) / pbinoc->svch;
            const float duration = t1 - t0;

            if (duration > 0.0f)
                scrollInterpolation = ((g_clock.t - pbinoc->tAchzSet) - t0) / duration;
        }

        char achzPartial[512];
        std::strncpy(achzPartial, pbinoc->achzDraw, sizeof(achzPartial) - 1);
        achzPartial[sizeof(achzPartial) - 1] = '\0';

        CRichText rtPartial(achzPartial, pbinoc->pfont);
        rtPartial.Trim(visibleChars);

        const bool drawCursor = visibleChars < totalChars || ((static_cast<int>(g_clock.t * 4.0f) & 1) != 0);

        if (drawCursor)
        {
            constexpr char cursorText[] = "&.~ffffff|";
            constexpr std::size_t cursorLength = sizeof(cursorText) - 1;
            const std::size_t textLength = std::strlen(achzPartial);

            if (textLength + cursorLength < sizeof(achzPartial))
                std::strcat(achzPartial, cursorText);
        }

        const float visibleHeight = (static_cast<float>(scrollSegment + 1) + scrollInterpolation) * textHeight;
        const float heightOverflow = visibleHeight - boxHeight;
        const float scrollOffset = heightOverflow >= 0.0f ? static_cast<float>(static_cast<int>(heightOverflow)) : 0.0f;

        CTextBox tbxText;
        tbxText.SetPos(clipX, clipY - scrollOffset);
        tbxText.SetSize(boxWidth, visibleHeight);
        tbxText.SetTextColor(&colorText);
        tbxText.SetHorizontalJust(JH_Left);
        tbxText.SetVerticalJust(JV_Top);

        rtPartial.Draw(&tbxText, &tbxClip);
    }

    pbinoc->pfont->PopScaling();

    if (pbinoc->chPause != '\0')
    {
        const float phase = RadNormalize(g_clock.t * 10.0f);
        const float pulse = std::cos(phase) * 0.5f + 0.5f;
        const float pauseScale = glm::mix(0.3f, 0.4f, pulse) * uiScale;

        float pauseXVirtual;
        float pauseYVirtual;

        if (state == BINOCS_Confront)
        {
            pauseXVirtual = 532.0f;
            pauseYVirtual = 472.80002f;
        }
        else
        {
            pauseXVirtual = offsetX + 482.0f;
            pauseYVirtual = offsetY + 472.80002f;
        }

        const float pauseX = UiX(pauseXVirtual);
        const float pauseY = UiY(pauseYVirtual);

        char achzPause[2] = { pbinoc->chPause, '\0' };
        CFontBrx* pfontJoy = PfontFromFont(1);

        if (pfontJoy != nullptr)
        {
            pfontJoy->PushScaling(pauseScale, pauseScale);

            CTextBox tbxPause;
            tbxPause.SetPos(pauseX, pauseY);
            tbxPause.SetSize(0.0f, 0.0f);

            glm::vec4 colorPause(128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f);

            tbxPause.SetTextColor(&colorPause);
            tbxPause.SetHorizontalJust(JH_Left);
            tbxPause.SetVerticalJust(JV_Top);

            pfontJoy->DrawPchz(achzPause, &tbxPause);
            pfontJoy->PopScaling();
        }
    }
}
void FreeBinocGL(BINOC* pbinoc)
{
    if (!pbinoc)
        return;

    if (pbinoc->triangleBinocVBO != 0)
    {
        glDeleteBuffers(1, &pbinoc->triangleBinocVBO);
        pbinoc->triangleBinocVBO = 0;
    }

    if (pbinoc->triangleBinocVAO != 0)
    {
        glDeleteVertexArrays(1, &pbinoc->triangleBinocVAO);
        pbinoc->triangleBinocVAO = 0;
    }

    if (pbinoc->binocIndicatorVBO != 0)
    {
        glDeleteBuffers(1, &pbinoc->binocIndicatorVBO);
        pbinoc->binocIndicatorVBO = 0;
    }

    if (pbinoc->binocIndicatorVAO != 0)
    {
        glDeleteVertexArrays(1, &pbinoc->binocIndicatorVAO);
        pbinoc->binocIndicatorVAO = 0;
    }

    if (pbinoc->reticleVBO != 0)
    {
        glDeleteBuffers(1, &pbinoc->reticleVBO);
        pbinoc->reticleVBO = 0;
    }

    if (pbinoc->reticleVAO != 0)
    {
        glDeleteVertexArrays(1, &pbinoc->reticleVAO);
        pbinoc->reticleVAO = 0;
    }
}

SCAN* NewScan()
{
    return new SCAN{};
}

int GetScanSize()
{
    return sizeof(SCAN);
}

void CloneScan(SCAN* pscan, SCAN* pscanBase)
{
    ClonePnt(pscan, pscanBase);
    pscan->pchzScan = pscanBase->pchzScan;
}

void LoadScanFromBrx(SCAN* pscan, CBinaryInputStream* pbis)
{
    LoadPntFromBrx(pscan, pbis);
    pscan->pchzScan = pbis->ReadStringSw();
}

void InitializeScanDisplay(SCAN* pscan)
{
    pscan->uScanTarget = 0.5;
    pscan->uScan = 0.5;
}

void DeleteScan(SCAN* pscan)
{
    delete pscan;
}

BINOC g_binoc;
CTextEdge g_teBinoc;
BEI s_beiUpper;
BEI s_beiLower;
BEI s_beiReticle;
CLQ s_clqUpper = { 80.0, -288.0, 288.0, 0.0 };
CLQ s_clqLower = { 280.80002, 376.0, -376.0, 0.0 };
CLQ s_clqReticle = { -8.0, -40.0, 40.0, 0.0 };
float g_dxPointsMax = 1.0;
const char* g_aachzPoints[8] =
{
    "N",
    "NW",
    "W",
    "SW",
    "S",
    "SE",
    "E",
    "NE"
};
glm::vec4 RGBA_DarkBlue = glm::vec4(0.098f, 0.118f, 0.431f, 0.502f);
glm::vec4 RGBA_DarkRed = glm::vec4(0.314f, 0.157f, 0.157f, 0.502f);
glm::vec4 RGBA_LightRed = glm::vec4(0.494f, 0.157f, 0.039f, 0.502f);
glm::vec4 RGBA_Green = glm::vec4(0.184f, 0.447f, 0.243f, 0.502f);
glm::vec4 RGBA_LightBlue = glm::vec4(0.000f, 0.322f, 0.494f, 0.502f);
glm::vec4 RGBA_Overlay = glm::vec4(0.000f, 0.000f, 0.000f, 0.75f);
float DT_BinocRoseInterval = 0.25;
int s_mpbfkgrfvault[2] = { 0xF0000000, 0xF0000000};
