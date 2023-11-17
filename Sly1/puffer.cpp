#include "puffer.h"

void InitPuffer(PUFFER* ppuffer)
{
	InitPo(ppuffer);
}

void InitPuffv(PUFFV* ppuffv)
{
	InitXfm(ppuffv);
}

void LoadPufferFromBrx(PUFFER *ppuffer, CBinaryInputStream *pbis)
{
	/*std::cout << "PUFFER Size: " << sizeof(PUFFER) << "\n";
	std::cout << "PUFFC Size: " << sizeof(PUFFC) << "\n";
	std::cout << "PUFFV Size: " << sizeof(PUFFV) << "\n";*/
	LoadSoFromBrx(ppuffer, pbis);
}
