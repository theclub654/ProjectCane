#pragma once
#include "pnt.h"

class SPIRE : public PNT
{
	public:
		DLE dleSpire;
};

void OnSpireAdd(SPIRE* pspire);
void DeleteSpire(LO* plo);