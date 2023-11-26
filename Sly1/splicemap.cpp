#include "splicemap.h"

void LoadSwSpliceFromBrx(SW *psw, CBinaryInputStream *pbis)
{
	psw->csplcSplice = pbis->U32Read();
	psw->symidMax = pbis->U32Read();
	psw->coptidExtra = pbis->U32Read();
	
	for (int i = 0; i < psw->csplcSplice; i++)
		PpairSerializeIn(pbis);
}
