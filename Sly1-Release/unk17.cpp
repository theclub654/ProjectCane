#include "unk17.h"

void* NewUnk17()
{
    return new UNK17{};
}

void CloneUnk17(UNK17* punk17, UNK17* punk17Base)
{
	LO lo = *punk17;
	*punk17 = *punk17Base;
	memcpy(punk17, &lo, sizeof(LO));

	CloneLo(punk17, punk17Base);

	ClearDl(&punk17->dlChild);

	punk17->pxa = nullptr;
	punk17->grfpvaXpValid = 0;
	punk17->pstso = nullptr;
}

void DeleteUnk17(UNK17* punk17)
{
    delete punk17;
}
