#include "trunk.h"

void StartupTrunkCtr(TRUNKCTR* ptrunkctr)
{
	ptrunkctr->pvttrunkctr = &g_vttrunkctr;
}

float DtVisibleTrunkctr(TRUNKCTR* ptrunkctr)
{
	return 0.0f;
}

TRUNKCTR g_trunkctr;