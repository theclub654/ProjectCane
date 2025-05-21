#include "logo.h"

void StartupLogo(LOGO* plogo)
{
    plogo->pvtlogo = &g_vtlogo;
}

void PostLogoLoad(LOGO* plogo)
{
    PostBlotLoad(plogo);

    if (g_pfontScreenCounters != nullptr)
    {
        plogo->pte = &g_teLogo;
        g_teLogo.m_pfont = g_pfontScreenCounters;
    }

    plogo->pshd = PshdFindShader(OID_shd_attract_logo);

    /*if (plogo->pshd == nullptr)
        plogo->pvtblot->pfnSetBlotAchzDraw(plogo, 0x29d990);
    else
        ResizeBlot(plogo, 366.75, 165.75);*/
}

void DrawLogo(LOGO* plogo)
{

}

LOGO g_logo;