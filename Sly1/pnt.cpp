#include "pnt.h"

void* NewPnt()
{
	return new PNT{};
}

int GetPntSize()
{
	return sizeof(PNT);
}

void LoadPntFromBrx(PNT* pnt, CBinaryInputStream* pbis)
{
	pnt->posLocal = pbis->ReadVector();
	LoadOptionsFromBrx(pnt, pbis);
}

void ClonePnt(PNT* pnt, PNT* pntBase)
{
	LO lo = *pnt;
	*pnt = *pntBase;
	memcpy(pnt, &lo, sizeof(LO));

	CloneLo(pnt, pntBase);
}

void SetPntParent(PNT* ppnt, ALO* paloParent)
{
	ConvertAloPos(ppnt->paloParent, paloParent, ppnt->posLocal, ppnt->posLocal);
	SetLoParent(ppnt, paloParent);
}

void ApplyPntProxy(PNT* ppnt, PROXY* pproxyApply)
{

}

void DeletePnt(LO* plo)
{
	delete(PNT*)plo;
}
