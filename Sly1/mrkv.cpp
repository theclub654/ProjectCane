#include "mrkv.h"

void* NewMrkv()
{
	return new MRKV;
}

void InitMrkv(MRKV* pmrkv)
{
	InitSo(pmrkv);
}

void DeleteMrkv(LO* plo)
{
	delete(MRKV*)plo;
}
