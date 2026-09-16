#pragma once
#include "po.h"

struct CTR;
class BLIPG;

enum MGVS { MGVS_Nil=-1, MGVS_Inactive=0, MGVS_Active=1, MGVS_Charm=2,
            MGVS_Dead=3, MGVS_TryAgain=4, MGVS_Max=5 };

class MGV : public PO
{
    public:
    MGVS mgvs;
    float tMgvs;
    ASEG* pasegInactive;
    ASEG* pasegActive;
    ASEG* pasegCharm;
    ASEG *pasegDead;
    ASEGA* pasegaCur;
    BLIPG* pblipgController;
    float gAimControllerX;
    float gAimControllerY;
    int fCharmUsed;
    float tCharmUsed;
    float tCharmState;
    float radMove, radAim;
    glm::vec3 vMove;
    SO* psoAimTarget;
    SM* psmAim;
    SMA* psmaAim;
    float uMove, uAim;
    ASEG* pasegImpact;
    ASEG* pasegDamage;
    ASEGA* pasegaDamage;
    int fDamagePending;
    SO* psoDeathSource;
    LO* ploSpecialContact;
    float sRadiusSelfCached;
    int cSecurityCodes;
    int clives;
    int cpntRespawn;
    PNT* appntRespawn[32];
    int cploSpawned;
    LO* aploSpawned[16];
};

MGV* NewMgv();
void InitMgv(MGV* pmgv);
int  GetMgvSize();
void LoadMgvFromBrx(MGV* pmgv, CBinaryInputStream* pbis);
void CloneMgv(MGV* pmgv, MGV* pmgvBase);
void PostMgvLoad(MGV* pmgv);
void UpdateMgv(MGV* pmgv, float dt);
void PresetMgvAccel(MGV* pmgv, float dt);
void UpdateMgvBlot(MGV* pmgv);
void UpdateMgvActive(MGV* pmgv, JOY* pjoy, float dt);
void AdjustMgvNewXp(MGV* pmgv, XP* pxp, int ixpd);
void AdjustMgvXpVelocity(MGV* pmgv, XP* pxp, int ixpd);
int FIsMgvInvulnerable(MGV* pmgv, ZPK zpk);
int FTakeMgvDamage(MGV* pmgv, ZPR* pzpr);
bool FIsMgvTakingDamage(MGV* pmgv);
void UseMgvCharm(MGV* pmgv);
void SetMgvMgvs(MGV* pmgv, MGVS mgvs);
void ApplyMgvDeathThrow(MGV* pmgv, LO* ploSource);
void AddMgvRespawnPoint(MGV* pmgv, PNT* ppnt);
void RespawnMgvObject(MGV* pmgv, LO* plo);
void DeleteMgv(MGV* pmgv);

void StartupSecurityCodeCtr(CTR* pctr);
void StartupMgvHealthCtr(CTR* pctr);

extern CTR g_securitycodectr;
extern CTR g_mgvhealthctr;

extern SNIP s_asnipMgv[12];
