#pragma once
#include "shape.h"

class LANDING : public SHAPE
{
	public:
		DLE dleLanding;
};

void* NewLanding();
void OnLandingAdd(LANDING* planding);
void DeleteLanding(LO* plo);