#include "unk9.h"

void*NewUnk9()
{
	return new UNK9{};
}

void CloneUnk9(UNK9* punk9, UNK9* punk9Base)
{
	LO lo = *punk9;
	*punk9 = *punk9Base;
	memcpy(punk9, &lo, sizeof(LO));

	CloneLo(punk9, punk9Base);

	ClearDl(&punk9->dlChild);
}

void DeleteUnk9(UNK9* punk9)
{
	delete punk9;
}
