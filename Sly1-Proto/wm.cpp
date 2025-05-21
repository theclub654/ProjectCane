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
    pwmc->pvtwmc = &g_vtwmc;
}

void PostWmcLoad(WMC* pwmc)
{
    PostBlotLoad(pwmc);

    if (g_pfontScreenCounters != nullptr)
    {
        pwmc->pte = &g_teWmc;
        g_teWmc.m_pfont = g_pfontScreenCounters;
    }

    // GOTTA COME BACK TO THIS
    pwmc->rgba.r = 0;
    pwmc->rgba.g = 0;
    pwmc->rgba.b = 0;
    pwmc->rgba.a = 0;

    SetBlotDtVisible(pwmc, 0.0);
    SetBlotFontScale(pwmc, 0.6);

    //pwmc->pvtblot->pfnSetBlotAchzDraw(pwmc, 0x29f208);

    pwmc->pwm = nullptr;
}

void OnWmcActive(WMC* pwmc, int fActive)
{

}

void UpdateWmcActive(WMC* pwmc, JOY* pjoy)
{

}

void DrawWmc(WMC* pwmc)
{

}

WMC g_wmc;