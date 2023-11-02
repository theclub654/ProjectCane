#include "break.h"

void InitBrk(BRK* pbrk)
{
	InitSo(pbrk);
}

void InitBreak(BREAK* pbreak)
{
	InitBrk(pbreak);
}

void InitFragile(FRAGILE* pfragile)
{
	InitBrk(pfragile);
}

void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbrk, pbis);
}