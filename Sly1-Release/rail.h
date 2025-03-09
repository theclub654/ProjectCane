#pragma once
#include "shape.h"

class RAIL : public SHAPE
{
	public:
		DLE dleRail;
		int fSlippery;
		float svMax;
		float rdvGravity;
};

RAIL*NewRail();
void InitSwRailDl(SW* psw);
int  GetRailSize();
void OnRailAdd(RAIL* prail);
void OnRailRemove(RAIL* prail);
void CloneRail(RAIL* prail, RAIL* prailBase);
void DeleteRail(RAIL* prail);