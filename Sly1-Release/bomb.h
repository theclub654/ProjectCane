#pragma once
#include "so.h"

enum BDT 
{
	BDT_Nil = -1,
	BDT_Impact = 0,
	BDT_Timed = 1,
	BDT_PropImpact = 2,
	BDT_Manual = 3,
	BDT_Player = 4,
	BDT_Max = 5
};

struct BOMBCUSTOMXP
{
	glm::vec3 pos;
	glm::vec3 normal;
	float gDist;
	SO* psoOther;
};

enum BDM
{
	BDM_SurfaceNormal = 0,
	BDM_Velocity = 1,
	BDM_LocalMatrix = 2
};

struct BombWaterEnteredMessage
{
	SO* pso;
	BOMB* pbomb;
};

class BOMB : public SO
{
	public:
	BDT bdt;
	float tDetonate;
	float sDetonateRadius;
	float sDetonateRadiusJt;
	int fTouchDamage;
	float tDelay;
	int iDetonationProp;
	float sdvMax;
	LM lmMass;
	float mMax;
	int cpbrkp;
	struct BRKP* apbrkp[16];
	struct EXPL* pexpl;
	float rExplodeScale;
	CLQ clqRadiusToSv;
	LM lmSv;
	CLQ clqRadiusToSw;
	LM lmSw;
	glm::vec3 posDetonate;
	glm::vec3 normalDetonate;
	int fExplodeEffects;
	int fReclaim;
	ZPK zpk;
	std::shared_ptr <SFX> psfxDet;
	int fDetonatePrepared;
	glm::mat3 matDetonate;
	glm::vec3 vDetonate;       
	glm::vec3 wDetonate;        
	SO* psoImpact;
	BDM bdm;
	float svDetonate;
	int fCustomXps;
	int cbombXp;
	std::vector <BOMBCUSTOMXP> abombXp;
};

BOMB*NewBomb();
void InitBomb(BOMB* pbomb);
void SetBombSDetonateRadius(BOMB* pbomb, float sDetonateRadius);
void*GetBombSDetonateRadius(BOMB* pbomb);
void SetBombSDetonateRadiusJt(BOMB* pbomb, float sDetonateRadiusJt);
void*GetBombSDetonateRadiusJt(BOMB* pbomb);
void SetBombBdt(BOMB* pbomb, BDT bdt);
void*GetBombBdt(BOMB* pbomb);
void SetBombBdm(BOMB* pbomb, BDM bdm);
void*GetBombBdm(BOMB* pbomb);
void SetBombFTouchDamage(BOMB* pbomb, int fTouchDamage);
void*GetBombFTouchDamage(BOMB* pbomb);
void SetBombSdvMax(BOMB* pbomb, float sdvMax);
void*GetBombSdvMax(BOMB* pbomb);
void SetBombMMax(BOMB* pbomb, float mMax);
void*GetBombMMax(BOMB* pbomb);
void SetBombLmMass(BOMB* pbomb, LM lmMass);
void*GetBombLmMass(BOMB* pbomb);
void SetBombRExplodeScale(BOMB* pbomb, float rExplodeScale);
void*GetBombRExplodeScale(BOMB* pbomb);
void SetBombRExplodeScale(BOMB* pbomb, float rExplodeScale);
void*GetBombRExplodeScale(BOMB* pbomb);
void SetBombFExplodeEffects(BOMB* pbomb, int fExplodeEffects);
void*GetBombFExplodeEffects(BOMB* pbomb);
void SetBombFReclaim(BOMB* pbomb, int fReclaim);
void*GetBombFReclaim(BOMB* pbomb);
void SetBombZpk(BOMB* pbomb, ZPK zpk);
void*GetBombZpk(BOMB* pbomb);
SFXID*PsfxEnsureBombSfxid(BOMB* pbomb, ENSK ensk);
float*PsfxEnsureBombSStart(BOMB* pbomb, ENSK ensk);
float*PsfxEnsureBombSFull(BOMB* pbomb, ENSK ensk);
float*PsfxEnsureBombUVol(BOMB* pbomb, ENSK ensk);
void SetBombSvDetonate(BOMB* pbomb, float svDetonate);
void*GetBombSvDetonate(BOMB* pbomb);
void SetBombFCustomXps(BOMB* pbomb, int fCustomXps);
void*GetBombFCustomXps(BOMB* pbomb);
void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis);
void CloneBomb(BOMB* pbomb, BOMB* pbombBase);
void PostBombLoad(BOMB* pbomb);
void HandleBombMessage(BOMB* pbomb, MSGID msgid, void* pv);
void UpdateBomb(BOMB* pbomb, float dt);
void UpdateBombCustomXps(BOMB* pbomb);
void AddBombCustomXps(BOMB* pbomb, SO* psoOther, int cbspPruned, BSP* abspPruned, BSP* pbspPruned, XP** ppxpFirst);
int  GetBombSize();
void AdjustBombNewXp(BOMB* pbomb, XP* pxp, int ixpd);
int  FAbsorbBombWkr(BOMB* pbomb, WKR* pwkr);
void PrepareBombDetonation(BOMB* pbomb, SO* psoImpact, const glm::vec3* ppos, const glm::vec3* pnormal);
void PrimeBomb(BOMB* pbomb, float dt);
void ApplyBombThrow(BOMB* pbomb, PO* ppo);
void DetonateBomb(BOMB* pbomb);
SFX* PsfxEnsureBomb(BOMB* pbomb, ENSK ensk);
void DeleteBomb(BOMB* pbomb);

extern LM s_lmMassDefault;
extern CLQ s_clqRadiusToSvBombDefault;
extern LM s_lmSvBombDefault;
extern LM s_lmSwBombDefault;
extern CLQ s_clqRadiusToSwBombDefault;
extern CLQ s_clqRvToUdv;
extern CLQ s_clqBombDist;
extern glm::vec3 VXY_BombThrowJt;
