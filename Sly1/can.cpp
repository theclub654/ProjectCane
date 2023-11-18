#include "can.h"

void InitCan(CAN* pcan)
{
	std::cout << "CAN Size: " << sizeof(CAN) << "\n";
	InitBreak(pcan);
}

void DeleteCan(LO* plo)
{
	delete (CAN*)plo;
}
