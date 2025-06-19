#include "ctr.h"

void PostCtrLoad(CTR* pctr)
{
    PostBlotLoad(pctr);

    pctr->pfont = g_pfontScreenCounters;
    pctr->nDisplay = -1;
    pctr->dgDisplayMax = 6000.0;

    //GOTTA COME BACK TO THIS
    switch (pctr->blotk)
    {
        case BLOTK_Clue:
        pctr->pnTotal = &g_psw->cclueAll;
        break;
        case BLOTK_Trunk:
        case BLOTK_Crusher:
        case BLOTK_Lap:
        case BLOTK_Boost:
        case BLOTK_Place:
        case BLOTK_Boss:
        case BLOTK_PuffCharge:
        pctr->pnActual = &pctr->nDisplay;
        break;
        default:
        pctr->pnActual = nullptr;
        break;
    }
}

float DtVisibleCtr(CTR* pctr)
{
    float fVar1;

    fVar1 = 0.0;
    if (g_clock.fEnabled != 0) {
        fVar1 = 5.0;
    }
    return fVar1;
}

void UpdateCtr(CTR* pctr)
{
    UpdateBlot(pctr);
    
    int display = pctr->nDisplay;
    int actual{};

    if (pctr->pnActual == nullptr)
    {
        actual = 0;
    }
    else
        actual = *pctr->pnActual;

    if (display != actual) {
        if (display < 0) {
            pctr->nDisplay = actual; // Hard reset
        }
        else {
            // Smoothly update nDisplay toward actual
            float updateAmount = g_clock.dt * pctr->dgDisplayMax + pctr->uRemain;
            int delta = (int)updateAmount;
            pctr->uRemain = updateAmount - delta;

            int nextDisplay = display;
            if (actual < display) {
                int dec = display - delta;
                nextDisplay = (actual < dec) ? dec : actual;
            }
            else {
                int inc = display + delta;
                nextDisplay = (inc < actual) ? inc : actual;
            }

            pctr->nDisplay = nextDisplay;
        }

        // If we've arrived at the target and a callback is set, invoke it
        //if (pctr->nDisplay == *pctr->pnActual && pctr->pfnsmack) {
        //    //pctr->pfnsmack(pctr->pv);
        //    pctr->pfnsmack = nullptr;
        //    pctr->pv = nullptr;
        //}
        
        RebuildCtrAchzDraw(pctr); // Rebuild string (likely turns number into string)

        // Adjust size of the counter's BLOT area
        if (pctr->achzDraw[0] == '\0') {
            ResizeBlot(pctr, 0.0f, 0.0f);
        }
        else {
            pctr->pfont->PushScaling(pctr->rFontScale, pctr->rFontScale);
            
            float dx = pctr->pfont->DxFromPchz(pctr->achzDraw);
            float dy = pctr->pfont->m_dyUnscaled * pctr->pfont->m_ryScale;
            ResizeBlot(pctr, dx, dy);
            pctr->pfont->PopScaling();
        }
    }
}

void RebuildCtrAchzDraw(CTR *pctr)
{
    switch (pctr->blotk) 
    {
        case BLOTK_Lives:
        pctr->achzDraw[0] = 'L';
        pctr->achzDraw[1] = '0';

        break;
        case BLOTK_Clue:
        if (*pctr->pnTotal == 0) {
            pctr->achzDraw[0] = '\0';
        }
        else {
            std::string numClues = std::to_string(*pctr->pnTotal);

            pctr->achzDraw[0] = '0';
            pctr->achzDraw[1] = '/';
            int stringLength = numClues.length();

            int idxClues = 0;
            int idxDraw  = 0;

            for (int i = 0; i < stringLength; i++)
            {
                pctr->achzDraw[2 + idxDraw] = numClues[idxClues];
                idxClues++;
                idxDraw++;
            }

            pctr->achzDraw[2 + idxDraw] = '?';
        }
        break;
        case BLOTK_Key:
        pctr->achzDraw[0] = '0';
        pctr->achzDraw[1] = 'K';
        break;
        case BLOTK_Gold:
        
        break;
        case BLOTK_Coin:
        pctr->achzDraw[0] = 'C';
        pctr->achzDraw[1] = '0';
        
        break;
        case BLOTK_Trunk:
        case BLOTK_Crusher:
        
        break;
        case BLOTK_Lap:
            pctr->achzDraw[0] = '3';
            pctr->achzDraw[1] = '0';
            pctr->achzDraw[2] = '2';
            pctr->achzDraw[3] = 'F';
            pctr->achzDraw[4] = '3';
            pctr->achzDraw[5] = '0';
            pctr->achzDraw[6] = '4';
            pctr->achzDraw[7] = '8';
        break;
        case BLOTK_Boost:
        
        break;
        case BLOTK_Place:
        
        break;
        case BLOTK_Boss:
        
        break;
        case BLOTK_PuffCharge:
        
        break;
    }
}

void DrawCtr(CTR* pctr)
{
    if (!pctr || pctr->achzDraw[0] == '\0')
        return;

    // If it's just a standard blot (no counter), draw normally
    if (!pctr->pnTotal) {
        DrawBlot(pctr);
        return;
    }

    // === Setup textbox ===
    CTextBox tbx;
    tbx.SetPos(pctr->x, pctr->y);
    tbx.SetSize(pctr->dx, pctr->dy);
    tbx.SetTextColor(&pctr->rgba);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    // === Parse achzDraw into num / denom / after ===
    char achzNum[10]{};
    char achzSlash[2] = "/";
    char achzDenom[10]{};
    char achzAfter[10]{};

    const char* fullText = pctr->achzDraw;
    const char* slash = strchr(fullText, '/');

    if (!slash) {
        strncpy_s(achzNum, sizeof(achzNum), fullText, _TRUNCATE);
    }
    else {
        size_t numLen = slash - fullText;
        strncpy_s(achzNum, sizeof(achzNum), fullText, numLen);

        const char* denomStart = slash + 1;
        size_t denomLen = 0;
        while (isdigit((unsigned char)*denomStart)) {
            if (denomLen < sizeof(achzDenom) - 1)
                achzDenom[denomLen++] = *denomStart;
            denomStart++;
        }
        achzDenom[denomLen] = '\0';

        strncpy_s(achzAfter, sizeof(achzAfter), denomStart, _TRUNCATE);
    }

    // === Measure widths ===
    float dxNum = pctr->pfont->DxFromPchz(achzNum);
    float dxSlash = pctr->pfont->DxFromPchz(achzSlash);
    float dxDenom = pctr->pfont->DxFromPchz(achzDenom);

    // === Offset for left-anchored layout ===
    float xOffset = (pctr->pbloti && pctr->pbloti->x < 0.0f) ? dxDenom * 0.5f : 0.0f;

    // === Optional edge ===
    if (pctr->pte && pctr->pte->m_pfont)
        pctr->pte->m_pfont->EdgeRect(pctr->pte, &tbx);

    float scale = pctr->rFontScale;
    float smallScale = 0.5f;

    // === Draw numerator ===
    pctr->pfont->PushScaling(scale, scale);
    tbx.m_x = pctr->x + xOffset;
    tbx.m_y = pctr->y;
    pctr->pfont->DrawPchz(achzNum, &tbx);
    pctr->pfont->PopScaling();

    // === Draw slash (same scale and color as numerator) ===
    pctr->pfont->PushScaling(scale, scale);
    tbx.m_x += dxNum;
    tbx.SetTextColor(&pctr->rgba);  // restore original color
    pctr->pfont->DrawPchz(achzSlash, &tbx);
    pctr->pfont->PopScaling();

    // === Draw denominator (smaller and tinted) ===
    tbx.m_x += dxSlash;
    tbx.m_y = pctr->y + 5.0f;
    glm::vec4 denomColor = glm::vec4(0.7f, 0.66f, 0.24f, 0.5f);  // yellow, 50% alpha
    tbx.SetTextColor(&denomColor);
    pctr->pfont->PushScaling(0.5f, 0.5f);
    pctr->pfont->DrawPchz(achzDenom, &tbx);
    float dxDenomText = pctr->pfont->DxFromPchz(achzDenom);
    pctr->pfont->PopScaling();

    // === Restore original text color ===
    tbx.SetTextColor(&pctr->rgba);

    // === Draw trailing suffix (e.g. %) ===
    if (achzAfter[0] != '\0') {
        tbx.m_x += dxDenomText + 1.0f;
        tbx.m_y = pctr->y;
        pctr->pfont->DrawPchz(achzAfter, &tbx);
    }
}