#include "ik.h"

void* NewIkh()
{
	return new IKH;
}

void DeleteIkh(LO* plo)
{
	delete(IKH*)plo;
}

void* NewLikh()
{
	return new LIKH;
}

void DeleteLikh(LO* plo)
{
	delete(LIKH*)plo;
}
