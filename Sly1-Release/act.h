#pragma once
#include "actdec.h"

struct ACT
{
	union
	{
        struct VTACT *pvtact;
        struct VTACTVAL *pvtactval;
        struct VTACTREF *pvtactref;
        struct VTACTADJ *pvtactadj;
        struct VTACTBANK *pvtactbank;
        struct VTACTSEG *pvtactseg;
        struct VTACTLA *pvtactla;
        struct VTACTADD *pvtactadd;
	};

    struct ALO *palo;
    DLE dleAlo;
    ACK ackPos;
    ACK ackRot;
    ACK ackScale;
    ACK ackPose;
    int nPriority;
    float tMatch;
};

std::shared_ptr<ACT> PactNew(SW* psw, ALO* palo, VTACT* pvtact);
std::shared_ptr<ACT> PactNewClone(ACT* pactBase, SW* psw, ALO* palo);
void InitAct(ACT* pact, ALO* palo);
void CloneAct(ACT* pact, ACT* pactBase);
void RetractAct(ACT* pact, GRFRA grfra);
void GetActPositionGoal(ACT* pact, float dtOffset, glm::vec3* ppos, glm::vec3* pv);
void GetActRotationGoal(ACT* pact, float dtOffset, glm::mat3* pmat, glm::vec3* pw);
void GetActTwistGoal(ACT* pact, float* pradTwist, float* pdradTwist);
void GetActScale(ACT* pact, glm::mat3* pmat);
float GetActPoseGoal(ACT* pact, int ipose);
void CalculateActDefaultAck(ACT* pact);
void SnapAct(ACT* pact, int fForce);
void CalculateAloPositionSpring(ALO* palo, float dt, const glm::vec3* pposGoal, const glm::vec3* pvGoal, glm::vec3* pdv);
void ProjectActPosition(ACT* pact);
void CalculateAloRotationSpring(ALO* palo, float dt, glm::mat3* pmatGoal, glm::vec3* pwGoal, glm::vec3* pdw);
void ProjectActRotation(ACT* pact);
void ProjectActPose(ACT* pact, int ipose);
void PredictAloPosition(ALO* palo, float dtOffset, glm::vec3* ppos, glm::vec3* pv);
void PredictAloRotation(ALO* palo, float dtOffset, glm::mat3* pmat, glm::vec3* pw);
void AdaptAct(ACT* pact);

extern SMP s_smpPose;