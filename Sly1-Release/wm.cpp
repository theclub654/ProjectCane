#include "wm.h"

WM* NewWm()
{
	return new WM{};
}

int GetWmSize()
{
	return sizeof(WM);
}

void CloneWm(WM* pwm, WM* pwmBase)
{
    CloneAlo(pwm, pwmBase);

    pwm->wms = pwmBase->wms;
    pwm->tWms = pwmBase->tWms;
    pwm->worldlevelCur = pwmBase->worldlevelCur;
    pwm->worldlevelGoal = pwmBase->worldlevelGoal;
    pwm->wmsActive = pwmBase->wmsActive;
    pwm->pasegOpen = pwmBase->pasegOpen;
    pwm->pasegClose = pwmBase->pasegClose;
    pwm->pasegaOpenClose = pwmBase->pasegaOpenClose;
    pwm->pasegaMove = pwmBase->pasegaMove;
    pwm->wmdMoveFrom = pwmBase->wmdMoveFrom;
    pwm->worldlevelMoveFrom = pwmBase->worldlevelMoveFrom;

    // Clone WMW array
    for (int i = 0; i < 9; i++) {
        pwm->awmw[i] = pwmBase->awmw[i];
    }

    pwm->worldlevelCursor = pwmBase->worldlevelCursor;
    pwm->posCursor = pwmBase->posCursor;
    pwm->posCursorTarget = pwmBase->posCursorTarget;
    pwm->fManualWarp = pwmBase->fManualWarp;
}

void BindWm(WM* pwm)
{
    BindAlo(pwm);
}

void RenderWmAll(WM* pwm, CM* pcm, RO* pro)
{
	RenderAloAll(pwm, pcm, pro);
}

void DeleteWm(WM* pwm)
{
	delete pwm;
}

void StartupWmc(WMC* pwmc)
{
    g_teWmc.m_rgba = glm::vec4(0.0f, 0.294f, 0.490f, 1.0f);
    g_teWmc.m_ch = '-';
    g_teWmc.m_dxExtra = 2.0f;
    g_teWmc.m_dyExtra = 1.0f;
    g_teWmc.m_rxScaling = 0.3f;
    g_teWmc.m_ryScaling = 0.3f;

    g_rgbaBoC = glm::vec4{ 0.0, 0.0, 0.0, 0.0 };  // All channels zero
    g_rgbaBoE = glm::vec4{ 0.0, 0.0, 0.0, 0.0 };
    pwmc->pvtwmc = &g_vtwmc;
}

void PostWmcLoad(WMC* pwmc)
{
    // Initialize BLOT base
    PostBlotLoad(pwmc);

    // Assign font for text edge rendering
    if (g_pfontScreenCounters)
    {
        pwmc->pte = &g_teWmc;
        g_teWmc.m_pfont = g_pfontScreenCounters;
    }

    // Set initial visibility to hidden (uDt = 0.0)
    SetBlotDtVisible(pwmc, 0.0f);

    // Set font scale to 0.6x
    SetBlotFontScale(pwmc, 0.6f);

    pwmc->rgba = glm::vec4(0.498f, 0.498f, 0.498f, 0.874f);

    // Set default draw string via virtual call
    if (pwmc->pvtblot && pwmc->pvtblot->pfnSetBlotAchzDraw)
    {
        pwmc->pvtblot->pfnSetBlotAchzDraw(pwmc, (char*)"&2T&.: Exit\n&2(&.: Move\n&2X&.: Warp"); // replace with real string if needed
    }

    // Null out WM pointer
    pwmc->pwm = nullptr;
}

void HideWm(WM* pwm)
{
    /*if ((pwm->wms < WMS_Warping) && (WMS_Hidden < pwm->wms)) {
        SetWmWms(pwm, WMS_Disappearing);
    }*/
}

void OnWmcActive(WMC* pwmc, int fActive)
{
    // Only proceed if the state is actually changing
    //if ((fActive != 0) == (pwmc->fActive == 0))
    //{
    //    if (fActive == 0)
    //    {
    //        // Deactivating WMC
    //        HideWm(pwmc->pwm);

    //        ASEG* asegClose = pwmc->pwm->pasegClose;
    //        VTBLOT* vtblot = pwmc->pvtblot;

    //        if (asegClose != nullptr)
    //            SetBlotDtDisappear(pwmc, asegClose->tMax);

    //        vtblot->pfnHideBlot(pwmc);
    //        RemoveGrfusr(1);
    //        SetUiUis(&g_ui, UIS_Unpausing);

    //        pwmc->fActive = 0;
    //    }
    //    else
    //    {
    //        // Activating WMC
    //        ForceHideBlots();

    //        ShowWm(pwmc->pwm, g_pgsCur->worldlevelCur, WMS_Manual);

    //        pwmc->uWarp = 1.0f;
    //        pwmc->uWarpTarget = 1.0f;

    //        ASEG* asegOpen = pwmc->pwm->pasegOpen;
    //        VTBLOT* vtblot = pwmc->pvtblot;

    //        if (asegOpen != nullptr)
    //            SetBlotDtAppear(pwmc, asegOpen->tMax);

    //        vtblot->pfnShowBlot(pwmc);
    //        AddGrfusr(1);
    //        StartJoySelection(&g_joy);
    //        SetUiUis(&g_ui, UIS_WorldMap);

    //        pwmc->fActive = fActive;
    //    }
    //}
}

void UpdateWmcActive(WMC* pwmc, JOY* pjoy)
{

}

void DrawWmc(WMC* pwmc)
{

}

WMC g_wmc;
CTextEdge g_teWmc;
glm::vec4 g_rgbaBoC;
glm::vec4 g_rgbaBoE;