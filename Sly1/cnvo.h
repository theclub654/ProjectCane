#pragma once
#include "so.h"

class CNVO : public SO
{
	public:
		float svBelt;
		float svuAvg;
		float svvAvg;
};

void* NewCnvo();
void InitCnvo(CNVO* pcnvo);
void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis);
void DeleteCnvo(LO* plo);