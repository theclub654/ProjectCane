#pragma once
#include "stepguard.h"

class MBG : public STEPGUARD
{
	public:
};

void LoadMbgFromBrx(MBG *pmbg, CBinaryInputStream *pbis);