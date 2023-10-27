#pragma once
#include "stepguard.h"

class MBG : public STEPGUARD
{
	public:
};

void InitMbg(MBG* pmbg);
void InitBhg(BHG* pbhg);
void LoadMbgFromBrx(MBG *pmbg, CBinaryInputStream *pbis);