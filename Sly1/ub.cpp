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
	LO lo = *pubg;
	*pubg = *pubgBase;
	memcpy(pubg, &lo, sizeof(LO));

	CloneLo(pubg, pubgBase);

	ClearDl(&pubg->dlChild);

	pubg->pxa = nullptr;
	pubg->grfpvaXpValid = 0;
	pubg->pstso = nullptr;
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
	LO lo = *pubp;
	*pubp = *pubpBase;
	memcpy(pubp, &lo, sizeof(LO));

	CloneLo(pubp, pubpBase);
}

void DeleteUbp(LO* plo)
{
	delete (UBP*)plo;
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
	LO lo = *pubv;
	*pubv = *pubvBase;
	memcpy(pubv, &lo, sizeof(LO));

	CloneLo(pubv, pubvBase);
}

void DeleteUbv(LO* plo)
{
	delete (UBV*)plo;
}
