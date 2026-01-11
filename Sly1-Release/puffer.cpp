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
	ClonePo(ppuffer, ppufferBase);

	// Shallow copy fields
	ppuffer->normalGround = ppufferBase->normalGround;
	ppuffer->pvolCollect = ppufferBase->pvolCollect;
	ppuffer->pwater = ppufferBase->pwater;
	ppuffer->pvolFire = ppufferBase->pvolFire;

	// Copy the pointers for appntFloor
	for (int i = 0; i < 4; ++i)
	{
		ppuffer->appntFloor[i] = ppufferBase->appntFloor[i];
	}

	// Shallow copy other ALO, ACTLA, EMITTER pointers
	ppuffer->paloFireGun = ppufferBase->paloFireGun;
	ppuffer->paloFireGunTarget = ppufferBase->paloFireGunTarget;
	ppuffer->pactlaFireGun = ppufferBase->pactlaFireGun;
	ppuffer->pemitterFire = ppufferBase->pemitterFire;
	ppuffer->pemitterSpray = ppufferBase->pemitterSpray;
	ppuffer->ppntFireNatural = ppufferBase->ppntFireNatural;
	ppuffer->ppufft = ppufferBase->ppufft;
	ppuffer->paloJt = ppufferBase->paloJt;
	ppuffer->paloGut = ppufferBase->paloGut;
	ppuffer->pactadjGut = ppufferBase->pactadjGut;
	ppuffer->paloHead = ppufferBase->paloHead;
	ppuffer->pactadjHead = ppufferBase->pactadjHead;

	// Copy scalar values
	ppuffer->npuffcEaten = ppufferBase->npuffcEaten;
	ppuffer->npufftLit = ppufferBase->npufftLit;
	ppuffer->npufftMax = ppufferBase->npufftMax;
	ppuffer->tGameMax = ppufferBase->tGameMax;
}

void PostPufferLoad(PUFFER* ppuffer)
{
	PostAloLoad(ppuffer);
}

void UpdatePuffer(PUFFER* ppuffer, float dt)
{
	UpdatePo(ppuffer, dt);
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
	CloneAlo(ppuffb, ppuffbBase);

	ppuffb->cppathzone = ppuffbBase->cppathzone;
	ppuffb->cppuffv = ppuffbBase->cppuffv;
	ppuffb->cppuffc = ppuffbBase->cppuffc;
	ppuffb->tSpawnNext = ppuffbBase->tSpawnNext;

	ppuffb->appathzone = ppuffbBase->appathzone;
	ppuffb->mpippathzonecpuffc = ppuffbBase->mpippathzonecpuffc;
	ppuffb->appuffv = ppuffbBase->appuffv;
	ppuffb->appuffc = ppuffbBase->appuffc;
}

void PostPuffbLoad(PUFFB* ppuffb)
{
	PostAloLoad(ppuffb);
}

void UpdatePuffb(PUFFB* ppuffb, float dt)
{
	UpdateAlo(ppuffb, dt);
}

void DeletePuffb(PUFFB* ppuffb)
{
	delete ppuffb;
}

PUFFV* NewPuffv()
{
	return new PUFFV{};
}

void InitPuffv(PUFFV* ppuffv)
{
	InitSo(ppuffv);
}

int GetPuffvSize()
{
	return sizeof(PUFFV);
}

void ClonePuffv(PUFFV* ppuffv, PUFFV* ppuffvBase)
{
	CloneSo(ppuffv, ppuffvBase);

	// Shallow copy of the value members
	ppuffv->oidPathzone = ppuffvBase->oidPathzone;
	ppuffv->oidPuffvJump = ppuffvBase->oidPuffvJump;
	ppuffv->coidPuffvLand = ppuffvBase->coidPuffvLand;
	ppuffv->cppuffvLand = ppuffvBase->cppuffvLand;
	ppuffv->tChosen = ppuffvBase->tChosen;

	// Shallow copy of pointer members
	ppuffv->ppathzone = ppuffvBase->ppathzone;
	ppuffv->ppuffvJump = ppuffvBase->ppuffvJump;
	std::copy(std::begin(ppuffvBase->aoidPuffvLand), std::end(ppuffvBase->aoidPuffvLand), std::begin(ppuffv->aoidPuffvLand));
	std::copy(std::begin(ppuffvBase->appuffvLand), std::end(ppuffvBase->appuffvLand), std::begin(ppuffv->appuffvLand));
}

void PostPuffvLoad(PUFFV* ppuffv)
{
	PostLoLoad(ppuffv);
}

void DeletePuffv(PUFFV* ppuffv)
{
	delete ppuffv;
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
	CloneStepguard(ppuffc, ppuffcBase);

	// Shallow copy of the value members
	ppuffc->posPathzoneNext = ppuffcBase->posPathzoneNext;

	// Shallow copy of the pointer members
	ppuffc->pwater = ppuffcBase->pwater;
	ppuffc->ppathzoneNext = ppuffcBase->ppathzoneNext;
	ppuffc->ppuffvNext = ppuffcBase->ppuffvNext;
	ppuffc->ppuffb = ppuffcBase->ppuffb;
}

void PostPuffcLoad(PUFFC* ppuffc)
{
	PostStepguardLoad(ppuffc);
	ppuffc->pvtlo->pfnRemoveLo(ppuffc);
}

void UpdatePuffc(PUFFC* ppuffc, float dt)
{
	UpdateStepguard(ppuffc, dt);
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
	CloneSo(ppufft, ppufftBase);

	ppufft->fLit = ppufftBase->fLit;

	ppufft->ppntFire = ppufftBase->ppntFire;
	ppufft->pemitterFire = ppufftBase->pemitterFire;
}

void PostPufftLoad(PUFFT* ppufft)
{
	PostAloLoad(ppufft);
}

void DeletePufft(PUFFT *ppufft)
{
	delete ppufft;
}