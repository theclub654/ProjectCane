#pragma once
#include "pnt.h"

class SPIRE : public PNT
{
	public:
		DLE dleSpire;
};

void*NewSpire();
int  GetSpireSize();
void OnSpireAdd(SPIRE* pspire);
void CloneSpire(SPIRE* pspire, SPIRE* pspireBase);
void DeleteSpire(LO* plo);