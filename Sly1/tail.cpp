#include "tail.h"

void* NewTail()
{
	return new TAIL;
}

void InitTail(TAIL* ptail)
{
	InitAlo(ptail);
}

void DeleteTail(LO* plo)
{
	delete(TAIL*)plo;
}
