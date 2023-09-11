#include "crv.h"

void LoadCrvlFromBrx(CBinaryInputStream *pbis)
{
	pbis->U8Read();
    int ccv = pbis->U8Read();

	for (int i = 0; i < ccv; i++)
	{
		pbis->F32Read();
		pbis->ReadVector();
	}
}

void LoadCrvcFromBrx(CBinaryInputStream *pbis)
{
	pbis->U8Read();
	int ccv = pbis->U8Read();

	for (int i = 0; i < ccv; i++)
	{
		pbis->F32Read();
		pbis->ReadVector();
		pbis->ReadVector();
		pbis->ReadVector();
	}
}
