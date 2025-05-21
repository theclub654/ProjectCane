#include "attract.h"

void StartupAttract(ATTRACT* pattract)
{
    pattract->pvtattract = &g_vtattract;
}

void PostAttractLoad(ATTRACT* pattract)
{
    PostBlotLoad(pattract);

    //pattract->pfont = std::shared_ptr <CFont>(pattract->pfont->PFontClone(0.69999999, 0.6));

    if (g_pfontScreenCounters != nullptr)
    {
        pattract->pte = &g_teAttract;
        g_teAttract.m_pfont = g_pfontScreenCounters;
    }

    //pattract->fJoyValid = g_joy.joys;

    /*if (pattract->fJoyValid != 0) {
        iVar4 = 0;
    }

    pattract->pvtblot->pfnSetBlotAchzDraw(g_aachzAttract);*/
}

void SetAttractAchzDraw(ATTRACT* pattract, char* pchz)
{

}

void SetAttractBlots(ATTRACT* pattract, BLOTS blots)
{

}

void UpdateAttract(ATTRACT* pattract)
{

}

void DrawAttract(ATTRACT* pattract)
{

}

ATTRACT g_attract;