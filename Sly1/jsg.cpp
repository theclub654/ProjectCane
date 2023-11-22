#include "jsg.h"

void* NewJsg()
{
	return new JSG;
}

void InitJsg(JSG* pjsg)
{
	InitLo(pjsg);
}

void DeleteJsg(LO* plo)
{
	delete(JSG*)plo;
}
