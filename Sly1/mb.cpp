#include "mb.h"

void* CreateMbg()
{
	MBG mbg;
	return &mbg;
}

void InitMbg(MBG *pmbg)
{
	std::cout << "MBG Size: " << sizeof(MBG) << "\n";
	std::cout << "BHG Size: " << sizeof(BHG) << "\n";
	InitStepGuard(pmbg);
}

void* CreateBhg()
{
	BHG bhg;
	return &bhg;
}

void InitBhg(BHG *pbhg)
{
	InitStepGuard(pbhg);
}

void DeleteBhg(LO* plo)
{
	delete (BHG*)plo;
}

void InitScentmap(SCENTMAP* pscentmap)
{
	std::cout << "SCENTMAP Size:" << sizeof(SCENTMAP) << "\n";
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
