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
	LoadSoFromBrx(ppuffer, pbis);
}
