#include "rip.h"

RIPG* NewRipg()
{
	return new RIPG{};
}

void InitRipg(RIPG *pripg)
{
	InitSo(pripg);
	InitDl(&pripg->dlRip, 0x10);
	//*(ulong*)&(pripg->bspcCamera).absp = *(ulong*)&(pripg->bspcCamera).absp | 0x8040000000000;
	pripg->sMRD = 1e+10;
	*(unsigned long*)&pripg->bitfield = *(unsigned long*)&pripg->bitfield & 0xffffffffcfffffff | 0x10020000000;
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
