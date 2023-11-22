#include "rip.h"

void* NewRipg()
{
	return new RIPG;
}

void InitRipg(RIPG *pripg)
{
	InitSo(pripg);
	InitDl(&pripg->dlRip, 0x1C + 0x10 + 0xC);
	pripg->sMRD = 10000000000;
}

void OnRipgRemove(RIPG* pripg)
{

}

void DeleteRipg(LO* plo)
{
	delete (RIPG*)plo;
}
