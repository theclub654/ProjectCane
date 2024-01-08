#include "break.h"

void* NewBrk()
{
	return new BRK;
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
	LO lo = *pbrk;
	*pbrk = *pbrkBase;
	memcpy(pbrk, &lo, sizeof(LO));

	CloneLo(pbrk, pbrkBase);

	ClearDl(&pbrk->dlChild);

	pbrk->pxa = nullptr;
	pbrk->grfpvaXpValid = 0;
	pbrk->pstso = nullptr;
}

void DeleteBrk(LO* plo)
{
	delete (BRK*)plo;
}

void* NewBrkp()
{
	return new BRKP;
}

int GetBrkpSize()
{
	return sizeof(BRKP);
}

void CloneBrkp(BRKP* prkp, BRKP* prkpBase)
{
	LO lo = *prkp;
	*prkp = *prkpBase;
	memcpy(prkp, &lo, sizeof(LO));

	CloneLo(prkp, prkpBase);

	ClearDl(&prkp->dlChild);

	prkp->pxa = nullptr;
	prkp->grfpvaXpValid = 0;
	prkp->pstso = nullptr;
}

void DeleteBrkp(LO* plo)
{
	delete (BRKP*)plo;
}

void* NewBreak()
{
	return new BREAK;
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
	LO lo = *pbreak;
	*pbreak = *pbreakBase;
	memcpy(pbreak, &lo, sizeof(LO));

	CloneLo(pbreak, pbreakBase);

	ClearDl(&pbreak->dlChild);

	pbreak->pxa = nullptr;
	pbreak->grfpvaXpValid = 0;
	pbreak->pstso = nullptr;
}

void DeleteBreak(LO* plo)
{
	delete (BREAK*)plo;
}

void* NewFragile()
{
	return new FRAGILE;
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
	LO lo = *pfragile;
	*pfragile = *pfragileBase;
	memcpy(pfragile, &lo, sizeof(LO));

	CloneLo(pfragile, pfragileBase);

	ClearDl(&pfragile->dlChild);

	pfragile->pxa = nullptr;
	pfragile->grfpvaXpValid = 0;
	pfragile->pstso = nullptr;
}

void DeleteFragile(LO* plo)
{
	delete (FRAGILE*)plo;
}

void* NewZapbreak()
{
	return new ZAPBREAK;
}

int GetZapbreakSize()
{
	return sizeof(ZAPBREAK);
}

void CloneZapbreak(ZAPBREAK* pzapbreak, ZAPBREAK* pzapbreakBase)
{
	LO lo = *pzapbreak;
	*pzapbreak = *pzapbreakBase;
	memcpy(pzapbreak, &lo, sizeof(LO));

	CloneLo(pzapbreak, pzapbreakBase);

	ClearDl(&pzapbreak->dlChild);

	pzapbreak->pxa = nullptr;
	pzapbreak->grfpvaXpValid = 0;
	pzapbreak->pstso = nullptr;
}

void DeleteZapbreak(LO* plo)
{
	delete (ZAPBREAK*)plo;
}