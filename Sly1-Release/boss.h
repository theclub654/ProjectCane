#pragma once
#include "blot.h"

struct BOSS : public BLOT
{
	int cPhaseMax;
	int cHealthPerPhase;
	int cHealthTotal;
	int cPhasesRemaining;
	int cHealthCurrent;

	float uHealthDisplay;
	float uHealthTarget;
	float tHealthChanged;

	float xHealthBar;
	float yHealthBar;
	float dxHealthBar;
	float dyHealthBar;

	float xPhaseCount;
	float yPhaseCount;
};

void StartupBoss(BOSS* pboss);
void PostBossLoad(BOSS* pboss);
void DecrementBossHealth(BOSS* pboss);
void DrawBoss(BOSS* pboss);

extern BOSS g_boss;
extern SMP s_smpBossctrSlide;
extern SMP BossHealthDisplay;