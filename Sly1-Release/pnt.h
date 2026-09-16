#pragma once
#include "lo.h"

class PNT : public LO
{
	public:
	glm::vec3 posLocal;
};

PNT* NewPnt();
int  GetPntSize();
void LoadPntFromBrx(PNT *pnt, CBinaryInputStream *pbis);
void GetPntPos(PNT* ppnt, glm::vec3* ppos);
void*GetPntPosLocal(PNT* ppnt);
void SetPntPosLocal(PNT* ppnt, glm::vec3 *pposlocal);
void ClonePnt(PNT* pnt, PNT* pntBase);
void SetPntParent(PNT* ppnt, ALO* paloParent);
void ApplyPntProxy(PNT* ppnt, PROXY* pproxyApply);
void DeletePnt(PNT* ppnt);