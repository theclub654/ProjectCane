#include "bomb.h"

void InitBomb(BOMB* pbomb)
{
	//std::cout << "BOMB Size: " << sizeof(BOMB) << "\n";
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
