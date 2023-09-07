#include "vis.h"

void LoadVismapFromBrx(CBinaryInputStream *pbis)
{
	pbis->ReadVbsp();

	uint16_t cgrfzon = pbis->U16Read();
	
	for (int i = 0; i < cgrfzon; i++)
		pbis->U32Read();

	pbis->U16Read();
}