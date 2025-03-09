#include "unk7.h"

void* NewUnk7()
{
	return new UNK7{};
}

void InitUnk7(UNK7* punk7)
{
	InitDprize(punk7);
}

void CloneUnk7(UNK7* punk7, UNK7* punk7Base)
{
	LO lo = *punk7;
	*punk7 = *punk7Base;
	memcpy(punk7, &lo, sizeof(LO));

	CloneLo(punk7, punk7Base);

	ClearDl(&punk7->dlChild);
}

void DeleteUnk7(UNK7* punk7)
{
	delete punk7;
}
