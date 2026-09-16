#pragma once
#include "target.h"

enum HNDK
{
	HNDK_Nil = -1,
	HNDK_Both = 0,
	HNDK_Forward = 1,
	HNDK_Backward = 2,
	HNDK_Max = 3
};

class HND : public TARGET
{
	public:
	HNDK hndk;
	float svzJumpBoost;
	SFXID sfxidGrab;
	SFXID sfxidRelease;
	float sRadiusHook;
	float dzJumpTargetMax;
	float unkFloat1;
};

HND* NewHnd();
void InitHnd(HND* phnd);
void*GetHndHndk(HND* phnd);
void SetHndHndk(HND* phnd, HNDK hndk);
void*GetHndSvzJumpBoost(HND* phnd);
void SetHndSvzJumpBoost(HND* phnd, float svzJumpBoost);
void*GetHndSfxidGrab(HND* phnd);
void SetHndSfxidGrab(HND* phnd, int sfxidGrab);
void*GetHndSfxidRelease(HND* phnd);
void SetHndSfxidRelease(HND* phnd, int sfxidRelease);
void*GetHndSRadiusHook(HND* phnd);
void SetHndSRadiusHook(HND* phnd, float sRadiusHook);
void*GetHndDzJumpTargetMax(HND* phnd);
void SetHndDzJumpTargetMax(HND* phnd, float dzJumpTargetMax);
void*GetHndUnkFloat1(HND* phnd);
void SetHndUnkFloat1(HND* phnd, int unkFloat1);
int  GetHndSize();
void CloneHnd(HND* phnd, HND* phndBase);
void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis);
void DeleteHnd(HND* phnd);