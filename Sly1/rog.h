#pragma once
#include "po.h"

class ROV : public PO
{
	public:
};

class ROH : public SO
{
	public:
};

class ROC : public SO
{
	public:
};

class ROST : public SO
{
	public:
};

class ROP : public SO
{
	public:
};

void LoadRovFromBrx(ROV *prov, CBinaryInputStream *pbis);
void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis);
void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis);
void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis);
void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis);