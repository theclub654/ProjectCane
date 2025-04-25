#include "hnd.h"

HND* NewHnd()
{
	return new HND{};
}

void InitHnd(HND* phnd)
{
	InitTarget(phnd);
}

int GetHndSize()
{
	return sizeof(HND);
}

void CloneHnd(HND* phnd, HND* phndBase)
{
	CloneTarget(phnd, phndBase);

	phnd->hndk = phndBase->hndk;
	phnd->svzJumpBoost = phndBase->svzJumpBoost;
	phnd->sfxidGrab = phndBase->sfxidGrab;
	phnd->sfxidRelease = phndBase->sfxidRelease;
	phnd->sRadiusHook = phndBase->sRadiusHook;
}

void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis)
{
	LoadXfmFromBrx(phnd, pbis);
}

void DeleteHnd(HND* phnd)
{
	delete phnd;
}
