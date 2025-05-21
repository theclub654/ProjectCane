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
    }

    pctr->pnActual = 0;
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

    //int display = pctr->nDisplay;
    //int actual = *pctr->pnActual;

    //if (display != actual) {
    //    if (display < 0) {
    //        pctr->nDisplay = actual; // Hard reset
    //    }
    //    else {
    //        // Smoothly update nDisplay toward actual
    //        float updateAmount = g_clock.dt * pctr->dgDisplayMax + pctr->uRemain;
    //        int delta = (int)updateAmount;
    //        pctr->uRemain = updateAmount - delta;

    //        int nextDisplay = display;
    //        if (actual < display) {
    //            int dec = display - delta;
    //            nextDisplay = (actual < dec) ? dec : actual;
    //        }
    //        else {
    //            int inc = display + delta;
    //            nextDisplay = (inc < actual) ? inc : actual;
    //        }

    //        pctr->nDisplay = nextDisplay;
    //    }

    //    // If we've arrived at the target and a callback is set, invoke it
    //    if (pctr->nDisplay == *pctr->pnActual && pctr->pfnsmack) {
    //        //pctr->pfnsmack(pctr->pv);
    //        pctr->pfnsmack = nullptr;
    //        pctr->pv = nullptr;
    //    }

    //    RebuildCtrAchzDraw(pctr); // Rebuild string (likely turns number into string)

    //    // Adjust size of the counter's BLOT area
    //    if (pctr->achzDraw[0] == '\0') {
    //        ResizeBlot(pctr, 0.0f, 0.0f);
    //    }
    //    else {
    //        pctr->pfont->PushScaling(pctr->rFontScale, pctr->rFontScale);
    //        
    //        float dx = pctr->pfont->DxFromPchz(pctr->achzDraw);
    //        float dy = pctr->pfont->m_dyUnscaled * pctr->pfont->m_ryScale;
    //        ResizeBlot(pctr, dx, dy);
    //        pctr->pfont->PopScaling();
    //    }
    //}
}

void RebuildCtrAchzDraw(CTR *pctr)
{
    switch (pctr->blotk) 
    {
        case BLOTK_Lives:
        _snprintf_s(pctr->achzDraw, 0x200, "%c%d");
        break;
        case BLOTK_Clue:
        if (*pctr->pnTotal == 0) {
            pctr->achzDraw[0] = '\0';
        }
        else {
            _snprintf_s(pctr->achzDraw, 0x200, "%d/%d%c");
        }
        break;
        case BLOTK_Key:
            _snprintf_s(pctr->achzDraw, 0x200, "%d%c");
        break;
        case BLOTK_Gold:
            _snprintf_s(pctr->achzDraw, 0x200, "%d/%d%c");
        break;
        case BLOTK_Coin:
            _snprintf_s(pctr->achzDraw, 0x200, "%c%d");
        break;
        case BLOTK_Trunk:
        case BLOTK_Crusher:
            _snprintf_s(pctr->achzDraw, 0x200, "%d/%d");
        break;
        case BLOTK_Lap:
            _snprintf_s(pctr->achzDraw, 0x200, "%c%d/%d");
        break;
        case BLOTK_Boost:
            _snprintf_s(pctr->achzDraw, 0x200, "%c%d");
        break;
        case BLOTK_Place:
            _snprintf_s(pctr->achzDraw, 0x200, "%d/%d%c");
        break;
        case BLOTK_Boss:
            _snprintf_s(pctr->achzDraw, 0x200, "rstuv");
        break;
        case BLOTK_PuffCharge:
            _snprintf_s(pctr->achzDraw, 0x200, "%d%c");
        break;
    }
}

void DrawCtr(CTR* pctr)
{

}