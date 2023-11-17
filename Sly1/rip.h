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

class RIPG : public SO
{
	public:
		RIPGT ripgt;
		float sExpand;
		DL dlRip;
		class RIPG *pripgNext;
};

void InitRipg(RIPG *pripg); // NOT FINISHED
void OnRipgRemove(RIPG *pripg); // NOT FINISHED