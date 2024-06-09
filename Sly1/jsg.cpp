#include "jsg.h"

JSG* NewJsg()
{
	return new JSG{};
}

void InitJsg(JSG* pjsg)
{
	InitLo(pjsg);
}

int GetJsgSize()
{
	return sizeof(JSG);
}

void CloneJsg(JSG* pjsg, JSG* pjsgBase)
{
	LO lo = *pjsg;
	*pjsg = *pjsgBase;
	memcpy(pjsg, &lo, sizeof(LO));

	CloneLo(pjsg, pjsgBase);
}

void DeleteJsg(LO* plo)
{
	delete(JSG*)plo;
}
