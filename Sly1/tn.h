#pragma once
#include "alo.h"

void LoadCrvlFromBrx(CBinaryInputStream* pbis);
void LoadCrvcFromBrx(CBinaryInputStream* pbis);

class TN : public ALO
{
	public:
};

void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis);
void LoadTbspFromBrx(CBinaryInputStream* pbis);