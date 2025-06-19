#pragma once
#include "ctr.h"

struct PUFFCHARGERCTR : public CTR
{

};

void StartupPuffChargerCtr(PUFFCHARGERCTR *ppuffchargerctr);
float DtVisiblePuffchargerctr(PUFFCHARGERCTR* ppuffchargectr);

extern PUFFCHARGERCTR g_puffchargectr;