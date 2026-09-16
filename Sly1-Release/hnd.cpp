#include "hnd.h"

HND* NewHnd()
{
	return new HND{};
}

void InitHnd(HND* phnd)
{
	InitTarget(phnd);
	phnd->grftak = 1;
	phnd->dzJumpTargetMax = FLT_MAX;
	phnd->sfxidGrab = (SFXID)8;
	phnd->sfxidRelease = (SFXID)9;
}

// Function bodies

void* GetHndHndk(HND* phnd)
{
	return &phnd->hndk;
}

void SetHndHndk(HND* phnd, HNDK hndk)
{
	phnd->hndk = hndk;
}

void* GetHndSvzJumpBoost(HND* phnd)
{
	return &phnd->svzJumpBoost;
}

void SetHndSvzJumpBoost(HND* phnd, float svzJumpBoost)
{
	phnd->svzJumpBoost = svzJumpBoost;
}

void* GetHndSfxidGrab(HND* phnd)
{
	return &phnd->sfxidGrab;
}

void SetHndSfxidGrab(HND* phnd, int sfxidGrab)
{
	phnd->sfxidGrab = (SFXID)sfxidGrab;
}

void* GetHndSfxidRelease(HND* phnd)
{
	return &phnd->sfxidRelease;
}

void SetHndSfxidRelease(HND* phnd, int sfxidRelease)
{
	phnd->sfxidRelease = (SFXID)sfxidRelease;
}

void* GetHndSRadiusHook(HND* phnd)
{
	return &phnd->sRadiusHook;
}

void SetHndSRadiusHook(HND* phnd, float sRadiusHook)
{
	phnd->sRadiusHook = sRadiusHook;
}

void* GetHndDzJumpTargetMax(HND* phnd)
{
	return &phnd->dzJumpTargetMax;
}

void SetHndDzJumpTargetMax(HND* phnd, float dzJumpTargetMax)
{
	phnd->dzJumpTargetMax = dzJumpTargetMax;
}

void* GetHndUnkFloat1(HND* phnd)
{
	return &phnd->unkFloat1;
}

void SetHndUnkFloat1(HND* phnd, int unkFloat1)
{
	phnd->unkFloat1 = static_cast<float>(unkFloat1);
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
	phnd->dzJumpTargetMax = phndBase->dzJumpTargetMax;
	phnd->unkFloat1 = phndBase->unkFloat1;
}

void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis)
{
	LoadXfmFromBrx(phnd, pbis);
}

void DeleteHnd(HND* phnd)
{
	delete phnd;
}
