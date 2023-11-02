#pragma once
#include "alo.h"
#include "po.h"

class MGC : public ALO
{
	public:
};

class MGV : public PO
{
	public:
};

void InitMgc(MGC* pmgc);
void InitMgv(MGV *pmgv);
void LoadMgcFromBrx(MGC* pmgc, CBinaryInputStream* pbis);
