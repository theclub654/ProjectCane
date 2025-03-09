#include "unk16.h"

void* NewUnk16()
{
    return new UNK16{};
}

void InitUnk16(UNK16* punk16)
{
    InitMbg(punk16);
}

void CloneUnk16(UNK16* punk16, UNK16* punk16Base)
{
	LO lo = *punk16;
	*punk16 = *punk16Base;
	memcpy(punk16, &lo, sizeof(LO));

	CloneLo(punk16, punk16Base);

	ClearDl(&punk16->dlChild);

	punk16->pxa = nullptr;
	punk16->grfpvaXpValid = 0;
	punk16->pstso = nullptr;
}

void DeleteUnk16(UNK16* punk16)
{
	delete punk16;
}
