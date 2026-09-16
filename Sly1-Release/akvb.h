#pragma once
#include "ac.h"

void LoadAkvbFromBrx(int* pckvb, std::vector <KVB>& pakvb, CBinaryInputStream* pbis);
void EvaluateAkvb(int ckvb, KVB* akvb, float t, float svt, GRFEVAL grfeval, glm::vec3* pvec, glm::vec3* pdvec);
void GetAkvbTimes(int ckvb, KVB* akvb, int* pct, std::vector <float>& pat);