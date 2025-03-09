#include "unk3.h"

void* NewUnk3()
{
	return new UNK3{};
}

void CloneUnk3(UNK3* punk3, UNK3* punk3Base)
{
	LO lo = *punk3;
	*punk3 = *punk3Base;
	memcpy(punk3, &lo, sizeof(LO));

	CloneLo(punk3, punk3Base);

	ClearDl(&punk3->dlChild);
}

void DeleteUnk3(UNK3* punk3)
{
	delete punk3;
}
