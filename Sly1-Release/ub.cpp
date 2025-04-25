#include "ub.h"

UBG* NewUbg()
{
	return new UBG{};
}

void InitUbg(UBG* pubg)
{
	InitGomer(pubg);
}

int GetUbgSize()
{
	return sizeof(UBG);
}

void CloneUbg(UBG* pubg, UBG* pubgBase)
{
	CloneGomer(pubg, pubgBase);

	pubg->psmWorld = pubgBase->psmWorld;
	pubg->psmaWorld = pubgBase->psmaWorld;
	pubg->oidPatrolGoal = pubgBase->oidPatrolGoal;
	pubg->cJump = pubgBase->cJump;
	pubg->pxfmMiddle = pubgBase->pxfmMiddle;

	for (int i = 0; i < 6; ++i)
		pubg->apsoPads[i] = pubgBase->apsoPads[i];

	pubg->psoLastDunk = pubgBase->psoLastDunk;

	for (int i = 0; i < 4; ++i)
		pubg->aichk[i] = pubgBase->aichk[i];

	pubg->nBossctr = pubgBase->nBossctr;
}

void RenderUbpAll(UBP* pubp, CM* pcm, RO* pro)
{
	RenderUbpAll(pubp, pcm, pro);
}

void DeleteUbg(UBG* pubg)
{
	delete pubg;
}

UBP* NewUbp()
{
	return new UBP{};
}

int GetUbpSize()
{
	return sizeof(UBP);
}

void CloneUbp(UBP* pubp, UBP* pubpBase)
{
	CloneSo(pubp, pubpBase);

	pubp->pubv = pubpBase->pubv;
	pubp->ubps = pubpBase->ubps;
	pubp->tUbps = pubpBase->tUbps;
	pubp->pactval = pubpBase->pactval;
	pubp->pbomb = pubpBase->pbomb;
}

void DeleteUbp(UBP *pubp)
{
	delete pubp;
}

UBV* NewUbv()
{
	return new UBV{};
}

int GetUbvSize()
{
	return sizeof(UBV);
}

void CloneUbv(UBV* pubv, UBV* pubvBase)
{
	CloneSo(pubv, pubvBase);

	pubv->ubvs = pubvBase->ubvs;
	pubv->tUbvs = pubvBase->tUbvs;

	for (int i = 0; i < 12; ++i)
		pubv->apubp[i] = pubvBase->apubp[i];

	for (int i = 0; i < 2; ++i)
		pubv->apubpBase[i] = pubvBase->apubpBase[i];

	pubv->dposPadRow = pubvBase->dposPadRow;
	pubv->dposPadCol = pubvBase->dposPadCol;
	pubv->dposBase = pubvBase->dposBase;

	pubv->cswap = pubvBase->cswap;
	pubv->iswap = pubvBase->iswap;

	for (int i = 0; i < 24; ++i)
		pubv->aswap[i] = pubvBase->aswap[i];

	for (int i = 0; i < 4; ++i)
		pubv->mprowcolSafe[i] = pubvBase->mprowcolSafe[i];
}

void DeleteUbv(UBV *pubv)
{
	delete pubv;
}
