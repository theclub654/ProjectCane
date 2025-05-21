#pragma once
#include "blot.h"

struct LETTERBOX : public BLOT
{

};

void StartupLetterbox(LETTERBOX *pletterbox);
void DrawLetterBox(LETTERBOX* pletterbox);

extern LETTERBOX g_letterbox;