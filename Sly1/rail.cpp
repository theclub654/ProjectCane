#include "rail.h"

void* NewRail()
{
	return new RAIL;
}

void OnRailAdd(RAIL* prail)
{
	OnLoAdd(prail);
	AppendDlEntry(&prail->psw->dlRail, prail);
}

void DeleteRail(LO* plo)
{
	delete(RAIL*)plo;
}
