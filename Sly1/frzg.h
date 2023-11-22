#pragma once
#include "alo.h"

class FRZG : public LO
{
	public:
		int coid;
		OID aoid[8];
		MRG mrg;
};

void* NewFrzg();
void DeleteFrzg(LO* plo);