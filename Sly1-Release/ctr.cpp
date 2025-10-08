#include "ctr.h"

void PostCtrLoad(CTR* pctr)
{
    PostBlotLoad(pctr);

    pctr->pfont = &g_afontBrx[2];
    pctr->nDisplay = -1;
    pctr->dgDisplayMax = 6000.0;

    //GOTTA COME BACK TO THIS
    switch (pctr->blotk)
    {
        case BLOTK_Clue:
        pctr->pnTotal = &g_psw->cclueAll;
        break;

        case BLOTK_Trunk:
        break;

        case BLOTK_Crusher:
        break;

        case BLOTK_Lap:
        break;

        case BLOTK_Boost:
        break;

        case BLOTK_Place:
        break;

        case BLOTK_Boss:
        break;

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
    float dt = 0.0;
    if (g_clock.fEnabled != 0) {
        dt = 2.5;
    }
    return dt;
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
            CRichText rtxt(pctr->achzDraw, pctr->pfont);

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
        pctr->achzDraw[0] = '&';   // start font switch
        pctr->achzDraw[1] = '3';   // font index (switch to font 3)
        pctr->achzDraw[2] = 'K';   // glyph 'K' drawn with font #3
        pctr->achzDraw[3] = '&';   // start restore
        pctr->achzDraw[4] = '.';   // restore to base font
        pctr->achzDraw[5] = '0';   // glyph '3' drawn with base font
        break;

        case BLOTK_Gold:

        break;

        case BLOTK_Coin:
        pctr->achzDraw[0] = '&';   // start font switch
        pctr->achzDraw[1] = '3';   // font index (switch to font 3)
        pctr->achzDraw[2] = 'C';   // glyph 'C' drawn with font #3
        pctr->achzDraw[3] = '&';   // start restore
        pctr->achzDraw[4] = '.';   // restore to base font
        pctr->achzDraw[5] = '0';   // glyph '3' drawn with base font
        break;

        case BLOTK_Trunk:

        break;

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
    if (!pctr || pctr->achzDraw[0] == '\0') return;

    // Pause overlay fade: only apply when pause UI is visible
    float fade = 1.0f;
    /*if (g_prompt.blots != BLOTS_Hidden) {
        fade = g_promptFade;
    }*/

    // Text box
    CTextBox tbx;
    tbx.SetPos(pctr->x, pctr->y);
    tbx.SetSize(pctr->dx, pctr->dy);
    tbx.SetTextColor(&pctr->rgba);
    tbx.SetHorizontalJust(JH_Left);
    tbx.SetVerticalJust(JV_Top);

    // Color with pause alpha modulation
    glm::vec4 mod = pctr->rgba;

    // multiply alpha by fade (clamped to 0–1)
    mod.a *= fade;
    if (mod.a > 1.0f) mod.a = 1.0f;
    if (mod.a < 0.0f) mod.a = 0.0f;

    tbx.SetTextColor(&mod);

    // Scale & draw rich text
    pctr->pfont->PushScaling(pctr->rFontScale, pctr->rFontScale);
    
    CRichText rtxt(pctr->achzDraw, pctr->pfont);
    rtxt.Draw(&tbx);

    pctr->pfont->PopScaling();
}