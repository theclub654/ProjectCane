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
	LO lo = *pwm;
	*pwm = *pwmBase;
	memcpy(pwm, &lo, sizeof(LO));

	CloneLo(pwm, pwmBase);

	ClearDl(&pwm->dlChild);
}

void RenderWmAll(WM* pwm, CM* pcm, RO* pro)
{
	RenderAloAll(pwm, pcm, pro);
}

void DeleteWm(LO* plo)
{
	delete(WM*)plo;
}
