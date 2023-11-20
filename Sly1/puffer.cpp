#include "puffer.h"

void InitPuffer(PUFFER* ppuffer)
{
	InitPo(ppuffer);
}

void DeletePuffer(LO* plo)
{
	delete (PUFFER*)plo;
}

void DeletePuffb(LO* plo)
{
	delete(PUFFB*)plo;
}

void InitPuffv(PUFFV* ppuffv)
{
	InitXfm(ppuffv);
}

void DeletePuffv(LO* plo)
{
	delete(PUFFV*)plo;
}

void* CreatePuffc()
{
	PUFFC puffc;
	return &puffc;
}

void LoadPufferFromBrx(PUFFER *ppuffer, CBinaryInputStream *pbis)
{
	/*std::cout << "PUFFER Size: " << sizeof(PUFFER) << "\n";
	std::cout << "PUFFC Size: " << sizeof(PUFFC) << "\n";
	std::cout << "PUFFV Size: " << sizeof(PUFFV) << "\n";*/
	LoadSoFromBrx(ppuffer, pbis);
}

void DeletePuffc(LO* plo)
{
	delete (PUFFC*)plo;
}

void DeletePufft(LO* plo)
{
	delete (PUFFT*)plo;
}
