#include "bomb.h"

void* NewBomb()
{
	return new BOMB;
}

void InitBomb(BOMB* pbomb)
{
	InitSo(pbomb);
}

void OnBombAdd(BOMB* pbomb)
{
	OnSoAdd(pbomb);
}

void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbomb, pbis);
}

void DeleteBomb(LO* plo)
{
	delete (BOMB*)plo;
}
