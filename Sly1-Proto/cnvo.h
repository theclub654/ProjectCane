#pragma once
#include "so.h"

class CNVO : public SO
{
	public:
		float svBelt;
		float svuAvg;
		float svvAvg;
};

CNVO*NewCnvo();
void InitCnvo(CNVO* pcnvo);
int  GetCnvoSize();
void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis);
void CloneCnvo(CNVO* pcnvo, CNVO* pcnvoBase);
void PostCnvoLoad(CNVO* pcnvo);
void DeleteCnvo(CNVO *pcnvo);