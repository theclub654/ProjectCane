#include "blot.h"

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
    for (int i = 0; i < 29; i++)
        s_apblot[i]->pvtblot->pfnPostBlotLoad(s_apblot[i]);

    SetBlotFontScale((BLOT*)&g_debugmenu, 0.75);
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
    for (int i = 0; i < 29; ++i) 
    {
        BLOT* dependent = s_apblot[i];
        if (!dependent || !dependent->pbloti) continue;

        BLOTK pegKey = dependent->pbloti->blotkPeg;
        if (pegKey != BLOTK_Nil && s_apblot[pegKey] == pblot)
        {
            RepositionBlot(dependent);
        }
    }
}

void RepositionBlot(BLOT* pblot)
{
    if (!pblot || !pblot->pbloti) return;

    BLOTI* bloti = pblot->pbloti;

    float left = 0.0f;
    float right = g_gl.width;
    float bottom = 0.0f;
    float top = g_gl.height;

    // Traverse peg chain
    while (bloti->blotkPeg != BLOTK_Nil) {
        BLOTK pegKey = bloti->blotkPeg;
        BLOT* pegBlot = s_apblot[pegKey];

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
        if (s_abloti[pegKey].blotkPeg == BLOTK_Nil) break;
        bloti = &s_abloti[s_abloti[pegKey].blotkPeg];
    }

    // Compute horizontal position
    float xHint = pblot->pbloti->x;
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
    float yHint = pblot->pbloti->y;
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
    switch (bloti->blote) {
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
    else {
        pblot->x = pblot->xOn;
        pblot->y = pblot->yOn;
    }

    // Recursively reposition any dependents
    RepositionBlotDependents(pblot);
}

void RepositionAllBlots()
{
    for (int i = 0; i < 29; i++)
        RepositionBlot(s_apblot[i]);
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
    pblot->pvtblot->pfnSetBlotBlots(pblot, BLOTS_Hidden);
}

void UpdateBlot(BLOT* pblot)
{
    BLOTS oldState = pblot->blots;
    BLOTS newState = oldState;
    float now = *pblot->ptNow;
    float uOn = pblot->uOn;

    switch (oldState) {
    case BLOTS_Visible: {
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
    for (int i = 0; i < 29; i++)
    {
        if (s_apblot[i]->pvtblot->pfnUpdateBlot != nullptr)
            s_apblot[i]->pvtblot->pfnUpdateBlot(s_apblot[i]);
    }
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

void DrawBlots()
{
    for (int i = 0; i < 29; i++)
    {
        if (s_apblot[i]->blots != BLOTS_Hidden)
            s_apblot[i]->pvtblot->pfnDrawBlot(s_apblot[i]);
    }
}

void ResetBlots()
{
    for (int i = 0; i < 29; i++)
        s_apblot[i]->pvtblot->pfnOnBlotReset(s_apblot[i]);
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
VTBOSSCTR g_vtbossctr;
VTNOTE g_vtnote;
VTCALL g_vtcall;
VTSCORES g_vtscores;
VTLGNR g_vtlgnr;
VTGOLDCTR g_vtgoldctr;
VTLAPCTR g_vtlapctr;
VTPROMPT g_vtprompt;