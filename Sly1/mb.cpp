#include "mb.h"

void* NewMbg()
{
	return new MBG{};
}

void InitMbg(MBG *pmbg)
{
	InitStepGuard(pmbg);
}

int GetMbgSize()
{
	return sizeof(MBG);
}

void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis)
{
	LoadStepGuardFromBrx(pmbg, pbis);
}

void CloneMbg(MBG* pmbg, MBG* pmbgBase)
{
	LO lo = *pmbg;
	*pmbg = *pmbgBase;
	memcpy(pmbg, &lo, sizeof(LO));

	CloneLo(pmbg, pmbgBase);

	ClearDl(&pmbg->dlChild);

	pmbg->pxa = nullptr;
	pmbg->grfpvaXpValid = 0;
	pmbg->pstso = nullptr;
}

void DeleteMbg(LO* plo)
{
	delete (MBG*)plo;
}

void* NewBhg()
{
	return new BHG{};
}

void InitBhg(BHG *pbhg)
{
	InitStepGuard(pbhg);
}

int GetBhgSize()
{
	return sizeof(BHG);
}

void CloneBhg(BHG* pbhg, BHG* pbhgBase)
{
	LO lo = *pbhg;
	*pbhg = *pbhgBase;
	memcpy(pbhg, &lo, sizeof(LO));

	CloneLo(pbhg, pbhgBase);

	ClearDl(&pbhg->dlChild);

	pbhg->pxa = nullptr;
	pbhg->grfpvaXpValid = 0;
	pbhg->pstso = nullptr;
}

void DeleteBhg(LO* plo)
{
	delete (BHG*)plo;
}

void* NewScentmap()
{
	return new SCENTMAP{};
}

void InitScentmap(SCENTMAP* pscentmap)
{
	InitAlo(pscentmap);
}

int GetScentmapSize()
{
	return sizeof(SCENTMAP);
}

void CloneScentmap(SCENTMAP* pscentmap, SCENTMAP* pscentmapBase)
{
	LO lo = *pscentmap;
	*pscentmap = *pscentmapBase;
	memcpy(pscentmap, &lo, sizeof(LO));

	CloneLo(pscentmap, pscentmapBase);

	ClearDl(&pscentmap->dlChild);
}

void DeleteScentmap(LO* plo)
{
	delete(SCENTMAP*)plo;
}