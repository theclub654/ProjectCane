#include "missile.h"

void InitMissile(MISSILE* pmissile)
{
	/*std::cout << "MISSILE Size: " << sizeof(MISSILE) << "\n";
	std::cout << "ACCMISS Size: " << sizeof(ACCMISS) << "\n";
	std::cout << "TARMISS Size: " << sizeof(TARMISS) << "\n";
	std::cout << "GROUNDMISS Size: " << sizeof(GROUNDMISS) << "\n";*/
	InitBomb(pmissile);
}

void DeleteMissile(LO* plo)
{
	delete (MISSILE*)plo;
}

void InitAccmiss(ACCMISS* paccmiss)
{
	InitMissile(paccmiss);
}

void DeleteAccmiss(LO* plo)
{
	delete (ACCMISS*)plo;
}

void InitTarmiss(TARMISS* ptarmiss)
{
	InitAccmiss(ptarmiss);
}

void DeleteTarmiss(LO* plo)
{
	delete (TARMISS*)plo;
}

void DeleteSplmiss(LO* plo)
{
	delete (SPLMISS*)plo;
}

void InitGroundmiss(GROUNDMISS* pgroundmiss)
{
	InitMissile(pgroundmiss);
}

void DeleteGroundmiss(LO* plo)
{
	delete (GROUNDMISS*)plo;
}

void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis)
{
	LoadBombFromBrx(pmissile, pbis);
}

void RenderMissileAll(MISSILE* pmissile, CM* pcm, RO* pro)
{

}
