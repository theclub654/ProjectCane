#include "can.h"

void* NewCan()
{
	return new CAN;
}

void InitCan(CAN* pcan)
{
	InitBreak(pcan);
}

void DeleteCan(LO* plo)
{
	delete (CAN*)plo;
}
