#pragma once
#include "blot.h"

struct DEBUGMENU : public BLOT
{

};

void StartupDebugMenu(DEBUGMENU* pdebugmenu);
void OnDebugmenuActive(DEBUGMENU* pdebugmenu, int fActive);
void UpdateDebugmenuActive(DEBUGMENU* pdebugmenu, JOY* pjoy);
void OnDebugmenuReset(DEBUGMENU* pdebugmenu);
void DrawDebugMenu(DEBUGMENU* pdebugmenu);

extern DEBUGMENU g_debugmenu;