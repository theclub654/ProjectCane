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
	LO lo = *pcnvo;
	*pcnvo = *pcnvoBase;
	memcpy(pcnvo, &lo, sizeof(LO));

	CloneLo(pcnvo, pcnvoBase);

	ClearDl(&pcnvo->dlChild);

	pcnvo->pxa = nullptr;
	pcnvo->grfpvaXpValid = 0;
	pcnvo->pstso = nullptr;
}

void DeleteCnvo(LO* plo)
{
	delete (CNVO*)plo;
}
