#include "cnvo.h"

void InitCnvo(CNVO* pcnvo)
{
	InitSo(pcnvo);
}

void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pcnvo, pbis);
	pbis->F32Read();
	pbis->F32Read();
}
