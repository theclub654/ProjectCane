#include "rog.h"

void InitRov(ROV* prov)
{
	InitPo(prov);
}

void InitRoh(ROH* proh)
{
	InitSo(proh);
}

void InitRoc(ROC* proc)
{
	InitSo(proc);
}

void InitRost(ROST* prost)
{
	InitSo(prost);
}

void InitRop(ROP* prop)
{
	InitSo(prop);
}

void InitRob(ROB* prob)
{
	InitAlo(prob);
}

void LoadRovFromBrx(ROV* prov, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prov, pbis);
}

void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(proh, pbis);
}

void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(proc, pbis);
}

void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prost, pbis);
}

void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prop, pbis);
}
