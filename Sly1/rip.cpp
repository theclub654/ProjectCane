#include "rip.h"

RIPG* NewRipg()
{
	return new RIPG{};
}

void InitRipg(RIPG *pripg)
{
	InitSo(pripg);
	InitDl(&pripg->dlRip, 0x1C + 0x10 + 0xC);
	pripg->sMRD = 10000000000;
}

int GetRipgSize()
{
	return sizeof(RIPG);
}

void CloneRipg(RIPG* pripg, RIPG* pripgBase)
{
	LO lo = *pripg;
	*pripg = *pripgBase;
	memcpy(pripg, &lo, sizeof(LO));

	CloneLo(pripg, pripgBase);

	ClearDl(&pripg->dlChild);

	pripg->pxa = nullptr;
	pripg->grfpvaXpValid = 0;
	pripg->pstso = nullptr;
}

void OnRipgRemove(RIPG* pripg)
{

}

void RenderRipgSelf(RIPG* pripg, CM* pcm, RO* pro)
{

}

void DeleteRipg(LO* plo)
{
	delete (RIPG*)plo;
}
