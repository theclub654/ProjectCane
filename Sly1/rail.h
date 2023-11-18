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


void OnRailAdd(RAIL* prail);
void DeleteRail(LO* plo);