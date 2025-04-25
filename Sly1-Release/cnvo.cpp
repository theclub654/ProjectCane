#include "cnvo.h"

CNVO* NewCnvo()
{
	return new CNVO{};
}

void InitCnvo(CNVO* pcnvo)
{
	InitSo(pcnvo);
}

int GetCnvoSize()
{
	return sizeof(CNVO);
}

void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pcnvo, pbis);
	pcnvo->svuAvg = pbis->F32Read();
	pcnvo->svvAvg = pbis->F32Read();
}

void CloneCnvo(CNVO* pcnvo, CNVO* pcnvoBase)
{
	CloneSo(pcnvo, pcnvoBase);

	pcnvo->svBelt = pcnvoBase->svBelt;
	pcnvo->svuAvg = pcnvoBase->svuAvg;
	pcnvo->svvAvg = pcnvo->svvAvg;
}

void DeleteCnvo(CNVO *pcnvo)
{
	delete pcnvo;
}
