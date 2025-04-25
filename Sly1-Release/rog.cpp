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
	ClonePo(prov, provBase); // Assuming ClonePo handles copying the base class (PO) members

	// Shallow copy of value members
	prov->rovs = provBase->rovs;  // Copying the ROVS object
	prov->rovts = provBase->rovts;  // Copying the ROVTS object
	prov->radJoyLeft = provBase->radJoyLeft;
	prov->uJoyDeflectLeft = provBase->uJoyDeflectLeft;
	prov->radJoyRight = provBase->radJoyRight;
	prov->uJoyDeflectRight = provBase->uJoyDeflectRight;
	prov->svMax = provBase->svMax;
	prov->vTarget = provBase->vTarget;

	// Shallow copy of the pointer members (simply copy the pointer, not the data)
	prov->psmBody = provBase->psmBody;
	prov->psmaBody = provBase->psmaBody;
	prov->pambBody = provBase->pambBody;
	prov->paloTurret = provBase->paloTurret;
	prov->psmTurret = provBase->psmTurret;
	prov->psmaTurret = provBase->psmaTurret;
	prov->pambTurret = provBase->pambTurret;
	prov->pemitterLeftGun = provBase->pemitterLeftGun;
	prov->pemitterRightGun = provBase->pemitterRightGun;
	prov->paloProp = provBase->paloProp;
	prov->cpemitterProp = provBase->cpemitterProp;

	// Shallow copy of the array of pointers
	for (int i = 0; i < 3; ++i) {
		prov->apemitterProp[i] = provBase->apemitterProp[i];
	}

	// Copying the linked structure members
	prov->prob = provBase->prob;
	prov->pasegAscend = provBase->pasegAscend;
	prov->pasegDescend = provBase->pasegDescend;
	prov->pasegaDrive = provBase->pasegaDrive;

	// Copying the flag for retracting the drive
	prov->fRetractDrive = provBase->fRetractDrive;
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
	CloneSo(proh, prohBase); // Assuming CloneSo handles copying the base class (SO) members

	// Shallow copy of value members
	proh->rohs = prohBase->rohs;  // Copying the ROHS object
	proh->tRohs = prohBase->tRohs;
	proh->svMax = prohBase->svMax;
	proh->dradMax = prohBase->dradMax;
	proh->posWander = prohBase->posWander;
	proh->tWanderNext = prohBase->tWanderNext;
	proh->lmDtWander = prohBase->lmDtWander;

	// Shallow copy of the pointer members (simply copy the pointer, not the data)
	proh->prob = prohBase->prob;
	proh->proc = prohBase->proc;
	proh->prost = prohBase->prost;
	proh->pasegMove = prohBase->pasegMove;
	proh->pasegMoveCollect = prohBase->pasegMoveCollect;
	proh->pasegGrab = prohBase->pasegGrab;
	proh->pasegDie = prohBase->pasegDie;
	proh->pasega = prohBase->pasega;
	proh->psmHappy = prohBase->psmHappy;
	proh->psmSad = prohBase->psmSad;
	proh->psma = prohBase->psma;
	proh->pexpl = prohBase->pexpl;

	// Shallow copy of the DLE (linked list)
	proh->dleRob = prohBase->dleRob;
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
	CloneSo(proc, procBase); // Assuming CloneSo handles copying the base class (SO) members

	// Shallow copy of value members
	proc->rocs = procBase->rocs;  // Copying the ROCS object
	proc->tRocs = procBase->tRocs;
	proc->cHitMax = procBase->cHitMax;
	proc->dtRocsHitDown = procBase->dtRocsHitDown;
	proc->dtRocsHitUp = procBase->dtRocsHitUp;

	// Shallow copy of the pointer members (simply copy the pointer, not the data)
	proc->prob = procBase->prob;
	proc->proh = procBase->proh;
	proc->psm = procBase->psm;
	proc->psma = procBase->psma;
	proc->pexpl = procBase->pexpl;
	proc->pemitterBubble = procBase->pemitterBubble;

	// Shallow copy of the LM and CLQ objects (assuming they are structures)
	proc->lmSvcBubble = procBase->lmSvcBubble;
	proc->clqBubble = procBase->clqBubble;

	// Shallow copy of the DLE (linked list)
	proc->dleRob = procBase->dleRob;
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
	CloneSo(prost, prostBase); // Assuming CloneSo handles copying the base class (SO) members

	// Shallow copy of value members
	prost->rosts = prostBase->rosts;  // Copying the ROSTS object
	prost->tRosts = prostBase->tRosts;

	// Shallow copy of pointer members (simply copy the pointer, not the data)
	prost->prob = prostBase->prob;
	prost->psm = prostBase->psm;
	prost->psma = prostBase->psma;
	prost->ppnt = prostBase->ppnt;

	// Shallow copy of the DLE (linked list)
	prost->dleRob = prostBase->dleRob;
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
	CloneSo(prop, probBase); // Assuming CloneSo handles copying the base class (SO) members

	// Shallow copy of value members
	prop->rops = probBase->rops;  // Copying the ROPS object
	prop->tRops = probBase->tRops;

	// Shallow copy of pointer members (simply copy the pointer, not the data)
	prop->prob = probBase->prob;
	prop->pkey = probBase->pkey;
	prop->pasegReveal = probBase->pasegReveal;
	prop->pasega = probBase->pasega;
	prop->pexpl = probBase->pexpl;
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
	CloneAlo(prob, probBase); // Assuming CloneAlo handles copying the base class (ALO) members

	// Shallow copy of value members
	prob->oidCollectible = probBase->oidCollectible;
	prob->oidHarvester = probBase->oidHarvester;
	prob->oidVehicle = probBase->oidVehicle;
	prob->oidPrize = probBase->oidPrize;
	prob->coidRost = probBase->coidRost;
	prob->posCenterArena = probBase->posCenterArena;
	prob->sRadiusArenaTube = probBase->sRadiusArenaTube;
	prob->sRadiusArenaRoc = probBase->sRadiusArenaRoc;
	prob->sRadiusArenaWander = probBase->sRadiusArenaWander;
	prob->rAspectArena = probBase->rAspectArena;
	prob->sCollectibleDropHeight = probBase->sCollectibleDropHeight;
	prob->crocActive = probBase->crocActive;
	prob->crocCreateMax = probBase->crocCreateMax;
	prob->crocCreated = probBase->crocCreated;
	prob->crocReturned = probBase->crocReturned;
	prob->crocDestroyed = probBase->crocDestroyed;
	prob->irodd = probBase->irodd;
	prob->crocActiveMax = probBase->crocActiveMax;
	prob->crohActiveMax = probBase->crohActiveMax;
	prob->tAddRoc = probBase->tAddRoc;
	prob->tAddRoh = probBase->tAddRoh;
	prob->svRohMax = probBase->svRohMax;
	prob->dradRohMax = probBase->dradRohMax;
	prob->tRobs = probBase->tRobs;

	// Shallow copy of pointer members (simply copy the pointers)
	prob->prov = probBase->prov;
	prob->prop = probBase->prop;
	prob->dlRocOwned = probBase->dlRocOwned;
	prob->dlRocUnowned = probBase->dlRocUnowned;
	prob->dlRocInactive = probBase->dlRocInactive;
	prob->dlRohActive = probBase->dlRohActive;
	prob->dlRohInactive = probBase->dlRohInactive;
	prob->dlRostAvailable = probBase->dlRostAvailable;
	prob->dlRostUnavailable = probBase->dlRostUnavailable;
	prob->clqTune = probBase->clqTune;
	prob->rodd = probBase->rodd;
	prob->pdialog = probBase->pdialog;
	// Assuming the arrays like arodd and aoidRost are shallow-copied by reference
	std::copy(std::begin(probBase->arodd), std::end(probBase->arodd), std::begin(prob->arodd));
	std::copy(std::begin(probBase->aoidRost), std::end(probBase->aoidRost), std::begin(prob->aoidRost));
}

void DeleteRob(ROB *prob)
{
	delete prob;
}