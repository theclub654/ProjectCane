#include "tail.h"

TAIL* NewTail()
{
	return new TAIL{};
}

void InitTail(TAIL* ptail)
{
	InitAlo(ptail);
}

int GetTailSize()
{
	return sizeof(TAIL);
}

void CloneTail(TAIL* ptail, TAIL* ptailBase)
{
	LO lo = *ptail;
	*ptail = *ptailBase;
	memcpy(ptail, &lo, sizeof(LO));

	CloneLo(ptail, ptailBase);

	ClearDl(&ptail->dlChild);
}

void DeleteTail(LO* plo)
{
	delete(TAIL*)plo;
}
