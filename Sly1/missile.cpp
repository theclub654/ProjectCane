#include "missile.h"

void InitMissile(MISSILE* pmissile)
{
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
