#pragma once
#include "step.h"


void PostJtLoadPipe(JT* pjt);
void GetJtPipeGoalMat(JT* pjt, PIPE* ppipe, float s, glm::mat3* pmat);
void PresetJtAccelPipe(JT* pjt);
void UpdateJtActivePipe(JT* pjt, JOY* pjoy);
void UpdateJtInternalXpsPipe(JT* pjt);
void SetJtJtpdk(JT* pjt, int jtpdk);
void PlaceJtOnPipe(JT* pjt, PIPE* ppipe);

constexpr float S_JtPipeLock = 10.0f;
constexpr float DS_JtPipeNormalGap = 50.0f;
constexpr float DS_JtPipeNormalSearch = 25.0f;

static SMP s_smpPipeXp =
{
    1500.0f, // svFast
    0.0f,    // svSlow
    0.5f     // dtFast
};

constexpr float Z_JtPipeVerticalThreshold = 0.3f;
constexpr float DS_JtPipeWindow = 300.0f;
