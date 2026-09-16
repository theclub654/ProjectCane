#pragma once
#include "act.h"

struct ACTVAL : public ACT
{
    glm::vec3 posGoal;
    glm::vec3 vGoal;
    glm::mat3 matGoal;
    glm::vec3 wGoal;
    float radTwistGoal;
    float dradTwistGoal;
    glm::mat3 matScale;
    int cgPoses;
    float *agPoses;
};

std::shared_ptr <ACTVAL> PactvalNew(SW* psw, ALO* palo);
void InitActval(ACTVAL* pactval, ALO* palo);
void CloneActval(ACTVAL* pactval, ACTVAL* pactvalBase);
void GetActvalPositionGoal(ACTVAL* pactval, float dtOffset, glm::vec3* ppos, glm::vec3* pv);
void GetActvalRotationGoal(ACTVAL* pactval, float dtOffset, glm::mat3* pmat, glm::vec3* pw);
void GetActvalTwistGoal(ACTVAL* pactval, float* pradTwist, float* pdradTwist);
void GetActvalScale(ACTVAL* pactval, glm::mat3* pmat);
float GetActvalPoseGoal(ACTVAL* pactval, int ipose);