#pragma once
#include "alo.h"

class FRZG : public LO
{
	public:
	int coid;
	short aoid[8];
	MRG mrg;
};

FRZG*NewFrzg();
int  GetFrzgSize();
void CloneFrzg(FRZG* pfrzg, FRZG* pfrzgBase);
void PostFrzgLoad(FRZG* pfrzg);
void AddFrzgObject(FRZG* pfrzg, OID oid);
void DeleteFrzg(FRZG* pfrzg);