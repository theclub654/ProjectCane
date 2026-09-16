#pragma once
#include "acg.h"

void LoadApacgFromBrx(std::shared_ptr<ACG> apacg[3], glm::vec3& pvecDefault, CBinaryInputStream* pbis);
void EvaluateApacg(std::shared_ptr<ACG> apacg[3], ALO* palo, float t, float svt, GRFEVAL grfeval, const glm::vec3* pvecDefault, glm::vec3* pvec, glm::vec3* pdvec);
void GetApacgTimes(std::shared_ptr<ACG> apacg[3], int* pct, std::vector<float> &pat);