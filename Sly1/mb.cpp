#include "mb.h"

void InitMbg(MBG *pmbg)
{
	std::cout << "MBG Size: " << sizeof(MBG) << "\n";
	std::cout << "BHG Size: " << sizeof(BHG) << "\n";
	InitStepGuard(pmbg);
}

void InitBhg(BHG *pbhg)
{
	InitStepGuard(pbhg);
}

void InitScentmap(SCENTMAP* pscentmap)
{
	std::cout << "SCENTMAP Size:" << sizeof(SCENTMAP) << "\n";
	InitAlo(pscentmap);
}

void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis)
{
	LoadStepGuardFromBrx(pmbg, pbis);
}
