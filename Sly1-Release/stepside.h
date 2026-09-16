#pragma once
#include "step.h"

void PostJtLoadSidestep(JT* pjt, BLSS* ablss, ASEGBL** ppasegbl);
void AdjustJtSidestep(JT* pjt, BLSS* ablss, ASEGBL* pasegbl);
void SetJtSidestepTarget(JT* pjt, BLSS* ablss, ASEGBL* pasegbl, float rad, float u);