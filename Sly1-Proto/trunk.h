#pragma once
#include "ctr.h"

struct TRUNKCTR : public CTR
{

};

void StartupTrunkCtr(TRUNKCTR* ptrunkctr);
float DtVisibleTrunkctr(TRUNKCTR* ptrunkctr);

extern TRUNKCTR g_trunkctr;