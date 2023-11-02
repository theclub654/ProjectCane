#pragma once
#include "lo.h"

class RCHM : public LO
{
	public:
};

void InitRchm(RCHM* prchm);
void LoadRchmFromBrx(RCHM* prchm, CBinaryInputStream* pbis);