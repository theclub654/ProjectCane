#pragma once
#include "lo.h"

struct FRM 
{
    int cframe;
    u32 grffont;
    int fBackgroundUploadRequired;
    DL dlBlipgPending;
    DL dlBlipPending;
    DL dlShadowPending;
    GRFZON grfzon;
};

void StartupFrame();
void OpenFrame();
void ClearAllPendingFrames();

