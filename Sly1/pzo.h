#pragma once
#include "po.h"

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
};

class SCPRIZE : public SPRIZE
{
	public:
		int ichkCollected;
};

class LIFETKN : public SCPRIZE
{
	public:
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
void DeleteSprize(SPRIZE* psprize);

SCPRIZE*NewScprize();
void InitScprize(SCPRIZE* pscprize);
int  GetScprizeSize();
void CloneScprize(SCPRIZE* pscprize, SCPRIZE* pscprizeBase);
void DeleteScprize(SCPRIZE* pscprize);

LIFETKN*NewLifetkn();
int  GetLifetknSize();
void CloneLifetkn(LIFETKN *plifetkn, LIFETKN *plifetknBase);
void DeleteLifetkn(LIFETKN *plifetkn);

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
void DeleteLock(LO* plo);

LOCKG*NewLockg();
int  GetLockgSize();
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);
void CloneLockg(LOCKG* plockg, LOCKG* plockgBase);
void DeleteLockg(LO* plo);