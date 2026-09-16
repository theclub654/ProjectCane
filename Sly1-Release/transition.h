#pragma once
#include "brx.h"
#include "phasemem.h"
#include <filesystem>

enum FTRANS 
{
    GRFTRANS_None = 0,
    FTRANS_Checkpoint = 1,
    FTRANS_RetryWorld = 2,
    FTRANS_ClearGame = 4,
    FTRANS_SameWorld = 8,
    FTRANS_ShowLives = 16
};
struct TRANS
{
    int fSet;
    LEVELINFO* plevel;
    OID oidWarp = OID_Nil;
    OID oidWarpContext = OID_Nil;
    GRFTRANS grftrans;
};

class CTransition
{
    public:
    LEVELINFO* m_plevelCurrent = nullptr;
    LEVELINFO* m_plevelPending = nullptr;

    std::string m_worldCurrent;
    std::string m_worldPending;

    OID m_oidWarp = OID_Nil;
    OID m_oidWarpContext = OID_Nil;
    GRFTRANS m_grftrans = static_cast<GRFTRANS>(0);

    int m_fPending = 0;

    // Reloads the current level. The value of FTRANS affects the load.
    void ResetWorld(int nParam);

    // Sets the conditions on the level if you died or loading a level
    void Set(LEVELINFO* plevel, OID oidWarp, OID oidWarpContext, GRFTRANS grftrans);

    // Executes the conditions from CTransition::Set by setting some engine vaules to default and loading the level file
    void Execute(std::string &file);
};

extern CTransition g_transition;