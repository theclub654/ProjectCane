#include "unk10.h"

void* NewUnk10()
{
	return new UNK10;
}

void CloneUnk10(UNK10* punk10, UNK10* punk10Base)
{
	LO lo = *punk10;
	*punk10 = *punk10Base;
	memcpy(punk10, &lo, sizeof(LO));

	CloneLo(punk10, punk10Base);

	ClearDl(&punk10->dlChild);
}

void DeleteUnk10(UNK10* punk10)
{
	delete punk10;
}
