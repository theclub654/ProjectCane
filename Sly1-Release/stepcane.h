#pragma once
#include "step.h"

void SetJtJtcs(JT* pjt, int jtcs);
void UpdateJtCane(JT* pjt);
void UpdateJtActiveCane(JT* pjt, JOY* pjoy);
void ChooseJtAttackTarget(JT* pjt, GRFTAK grftak, glm::vec3* pposNaturalLocal, float dtProj, float dradMax, TARGET** pptarget, glm::vec3* pdposProj);
void ChooseJtSweepTarget(JT* pjt, BL* ablSweep, ASEGBL* pasegbl);
void ChooseJtRushTarget(JT* pjt);
void ChooseJtSmashTarget(JT* pjt);

static float DS_JtAttackPoBonus = -250;
inline float g_uSweepFixed = -1.0f;

static glm::vec3 s_posSweep(175.0f, 0.0f, 0.0f);
static glm::vec3 s_posRush(250.0f, 0.0f, 125.0f);
static glm::vec3 s_posSmash(125.0f, 0.0f, 0.0f);

inline int g_fForceFore = 0;
inline int g_fForceBack = 0;

constexpr float DT_JtSweepForce = 0.06666667f;       // 1 / 15 second
constexpr float DT_JtSweepForeContact = 0.16666667f; // 1 / 6 second
constexpr float DT_JtSweepBackContact = 0.11666667f; // 7 / 60 second
static int s_mjtcsjtcsSweepTransition[12] =
{
    -1, // 0
    -1, // 1
    -1, // 2
    -1, // 3
     6, // 4 -> StandingSweepFore
     7, // 5 -> StandingSweepBack
     4, // 6 -> SweepFore
     5, // 7 -> SweepBack
    -1, // 8
    -1, // 9
    -1, // 10
    -1  // 11
};
