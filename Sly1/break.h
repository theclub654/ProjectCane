#pragma once
#include "so.h"

class BRK : public SO
{
	public:
};

class BREAK : public BRK
{
	public:
};

class FRAGILE : public BRK
{
	public:
};

void InitBrk(BRK* pbrk);
void InitBreak(BREAK* pbreak);
void InitFragile(FRAGILE* pfragile);
void LoadBrkFromBrx(BRK* pbrk, CBinaryInputStream* pbis);