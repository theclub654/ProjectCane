#pragma once
#include "blot.h"

struct LOGO : public BLOT
{
    struct SHD* pshd;
};

void StartupLogo(LOGO* plogo);
void PostLogoLoad(LOGO* plogo);
void DrawLogo(LOGO* plogo);

extern LOGO g_logo;
extern CTextEdge g_teLogo;
extern const char *logoPchz;
extern float RX_LogoText;
extern float RY_LogoText;