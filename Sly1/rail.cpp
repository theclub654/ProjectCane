#include "rail.h"

void* NewRail()
{
	return new RAIL{};
}

int GetRailSize()
{
	return sizeof(RAIL);
}

void OnRailAdd(RAIL* prail)
{
	OnLoAdd(prail);
	AppendDlEntry(&prail->psw->dlRail, prail);
}

void CloneRail(RAIL* prail, RAIL* prailBase)
{
	LO lo = *prail;
	*prail = *prailBase;
	memcpy(prail, &lo, sizeof(LO));

	CloneLo(prail, prailBase);
}

void DeleteRail(LO* plo)
{
	delete(RAIL*)plo;
}
