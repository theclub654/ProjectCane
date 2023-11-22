#include "dart.h"

void* NewDart()
{
	return new DART;
}

void InitDart(DART* pdart)
{
	InitSo(pdart);
}

void OnDartAdd(DART* pdart)
{
	OnSoAdd(pdart);
}

void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pdart, pbis);
}

void DeleteDart(LO* plo)
{
	delete(DART*)plo;
}
