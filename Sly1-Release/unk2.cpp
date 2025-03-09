#include "unk2.h"

void* NewUnk2()
{
	return new UNK2;
}

void InitUnk2(UNK2* punk2)
{
	InitPo(punk2);
}

void CloneUnk2(UNK2* punk2, UNK2* punk2Base)
{
	LO lo = *punk2;
	*punk2 = *punk2Base;
	memcpy(punk2, &lo, sizeof(LO));

	CloneLo(punk2, punk2Base);

	ClearDl(&punk2->dlChild);
}

void DeleteUnk2(UNK2* punk2)
{
	delete punk2;
}
