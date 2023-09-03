#pragma once

// This is resposnible for handling the global game state
enum PHASE {
    PHASE_Nil = -1,
    PHASE_None = 0,
    PHASE_Startup = 1,
    PHASE_Load = 2,
    PHASE_Main = 3,
    PHASE_Mpeg = 4,
    PHASE_Max = 5
};

static PHASE g_phase;
void SetPhase(PHASE phase); // This function sets the global stae of the game