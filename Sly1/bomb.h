#pragma once
#include "so.h"

class BOMB : public SO
{
	public:
};

void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis);