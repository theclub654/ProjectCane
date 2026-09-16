#pragma once
#include "asegbl.h"

void VerifyAeaEquivalence(int cea0, const EA* aea0, int cea1, const EA* aea1);
bool CopyBlendBytes(std::vector<std::byte>& destination, const void* source, int cbBl, int cbl);
void EnsureAsegBlendDynamic(ALO* palo, int cbBl, int cbl, void* abl, int cmrsgc, const int* mpimrsgcib, const int* mpimrsgccmrsg, ASEGBL** ppasegbl);
void FillBlAmrsg(int cbBl, int cbl, void* abl, int ibMrsg, int coid, const OID* aoid);
void CalculateBlendAmrsg(int cbBl, int cbl, void* abl, int ibMrsg, int cmrsg, MRSG* amrsg, float* ptMax);
int  ImrsgLookUp(float t, float tMax, int cmrsg, const MRSG* amrsg, float* pdtBest);
void ReblendAsegbl(ASEGBL* pasegbl, int cbBl, int cbl, const void* abl);
void SetAsegblSlerp(ASEGBL* pasegbl, ALO* palo);