#include "bomb.h"

void* NewBomb()
{
	return new BOMB{};
}

void InitBomb(BOMB* pbomb)
{
	InitSo(pbomb);
}

int GetBombSize()
{
	return sizeof(BOMB);
}

void OnBombAdd(BOMB* pbomb)
{
	OnSoAdd(pbomb);
}

void CloneBomb(BOMB* pbomb, BOMB* pbombBase)
{
	LO lo = *pbomb;
	*pbomb = *pbombBase;
	memcpy(pbomb, &lo, sizeof(LO));

	CloneLo(pbomb, pbombBase);

	ClearDl(&pbomb->dlChild);

	pbomb->pxa = nullptr;
	pbomb->grfpvaXpValid = 0;
	pbomb->pstso = nullptr;
}

void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pbomb, pbis);
}

void DeleteBomb(LO* plo)
{
	delete (BOMB*)plo;
}
