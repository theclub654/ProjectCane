#pragma once
#include "transition.h"
#include "game.h"

enum WIPES
{
    WIPES_Nil = -1,
    WIPES_Idle = 0,
    WIPES_WipingOut = 1,
    WIPES_Black = 2,
    WIPES_WipingIn = 3,
    WIPES_Max = 4
};
enum WIPEK
{
    WIPEK_Nil = -1,
    WIPEK_Fade = 0,
    WIPEK_Keyhole = 1,
    WIPEK_WorldMap = 2,
    WIPEK_Frozen = 3,
    WIPEK_Max = 4
};

struct WIPE
{
    WIPES wipes;
    float tWipes;
    float uBlack;
    TRANS trans;
    WIPEK wipek;
    TRANS transButton;
    WIPEK wipekButton;
};

void InitWipe(WIPE* pwipe);
void UpdateWipe(WIPE* pwipe, JOY* pjoy);
void DrawWipe(WIPE* pwipe);
void ActivateWipe(WIPE* pwipe, TRANS* ptrans, WIPEK wipek);
void SetWipeButtonTrans(WIPE* pwipe, TRANS* ptrans, WIPEK wipek);
int  FCatchWipeButtonTrans(WIPE* pwipe, JOY* pjoy, WIPES wipesNew);
void SetWipeWipes(WIPE* pwipe, WIPES wipes);
void WipeToWorldWarp(LEVELINFO* plevel, OID oidWarp, WIPEK wipek);
void WipeToWorldWarp(const std::string& levelName, OID oidWarp, WIPEK wipek);
void WipeToWorldWarp(const char* pchzLevelName, OID oidWarp, WIPEK wipek);
//GOTTA COME BACK TO THIS
void FadeFramesToBlack(float dt);

extern WIPE g_wipe;
extern GAMEWORLD g_gameWorldPrev;