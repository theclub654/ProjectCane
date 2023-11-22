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

void* NewVault();
void InitVault(VAULT* pvault);
void DeleteVault(LO* plo);
void* NewSprize();
void InitSprize(SPRIZE* psprize);
void DeleteSprize(LO* plo);
void* NewScprize();
void InitScprize(SCPRIZE* pscprize);
void DeleteScprize(LO* plo);
void* NewLifetkn();
void DeleteLifetkn(LO* plo);
void* NewClue();
void InitClue(CLUE* pclue);
void LoadSprizeFromBrx(SPRIZE *psprize, CBinaryInputStream *pbis);
void LoadClueFromBrx(CLUE *pclue, CBinaryInputStream *pbis);
void DeleteClue(LO* plo);
void* NewLock();
void LoadLockFromBrx(LOCK* plock, CBinaryInputStream* pbis);
void DeleteLock(LO* plo);
void* NewLockg();
void LoadLockgFromBrx(LOCKG* plockg, CBinaryInputStream* pbis);
void DeleteLockg(LO* plo);
void RenderClueAll(CLUE* pclue, CM* pcm, RO* pro);