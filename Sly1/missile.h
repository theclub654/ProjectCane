#pragma once
#include "bomb.h"

class MISSILE : public BOMB
{
	public:
};

class ACCMISS : public MISSILE
{
	public:
};

class TARMISS : public ACCMISS
{
	public:
};

class GROUNDMISS : public MISSILE
{
	public:
};

void InitMissile(MISSILE* pmissile);
void InitAccmiss(ACCMISS* paccmiss);
void InitTarmiss(TARMISS* ptarmiss);
void InitGroundmiss(GROUNDMISS* pgroundmiss);
void LoadMissileFromBrx(MISSILE* pmissile, CBinaryInputStream* pbis);