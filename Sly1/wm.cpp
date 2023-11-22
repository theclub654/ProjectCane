#include "wm.h"

void* NewWm()
{
	return new WM;
}

void RenderWmAll(WM* pwm, CM* pcm, RO* pro)
{

}

void DeleteWm(LO* plo)
{
	delete(WM*)plo;
}
