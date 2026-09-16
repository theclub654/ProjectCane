#pragma once
#include "act.h"

struct ACTREF : public ACT
{
    glm::vec3* pposGoal;
    glm::vec3* pvGoal;
    glm::mat3* pmatGoal;
    glm::vec3* pwGoal;
    float* pradTwistGoal;
    float* pdradTwistGoal;
    glm::mat3* pmatScale;
    int* pcgPoses;
    float* agPoses;
};

std::shared_ptr <ACTREF> PactrefNew(SW* psw, ALO* palo);
std::shared_ptr <ACTREF> PactNewClone(ACTREF* pactrefBase, SW* psw, ALO* palo);
void  InitActref(ACTREF* pactref, ALO* palo);
void  CloneActref(ACTREF* pactref, ACTREF* pactrefBase);
void  GetActrefPositionGoal(ACTREF* pactref, float dtOffset, glm::vec3* ppos, glm::vec3* pv);
void  GetActrefRotationGoal(ACTREF* pactref, float dtOffset, glm::mat3* pmat, glm::vec3* pw);
void  GetActrefTwistGoal(ACTREF* pactref, float* pradTwist, float* pdradTwist);
void  GetActrefScale(ACTREF* pactref, glm::mat3* pmat);
float GGetActrefPoseGoal(ACTREF* pactref, int ipose);