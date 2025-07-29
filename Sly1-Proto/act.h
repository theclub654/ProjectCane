#pragma once
#include "dl.h"

struct ACT
{
	union
	{

	};

    struct ALO* palo;
    DLE dleAlo;
    int nPriority;
    float tMatch;
};

void AdaptAct(ACT* pact);