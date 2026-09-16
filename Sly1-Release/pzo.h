#pragma once
#include "po.h"
#include "smack.h"
#include "ctr.h"

class XFM;

enum FSP 
{
	FSP_Nil = -1,
	FSP_Dive = 0,
	FSP_Ball = 1,
	FSP_SlowDownClock = 2,
	FSP_Mine = 3,
	FSP_SpeedUpClock = 4,
	FSP_Decoy = 5,
	FSP_Stun = 6,
	FSP_Max = 7
};

enum PCS 
{
	PCS_Nil = -1,
	PCS_Collected = 0,
	PCS_Collectible = 1,
	PCS_NotCollectible = 2,
	PCS_Max = 3
};

struct TMBL
{
	class  ALO* palo;
	struct SM*  psmDial;
	struct SMA* psmaDial;
};

struct GOAD
{
	int coidDialog;
	short aoidDialog[4];
	int cpdialog;
	struct DIALOG* apdialog[4];
	int ipdialog;
};

class SPRIZE : public SO
{
	public:
	float sCollect;
	int fCollect;
	int fNoExit;
	int coidAseg;
	short aoidAseg[4];
	int cpaseg;
	struct ASEG* apaseg[4];
	struct EXPL* pexpl;
	int mpipasegfDone[4];
	int fJtDone;
	int fForceCollect;
};

struct CLUECTR : public CTR
{

};

class CLUE : public SPRIZE
{
	public:
	int ibit;
	float dtFrame;
	float dtFrameMax;
	int cpaloRender;
	std::vector<XFM*> apaloRender;
	float swSpin;
	float radSpin;
	class ALO* paloSmack;
};

class LOCK : public ALO
{
	public:
	struct SM* psm;
	struct SMA* psma;
	class ALO* paloKey;
};

class LOCKG : public ALO
{
	public:
	int grfws;
	struct SM* psm;
	struct SMA* psma;
	int coidLock;
	short aoidLock[8];
	int cplock;
	LOCK* aplock[8];
};

class VAULT : public PO
{
	public:
	class SM* psmVault;
	class SMA* psmaVault;
	class XFM* pxfmJt;
	TMBL atmbl[3];
	int nCombination;
	short oidDialogCombo;
	struct DIALOG* pdialogCombo;
	short oidVolbtnGoad;
	struct VOLBTN* pvolbtnGoad;
	int fGoadStart;
	GOAD mpgoadkgoad[6];
	int fVault;
	int idialogInstruct;
	struct DIALOG* apdialogInstruct[28];
	int cpdialogPending;
	struct DIALOG* apdialogPending[6];
};

class SCPRIZE : public SPRIZE
{
	public:
	int ichkCollected;
};

VAULT*NewVault();
void InitVault(VAULT* pvault);
void SetVaultnCombination(VAULT* pvault, int nCombination);
void*GetVaultnCombination(VAULT* pvault);
void SetVaultoidVolbtnGoad(VAULT* pvault, OID oidVolbtnGoad);
void*GetVaultoidVolbtnGoad(VAULT* pvault);
void SetVaultoidDialogCombo(VAULT* pvault, OID oidDialogCombo);
void*GetVaultoidDialogCombo(VAULT* pvault);
void SetVaultfGoadStart(VAULT* pvault, int fGoadStart);
void*GetVaultfGoadStart(VAULT* pvault);
int  GetVaultSize();
void CloneVault(VAULT* pvault, VAULT* pvaultBase);
void PostTmblLoad(TMBL* ptmbl, OID oidInitialState);
void PostVaultLoad(VAULT* pvault);
void OnVaultActive(VAULT* pvault, int fActive);
void UpdateVaultActive(VAULT* pvault, JOY* pjoy, float dt);
void HandleVaultMessage(VAULT* pvault, MSGID msgid, void* pv);
void AddVaultGoadDialog(VAULT* pvault, int goadk, OID oidDialog);
void OpenVault(VAULT* pvault);
int FCanOpenVault(VAULT* pvault);
void DeleteVault(VAULT *pvault);

void GetBlueprintInfo(GRFVAULT* pgrfvault, int ipdialog);

SPRIZE*NewSprize();
void InitSprize(SPRIZE* psprize);
void*GetSprizeSCollect(SPRIZE* psprize);
void SetSprizeSCollect(SPRIZE* psprize, float sCollect);
void*GetSprizeFNoExit(SPRIZE* psprize);
void SetSprizeFNoExit(SPRIZE* psprize, int fNoExit);
int  GetSprizeSize();
void LoadSprizeFromBrx(SPRIZE* psprize, CBinaryInputStream* pbis);
void CloneSprize(SPRIZE* psprize, SPRIZE* psprizeBase);
void BindSprize(SPRIZE* psprize);
void PostSprizeLoad(SPRIZE* psprize);
void UpdateSprize(SPRIZE* psprize, float dt);
void CollectSprize(SPRIZE* psprize);
void EmitSprizeExplosion(SPRIZE* psprize);
PCS  PcsFromSprize(SPRIZE* psprize);
void AddSprizeAseg(SPRIZE* psprize, OID oidAseg);
void HandleSprizeMessage(SPRIZE* psprize, MSGID msgid, void* pv);
int  FIgnoreSprizeIntersection(SPRIZE* psprize, SO* psoOther);
void AdjustSprizeNewXp(SPRIZE* psprize, XP* pxp, int ixpd);
void DeleteSprize(SPRIZE* psprize);

SCPRIZE*NewScprize();
void InitScprize(SCPRIZE* pscprize);
int  GetScprizeSize();
void CloneScprize(SCPRIZE* pscprize, SCPRIZE* pscprizeBase);
PCS  PcsFromScprize(SCPRIZE* pscprize);
void CollectScprize(SCPRIZE* pscprize);
void DeleteScprize(SCPRIZE* pscprize);

void StartupClueCtr(CLUECTR* pcluectr);

CLUE*NewClue();
void InitClue(CLUE* pclue);
void SetClueDtFrameMax(CLUE* pclue, float dtFrameMax);
void*GetClueDtFrameMax(CLUE* pclue);
int  GetClueSize();
void LoadClueFromBrx(CLUE* pclue, CBinaryInputStream* pbis);
void CloneClue(CLUE* pclue, CLUE* pclueBase);
void PostClueLoad(CLUE* pclue);
void UpdateClue(CLUE* pclue, float dt);
void OnClueSmack(CLUE* pclue);
void CollectClue(CLUE* pclue);
void BreakClue(CLUE* pclue);
void CollectClueSilent(CLUE* pclue);
void GetClueCollected(CLUE* pclue, uint32_t* pfCollected);
void ImpactClue(CLUE* pclue, int fParentDirty);
int  FAbsorbClueWkr(CLUE* pclue, WKR* pwkr);
void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro);
void CollectAllClues();
void DeleteClue(CLUE *pclue);

LOCK*NewLock();
int  GetLockSize();
void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis);
void CloneLock(LOCK* plock, LOCK* plockBase);
void PostLockLoad(LOCK* plock);
void DeleteLock(LOCK* plock);

LOCKG*NewLockg();
int  GetLockgSize();
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);
void CloneLockg(LOCKG* plockg, LOCKG* plockgBase);
void PostLockgLoad(LOCKG* plockg);
void SetLockgIndex(LOCKG* plockg, int ifws);
void AddLockgLock(LOCKG* plockg, int oidLock);
void TriggerLockg(LOCKG* plockg);
void DeleteLockg(LOCKG* plockg);

extern CLUECTR g_cluectr;
extern GRFVAULT s_agrfvaultInstruct[28];
extern SNIP s_asnipVault[26];
extern float VXY_SprizeBreakDefault;
extern float VZ_SprizeBreakDefault;
extern SNIP asnipClue[1];
extern LM s_lmClueSpin;
extern LM s_lmClueBounce;
extern float sClueMagnetRadius;
extern SNIP asnipLockLoadBrx;
extern SNIP asnipPostLockLoad;
extern SNIP asnipLockgBrx;
extern const SMACKI s_smackiClue;
