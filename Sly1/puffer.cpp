#include "puffer.h"

PUFFER* NewPuffer()
{
	return new PUFFER{};
}

void InitPuffer(PUFFER* ppuffer)
{
	InitPo(ppuffer);
}

void LoadPufferFromBrx(PUFFER* ppuffer, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(ppuffer, pbis);
}

void ClonePuffer(PUFFER* ppuffer, PUFFER* ppufferBase)
{
	LO lo = *ppuffer;
	*ppuffer = *ppufferBase;
	memcpy(ppuffer, &lo, sizeof(LO));

	CloneLo(ppuffer, ppufferBase);

	ClearDl(&ppuffer->dlChild);

	ppuffer->pxa = nullptr;
	ppuffer->grfpvaXpValid = 0;
	ppuffer->pstso = nullptr;
}

int GetPufferSize()
{
	return sizeof(PUFFER);
}

void DeletePuffer(PUFFER *ppuffer)
{
	delete ppuffer;
}

PUFFB* NewPuffb()
{
	return new PUFFB{};
}

int GetPuffbSize()
{
	return sizeof(PUFFB);
}

void ClonePuffb(PUFFB* ppuffb, PUFFB* ppuffbBase)
{
	LO lo = *ppuffb;
	*ppuffb = *ppuffbBase;
	memcpy(ppuffb, &lo, sizeof(LO));

	CloneLo(ppuffb, ppuffbBase);

	ClearDl(&ppuffb->dlChild);
}

void DeletePuffb(LO* plo)
{
	delete(PUFFB*)plo;
}

PUFFV* NewPuffv()
{
	return new PUFFV{};
}

void InitPuffv(PUFFV* ppuffv)
{
	InitXfm(ppuffv);
}

int GetPuffvSize()
{
	return sizeof(PUFFV);
}

void ClonePuffv(PUFFV* ppuffv, PUFFV* ppuffvBase)
{
	LO lo = *ppuffv;
	*ppuffv = *ppuffvBase;
	memcpy(ppuffv, &lo, sizeof(LO));

	CloneLo(ppuffv, ppuffvBase);
}

void DeletePuffv(LO* plo)
{
	delete(PUFFV*)plo;
}

PUFFC* NewPuffc()
{
	return new PUFFC{};
}

int GetPuffcSize()
{
	return sizeof(PUFFC);
}

void ClonePuffc(PUFFC* ppuffc, PUFFC* ppuffcBase)
{
	LO lo = *ppuffc;
	*ppuffc = *ppuffcBase;
	memcpy(ppuffc, &lo, sizeof(LO));

	CloneLo(ppuffc, ppuffcBase);

	ClearDl(&ppuffc->dlChild);

	ppuffc->pxa = nullptr;
	ppuffc->grfpvaXpValid = 0;
	ppuffc->pstso = nullptr;
}

void DeletePuffc(PUFFC* ppuffc)
{
	delete ppuffc;
}

PUFFT* NewPufft()
{
	return new PUFFT{};
}

int GetPufftSize()
{
	return sizeof(PUFFT);
}

void ClonePufft(PUFFT* ppufft, PUFFT* ppufftBase)
{
	LO lo = *ppufft;
	*ppufft = *ppufftBase;
	memcpy(ppufft, &lo, sizeof(LO));

	CloneLo(ppufft, ppufftBase);

	ClearDl(&ppufft->dlChild);

	ppufft->pxa = nullptr;
	ppufft->grfpvaXpValid = 0;
	ppufft->pstso = nullptr;
}

void DeletePufft(PUFFT *ppufft)
{
	delete ppufft;
}
