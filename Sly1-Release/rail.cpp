#include "rail.h"

RAIL* NewRail()
{
	return new RAIL{};
}

void InitSwRailDl(SW* psw)
{
	InitDl(&psw->dlRail, offsetof(RAIL, dleRail));
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

void OnRailRemove(RAIL* prail)
{
	OnLoRemove(prail);
	RemoveDlEntry(&prail->psw->dlRail, prail);
}

void CloneRail(RAIL* prail, RAIL* prailBase)
{
	CloneShape(prail, prailBase);

	prail->fSlippery = prailBase->fSlippery;
	prail->svMax = prailBase->svMax;
	prail->rdvGravity = prailBase->rdvGravity;

	prail->dleRail = prailBase->dleRail;
}

void DeleteRail(RAIL* prail)
{
	delete prail;
}