#include "pnt.h"

void* NewPnt()
{
	return new PNT;
}

void LoadPntFromBrx(PNT* pnt, CBinaryInputStream* pbis)
{
	pnt->posLocal = pbis->ReadVector();
	LoadOptionFromBrx(pnt, pbis);
}

void DeletePnt(LO* plo)
{
	delete(PNT*)plo;
}
