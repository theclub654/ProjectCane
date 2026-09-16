#pragma once
#include "shape.h"

class RAIL : public SHAPE
{
	public:
	DLE dleRail;
	int fSlippery;
	float svMax;
	float rdvGravity;
	float dzJumpTargetMax;
	float unkInt;
	int oidRope;
	class ROPE *prope;
};

RAIL*NewRail();
void InitRail(RAIL* prail);
void*GetRailFSlippery(RAIL* prail);
void SetRailFSlippery(RAIL* prail, int fSlippery);
void*GetRailSvMax(RAIL* prail);
void SetRailSvMax(RAIL* prail, float svMax);
void*GetRailRdvGravity(RAIL* prail);
void SetRailRdvGravity(RAIL* prail, float rdvGravity);
void*GetRailDzJumpTargetMax(RAIL* prail);
void SetRailDzJumpTargetMax(RAIL* prail, float dzJumpTargetMax);
void*GetRailUnkInt(RAIL* prail);
void SetRailUnkInt(RAIL* prail, int unkInt);
void SetRailOidRope(RAIL* prail, int oidRope);
void*GetRailPrope(RAIL* prail);
void SetRailPrope(RAIL* prail, ROPE* prope);
int  GetRailSize();
void OnRailAdd(RAIL* prail);
void OnRailRemove(RAIL* prail);
void CloneRail(RAIL* prail, RAIL* prailBase);
void PostRailLoad(RAIL* prail);
void DeleteRail(RAIL* prail);