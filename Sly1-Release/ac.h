#pragma once

struct ACP;
struct ACR;
struct ACS;
struct ACG;

#include "lo.h"
#include "bez.h"

enum ACVK
{
    ACVK_Nil = -1,
    ACVK_Component = 0,
    ACVK_Bezier = 1,
    ACVK_Blend = 2,
    ACVK_Max = 3
};
enum ACGK
{
    ACGK_Nil = -1,
    ACGK_Bezier = 0,
    ACGK_BezierWeighted = 1,
    ACGK_Linear = 2,
    ACGK_BlendTwist = 3,
    ACGK_BlendPose = 4,
    ACGK_Max = 5
};
enum KGBTK
{
    KGBTK_Nil = -1,
    KGBTK_Global = 0,
    KGBTK_Fixed = 1,
    KGBTK_Linear = 2,
    KGBTK_Flat = 3,
    KGBTK_Smooth = 4,
    KGBTK_Step = 5,
    KGBTK_Slow = 6,
    KGBTK_Fast = 7,
    KGBTK_Clamped = 8,
    KGBTK_Max = 9
};

struct KVB
{
    float t;
    glm::vec3 vec;
    glm::vec3 dvecIn;
    glm::vec3 dvecOut;
};

struct MRSG 
{
    float t;
    float dt;
};

struct MRSGC
{
    int ibMrsg;
    int cmrsg;
    std::vector <MRSG> amrsg;
};

struct KGBT
{
    KGBTK kgbtk;
    float gSlope;
};

struct KGB
{
    float t;
    float g;
    KGBT kgbtIn;
    KGBT kgbtOut;
};

struct KGBWT
{
    KGBTK kgbtk;
    float dt;
    float g;
};

struct KGBW
{
    float t;
    float g;
    KGBWT kgbwtIn;
    KGBWT kgbwtOut;
};

struct KGL
{
    float t;
    float g;
};

void FindKey(float t, GRFEVAL grfeval, int cbKey, int ckey, char* abKey, float* pdt, float* pdtSeg, void** ppv);
void EvaluateAposG(float g, int cpos, glm::vec3* apos, float* mpiposg, int fClosed, glm::vec3* ppos, glm::vec3* pnormTangent);
