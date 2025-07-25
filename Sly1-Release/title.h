#pragma once
#include "blot.h"

struct TITLE : public BLOT
{
    int fReshow;
    char *pchzReshow;
};

void StartupTitle(TITLE* ptitle);
void PostTitleLoad(TITLE* ptitle);
int  FIncludeTitleForPeg(TITLE* ptitle, BLOT* pblotOther);
void SetTitleAchzDraw(TITLE* ptitle, char* pchz);
void SetTitleBlots(TITLE* ptitle, BLOTS blots);
void DrawTitle(TITLE* ptitle);

extern TITLE g_title;
extern CTextEdge g_teTitle;