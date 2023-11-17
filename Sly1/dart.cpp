#include "dart.h"

void InitDart(DART* pdart)
{
	//std::cout << "DART Size: " << sizeof(DART) << "\n";
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