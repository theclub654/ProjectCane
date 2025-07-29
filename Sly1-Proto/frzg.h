#pragma once
#include "alo.h"

class FRZG : public LO
{
	public:
		int coid;
		OID aoid[8];
		MRG mrg;
};

FRZG*NewFrzg();
int  GetFrzgSize();
void CloneFrzg(FRZG* pfrzg, FRZG* pfrzgBase);
void PostFrzgLoad(FRZG* pfrzg);
void DeleteFrzg(FRZG* pfrzg);