#pragma once
#include "so.h"

class CNVO : public SO
{
	public:
};

void InitCnvo(CNVO* pcnvo);
void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis);