#include "break.h"

void* NewBrk()
{
	return new BRK;
}

void InitBrk(BRK* pbrk)
{
	InitSo(pbrk);
}

void DeleteBrk(LO* plo)
{
	delete (BRK*)plo;
}

void* NewBrkp()
{
	return new BRKP;
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

void DeleteFragile(LO* plo)
{
	delete (FRAGILE*)plo;
}

void* NewZapbreak()
{
	return new ZAPBREAK;
}

void DeleteZapbreak(LO* plo)
{
	delete (ZAPBREAK*)plo;
}

void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbrk, pbis);
}