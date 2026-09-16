#pragma once
#include "lo.h"

struct VTACT
{
    void  (*pfnInitAct)(ACT*, ALO*) = InitAct;
    void  (*pfnCloneAct)(ACT*, ACT*) = CloneAct;
    void  (*pfnRetractAct)(ACT*, GRFRA) = RetractAct;
    void  (*pfnUpdateAct)(ACT*, float) = nullptr;
    void  (*pfnGetActPositionGoal)(ACT*, float, glm::vec3*, glm::vec3*) = GetActPositionGoal;
    void  (*pfnGetActRotationGoal)(ACT*, float, glm::mat3*, glm::vec3*) = GetActRotationGoal;
    void  (*pfnGetActTwistGoal)(ACT*, float*, float*) = GetActTwistGoal;
    void  (*pfnGetActScale)(ACT*, glm::mat3*) = GetActScale;
    float (*pfnGetActPoseGoal)(ACT*, int) = GetActPoseGoal;
};

inline VTACT g_vtact;

struct VTACTVAL
{
    void  (*pfnInitActval)(ACTVAL*, ALO*) = InitActval;
    void  (*pfnCloneActval)(ACTVAL*, ACTVAL*) = CloneActval;
    void  (*pfnRetractAct)(ACT*, GRFRA) = RetractAct;
    void  (*pfnUpdateAct) = nullptr;
    void  (*pfnGetActvalPositionGoal)(ACTVAL*, float, glm::vec3*, glm::vec3*) = GetActvalPositionGoal;
    void  (*pfnGetActvalRotationGoal)(ACTVAL*, float, glm::mat3*, glm::vec3*) = GetActvalRotationGoal;
    void  (*pfnGetActvalTwistGoal)(ACTVAL*, float*, float*) = GetActvalTwistGoal;
    void  (*pfnGetActvalScale)(ACTVAL*, glm::mat3*) = GetActvalScale;
    float (*pfnGGetActvalPoseGoal)(ACTVAL*, int) = GetActvalPoseGoal;
};

inline VTACTVAL g_vtactval;

struct VTACTREF
{
    void  (*pfnInitActref)(ACTREF*, ALO*) = InitActref;
    void  (*pfnCloneActref)(ACTREF*, ACTREF*) = CloneActref;
    void  (*pfnRetractAct)(ACT*, GRFRA) = RetractAct;
    void  (*pfnUpdateAct) = nullptr;
    void  (*pfnGetActrefPositionGoal)(ACTREF*, float, glm::vec3*, glm::vec3*) = GetActrefPositionGoal;
    void  (*pfnGetActrefRotationGoal)(ACTREF*, float, glm::mat3*, glm::vec3*) = GetActrefRotationGoal;
    void  (*pfnGetActrefTwistGoal)(ACTREF*, float*, float*) = GetActrefTwistGoal;
    void  (*pfnGetActrefScale)(ACTREF*, glm::mat3*) = GetActrefScale;
    float (*pfnGGetActrefPoseGoal)(ACTREF*, int) = GGetActrefPoseGoal;
};

inline VTACTREF g_vtactref;

struct VTACTADJ
{
    void  (*pfnInitActadj)(ACTADJ*, ALO*) = InitActadj;
    void  (*pfnCloneActadj)(ACTADJ*, ACTADJ*) = CloneActadj;
    void  (*pfnRetractAct)(ACT*, GRFRA) = RetractAct;
    void  (*pfnUpdateAct)() = nullptr;
    void  (*pfnGetActadjPositionGoal)(ACTADJ*, float, glm::vec3*, glm::vec3*) = GetActadjPositionGoal;
    void  (*pfnGetActadjRotationGoal)(ACTADJ*, float, glm::mat3*, glm::vec3*) = GetActadjRotationGoal;
    void  (*pfnGetActadjTwistGoal)(ACTADJ*, float*, float*) = GetActadjTwistGoal;
    void  (*pfnGetActadjScale)(ACTADJ*, glm::mat3*) = GetActadjScale;
    float (*pfnGGetActadjPoseGoal)(ACTADJ*, int) = GGetActadjPoseGoal;
};
inline VTACTADJ g_vtactadj;


struct VTACTBANK
{
    void  (*pfnInitActbank)(ACTBANK*, ALO*) = InitActbank;
    void  (*pfnCloneActbank)(ACTBANK*, ACTBANK*) = CloneActbank;
    void  (*pfnRetractAct)(ACT*, GRFRA) = RetractAct;
    void  (*pfnUpdateAct) = nullptr;
    void  (*pfnGetActPositionGoal)(ACT*, float, glm::vec3*, glm::vec3*) = GetActPositionGoal;
    void  (*pfnGetActbankRotationGoal)(ACTBANK*, float, glm::mat3*, glm::vec3*) = GetActbankRotationGoal;
    void  (*pfnGetActTwistGoal)(ACT*, float*, float*) = GetActTwistGoal;
    void  (*pfnGetActScale)(ACT*, glm::mat3*) = GetActScale;
    float (*pfnGGetActPoseGoal)(ACT*, int) = GetActPoseGoal;
};
inline VTACTBANK g_vtactbank;

struct VTACTADD
{
    void  (*pfnInitAct)(ACT* , ALO*) = InitAct;
    void  (*pfnCloneAct)(ACTADD*, ACTADD*) = CloneActadd;
    void  (*pfnRetractAct) = nullptr;
    void  (*pfnUpdateAct) = nullptr;
    void  (*pfnGetActaddPositionGoal)(ACTADD*, float, glm::vec3*, glm::vec3*) = GetActaddPositionGoal;
    void  (*pfnGetActaddRotationGoal)(ACTADD*, float, glm::mat3*, glm::vec3*) = GetActaddRotationGoal;
    void  (*pfnGetActTwistGoal)(ACT*, float*, float*) = GetActTwistGoal;
    void  (*pfnGetActScale)(ACT*, glm::mat3*) = GetActScale;
    float (*pfnGGetActPoseGoal)(ACT*, int) = GetActPoseGoal;
};

inline VTACTADD g_vtactadd;

struct VTACTLA
{
    void  (*pfnInitActla)(ACTLA*, ALO*) = InitActla;
    void  (*pfnCloneActla)(ACTLA*, ACTLA*) = CloneActla;
    void  (*pfnRetractAct)(ACT*, GRFRA) = RetractAct;
    void  (*pfnUpdateActla)(ACTLA*, float) = UpdateActla;
    void  (*pfnGetActPositionGoal)(ACT*, float, glm::vec3*, glm::vec3*) = GetActPositionGoal;
    void  (*pfnGetActlaRotationGoal)(ACTLA*, float, glm::mat3*, glm::vec3*) = GetActlaRotationGoal;
    void  (*pfnGetActTwistGoal)(ACT*, float*, float*) = GetActTwistGoal;
    void  (*pfnGetActScale)(ACT*, glm::mat3*) = GetActScale;
    float (*pfnGetActPoseGoal)(ACT*, int) = GetActPoseGoal;
};

inline VTACTLA g_vtactla;

struct VTACTSEG
{
    void  (*pfnInitAct)(ACT*, ALO*) = InitAct;
    void  (*pfnCloneActseg)(ACTSEG*, ACTSEG*) = CloneActseg;
    void  (*pfnRetractActseg)(ACTSEG*, GRFRA) = RetractActseg;
    void  (*pfnUpdateAct) = nullptr;
    void  (*pfnGetActsegPositionGoal)(ACTSEG*, float, glm::vec3*, glm::vec3*) = GetActsegPositionGoal;
    void  (*pfnGetActsegRotationGoal)(ACTSEG*, float, glm::mat3*, glm::vec3*) = GetActsegRotationGoal;
    void  (*pfnGetActsegTwistGoal)(ACTSEG*, float*, float*) = GetActsegTwistGoal;
    void  (*pfnGetActsegScale)(ACTSEG*, glm::mat3*) = GetActsegScale;
    float (*pfnGGetActsegPoseGoal)(ACTSEG*, int) = GGetActsegPoseGoal;
};

inline VTACTSEG g_vtactseg;
