#pragma once
#include "brx.h"
#include "bis.h"
#include "phasemem.h"
#include "sw.h"

typedef int GRFTRANS;

enum FTRANS {
    GRFTRANS_None = 0,
    FTRANS_Checkpoint = 1,
    FTRANS_RetryWorld = 2,
    FTRANS_ClearGame = 4,
    FTRANS_SameWorld = 8,
    FTRANS_ShowLives = 16
};

class CTransition
{
protected:
    char* m_pchzWorld;      // This is the ptr to level sector offset and size in memory
    OID   m_oidWarp;        // Which checkpoint you spawn at when you start a level
    OID   m_oidWarpContext;

public:
    GRFTRANS grftrans;       // Flags that affect level loading, one of them checks if you lost all lives when you die.
    char m_achzWorldCur[64]; // File description contents.
    int  m_fPending = 0;     // Basically like a level pending flag.

    // Reloads the current level. The value of FTRANS affects the load.
    void ResetWorld(FTRANS ftrans);

    // Sets the conditions on the level if you died or loading a level
    void Set(char* pchzWorld, OID oidWarp, OID oidWarpContext, GRFTRANS grftrans);

    // Executes the conditions from CTransition::Set by setting some engine vaules to default and loading the level file
    void Execute();
};

static CTransition g_transition;