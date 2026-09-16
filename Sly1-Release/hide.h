#pragma once
#include "so.h"
#include "shape.h"
#include "pnt.h"

enum JTHK
{
	JTHK_Nil = -1,
	JTHK_Flatten = 0,
	JTHK_Duck = 1,
	JTHK_Basket = 2,
	JTHK_Nonchalant = 3,
	JTHK_Rail = 4,
	JTHK_Spire = 5,
	JTHK_Reach = 6,
	JTHK_Vault = 7,
	JTHK_Max = 8
};
enum HBSKS 
{
	HBSKS_Nil = -1,
	HBSKS_Available = 0,
	HBSKS_InUse = 1,
	HBSKS_Max = 2
};

class HBSK : public SO
{
	public:
	HBSKS hbsks;
	float tHbsks;
	DLE dleHbsk;
	float sFlattenRadius;
	int cMaxDartsStuck;
	float dzJumpMax;
};

class HSHAPE : public SHAPE
{
	public:
	DLE dleHshape;
	JTHK jthk;
	short oidTnHide;
	struct TN* ptnHide;
	int fTunnel;
	GRFHP grfhp;
	int fDetect;
	float dzHideMax;
	float dzMax;
};

class HPNT : public PNT
{
	public:
	DLE dleHpnt;
	JTHK jthk;
	float sFlattenRadius;
	short oidTnHide;
	struct TN* ptnHide;
	int fTunnel;
	int fDetect;
	float dzIgnore;
	float dzJumpTargetMax;
	int unkInt;
};

void StartupHide();
void ResetHideList();

HBSK*NewHbsk();
void InitHbsk(HBSK* phbsk);
void*GetHbskSFlattenRadius(HBSK* phbsk);
void SetHbskSFlattenRadius(HBSK* phbsk, float sFlattenRadius);
void*GetHbskCMaxDartsStuck(HBSK* phbsk);
void SetHbskCMaxDartsStuck(HBSK* phbsk, int cMaxDartsStuck);
void*GetHbskDzJumpMax(HBSK* phbsk);
void SetHbskDzJumpMax(HBSK* phbsk, float dzJumpMax);
void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis);
int  GetHbskSize();
void OnHbskAdd(HBSK* phbsk);
void OnHbskRemove(HBSK* phbsk);
void CloneHbsk(HBSK* phbsk, HBSK* phbskBase);
int  FIgnoreHbskIntersection(HBSK* phbsk, JT* psoOther);
void PresetHbskAccel(HBSK* phbsk, float dt);
void SetHbskHbsks(HBSK* phbsk, HBSKS hbsks);
int  CdartStuckHbsk(HBSK* phbsk);
DART* PdartOldestHbskStuck(HBSK* phbsk);
void GetHbskClosestHidePos(HBSK* phbsk, glm::vec3* ppos, float* prad);
void DeleteHbsk(HBSK *phbsk);

HSHAPE*NewHshape();
void InitHshape(HSHAPE* phshape);
void*GetHshapeJthk(HSHAPE* phshape);
void SetHshapeJthk(HSHAPE* phshape, JTHK jthk);
void*GetHshapeFTunnel(HSHAPE* phshape);
void SetHshapeFTunnel(HSHAPE* phshape, int fTunnel);
void*GetHshapeOidTnHide(HSHAPE* phshape);
void SetHshapeOidTnHide(HSHAPE* phshape, int oidTnHide);
void*GetHshapeGrfhp(HSHAPE* phshape);
void SetHshapeGrfhp(HSHAPE* phshape, GRFHP grfhp);
void*GetHshapeFDetect(HSHAPE* phshape);
void SetHshapeFDetect(HSHAPE* phshape, int fDetect);
void*GetHshapeDzHideMax(HSHAPE* phshape);
void SetHshapeDzHideMax(HSHAPE* phshape, float dzHideMax);
void*GetHshapeDzMax(HSHAPE* phshape);
void SetHshapeDzMax(HSHAPE* phshape, int fDzMax);
int  GetHshapeSize();
void OnHshapeAdd(HSHAPE* phshape);
void OnHshapeRemove(HSHAPE* phshape);
void CloneHshape(HSHAPE* phshape, HSHAPE* phshapeBase);
void BindHshape(HSHAPE* phshape);
void GetHshapeHidePos(HSHAPE* phshape, float sParam, glm::vec3* pposHide, float* pradTarget);
void GetHshapeClosestHidePos(HSHAPE* phshape, float* psParamHshape, glm::vec3* pposHide, float* pradTarget);
void DeleteHshape(HSHAPE* phshape);

HPNT*NewHpnt();
void InitHpnt(HPNT* phpnt);
void*GetHpntSFlattenRadius(HPNT* phpnt);
void SetHpntSFlattenRadius(HPNT* phpnt, float sFlattenRadius);
void*GetHpntJthk(HPNT* phpnt);
void SetHpntJthk(HPNT* phpnt, JTHK jthk);
void SetHpntOidTnHide(HPNT* phpnt, int oidTnHide);
void*GetHpntFTunnel(HPNT* phpnt);
void SetHpntFTunnel(HPNT* phpnt, int fTunnel);
void*GetHpntFDetect(HPNT* phpnt);
void SetHpntFDetect(HPNT* phpnt, int fDetect);
void*GetHpntDzJumpTargetMax(HPNT* phpnt);
void SetHpntDzJumpTargetMax(HPNT* phpnt, float dzJumpTargetMax);
void*GetHpntUnkInt(HPNT* phpnt);
void SetHpntUnkInt(HPNT* phpnt, int unkInt);
int  GetHpntSize();
void OnHpntAdd(HPNT* phpnt);
void OnHpntRemove(HPNT* phpnt);
void CloneHpnt(HPNT* phpnt, HPNT* phpntBase);
void BindHpnt(HPNT* phpnt);
void GetHpntClosestHidePos(HPNT* phpnt, float sParam, glm::vec3* pposHide, float* pradTarget);
void GetHpntHidePos(HPNT* phpnt, float sParam, glm::vec3* pposHide, float* pradTarget);
void DeleteHpnt(HPNT* phpnt);

extern DL g_dlHbsk;
extern DL g_dlHshape;
extern DL g_dlHpnt;
