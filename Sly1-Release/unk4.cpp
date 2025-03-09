#include "unk4.h"

void* NewUnk4()
{
	return new UNK4{};
}

void InitUnk4(UNK4* punk4)
{
	InitSo(punk4);
}

void CloneUnk4(UNK4* punk4, UNK4* punk4Base)
{
	LO lo = *punk4;
	*punk4 = *punk4Base;
	memcpy(punk4, &lo, sizeof(LO));

	CloneLo(punk4, punk4Base);

	ClearDl(&punk4->dlChild);

	punk4->pxa = nullptr;
	punk4->grfpvaXpValid = 0;
	punk4->pstso = nullptr;
}

void DeleteUnk4(UNK4* punk4)
{
	delete punk4;
}
