#include "unk5.h"

void* NewUnk5()
{
	return new UNK5{};
}

void InitUnk5(UNK5* punk5)
{
	InitSo(punk5);
}

void CloneUnk5(UNK5* punk5, UNK5* punk5Base)
{
	LO lo = *punk5;
	*punk5 = *punk5Base;
	memcpy(punk5, &lo, sizeof(LO));

	CloneLo(punk5, punk5Base);

	ClearDl(&punk5->dlChild);

	punk5->pxa = nullptr;
	punk5->grfpvaXpValid = 0;
	punk5->pstso = nullptr;
}

void DeleteUnk5(UNK5* punk5)
{
	delete punk5;
}
