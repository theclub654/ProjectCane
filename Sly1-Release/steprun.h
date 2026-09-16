#pragma once
#include "step.h"
#include "blend.h"

void PostStepLoadRun(STEP* pstep, BLRUN* ablrun, ASEGBL** ppasegbl);
void CalculateBlrunV(int cblrun, const BLRUN* ablrun, const MRSG* pmrsgBlend, glm::vec3* pv);
void ProjectStepRunPlant(STEP* pstep, const BLRUN* ablrun, float uPlant, float rJt, int iblrunWalk, int iblrunRun, glm::vec3* pposPlant);
int  FIntersectStepRunPlant(STEP* pstep, const glm::vec3* pposUp, const glm::vec3* pposDown, float* pu);
int  FReblendAbl2D(int cbBl, int c1, int c2, BL* abl, float rUp, float rDown, float rJt);
void AdjustStepRun(STEP* pstep, BLRUN* ablrun, ASEGBL* pasegbl, ASEGA* pasega);
void SetStepRunTarget(STEP* pstep, float rad, float u, BLRUN* ablrun, ASEGBL* pasegbl);

extern int s_mpfootsoidLeft[2];
extern int s_mpfootsoidRight[2];
extern int s_mpimrsgccmrsgRun[2];
extern int s_mpimrsgcibRun[2];