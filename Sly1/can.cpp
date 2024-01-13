#include "can.h"

void* NewCan()
{
	return new CAN{};
}

void InitCan(CAN* pcan)
{
	InitBreak(pcan);
}

int GetCanSize()
{
	return sizeof(CAN);
}

void CloneCan(CAN* pcan, CAN* pcanBase)
{
	LO lo = *pcan;
	*pcan = *pcanBase;
	memcpy(pcan, &lo, sizeof(LO));

	CloneLo(pcan, pcanBase);

	ClearDl(&pcan->dlChild);

	pcan->pxa = nullptr;
	pcan->grfpvaXpValid = 0;
	pcan->pstso = nullptr;
}

void DeleteCan(LO* plo)
{
	delete (CAN*)plo;
}
