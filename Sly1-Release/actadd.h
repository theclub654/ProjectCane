#pragma once
#include "act.h"

struct ACTADD : public ACT
{
    int cpactAdd;
};

std::shared_ptr <ACTADD> PactaddNew(SW* psw, ALO* palo);
void CloneActadd(ACTADD* pactadd, ACTADD* pactaddBase);
void GetActaddPositionGoal(ACTADD* pactadd, float dtOffset, glm::vec3* ppos, glm::vec3* pv);
void GetActaddRotationGoal(ACTADD* pactadd, float dtOffset, glm::mat3* pmat, glm::vec3* pw);