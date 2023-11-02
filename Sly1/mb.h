#pragma once
#include "stepguard.h"

class MBG : public STEPGUARD
{
	public:
};

class SCENTMAP : public ALO
{
	public:
};

void InitMbg(MBG* pmbg);
void InitBhg(BHG* pbhg);
void InitScentmap(SCENTMAP* pscentmap);
void LoadMbgFromBrx(MBG *pmbg, CBinaryInputStream *pbis);