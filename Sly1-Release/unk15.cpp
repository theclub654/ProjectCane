#include "unk15.h"

void* NewUnk15()
{
	return new UNK15{};
}

void InitUnk15(UNK15* punk15)
{
	InitDprize(punk15);
}

void CloneUnk15(UNK15* punk15, UNK15* punk15Base)
{
	LO lo = *punk15;
	*punk15 = *punk15Base;
	memcpy(punk15, &lo, sizeof(LO));

	CloneLo(punk15, punk15Base);

	ClearDl(&punk15->dlChild);
}

void DeleteUnk15(UNK15* punk15)
{
	delete punk15;
}
