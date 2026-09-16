#include "blot.h"
#include "totals.h"

void InitBlot(BLOT* pblot, BLOTK blotk)
{
    pblot->ptNow = &g_clock.tReal;
    pblot->pbloti = &s_abloti[blotk];
    pblot->dtAppear = 0.25;
    pblot->blotk = blotk;
    pblot->dtDisappear = 0.25;
    ResizeBlot(pblot, pblot->dx, pblot->dy);
    pblot->x = pblot->xOff;
    pblot->y = pblot->yOff;
}

void PostBlotsLoad()
{
    for (int i = 0; i < BLOTK_Max; i++)
    {
        BLOT* pblot = PblotFromBlotk(i);
        if (pblot)
            pblot->pvtblot->pfnPostBlotLoad(pblot);
    }

    //SetBlotFontScale((BLOT*)&g_debugmenu, 0.75);
}

void PostBlotLoad(BLOT* pblot)
{
    pblot->pfont = g_pfont;
    pblot->rFontScale = 1.0;

    pblot->rgba.r = 1.0;
    pblot->rgba.g = 1.0;
    pblot->rgba.b = 1.0;
    pblot->rgba.a = 1.0;

    pblot->achzDraw[0] = '\0';
}

void SetBlotAchzDraw(BLOT *pblot, char *pchz)
{
    if (pchz && *pchz != '\0') {
        // Set the draw string
        std::strncpy(pblot->achzDraw, pchz, sizeof(pblot->achzDraw) - 1);
        pblot->achzDraw[sizeof(pblot->achzDraw) - 1] = '\0';
    }
    else {
        pblot->achzDraw[0] = '\0';
    }

    float scale = pblot->rFontScale;

    // Apply font scale
    pblot->pfont->PushScaling(scale, scale);

    // Compute extents using CRichText
    CRichText rt(pblot->achzDraw, pblot->pfont);
    float dxBlot = 0.0f;
    float dyBlot = 0.0f;
    rt.GetExtents(&dxBlot, &dyBlot, 0.0f);

    // Resize the BLOT to fit text
    ResizeBlot(pblot, dxBlot, dyBlot);

    // Clean up scaling
    pblot->pfont->PopScaling();
}

char* FormatBlotRichText(BLOT* pblot)
{
    char formatted[512]{};

    const char* src = pblot->achzDraw;
    char* dst = formatted;

    while (*src != '\0')
    {
        const char ch = *src;

        if (!pblot->pfont->FValid(ch))
        {
            CFontBrx* fallbackFont = PfontFromFont(3);
            char fontCode = '3';

            if (fallbackFont == nullptr || !fallbackFont->FValid(ch))
            {
                fallbackFont = PfontFromFont(4);
                fontCode = '4'; // DAT_00262254
            }

            if (fallbackFont != nullptr && fallbackFont->FValid(ch))
            {
                // Format string at 0x0024CBD0.
                dst += std::sprintf(dst, "&%c%c&.", fontCode, ch);
                ++src;
                continue;
            }

            // Character wasn't found in any fallback font.
            *dst++ = ch;
            ++src;
            continue;
        }

        if (ch == '/' && std::isdigit(static_cast<unsigned char>(src[1])))
        {
            // Format string at 0x0024CBD8 begins denominator formatting.
            dst += std::sprintf(dst, "%s", "^05~b2a83d/");

            // Skip the original slash.
            ++src;

            while (*src != '\0' && std::isdigit(static_cast<unsigned char>(*src)))
                *dst++ = *src++;

            // Format string at 0x0024CBE8 restores normal formatting.
            dst += std::sprintf(dst, "%s", "~.^.");
            continue;
        }

        *dst++ = ch;
        ++src;
    }

    *dst = '\0';
    return std::strcpy(pblot->achzDraw, formatted);
}

void OnBlotActive(BLOT* pblot, int fActive)
{

}

void ShowBlot(BLOT* pblot)
{
    BLOTS blots = pblot->blots;

    if (blots == BLOTS_Visible) {
        pblot->tBlots = *pblot->ptNow;
    }
    else 
    {
        if (blots < BLOTS_Disappearing) {
            if (blots != BLOTS_Hidden) {
                return;
            }
        }
        else if (blots != BLOTS_Disappearing) {
            return;
        }

        pblot->pvtblot->pfnSetBlotBlots(pblot, BLOTS_Appearing);
    }
}

void HideBlot(BLOT* pblot)
{
    if ((pblot->blots < BLOTS_Disappearing) && (BLOTS_Hidden < pblot->blots)) {
        pblot->pvtblot->pfnSetBlotBlots(pblot, BLOTS_Disappearing);
    }
}

void ForceHideBlots()
{
    for (int i = 28; i >= 0; --i)
    {
        BLOT* pblot = PblotFromBlotk(i);
        if (pblot)
            pblot->pvtblot->pfnSetBlotBlots(pblot, BLOTS_Hidden);
    }
}

BLOTS GetBlot(BLOT* pblot)
{
    return pblot->blots;
}

void SetBlotClock(BLOT* pblot, float* pt)
{
    float fVar1;

    if (pblot->ptNow != pt) {
        if (pt == nullptr) {
            pt = &g_clock.tReal;
            fVar1 = g_clock.tReal;
        }
        else {
            fVar1 = *pt;
        }
        pblot->tBlots = fVar1;
    }
    pblot->ptNow = pt;
}

void SetBlotFontScale(BLOT* pblot, float rFontScale)
{
    pblot->rFontScale = rFontScale;
}

void SetBlotDtVisible(BLOT* pblot, float dtVisible)
{
    pblot->dtVisible = dtVisible;
}

void SetBlotDtAppear(BLOT* pblot, float dtAppear)
{
    pblot->dtAppear = dtAppear;
}

void SetBlotDtDisappear(BLOT* pblot, float dtDisappear)
{
    pblot->dtDisappear = dtDisappear;
}

void SetBlotBlots(BLOT* pblot, BLOTS blots)
{
    if (pblot->blots == blots)
        return;

    bool reposition = false;
    float duration = 0.0f;

    switch (blots) {
    case BLOTS_Appearing:
        duration = pblot->pvtblot->pfnDtAppearBlot(pblot);
        pblot->dtBlotsExtra = pblot->uOn * duration;
        if (pblot->blots != BLOTS_Hidden && pblot->blots != BLOTS_Visible) {
            pblot->blots = blots;
            break;
        }
        reposition = true;
        break;

    case BLOTS_Disappearing:
        duration = pblot->pvtblot->pfnDtDisappearBlot(pblot);
        pblot->dtBlotsExtra = (1.0f - pblot->uOn) * duration;
        if (pblot->blots != BLOTS_Hidden && pblot->blots != BLOTS_Visible) {
            pblot->blots = blots;
            break;
        }
        reposition = true;
        break;

    case BLOTS_Hidden:
        pblot->x = pblot->xOff;
        pblot->y = pblot->yOff;
        pblot->uOn = 0.0f;
        pblot->dtBlotsExtra = 0.0f;
        reposition = true;
        break;

    case BLOTS_Visible:
        pblot->x = pblot->xOn;
        pblot->y = pblot->yOn;
        pblot->uOn = 1.0f;
        pblot->dtBlotsExtra = 0.0f;
        reposition = true;
        break;

    default:
        reposition = true;
        break;
    }

    pblot->blots = blots;
    pblot->tBlots = *pblot->ptNow;

    if (reposition == true)
        RepositionBlot(pblot);
}

float DtAppearBlot(BLOT* pblot)
{
    return pblot->dtAppear;
}

float DtVisibleBlot(BLOT* pblot)
{
    return pblot->dtVisible;
}

float DtDisappearBlot(BLOT* pblot)
{
    return pblot->dtDisappear;
}

void ResizeBlot(BLOT* pblot, float dx, float dy)
{
    if (0.0 <= dx) {
        pblot->dx = dx;
    }

    if (0.0 <= dy) {
        pblot->dy = dy;
    }

    RepositionBlot(pblot);
}

void RepositionBlotDependents(BLOT* pblot)
{
    for (int i = 0; i < BLOTK_Max; ++i)
    {
        BLOT* dependent = PblotFromBlotk(i);
        if (!dependent || !dependent->pbloti) continue;

        BLOTK pegKey = dependent->pbloti->blotkPeg;
        if (pegKey >= 0 && pegKey < BLOTK_Max && PblotFromBlotk(pegKey) == pblot)
        {
            RepositionBlot(dependent);
        }
    }
}

void RepositionBlot(BLOT* pblot)
{
    if (!pblot || !pblot->pbloti) return;

    BLOTI* const placement = pblot->pbloti;
    BLOTI* bloti = placement;

    float left = 0.0f;
    float right = g_gl.width;
    float bottom = 0.0f;
    float top = g_gl.height;

    // Traverse peg chain
    int pegDepth = 0;
    while (bloti->blotkPeg != BLOTK_Nil && pegDepth++ < BLOTK_Max) {
        BLOTK pegKey = bloti->blotkPeg;
        if (pegKey < 0 || pegKey >= BLOTK_Max)
            break;

        BLOT* pegBlot = PblotFromBlotk(pegKey);

        if (pegBlot && pegBlot->pvtblot->pfnFIncludeBlotForPeg(pegBlot, pblot)) {
            BLOTE edge = bloti->blotePeg;

            switch (edge) {
                case BLOTE_Left:
                if (pegBlot->dx != 0.0f)
                    left = pegBlot->x + pegBlot->dx;
                break;

                case BLOTE_Right:
                if (pegBlot->dx != 0.0f)
                    right = pegBlot->x;
                break;

                case BLOTE_Top:
                if (pegBlot->dy != 0.0f)
                    bottom = pegBlot->y + pegBlot->dy;
                break;

                case BLOTE_Bottom:
                if (pegBlot->dy != 0.0f)
                    top = pegBlot->y;
                break;

                default:
                break;
            }

            break;
        }

        // Advance to next peg in chain
        pegKey = bloti->blotkPeg;
        const BLOTK nextPegKey = s_abloti[pegKey].blotkPeg;
        if (nextPegKey < 0 || nextPegKey >= BLOTK_Max)
            break;

        bloti = &s_abloti[nextPegKey];
    }

    // Compute horizontal position
    float xHint = placement->x;
    if (xHint == 0.0f) {
        pblot->xOn = left + (right - left - pblot->dx) * 0.5f;
    }
    else if (xHint > 0.0f) {
        pblot->xOn = left + xHint;
    }
    else {
        pblot->xOn = right + xHint - pblot->dx;
    }

    // Compute vertical position
    float yHint = placement->y;
    if (yHint == 0.0f) {
        pblot->yOn = bottom + (top - bottom - pblot->dy) * 0.5f;
    }
    else if (yHint > 0.0f) {
        pblot->yOn = bottom + yHint;
    }
    else {
        pblot->yOn = top + yHint - pblot->dy;
    }

    // Apply edge anchoring offset
    // The peg-chain walk above may leave bloti pointing at an ancestor.
    // Off-screen motion belongs to this blot's own placement descriptor.
    switch (placement->blote) {
        case BLOTE_Left:
        pblot->xOff = -pblot->dx;
        pblot->yOff = pblot->yOn;
        break;
        case BLOTE_Right:
        pblot->xOff = g_gl.width;
        pblot->yOff = pblot->yOn;
        break;
        case BLOTE_Top:
        pblot->xOff = pblot->xOn;
        pblot->yOff = -pblot->dy;
        break;
        case BLOTE_Bottom:
        pblot->xOff = pblot->xOn;
        pblot->yOff = g_gl.height;
        break;
        default:
        pblot->xOff = pblot->xOn;
        pblot->yOff = pblot->yOn;
        break;
    }

    // Determine final onscreen position
    if (pblot->blots == BLOTS_Hidden) {
        pblot->x = pblot->xOff;
        pblot->y = pblot->yOff;
    }
    else if (pblot->blots == BLOTS_Visible) {
        pblot->x = pblot->xOn;
        pblot->y = pblot->yOn;
    }
    else {
        // Preserve an appear/disappear animation when a resize rebuilds its
        // endpoints instead of snapping it to one side of the screen.
        const float u = std::clamp(pblot->uOn, 0.0f, 1.0f);
        const float smooth = u * (2.0f - u);
        pblot->x = pblot->xOff + smooth * (pblot->xOn - pblot->xOff);
        pblot->y = pblot->yOff + smooth * (pblot->yOn - pblot->yOff);
    }

    // Recursively reposition any dependents
    RepositionBlotDependents(pblot);
}

void RepositionAllBlots()
{
    for (int i = 0; i < BLOTK_Max; i++)
        RepositionBlot(PblotFromBlotk(i));
}

int FIncludeBlotForPeg(BLOT* pblot, BLOT* pblotOther)
{
    int iVar1;

    iVar1 = 0;
    if (((uint32_t)(pblot->blots + BLOTS_Nil) < 2) && ((uint32_t)(pblotOther->blots + BLOTS_Nil) < 2)) {
        iVar1 = 1;
    }
    else if ((pblot->blots == BLOTS_Disappearing) &&
        ((pblotOther->blots == BLOTS_Disappearing || (pblotOther->blots == BLOTS_Visible)))) {
        iVar1 = 1;
    }
    return iVar1;
}

void OnBlotReset(BLOT* pblot)
{
    SetBlotBlots(pblot, BLOTS_Hidden);
}

void UpdateBlot(BLOT* pblot)
{
    BLOTS oldState = pblot->blots;
    BLOTS newState = oldState;
    float now = *pblot->ptNow;
    float uOn = pblot->uOn;

    switch (oldState) 
    {
    case BLOTS_Visible: 
    {
        float duration = ((VTBLOT*)pblot->pvtblot)->pfnDtVisibleBlot(pblot);
        if (duration == 0.0f || (now - pblot->tBlots) < duration) {
            break;
        }
        newState = BLOTS_Disappearing;
        break;
    }

    case BLOTS_Appearing: {
        float duration = ((VTBLOT*)pblot->pvtblot)->pfnDtAppearBlot(pblot);
        if (duration == 0.0f) {
            uOn = pblot->uOn;
        }
        else {
            float t = (now - pblot->tBlots + pblot->dtBlotsExtra) / duration;
            t = glm::clamp(t, 0.0f, 1.0f);
            pblot->uOn = t;
            uOn = t;
        }
        if (uOn >= 1.0f) {
            newState = BLOTS_Visible;
        }
        break;
    }

    case BLOTS_Disappearing: {
        float duration = ((VTBLOT*)pblot->pvtblot)->pfnDtDisappearBlot(pblot);
        if (duration == 0.0f) {
            uOn = pblot->uOn;
        }
        else {
            float t = (now - pblot->tBlots + pblot->dtBlotsExtra) / duration;
            t = glm::clamp(t, 0.0f, 1.0f);
            pblot->uOn = 1.0f - t;
            uOn = pblot->uOn;
        }
        if (uOn <= 0.0f) {
            newState = BLOTS_Hidden;
        }
        break;
    }

    default:
        break;
    }

    // If state didn't change, update position smoothly
    if (newState == oldState) {
        float t = 0.0 + pblot->uOn * (2.0 + pblot->uOn * -1.0);
        pblot->x = pblot->xOff + t * (pblot->xOn - pblot->xOff);
        pblot->y = pblot->yOff + t * (pblot->yOn - pblot->yOff);
        RepositionBlotDependents(pblot);
    }

    // Apply final state change
    VTBLOT* vt = (VTBLOT*)pblot->pvtblot;
    vt->pfnSetBlotBlots(pblot, newState);
}

void UpdateBlotActive(BLOT* pblot, JOY* pjoy)
{

}

void UpdateBlots()
{
    for (int i = 0; i < BLOTK_Max; ++i)
    {
        BLOT* pblot = PblotFromBlotk(i);
        if (pblot && pblot->pvtblot->pfnUpdateBlot != nullptr)
            pblot->pvtblot->pfnUpdateBlot(pblot);
    }

    UpdateWipe(&g_wipe, &g_joy);
}

void DrawBlot(BLOT* pblot)
{
    if (!pblot || pblot->achzDraw[0] == '\0') return;

    // Setup textbox dimensions and color
    CTextBox tbx;
    tbx.SetPos(pblot->x, pblot->y);
    tbx.SetSize(pblot->dx, pblot->dy);
    tbx.SetTextColor(&pblot->rgba);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    // If edge text effect is enabled, draw it
    if (pblot->pte && pblot->pte->m_pfont) {
        pblot->pte->m_pfont->EdgeRect(pblot->pte, &tbx);
    }
    
    // Push font scale
    float scale = pblot->rFontScale;
    pblot->pfont->PushScaling(scale, scale);

    // Draw the text using the current font and text box
    pblot->pfont->DrawPchz(pblot->achzDraw, &tbx);

    // Pop font scale
    pblot->pfont->PopScaling();
}

void RenderBlots()
{
    for (int i = 0; i < BLOTK_Max; ++i)
    {
        BLOT* pblot = PblotFromBlotk(i);

        if (pblot && pblot->blots != BLOTS_Hidden && pblot->pvtblot->pfnRenderBlot != nullptr)
            pblot->pvtblot->pfnRenderBlot(pblot);
    }
}

void DrawBlots()
{
    for (int i = 0; i < BLOTK_Max; ++i)
    {
        BLOT* pblot = PblotFromBlotk(i);
        if (pblot && pblot->blots != BLOTS_Hidden)
            pblot->pvtblot->pfnDrawBlot(pblot);
    }

    DrawWipe(&g_wipe);
    DrawAutoSave(&g_autosave);
}

void ResetBlots()
{
    for (int i = 0; i < BLOTK_Max; ++i)
    {
        BLOT* pblot = PblotFromBlotk(i);
        if (pblot)
            pblot->pvtblot->pfnOnBlotReset(pblot);
    }
}

VTLIFECTR g_vtlifectr;
VTKEYCTR g_vtkeyctr;
VTLETTERBOX g_vtletterbox;
VTCLUECTR g_vtcluectr;
VTCOINCTR g_vtcoinctr;
VTTITLE g_vttitle;
VTWMC g_vtwmc;
VTBINOC g_vtbinoc;
VTATTRACT g_vtattract;
VTBLOT g_vtblot;
VTTIMER g_vttimer;
VTTRUNKCTR g_vttrunkctr;
VTPUFFCHARGECTR g_vtpuffchargectr;
VTBOOSTCTR g_vtboostctr;
VTCREDIT g_vtcredit;
VTCTR g_vtctr;
VTCRUSHERCTR g_vtcrusherctr;
VTTOTALS g_vttotals;
VTTV g_vttv;
VTPLACECTR g_vtplacectr;
VTDEBUGMENU g_vtdebugmenu;
VTLOGO g_vtlogo;
VTBOSS g_vtboss;
VTNOTE g_vtnote;
VTCALL g_vtcall;
VTSCORES g_vtscores;
VTVAN g_vtvan;
VTLGNR g_vtlgnr;
VTGOLDCTR g_vtgoldctr;
VTLAPCTR g_vtlapctr;
VTPROMPT g_vtprompt;
BLOTI s_abloti[37] =
{
    {  0.0f,   0.0f, static_cast<BLOTE>(-1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {  0.0f,   0.0f, static_cast<BLOTE>(-1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {  0.0f,   0.0f, static_cast<BLOTE>(-1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {  0.0f,  20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {-18.0f,  20.0f, static_cast<BLOTE>(1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {-18.0f,  20.0f, static_cast<BLOTE>(1), static_cast<BLOTK>(4), static_cast<BLOTE>(2) },
    { 18.0f,  20.0f, static_cast<BLOTE>(0), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {-18.0f,  20.0f, static_cast<BLOTE>(1), static_cast<BLOTK>(5), static_cast<BLOTE>(2) },
    {-18.0f,  20.0f, static_cast<BLOTE>(1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { 18.0f,  20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(6), static_cast<BLOTE>(2) },
    {  0.0f,  20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(3), static_cast<BLOTE>(2) },
    {-18.0f,  20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(5), static_cast<BLOTE>(2) },
    {-18.0f,  20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(5), static_cast<BLOTE>(2) },
    { 18.0f,  20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(6), static_cast<BLOTE>(2) },
    { 18.0f,  20.0f, static_cast<BLOTE>(0), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { 18.0f,  20.0f, static_cast<BLOTE>(0), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { 18.0f, -20.0f, static_cast<BLOTE>(0), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { 18.0f, -20.0f, static_cast<BLOTE>(0), static_cast<BLOTK>(16), static_cast<BLOTE>(3) },
    { 18.0f, -20.0f, static_cast<BLOTE>(-1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {-18.0f, -20.0f, static_cast<BLOTE>(1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {-18.0f, -20.0f, static_cast<BLOTE>(1), static_cast<BLOTK>(19), static_cast<BLOTE>(3) },
    { 18.0f, -20.0f, static_cast<BLOTE>(0), static_cast<BLOTK>(18), static_cast<BLOTE>(3) },
    { 18.0f, -20.0f, static_cast<BLOTE>(-1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {  0.0f, -40.0f, static_cast<BLOTE>(-1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {  0.0f,   0.0f, static_cast<BLOTE>(-1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {  0.0f,   0.0f, static_cast<BLOTE>(-1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {  1.0f, -20.0f, static_cast<BLOTE>(0), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { -1.0f, -20.0f, static_cast<BLOTE>(1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {-18.0f,  20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { 18.0f,  20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { 18.0f,  20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {-18.0f, -20.0f, static_cast<BLOTE>(3), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { 18.0f,  20.0f, static_cast<BLOTE>(-1), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { 18.0f, -20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { 18.0f, -20.0f, static_cast<BLOTE>(2), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    { 18.0f, -20.0f, static_cast<BLOTE>(3), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) },
    {  0.0f, -20.0f, static_cast<BLOTE>(3), static_cast<BLOTK>(-1), static_cast<BLOTE>(-1) }
};
