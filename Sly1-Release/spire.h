#pragma once
#include "pnt.h"

class SPIRE : public PNT
{
	public:
	DLE dleSpire;
	float dzJumpTargetMax;
	int unkInt;
};

SPIRE*NewSpire();
void InitSpire(SPIRE* pspire);
void*GetSpireDzJumpTargetMax(SPIRE* pspire);
void SetSpireDzJumpTargetMax(SPIRE* pspire, float dzJumpTargetMax);
void*GetSpireUnkInt(SPIRE* pspire);
void SetSpireUnkInt(SPIRE* pspire, int unkInt);
int  GetSpireSize();
void OnSpireAdd(SPIRE* pspire);
void OnSpireRemove(SPIRE* pspire);
void CloneSpire(SPIRE* pspire, SPIRE* pspireBase);
void DeleteSpire(SPIRE* pspire);