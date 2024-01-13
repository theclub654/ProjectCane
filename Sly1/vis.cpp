#include "vis.h"

void* NewVismap()
{
	return new VISMAP{};
}

void InitVismap(VISMAP *pvismap)
{
	InitLo((LO*)pvismap);
	pvismap->psw->pvismap = pvismap;
}

int GetVismapSize()
{
	return sizeof(VISMAP);
}

void LoadVismapFromBrx(VISMAP *pvismap, CBinaryInputStream *pbis)
{
	pbis->ReadVbsp();

	uint16_t cgrfzon = pbis->U16Read();
	
	for (int i = 0; i < cgrfzon; i++)
		pbis->U32Read();

	pbis->U16Read();
}

void DeleteVismap(LO* plo)
{
	delete(VISMAP*)plo;
}
