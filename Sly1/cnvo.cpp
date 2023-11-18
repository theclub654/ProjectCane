#include "cnvo.h"

void InitCnvo(CNVO* pcnvo)
{
	//std::cout << "CNVO Size: " << sizeof(CNVO) << "\n";
	InitSo(pcnvo);
}

void LoadCnvoFromBrx(CNVO* pcnvo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pcnvo, pbis);
	pbis->F32Read();
	pbis->F32Read();
}

void DeleteCnvo(LO* plo)
{
	delete (CNVO*)plo;
}
