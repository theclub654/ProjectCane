#include "missile.h"

void InitMissile(MISSILE* pmissile)
{
	/*std::cout << "MISSILE Size: " << sizeof(MISSILE) << "\n";
	std::cout << "ACCMISS Size: " << sizeof(ACCMISS) << "\n";
	std::cout << "TARMISS Size: " << sizeof(TARMISS) << "\n";
	std::cout << "GROUNDMISS Size: " << sizeof(GROUNDMISS) << "\n";*/
	InitBomb(pmissile);
}

void InitAccmiss(ACCMISS* paccmiss)
{
	InitMissile(paccmiss);
}

void InitTarmiss(TARMISS* ptarmiss)
{
	InitAccmiss(ptarmiss);
}

void InitGroundmiss(GROUNDMISS* pgroundmiss)
{
	InitMissile(pgroundmiss);
}

void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis)
{
	LoadBombFromBrx(pmissile, pbis);
}

void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro)
{

}
