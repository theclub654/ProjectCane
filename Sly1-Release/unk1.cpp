#include "unk1.h"

void* NewUnk1()
{
	return new UNK1{};
}

void LoadUnk1FromBrx(UNK1* punk1, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(punk1, pbis);
}

void InitUnk1(UNK1* punk1)
{
	InitPo(punk1);
}

void CloneUnk1(UNK1* punk1, UNK1* punk1Base)
{
	LO lo = *punk1;
	*punk1 = *punk1Base;
	memcpy(punk1, &lo, sizeof(LO));

	CloneLo(punk1, punk1Base);

	ClearDl(&punk1->dlChild);
}

void DeleteUnk1(UNK1* punk1)
{
	delete punk1;
}
