#include "unk6.h"

void* NewUnk6()
{
	return new UNK6{};
}

void CloneUnk6(UNK6* punk6, UNK6* punk6Base)
{
	LO lo = *punk6;
	*punk6 = *punk6Base;
	memcpy(punk6, &lo, sizeof(LO));

	CloneLo(punk6, punk6Base);

	ClearDl(&punk6->dlChild);

	punk6->pxa = nullptr;
	punk6->grfpvaXpValid = 0;
	punk6->pstso = nullptr;
}

void DeleteUnk6(UNK6* punk6)
{
	delete punk6;
}
