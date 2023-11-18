#include "break.h"

void InitBrk(BRK* pbrk)
{
	InitSo(pbrk);
}

void DeleteBrk(LO* plo)
{
	delete (BRK*)plo;
}

void DeleteBrkp(LO* plo)
{
	delete (BRKP*)plo;
}

void InitBreak(BREAK* pbreak)
{
	InitBrk(pbreak);
}

void DeleteBreak(LO* plo)
{
	delete (BREAK*)plo;
}

void InitFragile(FRAGILE* pfragile)
{
	InitBrk(pfragile);
}

void DeleteFragile(LO* plo)
{
	delete (FRAGILE*)plo;
}

void DeleteZapBreak(LO* plo)
{
	delete (ZAPBREAK*)plo;
}

void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbrk, pbis);
}