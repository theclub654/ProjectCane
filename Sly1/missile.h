#pragma once
#include "bomb.h"

class MISSILE : public BOMB
{
	public:
};

void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis);