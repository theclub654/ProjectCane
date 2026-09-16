#pragma once
#include "act.h"

struct ACTLA : public ACT
{
    int fUseTarget;
    class ALO* paloTarget;
    glm::vec3 posTargetLocal;
    float sIgnore;
    int nPriorityEnabled;
    int nPriorityDisabled;
    int fPaused;
    CLQ clqPanToPan;
    LM lmPan;
    CLQ clqTiltToTilt;
    LM lmTilt;
};

std::shared_ptr<ACTLA> PactlaNew(SW* psw, ALO* palo, VTACTLA* pvtact);
std::shared_ptr<ACTLA> PactlaNewClone(ACT* pactBase, SW* psw, ALO* palo);
void InitActla(ACTLA* pactla, ALO* palo);
void CloneActla(ACTLA* pactla, ACTLA* pactlaBase);
void UpdateActla(ACTLA* pactla, float dt);
ALO* PaloGetActlaTarget(ACTLA* pactla);
void GetActlaRotationGoal(ACTLA* pactla, float dtOffset, glm::mat3* pmat, glm::vec3* pw);
void SetActlaTarget(ACTLA* pactla, ALO* paloTarget, glm::vec3* pposLocal);