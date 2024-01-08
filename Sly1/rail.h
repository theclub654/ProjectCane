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

void*NewRail();
int  GetRailSize();
void OnRailAdd(RAIL* prail);
void CloneRail(RAIL* prail, RAIL* prailBase);
void DeleteRail(LO* plo);