#include "wm.h"

void* NewWm()
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

}

void DeleteWm(LO* plo)
{
	delete(WM*)plo;
}
