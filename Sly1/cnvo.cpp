#include "cnvo.h"

void* NewCnvo()
{
	return new CNVO;
}

void InitCnvo(CNVO* pcnvo)
{
	InitSo(pcnvo);
}

void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pcnvo, pbis);
	pcnvo->svuAvg = pbis->F32Read();
	pcnvo->svvAvg = pbis->F32Read();
}

void DeleteCnvo(LO* plo)
{
	delete (CNVO*)plo;
}
