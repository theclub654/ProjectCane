#pragma once
#include "pnt.h"

class SPIRE : public PNT
{
	public:
		DLE dleSpire;
};

SPIRE*NewSpire();
void InitSwSpireDl(SW* psw);
int  GetSpireSize();
void OnSpireAdd(SPIRE* pspire);
void OnSpireRemove(SPIRE* pspire);
void CloneSpire(SPIRE* pspire, SPIRE* pspireBase);
void DeleteSpire(LO* plo);