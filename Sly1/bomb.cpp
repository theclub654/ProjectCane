#include "bomb.h"

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
