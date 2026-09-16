#pragma once
#include "act.h"

struct ACTADJ : public ACT
{
    glm::vec3 dposGoal;
    glm::vec3 dvGoal;
    glm::mat3 dmatGoal;
    glm::vec3 dwGoal;
    float dradTwistGoal;
    float ddradTwistGoal;
    glm::mat3 dmatScale;
};

ACTADJ* PactadjNew(SW* psw, ALO* palo);
ACTADJ* PactadjNewClone(ACTADJ* pactadjBase, SW* psw, ALO* palo);
void  InitActadj(ACTADJ* pactadj, ALO* palo);
void  CloneActadj(ACTADJ* pactadj, ACTADJ* pactadjBase);
void  GetActadjPositionGoal(ACTADJ* pactadj, float dtOffset, glm::vec3* ppos, glm::vec3* pv);
void  GetActadjRotationGoal(ACTADJ* pactadj, float dtOffset, glm::mat3* pmat, glm::vec3* pw);
void  GetActadjTwistGoal(ACTADJ* pactadj, float* pradTwist, float* pdradTwist);
void  GetActadjScale(ACTADJ* pactadj, glm::mat3* pmat);
float GGetActadjPoseGoal(ACTADJ* pactadj, int ipose);