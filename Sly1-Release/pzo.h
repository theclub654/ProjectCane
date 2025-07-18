#pragma once
#include "po.h"

struct TMBL
{
	struct ALO* palo;
	struct SM* psmDial;
	struct SMA* psmaDial;
};

struct GOAD
{
	int coidDialog;
	enum OID aoidDialog[4];
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
		OID aoidAseg[4];
		int cpaseg;
		struct ASEG* apaseg[4];
		struct EXPL* pexpl;
		int mpipasegfDone[4];
		int fJtDone;
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
		struct ALO** apaloRender;
		float swSpin;
		float radSpin;
		struct ALO* paloSmack;
};

class LOCK : public ALO
{
	public:
		struct SM* psm;
		struct SMA* psma;
		struct ALO* paloKey;
};

class LOCKG : public ALO
{
	public:
		int grfws;
		struct SM* psm;
		struct SMA* psma;
		int coidLock;
		OID aoidLock[8];
		int cplock;
		LOCK* aplock[8];
};

class VAULT : public PO
{
	public:
		struct SM* psmVault;
		struct SMA* psmaVault;
		struct XFM* pxfmJt;
		TMBL atmbl[3];
		int nCombination;
		OID oidDialogCombo;
		struct DIALOG* pdialogCombo;
		OID oidVolbtnGoad;
		struct VOLBTN* pvolbtnGoad;
		int fGoadStart;
		GOAD mpgoadkgoad[5];
		int fvault;
		struct DIALOG* apdialogInstruct[33];
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
int  GetVaultSize();
void CloneVault(VAULT* pvault, VAULT* pvaultBase);
void DeleteVault(VAULT *pvault);

SPRIZE*NewSprize();
void InitSprize(SPRIZE* psprize);
int  GetSprizeSize();
void LoadSprizeFromBrx(SPRIZE* psprize, CBinaryInputStream* pbis);
void CloneSprize(SPRIZE* psprize, SPRIZE* psprizeBase);
void BindSprize(SPRIZE* psprize);
void DeleteSprize(SPRIZE* psprize);

SCPRIZE*NewScprize();
void InitScprize(SCPRIZE* pscprize);
int  GetScprizeSize();
void CloneScprize(SCPRIZE* pscprize, SCPRIZE* pscprizeBase);
void DeleteScprize(SCPRIZE* pscprize);

void StartupClueCtr(CLUECTR* pcluectr);

CLUE*NewClue();
void InitClue(CLUE* pclue);
int  GetClueSize();
void CloneClue(CLUE* pclue, CLUE* pclueBase);
void LoadClueFromBrx(CLUE *pclue, CBinaryInputStream *pbis);
void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro);
void DeleteClue(CLUE *pclue);

LOCK*NewLock();
int  GetLockSize();
void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis);
void CloneLock(LOCK* plock, LOCK* plockBase);
void DeleteLock(LOCK* plock);

LOCKG*NewLockg();
int  GetLockgSize();
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);
void CloneLockg(LOCKG* plockg, LOCKG* plockgBase);
void DeleteLockg(LOCKG* plockg);

extern CLUECTR g_cluectr;