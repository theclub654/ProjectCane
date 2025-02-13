#include "rog.h"

ROV* NewRov()
{
	return new ROV{};
}

void InitRov(ROV* prov)
{
	InitPo(prov);
}

int GetRovSize()
{
	return sizeof(ROV);
}

void LoadRovFromBrx(ROV* prov, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prov, pbis);
}

void CloneRov(ROV* prov, ROV* provBase)
{
	LO lo = *prov;
	*prov = *provBase;
	memcpy(prov, &lo, sizeof(LO));

	CloneLo(prov, provBase);

	ClearDl(&prov->dlChild);

	prov->pxa = nullptr;
	prov->grfpvaXpValid = 0;
	prov->pstso = nullptr;
}

void DeleteRov(ROV *prov)
{
	delete prov;
}

ROH* NewRoh()
{
	return new ROH{};
}

void InitRoh(ROH* proh)
{
	InitSo(proh);
}

int GetRohSize()
{
	return sizeof(ROH);
}

void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(proh, pbis);
}

void CloneRoh(ROH* proh, ROH* prohBase)
{
	LO lo = *proh;
	*proh = *prohBase;
	memcpy(proh, &lo, sizeof(LO));

	CloneLo(proh, prohBase);

	ClearDl(&proh->dlChild);

	proh->pxa = nullptr;
	proh->grfpvaXpValid = 0;
	proh->pstso = nullptr;
}

void DeleteRoh(ROH *proh)
{
	delete proh;
}

ROC* NewRoc()
{
	return new ROC{};
}

void InitRoc(ROC* proc)
{
	InitSo(proc);
}

int GetRocSize()
{
	return sizeof(ROC);
}

void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(proc, pbis);
}

void CloneRoc(ROC* proc, ROC* procBase)
{
	LO lo = *proc;
	*proc = *procBase;
	memcpy(proc, &lo, sizeof(LO));

	CloneLo(proc, procBase);

	ClearDl(&proc->dlChild);

	proc->pxa = nullptr;
	proc->grfpvaXpValid = 0;
	proc->pstso = nullptr;
}

void DeleteRoc(ROC *proc)
{
	delete proc;
}

ROST* NewRost()
{
	return new ROST{};
}

void InitRost(ROST* prost)
{
	InitSo(prost);
}

int GetRostSize()
{
	return sizeof(ROST);
}

void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prost, pbis);
}

void CloneRost(ROST* prost, ROST* prostBase)
{
	LO lo = *prost;
	*prost = *prostBase;
	memcpy(prost, &lo, sizeof(LO));

	CloneLo(prost, prostBase);

	ClearDl(&prost->dlChild);

	prost->pxa = nullptr;
	prost->grfpvaXpValid = 0;
	prost->pstso = nullptr;
}

void DeleteRost(ROST *prost)
{
	delete prost;
}

ROP* NewRop()
{
	return new ROP{};
}

void InitRop(ROP* prop)
{
	InitSo(prop);
}

int GetRopSize()
{
	return sizeof(ROP);
}

void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prop, pbis);
}

void CloneRop(ROP* prop, ROP* probBase)
{
	LO lo = *prop;
	*prop = *probBase;
	memcpy(prop, &lo, sizeof(LO));

	CloneLo(prop, probBase);

	ClearDl(&prop->dlChild);

	prop->pxa = nullptr;
	prop->grfpvaXpValid = 0;
	prop->pstso = nullptr;
}

void DeleteRop(ROP *prop)
{
	delete prop;
}

ROB* NewRob()
{
	return new ROB{};
}

void InitRob(ROB* prob)
{
	InitAlo(prob);
	InitDl(&prob->dlRocOwned, offsetof(ROC, dleRob));
	InitDl(&prob->dlRocUnowned, offsetof(ROC, dleRob));
	InitDl(&prob->dlRocInactive, offsetof(ROC, dleRob));
	InitDl(&prob->dlRohActive, offsetof(ROH, dleRob));
	InitDl(&prob->dlRohInactive, offsetof(ROH, dleRob));
	InitDl(&prob->dlRostAvailable, offsetof(ROST, dleRob));
	InitDl(&prob->dlRostUnavailable, offsetof(ROST, dleRob));
}

int GetRobSize()
{
	return sizeof(ROB);
}

void CloneRob(ROB* prob, ROB* probBase)
{
	LO lo = *prob;
	*prob = *probBase;
	memcpy(prob, &lo, sizeof(LO));

	CloneLo(prob, probBase);

	ClearDl(&prob->dlChild);
}

void DeleteRob(ROB *prob)
{
	delete prob;
}