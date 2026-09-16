#pragma once
#include "blot.h"

struct CALL : public BLOT
{
	class DIALOG* pdialogTriggered;
};

void StartupCall(CALL* pcall);
void PostCallLoad(CALL* pcall);
void UpdateCall(CALL* pcall);
void DrawCall(CALL* pcall);

extern CALL g_call;