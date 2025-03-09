#include "unk11.h"

void* NewUnk11()
{
	return new UNK11;
}

void InitUnk11(UNK11* punk11)
{
	InitAlo(punk11);
}

void CloneUnk11(UNK11* punk11, UNK11* punk11Base)
{
	LO lo = *punk11;
	*punk11 = *punk11Base;
	memcpy(punk11, &lo, sizeof(LO));

	CloneLo(punk11, punk11Base);

	ClearDl(&punk11->dlChild);
}

void DeleteUnk11(UNK11* punk11)
{
	delete punk11;
}
