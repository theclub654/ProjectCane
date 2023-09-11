#include "cnvo.h"

void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(0, pbis);
	pbis->F32Read();
	pbis->F32Read();
}
