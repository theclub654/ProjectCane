#include "splicemap.h"

void LoadSwSpliceFromBrx(SW *psw, CBinaryInputStream *pbis)
{
	uint32_t unk0 = pbis->U32Read();
	uint32_t unk1 = pbis->U32Read();
	uint32_t unk2 = pbis->U32Read();

	for (int i = 0; i < unk0; i++)
		PpairSerializeIn(pbis);
}
