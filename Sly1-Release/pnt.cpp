#include "pnt.h"

PNT* NewPnt()
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

void GetPntPos(PNT* ppnt, glm::vec3* ppos)
{
	ConvertAloPos(ppnt->paloParent, nullptr, &ppnt->posLocal, ppos);
}

void* GetPntPosLocal(PNT* ppnt)
{
	return &ppnt->posLocal;
}

void SetPntPosLocal(PNT* ppnt, glm::vec3 *pposlocal)
{
	ppnt->posLocal = *pposlocal;
}

void ClonePnt(PNT* pnt, PNT* pntBase)
{
	CloneLo(pnt, pntBase);

	pnt->posLocal = pntBase->posLocal;
}

void SetPntParent(PNT* ppnt, ALO* paloParent)
{
	ConvertAloPos(ppnt->paloParent, paloParent, &ppnt->posLocal, &ppnt->posLocal);
	SetLoParent(ppnt, paloParent);
}

void ApplyPntProxy(PNT* ppnt, PROXY* pproxyApply)
{
	ConvertAloPos((ALO*)pproxyApply, nullptr, &ppnt->posLocal, &ppnt->posLocal);
}

void DeletePnt(PNT* ppnt)
{
	delete ppnt;
}
