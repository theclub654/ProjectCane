#pragma once
#include "po.h"

struct SXP 
{
    struct ALO* paloWheel;
    struct ALO* paloHub;
    struct PNT* ppntTop;
    int xsxp;
    glm::vec3 posCur;
    glm::vec3 posGround;
    glm::vec3 normalGround;
    glm::vec3 vGround;
    float swSpin;
    float radSpin;
    float cParticleDirt;
    float cParticleDust;
};
enum CYCLES 
{
    CYCLES_Nil = -1,
    CYCLES_Brake = 0,
    CYCLES_Accelerate = 1,
    CYCLES_Coast = 2,
    CYCLES_Max = 3
};

class CYCLE : public PO
{
	public:
        float sRadiusFrontWheel;
        float sRadiusRearWheel;
        float svMax;
        float radTarget;
        float svTarget;
        float radFront;
        float radBankTarget;
        int xsxp;
        SXP asxp[2];
        struct EXPL* pexplBoost;
        struct EXPL* pexplDirt;
        struct EXPL* pexplDust;
        float cParticleBoost;
        float cParticleDirt;
        float cParticleDust;
        float tBoost;
        int cBoost;
        float tGround;
        CYCLES cycles;
        float tCycles;
};

void InitCycle(CYCLE *pcycle);
void RenderCycleSelf(CYCLE* pcycle, CM* pcm, RO* pro);
void DeleteCycle(LO* plo);