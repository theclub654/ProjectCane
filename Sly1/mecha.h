#pragma once
#include "step.h"

enum MECHAS 
{
    MECHAS_Nil = -1,
    MECHAS_Passive = 0,
    MECHAS_Stand = 1,
    MECHAS_Run = 2,
    MECHAS_Jump = 3,
    MECHAS_Fall = 4,
    MECHAS_Zap = 5,
    MECHAS_Dead = 6,
    MECHAS_Peek = 7,
    MECHAS_Kick = 8,
    MECHAS_Max = 9
};

class MECHA : public STEP
{
	public:
        MECHAS mechas;
        float tMechas;
        struct ASEGA* pasegaCur;
        struct SMA* psmaCur;
        int fRetractAsega;
        struct SM* psmIdle;
        BLRUN ablrunRun[6];
        struct ASEGBL* pasegblRun;
        SFF sffRun;
        struct ASEG* pasegKick0;
        struct ASEG* pasegKick1;
        struct SO* psoKickEffect;
        struct XP* pxpEffect;
        int cpaloIgnoreEffect;
        struct ALO* apaloIgnoreEffect[16];
        struct SM* psmGun;
        struct SMA* psmaGun;
        struct ASEG* pasegJump;
        glm::vec3 vTargetCoast;
};

MECHA*NewMecha();
void InitMecha(MECHA* pmecha);
int  GetMechaSize();
void CloneMecha(MECHA* pmecha, MECHA* pmechaBase);
void RenderMechaSelf(MECHA* pmecha, CM* pcm, RO* pro);
void DeleteMecha(LO* plo);