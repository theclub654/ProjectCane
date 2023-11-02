#include "dart.h"

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
