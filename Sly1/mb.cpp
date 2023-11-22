#include "mb.h"

void* NewMbg()
{
	return new MBG;
}

void InitMbg(MBG *pmbg)
{
	InitStepGuard(pmbg);
}

void* NewBhg()
{
	return new BHG;
}

void InitBhg(BHG *pbhg)
{
	InitStepGuard(pbhg);
}

void DeleteBhg(LO* plo)
{
	delete (BHG*)plo;
}

void* NewScentmap()
{
	return new SCENTMAP;
}

void InitScentmap(SCENTMAP* pscentmap)
{
	InitAlo(pscentmap);
}

void DeleteScentmap(LO* plo)
{
	delete(SCENTMAP*)plo;
}

void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis)
{
	LoadStepGuardFromBrx(pmbg, pbis);
}

void DeleteMbg(LO* plo)
{
	delete (MBG*)plo;
}
