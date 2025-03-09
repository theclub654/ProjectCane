#include "unk8.h"

void* NewUnk8()
{
	return new UNK8{};
}

void InitUnk8(UNK8* punk8)
{
	InitAlo(punk8);
}

void CloneUnk8(UNK8* punk8, UNK8* punk8Base)
{
	LO lo = *punk8;
	*punk8 = *punk8Base;
	memcpy(punk8, &lo, sizeof(LO));

	CloneLo(punk8, punk8Base);

	ClearDl(&punk8->dlChild);
}

void DeleteUnk8(UNK8* punk8)
{
	delete punk8;
}
