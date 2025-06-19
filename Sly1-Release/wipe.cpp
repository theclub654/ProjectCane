#include "wipe.h"

void InitWipe(WIPE* pwipe)
{
	pwipe->wipes = WIPES_Nil;
	//SetWipeWipes(pwipe, WIPES_Idle);
}

WIPE g_wipe;