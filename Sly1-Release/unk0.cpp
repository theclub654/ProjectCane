#include "unk0.h"

void* NewUnk0()
{
	return new UNK0{};
}

void InitUnk0(UNK0* punk0)
{
	InitPo(punk0);
}

void CloneUnk0(UNK0* punk0, UNK0* punk0Base)
{
	LO lo = *punk0;
	*punk0 = *punk0Base;
	memcpy(punk0, &lo, sizeof(LO));

	CloneLo(punk0, punk0Base);

	ClearDl(&punk0->dlChild);
}

void DeleteUnk0(UNK0* punk0)
{
	delete punk0;
}
