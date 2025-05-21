#include "debugmenu.h"

void StartupDebugMenu(DEBUGMENU* pdebugmenu)
{
	pdebugmenu->pvtdebugmenu = &g_vtdebugmenu;
}

void OnDebugmenuActive(DEBUGMENU* pdebugmenu, int fActive)
{

}

void UpdateDebugmenuActive(DEBUGMENU* pdebugmenu, JOY* pjoy)
{

}

void OnDebugmenuReset(DEBUGMENU* pdebugmenu)
{

}

void DrawDebugMenu(DEBUGMENU* pdebugmenu)
{

}

DEBUGMENU g_debugmenu;