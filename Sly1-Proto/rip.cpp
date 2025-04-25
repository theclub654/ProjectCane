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
	CloneSo(pripg, pripgBase);

	pripg->ripgt = pripgBase->ripgt;
	pripg->sExpand = pripgBase->sExpand;

	pripg->dlRip = pripgBase->dlRip;

	pripg->pripgNext = pripgBase->pripgNext;
}

void OnRipgRemove(RIPG* pripg)
{

}

void RenderRipgSelf(RIPG* pripg, CM* pcm, RO* pro)
{

}

void DeleteRipg(RIPG* pripg)
{
	delete pripg;
}
