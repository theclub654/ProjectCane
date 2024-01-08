#include "asega.h"

int GetAsegaSize()
{
	return sizeof(ASEGA);
}

void DeleteAsega(LO* plo)
{
	delete(ASEGA*)plo;
}
