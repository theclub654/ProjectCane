#include "rog.h"

void* NewRov()
{
	return new ROV;
}

void InitRov(ROV* prov)
{
	InitPo(prov);
}

void* NewRoh()
{
	return new ROH;
}

void InitRoh(ROH* proh)
{
	InitSo(proh);
}

void DeleteRoh(LO* plo)
{
	delete (ROH*)plo;
}

void* NewRoc()
{
	return new ROC;
}

void InitRoc(ROC* proc)
{
	InitSo(proc);
}

void DeleteRoc(LO* plo)
{
	delete(ROC*)plo;
}

void* NewRost()
{
	return new ROST;
}

void InitRost(ROST* prost)
{
	InitSo(prost);
}

void DeleteRost(LO* plo)
{
	delete(ROST*)plo;
}

void* NewRop()
{
	return new ROP;
}

void InitRop(ROP* prop)
{
	InitSo(prop);
}

void DeleteRop(LO* plo)
{
	delete (ROP*)plo;
}

void* NewRob()
{
	return new ROB;
}

void InitRob(ROB* prob)
{
	InitAlo(prob);
}

void DeleteRob(LO* plo)
{
	delete(ROB*)plo;
}

void LoadRovFromBrx(ROV* prov, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prov, pbis);
}

void DeleteRov(LO* plo)
{
	delete(ROV*)plo;
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
