#include "boss.h"

void StartupBossCtr(BOSSCTR* bossctr)
{
	bossctr->pvtbossctr = &g_vtbossctr;
}

void DrawBossCtr(BOSSCTR* pbossctr)
{

}

BOSSCTR g_bossctr;