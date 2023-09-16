#include "pnt.h"

void LoadPntFromBrx(PNT* pnt, CBinaryInputStream* pbis)
{
	pbis->ReadVector();
	LoadOptionFromBrx(pnt, pbis);
}
