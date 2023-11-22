#include "missile.h"

void* NewMissile()
{
	return new MISSILE;
}

void InitMissile(MISSILE* pmissile)
{
	InitBomb(pmissile);
}

void DeleteMissile(LO* plo)
{
	delete (MISSILE*)plo;
}

void* NewAccmiss()
{
	return new ACCMISS;
}

void InitAccmiss(ACCMISS* paccmiss)
{
	InitMissile(paccmiss);
}

void DeleteAccmiss(LO* plo)
{
	delete (ACCMISS*)plo;
}

void* NewTarmiss()
{
	return new TARMISS;
}

void InitTarmiss(TARMISS* ptarmiss)
{
	InitAccmiss(ptarmiss);
}

void DeleteTarmiss(LO* plo)
{
	delete (TARMISS*)plo;
}

void* NewSplmiss()
{
	return new SPLMISS;
}

void DeleteSplmiss(LO* plo)
{
	delete (SPLMISS*)plo;
}

void* NewGroundmiss()
{
	return new GROUNDMISS;
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
