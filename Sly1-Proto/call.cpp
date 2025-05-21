#include "call.h"

void StartupCall(CALL* pcall)
{
    pcall->pvtcall = &g_vtcall;
}

void PostCallLoad(CALL* pcall)
{
    PostBlotLoad(pcall);
}

void DrawCall(CALL* pcall)
{

}

CALL g_call;