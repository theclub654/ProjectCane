#include "binoc.h"

void StartupBinoc(BINOC *pbinoc)
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

    // Setup edge font if available
    if (g_pfontScreenCounters)
    {
        pbinoc->pte = &g_teBinoc;
        g_teBinoc.m_pfont = g_pfontScreenCounters;
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
    *px = pbinoc->dxReticle + (g_gl.width  * 0.5);
    *py = pbinoc->dyReticle + (g_gl.height / 2) - 90;
}

void SetBinocLookat(BINOC* pbinoc, ALO* paloLookat)
{
    pbinoc->paloLookat = paloLookat;
}

void OnBinocActive(BINOC* pbinoc, int fActive)
{
    // Only process if the desired active state differs from the current one
    if (fActive != static_cast<bool>(pbinoc->fActive))
    {
        // Case 1: No dialog is playing
        if (pbinoc->pdialogPlaying == nullptr)
        {
            pbinoc->fActive = fActive;
        }
        // Case 2: A dialog is currently playing
        else if (pbinoc->pdialogPlaying->dialogs == DIALOGS_Playing)
        {
            if (!fActive)
            {
                //PauseVag();             // Pause dialog audio
                pbinoc->fActive = false;
            }
            else
            {
                //ContinueVag();          // Resume dialog audio
                pbinoc->fActive = true;
            }
        }
        // Case 3: Dialog is not in playing state, just set flag
        else
        {
            pbinoc->fActive = fActive;
        }

        // Always reset targeting when toggling
        pbinoc->fTargeting = 0;

        if (!fActive)
        {
            // Deactivating: save previous bfk state and clear it
            pbinoc->bfkPrev = pbinoc->bfk;
            //SetBinocBfk(pbinoc, BFK_None, true);
        }
        else
        {
            // Activating: only restore previous bfk if vault flags match
            /*GRFVAULT blueprintVaultFlags = {};
            GetBlueprintInfo(&blueprintVaultFlags, nullptr);

            if ((g_pgsCur->grfvault & blueprintVaultFlags) &&
                (g_pgsCur->grfvault & s_mpbfkgrfvault[pbinoc->bfkPrev]))
            {
                SetBinocBfk(pbinoc, pbinoc->bfkPrev, true);
            }*/
        }
    }
}

void UpdateBinocActive(BINOC* pbinoc, JOY* pjoy)
{
    /*if (pjoy->grfbtnPressed & 0x400U)
        SetJoyBtnHandled(&g_joy, 0x400);

    if (pbinoc->binocs == BINOCS_Peek || pbinoc->binocs == BINOCS_Sniper) {
        float inputX = std::clamp(pjoy->x + pjoy->x2, -1.0f, 1.0f);
        float inputY = std::clamp(pjoy->y, -1.0f, 1.0f);
        if (!g_fReverseLook) inputY = -inputY;

        SMP slow = { 50.0f, 0.5f };
        SMP fast = { 200.0f, 0.1f };
        SMP *smoothing = (pjoy->uDeflect + pjoy->uDeflect2 > 0.0001f) ? &slow : &fast;

        VU_VECTOR cur = { pbinoc->dxReticle, pbinoc->dyReticle, 0.0f };
        VU_VECTOR target = { inputX * 50.0f, inputY * 50.0f, 0.0f };

        VU_VECTOR* result = PosSmooth(&pbinoc->dxReticle, cur, target, g_clock.dt, inputX * 50.0f, smoothing);

        pbinoc->dxReticle = result->x;
        pbinoc->dyReticle = result->y;

        pbinoc->uCompassBarOffset = GModPositive(pbinoc->uCompassBarOffset + inputX * -0.08f, 1.0f);
    }

    UpdateBinocActiveFilter(pbinoc, pjoy);*/
}

void OnBinocReset(BINOC* pbinoc)
{
    OnBlotReset(pbinoc);
    if (pbinoc->pdialogPlaying != nullptr) {
        //SetDialogDialogs(pbinoc->pdialogPlaying, DIALOGS_Disabled);
    }
    g_pdialogCalling = nullptr;
    g_pdialogPlaying = nullptr;
    g_pdialogTriggered = nullptr;
}

void OnBinocPush(BINOC* pbinoc)
{
    // 1. Set Binoc UI to appear (state = Appearing)
    pbinoc->pvtblot->pfnSetBlotBlots(pbinoc, BLOTS_Visible);
    // 2. If there's no dialog currently playing
    if (!pbinoc->pdialogPlaying) {
        // Default binocular view
        //SetBinocBinocs(pbinoc, BINOCS_Peek);
        return;
    }

    // 3. Get the type of dialog currently playing
    DIALOGK type = pbinoc->pdialogPlaying->dialogk;

    // 4. Choose binocular mode based on dialog type
    switch (type) {
    case DIALOGK_Binoc:
        //SetBinocBinocs(pbinoc, BINOCS_Dialog);
        break;
    case DIALOGK_Instruct:
        //SetBinocBinocs(pbinoc, BINOCS_Instruct);
        break;
    case DIALOGK_Confront:
        //SetBinocBinocs(pbinoc, BINOCS_Confront);
        break;
    default:
        // Unknown or other dialog type: just continue playing
        break;
    }

    // 5. Resume dialog playback if it's paused
    //SetDialogDialogs(pbinoc->pdialogPlaying, DIALOGS_Playing);
}

void OnBinocPop(BINOC* pbinoc)
{
    pbinoc->pvtblot->pfnSetBlotBlots(pbinoc, BLOTS_Hidden);
    if (pbinoc->pdialogPlaying != nullptr) {
        //SetDialogDialogs(pbinoc->pdialogPlaying, DIALOGS_Enabled);
        pbinoc->pdialogPlaying = nullptr;
    }
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

float DtAppearBinoc(BINOC* pbinoc)
{
    float dt;

    switch (pbinoc->binocs) {
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

    switch (pbinoc->binocs) {
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

void SetBinocBlots(BINOC* pbinoc, BLOTS blots)
{
    SetBlotBlots(pbinoc, blots);
}

void BuildBinocBackGround(BINOC* pbinoc)
{
    if (pbinoc->backGroundBinocVAO != 0)
    {
        glDeleteVertexArrays(1, &pbinoc->backGroundBinocVAO);
        glDeleteBuffers(1, &pbinoc->backGroundBinocVBO);
        glDeleteBuffers(1, &pbinoc->backGroundBinocEBO);
    }

    int kSteps = 24;

    float screenCenter = g_gl.width * 0.5f;
    float bandWidth = g_gl.width;  // or a fixed width if desired
    float halfWidth = bandWidth * 0.5f;

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    std::vector <Vertex> vertices;
    std::vector <uint16_t>& indices = pbinoc->backGroundBinocIndices;

    indices.clear(); // Make sure it's empty

    auto pushQuad = [&](uint16_t base) {
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        };

    // === Top Band ===
    for (int i = 0; i < kSteps; ++i)
    {
        float t0 = float(i + 0) / kSteps;
        float t1 = float(i + 1) / kSteps;

        float x0 = (float(i + 0) / kSteps) * bandWidth;
        float x1 = (float(i + 1) / kSteps) * bandWidth;

        float y0Curve = GEvaluateBei(s_beiUpper, i);
        float y1Curve = GEvaluateBei(s_beiUpper, i + 1);
        float y0 = 0.0f; // straight edge (top bar base)

        uint16_t base = static_cast<uint16_t>(vertices.size());

        vertices.push_back({ {x0, y0},      {0.0f, 0.0f} });
        vertices.push_back({ {x1, y0},      {1.0f, 0.0f} });
        vertices.push_back({ {x1, y1Curve}, {1.0f, 1.0f} });
        vertices.push_back({ {x0, y0Curve}, {0.0f, 1.0f} });

        pushQuad(base);
    }

    // === Bottom Band ===
    for (int i = 0; i < kSteps; ++i)
    {
        float t0 = float(i + 0) / kSteps;
        float t1 = float(i + 1) / kSteps;

        float x0 = (float(i + 0) / kSteps) * bandWidth;
        float x1 = (float(i + 1) / kSteps) * bandWidth;

        float y0Curve = GEvaluateBei(s_beiLower, i + 0);
        float y1Curve = GEvaluateBei(s_beiLower, i + 1);

        float yFlat = g_gl.height; // bottom of screen

        uint16_t base = static_cast<uint16_t>(vertices.size());

        vertices.push_back({ {x0, y0Curve}, {0.0f, 1.0f} }); // curve
        vertices.push_back({ {x1, y1Curve}, {1.0f, 1.0f} }); // curve
        vertices.push_back({ {x1, yFlat},   {1.0f, 0.0f} }); // flat
        vertices.push_back({ {x0, yFlat},   {0.0f, 0.0f} }); // flat

        pushQuad(base);
    }

    // === Upload to GPU ===
    glGenVertexArrays(1, &pbinoc->backGroundBinocVAO);
    glGenBuffers(1, &pbinoc->backGroundBinocVBO);
    glGenBuffers(1, &pbinoc->backGroundBinocEBO);

    glBindVertexArray(pbinoc->backGroundBinocVAO);

    glBindBuffer(GL_ARRAY_BUFFER, pbinoc->backGroundBinocVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pbinoc->backGroundBinocEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // a_position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));

    glEnableVertexAttribArray(1); // a_texcoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

    glBindVertexArray(0);
}

void BuildBinocOutline(BINOC* pbinoc)
{
    if (pbinoc->outlineVAO != 0)
    {
        glDeleteVertexArrays(1, &pbinoc->outlineVAO);
        glDeleteBuffers(1, &pbinoc->outlineVBO);
        glDeleteBuffers(1, &pbinoc->outlineEBO);
    }

    int kSteps = 24;
    float bandWidth = g_gl.width;

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    std::vector<Vertex> vertices;
    std::vector<uint16_t>& indices = pbinoc->outlineIndices;

    indices.clear();

    auto pushQuad = [&](uint16_t base) {
        indices.push_back(base + 0);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 0);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
        };

    auto emitBand = [&](const BEI& bei, float direction, float offset0, float offset1) {
        for (int i = 0; i < kSteps; ++i)
        {
            float t0 = float(i + 0) / kSteps;
            float t1 = float(i + 1) / kSteps;

            float x0 = t0 * bandWidth;
            float x1 = t1 * bandWidth;

            float y0Curve = GEvaluateBei(bei, i + 0);
            float y1Curve = GEvaluateBei(bei, i + 1);

            float y0a = (y0Curve + offset0 * direction);
            float y1a = (y1Curve + offset0 * direction);

            float y0b = (y0Curve + offset1 * direction);
            float y1b = (y1Curve + offset1 * direction);

            uint16_t base = static_cast<uint16_t>(vertices.size());

            vertices.push_back({ {x0, y0a}, {0.0f, 0.0f} });
            vertices.push_back({ {x1, y1a}, {1.0f, 0.0f} });
            vertices.push_back({ {x1, y1b}, {1.0f, 1.0f} });
            vertices.push_back({ {x0, y0b}, {0.0f, 1.0f} });

            pushQuad(base);
        }
        };

    // Top bands (direction = +1)
    emitBand(s_beiUpper, +1.0f, 0.0f, 4.0f); // outer
    emitBand(s_beiUpper, +1.0f, 4.0f, 0.0f); // mid
    emitBand(s_beiUpper, +1.0f, 0.0f, 8.0f); // inner

    // Bottom bands (direction = -1)
    emitBand(s_beiLower, -1.0f, 0.0f, 4.0f); // outer
    emitBand(s_beiLower, -1.0f, 4.0f, 0.0f); // mid
    emitBand(s_beiLower, -1.0f, 0.0f, 8.0f); // inner

    glGenVertexArrays(1, &pbinoc->outlineVAO);
    glGenBuffers(1, &pbinoc->outlineVBO);
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

    glBindVertexArray(0);
}

void DrawBinocReticle(BINOC* pbinoc)
{
    // Spin animation
    float spinSpeed = 3.0f;
    if (g_joy.stick > 0.0f)      spinSpeed = 6.0f;
    else if (g_joy.stick < 0.0f) spinSpeed = -6.0f;

    pbinoc->radReticle = RadNormalize(pbinoc->radReticle + g_clock.dt * spinSpeed);

    float zoom = g_pcm->cplook.uZoom + 1.0f;
    float alphaZoom = glm::clamp(zoom * 25.0f, 0.0f, 255.0f) / 255.0f;

    // Base colors
    glm::vec4 DarkBlue  = glm::vec4(RGBA_DarkBlue.r,  RGBA_DarkBlue.g,  RGBA_DarkBlue.b,  alphaZoom);
    glm::vec4 LightBlue = glm::vec4(RGBA_LightBlue.r, RGBA_LightBlue.g, RGBA_LightBlue.b, alphaZoom);
    glm::vec4 DarkRed   = glm::vec4(RGBA_DarkRed.r,   RGBA_DarkRed.g,   RGBA_DarkRed.b,   alphaZoom);
    glm::vec4 LightRed  = glm::vec4(RGBA_LightRed.r,  RGBA_LightRed.g,  RGBA_LightRed.b,  alphaZoom);

    glm::vec4 rgbaDark  = pbinoc->fTargeting ? DarkRed : DarkBlue;
    glm::vec4 rgbaLight = pbinoc->fTargeting ? LightRed : LightBlue;

    // Animated color shift
    float tOuter = 0.5f - std::cos(pbinoc->radReticle - 0.196f)  * 0.5f;
    float tInner = 0.5f - std::cos(pbinoc->radReticle - 0.4417f) * 0.5f;
    glm::vec4 rgbaOuter = glm::mix(rgbaDark, rgbaLight, tOuter);
    glm::vec4 rgbaInner = glm::mix(rgbaDark, rgbaLight, tInner);
  
    float xFocus; 
    float yFocus;

    GetBinocReticleFocus(pbinoc, &xFocus, &yFocus);
    glm::vec2 focus(xFocus, yFocus);

    // Set shader and VAO
    glBlotShader.Use();
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glBindVertexArray(g_gl.gao);

    glBindTexture(GL_TEXTURE_2D, whiteTex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);

    int numTicks = 24;
    float arcWidth = 70.0f * zoom;
    float arcHalf = arcWidth * 0.5f;
    float barHeight = 5.0f * zoom;
    float barWidth = glm::max(3.0f * zoom, 2.0f);

    for (int i = 0; i < numTicks; ++i) {
        float t = float(i + 0.5f) / numTicks; // center of bar
        float xOffset = t * arcWidth - arcHalf;
        float yOffset = GEvaluateBei(s_beiReticle, i + 0.5f) * zoom;

        for (int side = -1; side <= 1; side += 2) {
            glm::vec2 posTop = glm::vec2(xFocus + side * xOffset, yFocus - yOffset);
            glm::vec2 posBot = glm::vec2(xFocus + side * xOffset, yFocus + yOffset);

            posTop = glm::round(posTop);
            posBot = glm::round(posBot);

            // Top bar (outer)
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(posTop, 0.0f));
            model = glm::scale(model, glm::vec3(barWidth, barHeight, 1.0f));
            glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform4fv(blotColorLoc, 1, glm::value_ptr(rgbaOuter));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            // Top bar (inner)
            model = glm::translate(glm::mat4(1.0f), glm::vec3(posTop.x, posTop.y + barHeight * 0.1f, 0.0f));
            model = glm::scale(model, glm::vec3(barWidth, barHeight * 0.8f, 1.0f));
            glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform4fv(blotColorLoc, 1, glm::value_ptr(rgbaInner));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            // Bottom bar (outer)
            model = glm::translate(glm::mat4(1.0f), glm::vec3(posBot, 0.0f));
            model = glm::scale(model, glm::vec3(barWidth, barHeight, 1.0f));
            glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform4fv(blotColorLoc, 1, glm::value_ptr(rgbaOuter));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            // Bottom bar (inner)
            model = glm::translate(glm::mat4(1.0f), glm::vec3(posBot.x, posBot.y + barHeight * 0.1f, 0.0f));
            model = glm::scale(model, glm::vec3(barWidth, barHeight * 0.8f, 1.0f));
            glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform4fv(blotColorLoc, 1, glm::value_ptr(rgbaInner));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        }
    }

    if (pbinoc->binocs != BINOCS_Sniper)
    {
        float tickWidth = 2.5f * zoom;
        float tickHeight = 10.0f * zoom;

        float dxStart = 41.0f;
        float dxStep = 8.0f;
        float dxMax = (g_gl.width * 0.5f) / zoom;

        for (float dx = dxStart; dx <= dxMax; dx += dxStep)
        {
            float angle = pbinoc->radReticle + (dx / 256.0f) * glm::pi<float>();
            float t = 0.5f - cosf(angle) * 0.5f;
            glm::vec4 rgbaTick = glm::mix(rgbaDark, rgbaLight, t);

            for (int side = -1; side <= 1; side += 2)
            {
                float x = xFocus + zoom * dx * float(side);
                float y = yFocus + zoom * -5.0f;

                glm::vec2 pos = glm::round(glm::vec2(x, y));
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
                model = glm::scale(model, glm::vec3(tickWidth, tickHeight, 1.0f));
                glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glUniform4fv(blotColorLoc, 1, glm::value_ptr(rgbaTick));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            }
        }

        // === Vertical arc ===
        float dyStart = 24.0f;
        float dyStep = 8.0f;
        float dyMax = (g_gl.height * 0.4) / zoom;

        for (float dy = dyStart; dy <= dyMax; dy += dyStep)
        {
            float angle = pbinoc->radReticle + (dy / 128.0f) * glm::pi<float>();
            float t = 0.5f - cosf(angle) * 0.5f;
            glm::vec4 rgbaTick = glm::mix(rgbaDark, rgbaLight, t);

            for (int side = -1; side <= 1; side += 2)
            {
                float y = yFocus + zoom * dy * float(side);
                float x = xFocus + zoom * -5.0f;

                glm::vec2 pos = glm::round(glm::vec2(x, y));
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
                model = glm::scale(model, glm::vec3(tickHeight, tickWidth, 1.0f)); // horizontal bar
                glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glUniform4fv(blotColorLoc, 1, glm::value_ptr(rgbaTick));
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
            }
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void DrawBinocBackground(BINOC* pbinoc)
{
    glBlotShader.Use();

    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));

    float scaleY = g_gl.height / 492;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0f));
    model = glm::scale(model, glm::vec3(1.0, scaleY, 1.0f));

    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniform4f(uvRectLoc, 0, 0, 1, 1);
    glUniform4f(blotColorLoc, RGBA_Overlay.r, RGBA_Overlay.g, RGBA_Overlay.b, RGBA_Overlay.a);
    glBindTexture(GL_TEXTURE_2D, whiteTex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(pbinoc->backGroundBinocVAO);
    glDrawElements(GL_TRIANGLES, pbinoc->backGroundBinocIndices.size(), GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

inline float FadeChannel(float base, float target, float t)
{
    if (base == target) return base;
    return (base < target)
        ? base + (target - base) * t
        : base - (base - target) * (1.0f - t);
}

void DrawBinocCompass(BINOC* pbinoc)
{
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_STENCIL_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // RECTANGLE
    float width  = (g_gl.width  / 640) * 94.0f;  // 12% of screen width
    float height = (g_gl.height / 492) * 35.0f; // 3% of screen height

    float x = (g_gl.width  - width)  * 0.5f;
    float y = (g_gl.height - height) * 0.02f; // 5% down from top

    glm::vec2 pos  = glm::vec2(x, y);
    glm::vec2 size = glm::vec2(width, height);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    glBlotShader.Use();

    glStencilFunc(GL_ALWAYS, 0, 255);
    glStencilOp(GL_KEEP, GL_KEEP, GL_NONE);
    glColorMask(0, 0, 0, 0);

    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glBindTexture(GL_TEXTURE_2D, whiteTex);
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

    // TICKS
    float scaleY = g_gl.height / 492.8f;
    float scaleX = g_gl.width / 640.0f;

    float baseX = 273.0f;
    float baseY = 9.0f; // Pull to top
    
    float tickWidth  = 7.52f * scaleX;
    float tickHeight = 35.5f * scaleY;
    y = baseY * scaleY;

    glStencilFunc(GL_NOTEQUAL, 0, 255);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glColorMask(1, 1, 1, 1);

    glUniform4f(blotColorLoc, RGBA_DarkBlue.r, RGBA_DarkBlue.g, RGBA_DarkBlue.b, RGBA_DarkBlue.a);

    for (int i = 0; i < 11; i++) {

        float xOffset = (baseX + (i - pbinoc->uCompassBarOffset) * 0.1f * 94.0f) * scaleX;

        model = glm::translate(glm::mat4(1.0f), glm::vec3(xOffset, y, 0.0f));
        model = glm::scale(model, glm::vec3(tickWidth, tickHeight, 1.0f));

        glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }

    float spacing = g_dxPointsMax + 4.0f;
    float yaw = GModPositive(g_pcm->cplook.radPan + 0.3926991f, 6.283185f);
    int idx = (int)(yaw / 0.7853982f) & 7;
    float interp = GModPositive(yaw, 0.7853982f) / 0.7853982f;

    float dxCenter = pbinoc->pfontCompass->DxFromPchz((char*)g_aachzPoints[idx]);
    float dxRight  = pbinoc->pfontCompass->DxFromPchz((char*)g_aachzPoints[(idx + 1) & 7]);
    float dxLeft   = pbinoc->pfontCompass->DxFromPchz((char*)g_aachzPoints[(idx + 7) & 7]);

    float offset = interp * spacing;
    float screenCenterX = g_gl.width * 0.5f;
    baseY = 18.0f;  // top placement scaled

    float xLeft   = (screenCenterX - spacing) - (offset - (spacing - dxLeft) * 0.5f);
    float xRight  = (screenCenterX + spacing) - (offset - (spacing - dxRight) * 0.5f);
    float xCenter =  screenCenterX - (offset - (spacing - dxCenter) * 0.5f);

    CTextBox tbx;
    tbx.SetPos(0, 0);
    tbx.SetSize(g_gl.width, g_gl.height);
    glm::vec4 clqBright;
    tbx.SetTextColor(&clqBright);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    tbx.m_dx = g_dxPointsMax;
    tbx.m_dy = (float)pbinoc->pfontCompass->m_dyUnscaled * pbinoc->pfontCompass->m_ryScale * scaleY;
    tbx.m_rgba = RGBA_LightBlue;
    tbx.m_y = baseY * scaleY;

    tbx.m_x = xLeft;
    pbinoc->pfontCompass->DrawPchz((char*)g_aachzPoints[(idx + 7) & 7], &tbx);

    tbx.m_x = xRight;
    pbinoc->pfontCompass->DrawPchz((char*)g_aachzPoints[(idx + 1) & 7], &tbx);

    // Center text fade effect
    float fade = interp * (interp * -4.0f + 4.0f);
    glm::vec4 fadeColor = RGBA_LightBlue;

    fadeColor.r = FadeChannel(fadeColor.r, 0.19607843f, fade);
    fadeColor.g = FadeChannel(fadeColor.g, 0.51764706f, fade);
    fadeColor.b = FadeChannel(fadeColor.b, 0.69019608f, fade);
    fadeColor.a = FadeChannel(fadeColor.a, 0.50196078f, fade);

    tbx.m_rgba = fadeColor;
    tbx.m_x = xCenter;
    pbinoc->pfontCompass->DrawPchz((char*)g_aachzPoints[idx], &tbx);

    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDepthFunc(GL_LESS);
}

void DrawBinocZoom(BINOC* pbinoc)
{
    glBlotShader.Use();
    glBindTexture(GL_TEXTURE_2D, whiteTex);
    glBindVertexArray(g_gl.gao);
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glDisable(GL_DEPTH_TEST);

    int tickCount = 8;
    float zoom = g_pcm->cplook.uZoom;

    float centerX = g_gl.width * 0.5f;
    float shift = 300.0;

    float baseY = 21.0f + shift;
    float topY  = 0.14f * 492.0f + shift;

    float scaleY = g_gl.height / 492.0f;

    float tickHeight = 3.0f;
    float minWidth = g_gl.width * 0.014f;
    float maxWidth = g_gl.width * 0.14f;

    for (int i = 0; i < tickCount; ++i)
    {
        float t0 = float(i) / float(tickCount);
        float t1 = float(i + 1) / float(tickCount);
        float zoomThresh = (t0 + t1) * 0.5f;

        glm::vec4 color = (zoom < zoomThresh) ? RGBA_DarkBlue : RGBA_LightBlue;

        float y = glm::mix(baseY, topY, (t0 + t1) * 0.5f);
        float width = glm::mix(minWidth, maxWidth, (t0 + t1) * 0.5f);

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(centerX - width * 0.5f, y * scaleY, 0.0f));
        model = glm::scale(model, glm::vec3(width, tickHeight * scaleY, 1.0f));

        glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    }

    glEnable(GL_DEPTH_TEST);

    // Draw stick glyphs
    //if (pbinoc->binocs == BINOCS_Peek)
    //{
    //    float baseY = 390.8f * scaleY;

    //    CTextBox tbx;
    //    tbx.SetPos(0.0f, 0.0f);
    //    tbx.SetSize((float)g_gl.width, (float)g_gl.height);
    //    tbx.SetHorizontalJust(JH_Center);
    //    tbx.SetVerticalJust(JV_Center);
    //    tbx.m_rgba = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    //    tbx.m_dx = 0.0f;
    //    tbx.m_dy = 0.0f;

    //    //JOY* joy = (g_grfjoyt & 2) ? &g_joy : &g_joyZero;

    //    // Left stick
    //    //float scaleL = (pbinoc->fActive && joy->uDeflect != 0.0f) ? 0.5f : 0.4f;
    //    g_pfontJoy->PushScaling(1.0, 1.0);
    //    tbx.m_x = 265.0f;
    //    tbx.m_y = baseY;
    //    g_pfontJoy->DrawPchz((char*)"@", &tbx);
    //    g_pfontJoy->PopScaling();

    //    // Right stick
    //    //float scaleR = (pbinoc->fActive && joy->uDeflect2 != 0.0f) ? 0.5f : 0.4f;
    //    g_pfontJoy->PushScaling(1.0, 1.0);
    //    tbx.m_x = 375.0f;
    //    g_pfontJoy->DrawPchz((char*)"R", &tbx);
    //    g_pfontJoy->PopScaling();
    //}
}

void DrawBinocOutline(BINOC* pbinoc)
{
    glDepthFunc(GL_ALWAYS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // === Calculate band blend colors based on radReticle ===
    float rad = pbinoc->radReticle;

    float tOuter = 0.5f - std::cos(rad + glm::pi<float>() * 0.0f) * 0.5f;
    float tMid   = 0.5f - std::cos(rad + glm::pi<float>() * 1.0f) * 0.5f;
    float tInner = 0.5f - std::cos(rad + glm::pi<float>() * 1.5f) * 0.5f;

    glm::vec4 rgbaOuter = glm::mix(RGBA_DarkBlue, RGBA_LightBlue, tOuter);
    glm::vec4 rgbaMid   = glm::mix(RGBA_DarkBlue, RGBA_LightBlue, tMid);
    glm::vec4 rgbaInner = glm::mix(RGBA_DarkBlue, RGBA_LightBlue, tInner);

    float scaleY = g_gl.height / 492;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0f));
    model = glm::scale(model, glm::vec3(1.0, scaleY, 1.0f));

    glBlotShader.Use();
    glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
    glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniform4fv(uvRectLoc, 1, glm::value_ptr(glm::vec4(0, 0, 1, 1)));

    glBindVertexArray(pbinoc->outlineVAO);
    glBindTexture(GL_TEXTURE_2D, whiteTex);

    // === Draw 3 bands ===
    int numTicks = 64;                  // MUST match BuildBinocOutline
    int numArcs = 2;                    // upper + lower
    int trisPerTick = 2;               // 2 triangles per strip
    int indicesPerTri = 3;             // triangle = 3 indices
    int indicesPerBand = numTicks * numArcs * trisPerTick * indicesPerTri;

    const glm::vec4 bandColors[3] = { rgbaOuter, rgbaMid, rgbaInner };

    for (int i = 0; i < 3; ++i)
    {
        glUniform4fv(blotColorLoc, 1, glm::value_ptr(bandColors[i]));
        glDrawElements(GL_TRIANGLES, indicesPerBand, GL_UNSIGNED_SHORT, (void*)(i * indicesPerBand * sizeof(uint16_t)));
    }

    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
}

void DrawBinocFilter(BINOC* pbinoc)
{

}

void DrawBinoc(BINOC* pbinoc)
{
    if (g_wipe.wipes != WIPES_Idle) return;

    //BINOCS state = pbinoc->binocs;

    pbinoc->binocs = BINOCS_Dialog;

    BINOCS state = pbinoc->binocs;

    if (state > BINOCS_None && state < BINOCS_Instruct) {
        DrawBinocReticle(pbinoc);
        DrawBinocBackground(pbinoc);
        DrawBinocCompass(pbinoc);
        DrawBinocZoom(pbinoc);
        DrawBinocOutline(pbinoc);
        //DrawBinocFilter(pbinoc);
    }
    else if (state == BINOCS_Sniper) {
        DrawBinocReticle(pbinoc);
    }

    const char* text = pbinoc->achzDraw;
    if (!text || text[0] == '\0') return;

    float scale = (state == BINOCS_Confront) ? 0.8f : 1.0f;
    pbinoc->pfont->PushScaling(scale, scale);

    glm::vec4 colorText = pbinoc->rgbaText;
    glm::vec4 colorEdge = pbinoc->pte ? pbinoc->pte->m_rgba : glm::vec4(0.0f);

    CRichText rt((char*)text, pbinoc->pfont);
    int totalChars = rt.Cch();

    float textHeight = pbinoc->pfont->m_dyUnscaled * pbinoc->pfont->m_ryScale;
    float boxWidth = (state == BINOCS_Confront) ? 380.0f : 280.0f;
    float boxHeight = textHeight * ((state == BINOCS_Confront) ? 2.0f : 3.0f);
    float offsetX = (state == BINOCS_Confront) ? -50.0f : 0.0f;
    float offsetY = (state == BINOCS_Confront) ? 40.0f : ((state == BINOCS_Instruct) ? -8.0f : 0.0f);


    float screenWidth = static_cast<float>(g_gl.width);
    float screenHeight = static_cast<float>(g_gl.height);

    float posX = (screenWidth - boxWidth) * 0.5f + offsetX;
    float posY = screenHeight - 128.0f + offsetY;  // adjust this to taste

    CTextBox tbx;
    tbx.SetPos(posX, posY);
    tbx.SetSize(boxWidth, boxHeight);
    glm::vec4 rgba = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    tbx.SetTextColor(&rgba); // gray
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    // Fade alpha
    if (pbinoc->uOn < 1.0f) {
        colorText.a *= pbinoc->uOn;
        colorEdge.a *= pbinoc->uOn;
    }

    // Draw edge if present
    if (state == BINOCS_Instruct && pbinoc->pte && pbinoc->pte->m_pfont) {
        pbinoc->pte->m_rgba = colorEdge;
        pbinoc->pte->m_pfont->EdgeRect(pbinoc->pte, &tbx);
    }

    if (totalChars > 0) {
        int visibleChars = static_cast<int>((g_clock.t - pbinoc->tAchzSet) * pbinoc->svch);
        visibleChars = std::min(visibleChars, totalChars);

        int scrollSegment = 0;
        for (int i = 0; i < pbinoc->cichLR && pbinoc->aichLR[i] <= visibleChars; ++i)
            scrollSegment = i + 1;

        float scrollInterp = 0.0f;
        if (scrollSegment < pbinoc->cichLR) {
            float t0 = static_cast<float>(pbinoc->aichLR[scrollSegment - 1]) / pbinoc->svch;
            float t1 = static_cast<float>(pbinoc->aichLR[scrollSegment]) / pbinoc->svch;
            float current = g_clock.t - pbinoc->tAchzSet;
            scrollInterp = (current - t0) / (t1 - t0);
        }

        char achzPartial[512];
        strcpy(achzPartial, text);

        CRichText rtPartial(achzPartial, pbinoc->pfont);
        rtPartial.Trim(visibleChars);

        if (visibleChars == totalChars && ((int)(g_clock.t * 4.0f) & 1)) {
            strcat(achzPartial, "&.~ffffff|");
        }

        float scrollOffset = ((float)(scrollSegment + 1) + scrollInterp) * textHeight;

        CTextBox tbxScroll;
        tbxScroll.SetPos(tbx.m_x, tbx.m_y - scrollOffset);
        tbxScroll.SetSize(boxWidth, scrollOffset);
        tbxScroll.SetTextColor(&colorText);
        tbxScroll.SetHorizontalJust(JH_Left);
        tbxScroll.SetVerticalJust(JV_Top);

        rtPartial.Draw(&tbx);
    }

    pbinoc->pfont->PopScaling();

    if (pbinoc->chPause != '\0') {
        float t = RadNormalize(g_clock.t * 10.0f);
        float blink = cosf(t) * 0.5f + 0.5f;
        float scaleJoy = glm::mix(0.3f, 0.4f, blink);

        g_pfontJoy->PushScaling(scaleJoy, scaleJoy);

        CTextBox tbxPause;
        tbxPause.SetPos(180.0f + 302.0f, 386.8f + 86.0f);
        tbxPause.SetSize(0.0f, 0.0f);
        glm::vec4 color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        tbxPause.SetTextColor(&color);
        tbxPause.SetHorizontalJust(JH_Left);
        tbxPause.SetVerticalJust(JV_Top);

        g_pfontJoy->DrawPchz(&pbinoc->chPause, &tbxPause);

        g_pfontJoy->PopScaling();
    }
}

SCAN*NewScan()
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
	
	pscan->tbidCaption = pscanBase->tbidCaption;
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
CLQ s_clqUpper = {80.0, -288.0, 288.0, 0.0};
CLQ s_clqLower = {280.80002, 376.0, -376.0, 0.0};
CLQ s_clqReticle = {-8.0, -40.0, 40.0, 0.0};
float g_dxPointsMax = 1.0;
const char *g_aachzPoints[8] =
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