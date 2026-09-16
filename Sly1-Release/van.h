#pragma once
#include "ctr.h"

struct VAN : public CTR
{
};

void StartupVan(VAN* pvan);

extern VAN g_van;
extern int g_cCpuCollected;
