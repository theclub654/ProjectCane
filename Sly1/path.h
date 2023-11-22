#pragma once
#include "lo.h"

struct CG 
{
    int ccgv;
    struct CGV* acgv;
    int ccge;
    int ccgeBoundary;
    struct CGE* acge;
    int ccgt;
    struct CGT* acgt;
    int ccbsp;
    struct CBSP* acbsp;
};

class PATHZONE : public LO
{
	public:
		CG cg;
		DLE dlePathzone;
};

void* NewPathzone();
void OnPathzoneAdd(PATHZONE* ppathzone);
void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis);
void DeletePathzone(LO* plo);