#include "cnvo.h"

void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pcnvo, pbis);
	pbis->F32Read();
	pbis->F32Read();
}
