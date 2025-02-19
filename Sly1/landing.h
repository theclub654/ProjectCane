#pragma once
#include "shape.h"

class LANDING : public SHAPE
{
	public:
		DLE dleLanding;
};

LANDING*NewLanding();
void InitSwLandingDl(SW* psw);
int  GetLandingSize();
void OnLandingAdd(LANDING* planding);
void OnLandingRemove(LANDING* planding);
void CloneLanding(LANDING* planding, LANDING* plandingBase);
void DeleteLanding(LANDING* planding);