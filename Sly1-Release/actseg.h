#pragma once
#include "act.h"

struct ACTSEG : public ACT
{
    struct ASEGA* pasega;
    DLE dleAsega;
    int ichn;
};

ACTSEG*PactsegNew(SW* psw, ALO* palo);
ACTSEG*PactsegNewClone(ACTSEG* pactsegBase, SW* psw, ALO* palo);
void  CloneActseg(ACTSEG* pactseg, ACTSEG* pactsegBase);
void  RetractActseg(ACTSEG* pactseg, GRFRA grfra); 
void  GetActsegPositionGoal(ACTSEG* pactseg, float dtOffset, glm::vec3* ppos, glm::vec3* pv);
void  GetActsegRotationGoal(ACTSEG* pactseg, float dtOffset, glm::mat3* pmat, glm::vec3* pw);
void  GetActsegTwistGoal(ACTSEG* pactseg, float* pradTwist, float* pdradTwist);
void  GetActsegScale(ACTSEG* pactseg, glm::mat3* pmat);
float GGetActsegPoseGoal(ACTSEG* pactseg, int ipose);