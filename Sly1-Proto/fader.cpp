#include "fader.h"

void InitSwFaderDl(SW* psw)
{
	InitDl(&psw->dlFader, offsetof(FADER, dleFader));
}

void InitSwRealClockFader(SW* psw)
{
	InitDl(&psw->dlRealClockFader, offsetof(FADER, dleFader));
}
