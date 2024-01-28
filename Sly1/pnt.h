#pragma once
#include "lo.h"

class PNT : public LO
{
	public:
		glm::vec3 posLocal;
};

void*NewPnt();
int  GetPntSize();
void LoadPntFromBrx(PNT *pnt, CBinaryInputStream *pbis);
void ClonePnt(PNT* pnt, PNT* pntBase);
void SetPntParent(PNT* ppnt, ALO* paloParent);
void ApplyPntProxy(PNT* ppnt, PROXY* pproxyApply);
void DeletePnt(LO* plo);