#include "puffcharger.h"

void StartupPuffChargerCtr(PUFFCHARGERCTR* ppuffchargerctr)
{
	ppuffchargerctr->pvtpuffchargectr = &g_vtpuffchargectr;
}

float DtVisiblePuffchargerctr(PUFFCHARGERCTR* ppuffchargectr)
{
	return 0.0f;
}

PUFFCHARGERCTR g_puffchargectr;