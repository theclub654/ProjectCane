#include "mrkv.h"

void InitMrkv(MRKV* pmrkv)
{
	//std::cout << "MRKV Size: " << sizeof(MRKV) << "\n";
	InitSo(pmrkv);
}

void DeleteMrkv(LO* plo)
{
	delete(MRKV*)plo;
}
