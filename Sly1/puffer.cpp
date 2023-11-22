#include "puffer.h"

void* NewPuffer()
{
	return new PUFFER;
}

void InitPuffer(PUFFER* ppuffer)
{
	InitPo(ppuffer);
}

void DeletePuffer(LO* plo)
{
	delete (PUFFER*)plo;
}

void* NewPuffb()
{
	return new PUFFB;
}

void DeletePuffb(LO* plo)
{
	delete(PUFFB*)plo;
}

void* NewPuffv()
{
	return new PUFFV;
}

void InitPuffv(PUFFV* ppuffv)
{
	InitXfm(ppuffv);
}

void DeletePuffv(LO* plo)
{
	delete(PUFFV*)plo;
}

void* NewPuffc()
{
	return new PUFFC;
}

void LoadPufferFromBrx(PUFFER *ppuffer, CBinaryInputStream *pbis)
{
	LoadSoFromBrx(ppuffer, pbis);
}

void* NewPufft()
{
	return new PUFFT;
}

void DeletePuffc(LO* plo)
{
	delete (PUFFC*)plo;
}

void DeletePufft(LO* plo)
{
	delete (PUFFT*)plo;
}
