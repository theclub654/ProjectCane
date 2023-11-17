#include "mouthgame.h"

void InitMgc(MGC* pmgc)
{
	std::cout << "MGC Size: " << sizeof(MGC) << "\n";
	InitAlo(pmgc);
}

void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pmgc, pbis);
}
