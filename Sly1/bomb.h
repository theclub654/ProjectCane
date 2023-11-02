#pragma once
#include "so.h"

class BOMB : public SO
{
	public:
};

void InitBomb(BOMB* pbomb);
void OnBombAdd(BOMB* pbomb);
void LoadBombFromBrx(BOMB* pbomb, CBinaryInputStream* pbis);