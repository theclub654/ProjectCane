#include "spire.h"

SPIRE* NewSpire()
{
	return new SPIRE{};
}

void InitSpire(SPIRE* pspire)
{
	// SPIRE has a custom initializer in its vtable, so it must perform the
	// base LO initialization itself. InitLo adds the object to the hierarchy;
	// that dispatches OnSpireAdd and links it into SW::dlSpire.
	InitLo(reinterpret_cast<LO*>(pspire));

	pspire->dzJumpTargetMax = FLT_MAX;
}

void* GetSpireDzJumpTargetMax(SPIRE* pspire)
{
	return &pspire->dzJumpTargetMax;
}

void SetSpireDzJumpTargetMax(SPIRE* pspire, float dzJumpTargetMax)
{
	pspire->dzJumpTargetMax = dzJumpTargetMax;
}

void* GetSpireUnkInt(SPIRE* pspire)
{
	return &pspire->unkInt;
}

void SetSpireUnkInt(SPIRE* pspire, int unkInt)
{
	pspire->unkInt = unkInt;
}

int GetSpireSize()
{
	return sizeof(SPIRE);
}

void OnSpireAdd(SPIRE* pspire)
{
	OnLoAdd(pspire);
	AppendDlEntry(&pspire->psw->dlSpire, pspire);
}

void OnSpireRemove(SPIRE* pspire)
{
	OnLoRemove(pspire);
	RemoveDlEntry(&pspire->psw->dlSpire, pspire);
}

void CloneSpire(SPIRE* pspire, SPIRE* pspireBase)
{
	DLE savedDle = pspire->dleSpire;

	ClonePnt(pspire, pspireBase);

	pspire->dleSpire = savedDle;
}

void DeleteSpire(SPIRE* pspire)
{
	delete pspire;
}
