#include "break.h"

BRK* NewBrk()
{
	return new BRK{};
}

void InitBrk(BRK* pbrk)
{
	InitSo(pbrk);
}

int GetBrkSize()
{
	return sizeof(BRK);
}

void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbrk, pbis);
}

void CloneBrk(BRK* pbrk, BRK* pbrkBase)
{
	int ichkBroken = pbrk->ichkBroken;
	CloneSo(pbrk, pbrkBase);
	pbrk->ichkBroken = ichkBroken;
}

void PostBrkLoad(BRK* pbrk)
{
	PostAloLoad(pbrk);
}

void UpdateBrk(BRK* pbrk, float dt)
{
	UpdateSo(pbrk, dt);
}

void DeleteBrk(BRK *pbrk)
{
	delete pbrk;
}

BRKP* NewBrkp()
{
	return new BRKP{};
}

int GetBrkpSize()
{
	return sizeof(BRKP);
}

void CloneBrkp(BRKP* prkp, BRKP* prkpBase)
{
	CloneSo(prkp, prkpBase);
}

void UpdateBrkp(BRKP* pbrkp, float dt)
{
	UpdateSo(pbrkp, dt);
}

void DeleteBrkp(BRKP* pbrkp)
{
	delete pbrkp;
}

BREAK* NewBreak()
{
	return new BREAK{};
}

void InitBreak(BREAK* pbreak)
{
	InitBrk(pbreak);
}

int GetBreakSize()
{
	return sizeof(BREAK);
}

void CloneBreak(BREAK* pbreak, BREAK* pbreakBase)
{
	CloneBrk(pbreak, pbreakBase);
}

void DeleteBreak(BREAK* pbreak)
{
	delete pbreak;
}

FRAGILE* NewFragile()
{
	return new FRAGILE{};
}

void InitFragile(FRAGILE* pfragile)
{
	InitBrk(pfragile);
}

int GetFragileSize()
{
	return sizeof(FRAGILE);
}

void CloneFragile(FRAGILE* pfragile, FRAGILE* pfragileBase)
{
	CloneBrk(pfragile, pfragileBase);

	pfragile->psoImpacting = pfragileBase->psoImpacting;
}

void DeleteFragile(FRAGILE* pfragile)
{
	delete pfragile;
}

ZAPBREAK* NewZapbreak()
{
	return new ZAPBREAK{};
}

int GetZapbreakSize()
{
	return sizeof(ZAPBREAK);
}

void CloneZapbreak(ZAPBREAK* pzapbreak, ZAPBREAK* pzapbreakBase)
{
	CloneFragile(pzapbreak, pzapbreakBase);

	pzapbreak->zpk = pzapbreakBase->zpk;
	pzapbreak->ppoZap = pzapbreakBase->ppoZap;
}

void UpdateZapbreak(ZAPBREAK* pzapbreak, float dt)
{
	UpdateBrk(pzapbreak, dt);
}

void DeleteZapbreak(ZAPBREAK* pzapbreak)
{
	delete pzapbreak;
}