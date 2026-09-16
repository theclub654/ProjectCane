#pragma once
#include "bomb.h"

class MINE : public BOMB
{
	public:
    int fSearch;
    float tSearch;
};

MINE*NewMine();
void CloneMine(MINE* pmine, MINE* pmineBase);
void DeployJtMine(JT* pjt);
bool FIgnoreMineIntersection(MINE* pmine, SO* pso);
void PresetMineAccel(MINE* pmine, float dt);
void AdjustMineNewXp(MINE* pmine, XP* pxp, int ixpd);
void HandleMineMessage(MINE* pmine, int msgid, void *pv);
void DeleteMine(MINE* pmine);

constexpr float R_MineAngularTarget = 10.0f; // 0x275188
constexpr float R_MinePositionTarget = 50.0f; // 0x27518C

const CLQ s_clqMineRotationSpring =
{
    0.0f,
    45.0f,
    0.0f
}; // 0x275190

const CLQ s_clqMineAngularDamping =
{
    0.0f,
    2.5f,
    0.0f
}; // 0x2751A0

const CLQ s_clqMinePositionSpring =
{
    0.0f,
    7.5f,
    0.0f
}; // 0x2751B0

const CLQ s_clqMineSearchBlend =
{
    0.0f,
    4.0f,
    0.0f
}; // 0x2751C0

const CLQ s_clqMineRotationDisabled =
{
    0.0f,
    0.0f,
    0.0f
}; // 0x275C40