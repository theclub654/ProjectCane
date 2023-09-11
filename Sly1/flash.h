#pragma once
#include "alo.h"

void LoadTbspFromBrx(CBinaryInputStream *pbis);

class FLASH : public ALO
{
	public:
};

void LoadFlashFromBrx(FLASH* pflash, CBinaryInputStream* pbis);