#pragma once
#include "step.h"

enum TANKS 
{
    TANKS_Nil = -1,
    TANKS_Passive = 0,
    TANKS_Active = 1,
    TANKS_Zap = 2,
    TANKS_Dead = 3,
    TANKS_Peek = 4,
    TANKS_Max = 5
};

class TANK : public STEP
{
	public:
    TANKS tanks;
    float tTanks;
    int fFlash;
    float tFlash;
    float tCharm;
    float tCharmPending;
    int fCharmEnabled;
    int fAllowEject;
    struct XP* pxpZap;
    class ALO* paloJt;
    class ALO* paloGut;
    struct ACTADJ* pactadjGut;
    class ALO* paloHead;
    struct ACTADJ* pactadjHead;
    class SM* psm;
    class SMA* psma;
    class PNT* ppntAnchor;
};

TANK*NewTank();
void InitTank(TANK* ptank);
void SetTankTanks(TANK* ptank, TANKS tanks);
void*GetTankTanks(TANK* ptank);
void SetfAllowEject(TANK* ptank, int fAllowEject);
void*GetfAllowEject(TANK* ptank);
void PostTankLoad(TANK* ptank);
void UpdateTank(TANK* ptank, float dt);
void UpdateTankDriveTarget(TANK* ptank);
void UseTankCharm(TANK* ptank);
void UpdateTankActive(TANK* ptank, JOY* pjoy, float dt);
void OnTankActive(TANK* ptank, int fActive);
void RenderTankAll(TANK* ptank, CM* pcm, RO* pro);
void ImpactTank(TANK* ptank, int fParentDirty);
int  FInvulnerableTank(TANK* ptank, ZPK zpk);
void ApplyTankThrow(TANK* ptank, PO* ppo);
int FTakeTankDamage(TANK* ptank, ZPR* pzpr);
void AdjustTankXps(TANK* ptank);
void AdjustTankNewXp(TANK* ptank, XP* pxp, int ixpd);
int  GetTankSize();
void CloneTank(TANK* ptank, TANK* ptankBase);
void HandleTankMessage(TANK* ptank, MSGID msgid, void* pv);
JTHS JthsCurrentTank(TANK* ptank);
void DeleteTank(TANK* ptank);

extern CLQ s_clqDtDamageToUMu;
extern float R_TankGutPosSpring;
extern float R_TankGutRotSpring;
extern glm::vec3 R_TankImpact;
extern float VXY_GutDamage;
extern float VZ_GutDamage;
extern float SV_TankDamageDead;
extern float SW_GutDamage;
extern float MU_TankInternal;
extern SNIP s_asnipTank[9];
extern float g_fTankFireBlocked;
