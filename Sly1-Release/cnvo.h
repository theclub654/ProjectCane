#pragma once
#include "so.h"
#include "scroller.h"

class CNVO : public SO
{
	public:
	float svBelt;
	float svuAvg;
	float svvAvg;
};

CNVO*NewCnvo();
void InitCnvo(CNVO* pcnvo);
void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis);
void CloneCnvo(CNVO* pcnvo, CNVO* pcnvoBase);
void PostCnvoLoad(CNVO* pcnvo);
void AdjustCnvoXpVelocity(CNVO* pcnvo, XP* pxp, int ixpd);
int  GetCnvoSize();
void SetCnvoBeltSpeed(CNVO* pcnvo, float svBelt);
void MatchCnvoScrollerToBeltSpeed(CNVO* pcnvo);
void DeleteCnvo(CNVO *pcnvo);