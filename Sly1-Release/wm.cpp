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
