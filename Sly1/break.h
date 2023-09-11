#pragma once
#include "so.h"

class BRK : public SO
{
	public:
};

void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis);