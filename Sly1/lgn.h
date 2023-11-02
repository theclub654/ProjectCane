#pragma once
#include "po.h"
#include "break.h"

class LGN : public PO
{
	public:
};

class LGNB : public SO
{
	public:
};

class SWP : public BREAK
{
	public:
};

void InitLgn(LGN* plgn);
void InitLgnb(LGNB* plgnb);
void InitSwp(SWP* pswp);