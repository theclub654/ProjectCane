#pragma once
#include "blot.h"

struct CALL : public BLOT
{

};

void StartupCall(CALL* pcall);
void PostCallLoad(CALL* pcall);
void DrawCall(CALL* pcall);

extern CALL g_call;