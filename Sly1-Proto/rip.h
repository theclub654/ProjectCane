#pragma once
#include "so.h"

enum RIPGT 
{
    RIPGT_Nil = -1,
    RIPGT_Default = 0,
    RIPGT_Bounce = 1,
    RIPGT_Loot = 2,
    RIPGT_Max = 3
};
enum RIPT {
    RIPT_Nil = -1,
    RIPT_Rip = 0,
    RIPT_Shadow = 1,
    RIPT_Droplet = 2,
    RIPT_Bublet = 3,
    RIPT_Ripple = 4,
    RIPT_Puff = 5,
    RIPT_Dablet = 6,
    RIPT_Flake = 7,
    RIPT_Spark = 8,
    RIPT_Burst = 9,
    RIPT_Trail = 10,
    RIPT_Fireball = 11,
    RIPT_SmokeCloud = 12,
    RIPT_SmokeTrail = 13,
    RIPT_Debris = 14,
    RIPT_Orbit = 15,
    RIPT_Smack = 16,
    RIPT_Ray = 17,
    RIPT_Rose = 18,
    RIPT_Flying = 19,
    RIPT_Stuck = 20,
    RIPT_Leaf = 21,
    RIPT_Flame = 22,
    RIPT_Bullet = 23,
    RIPT_Shrapnel = 24,
    RIPT_Glint = 25,
    RIPT_Match = 26,
    RIPT_Emitter = 27,
    RIPT_Max = 28
};


class RIPG : public SO
{
	public:
		RIPGT ripgt;
		float sExpand;
		DL dlRip;
		class RIPG *pripgNext;
};

RIPG*NewRipg();
void InitRipg(RIPG *pripg); // NOT FINISHED
int  GetRipgSize();
void CloneRipg(RIPG* pripg, RIPG* pripgBase);
void OnRipgRemove(RIPG *pripg); // NOT FINISHED
void RenderRipgSelf(RIPG* pripg, CM* pcm, RO* pro);
void DeleteRipg(RIPG* pripg);