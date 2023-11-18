#include "tank.h"

void InitTank(TANK* ptank)
{
	std::cout << "TANK Size: " << sizeof(TANK) << "\n";
	InitStep(ptank);
}

void RenderTankAll(TANK* ptank, CM* pcm, RO* pro)
{

}

void DeleteTank(LO* plo)
{
	delete(TANK*)plo;
}