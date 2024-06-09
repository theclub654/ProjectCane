#pragma once
#include "lo.h"

struct CBSP 
{
    glm::vec3 normal;
    float gDot;

    union
    {
        struct CBSP* pcbspNeg;
        struct CGT* pcgtNeg;
    };

    union 
    {
        struct CBSP* pcbspPos;
        struct CGT* pcgtPos;
    };
};

struct CGT 
{
    struct CGV* apcgv[3];
};

struct CGE 
{
    struct CGV* apcgv[2];
    struct CGT* apcgt[2];
    float s;
};

struct CGV 
{
    glm::vec3 pos;
    int cpcge;
    struct CGE** apcge;
};

struct CG 
{
    int ccgv;
    struct std::vector <CGV> acgv;
    int ccge;
    int ccgeBoundary;
    std::vector <CGE> acge;
    int ccgt;
    std::vector <CGT> acgt;
    int ccbsp;
    struct CBSP* acbsp;
};

class PATHZONE : public LO
{
	public:
		CG cg;
		DLE dlePathzone;
};

PATHZONE*NewPathzone();
void InitSwPathzoneDl(SW* psw);
int  GetPathzoneSize();
void OnPathzoneAdd(PATHZONE* ppathzone);
void OnPathzoneRemove(PATHZONE* ppathzone);
void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis);
void ClonePathzone(PATHZONE* ppathzone, PATHZONE* ppathzoneBase);
void DeletePathzone(LO* plo);