#include "van.h"

void StartupVan(VAN* pvan)
{
    pvan->pvtvan = &g_vtvan;
}

VAN g_van;
int g_cCpuCollected;
