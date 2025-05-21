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