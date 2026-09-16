#pragma once
#include "act.h"

struct ACTBANK : public ACT
{
    float uBank;
    float dtPredict;
};

std::shared_ptr <ACTBANK> PactbankNew(SW* psw, ALO* palo, VTACTBANK* pvtactbank);
std::shared_ptr <ACTBANK> PactbankNewClone(ACTBANK* pactbankBase, SW* psw, ALO* palo);
void InitActbank(ACTBANK* pactbank, ALO* palo);
void CloneActbank(ACTBANK* pactbank, ACTBANK* pactbankBase);
void GetActbankRotationGoal(ACTBANK* pactbank, float dtOffset, glm::mat3* pmat, glm::vec3* pw);