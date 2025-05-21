#include "title.h"

void StartupTitle(TITLE* ptitle)
{
    ptitle->pvttitle = &g_vttitle;
}

void PostTitleLoad(TITLE* ptitle)
{
    PostBlotLoad(ptitle);
}

int FIncludeTitleForPeg(TITLE* ptitle, BLOT* pblotOther)
{
    return 0;
}

void SetTitleBlots(TITLE* ptitle, BLOTS blots)
{
    if ((ptitle->fReshow != 0) && (blots == BLOTS_Hidden)) {
        ptitle->fReshow = 0;
        SetBlotAchzDraw(ptitle, ptitle->pchzReshow);
        blots = BLOTS_Appearing;
    }
    SetBlotBlots((BLOT*)ptitle, blots);
}

void DrawTitle(TITLE* ptitle)
{

}

TITLE g_title;