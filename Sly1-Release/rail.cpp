#include "rail.h"

RAIL* NewRail()
{
	return new RAIL{};
}

void InitRail(RAIL* prail)
{
	InitLo(prail);

	prail->oidRope = -1;
	prail->dzJumpTargetMax = FLT_MAX;
}

void* GetRailFSlippery(RAIL* prail)
{
	return &prail->fSlippery;
}

void SetRailFSlippery(RAIL* prail, int fSlippery)
{
	prail->fSlippery = fSlippery;
}

void* GetRailSvMax(RAIL* prail)
{
	return &prail->svMax;
}

void SetRailSvMax(RAIL* prail, float svMax)
{
	prail->svMax = svMax;
}

void* GetRailRdvGravity(RAIL* prail)
{
	return &prail->rdvGravity;
}

void SetRailRdvGravity(RAIL* prail, float rdvGravity)
{
	prail->rdvGravity = rdvGravity;
}

void* GetRailDzJumpTargetMax(RAIL* prail)
{
	return &prail->dzJumpTargetMax;
}

void SetRailDzJumpTargetMax(RAIL* prail, float dzJumpTargetMax)
{
	prail->dzJumpTargetMax = dzJumpTargetMax;
}

void* GetRailUnkInt(RAIL* prail)
{
	return &prail->unkInt;
}

void SetRailUnkInt(RAIL* prail, int unkInt)
{
	prail->unkInt = unkInt;
}

void SetRailOidRope(RAIL* prail, int oidRope)
{
	prail->oidRope = oidRope;
}

void* GetRailPrope(RAIL* prail)
{
	return &prail->prope;
}

void SetRailPrope(RAIL* prail, ROPE* prope)
{
	prail->prope = prope;
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
	DLE savedDleRail = prail->dleRail;

	CloneShape(prail, prailBase);

	prail->fSlippery = prailBase->fSlippery;
	prail->svMax = prailBase->svMax;
	prail->rdvGravity = prailBase->rdvGravity;

	prail->dleRail = savedDleRail;
}

void PostRailLoad(RAIL* prail)
{
	PostLoLoad(prail);

	if (prail->oidRope != -1) 
		prail->prope = (ROPE*)PloFindSwObject(prail->psw, 4, (OID)prail->oidRope, (LO*)prail);
}

void DeleteRail(RAIL* prail)
{
	delete prail;
}