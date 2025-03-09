#include "phasemem.h"

void SetPhase(PHASE phase)
{
    if (g_phase != phase) {
        g_phase = phase;
    }
}

void ClearPhase(PHASE phase)
{
    g_phase = PHASE::PHASE_None;
}