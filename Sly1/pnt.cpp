#include "pnt.h"

void LoadPntFromBrx(PNT* pnt, CBinaryInputStream* pbis)
{
	//std::cout << "PNT Size: " << sizeof(PNT) << "\n";
	pnt->posLocal = pbis->ReadVector();
	LoadOptionFromBrx(pnt, pbis);
}

void DeletePnt(LO* plo)
{
	delete(PNT*)plo;
}
