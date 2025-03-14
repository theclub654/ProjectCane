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
};

HND* NewHnd();
void InitHnd(HND* phnd);
int  GetHndSize();
void CloneHnd(HND* phnd, HND* phndBase);
void LoadHndFromBrx(HND* phnd, CBinaryInputStream* pbis);
void DeleteHnd(HND* phnd);