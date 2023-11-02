#pragma once
#include "lo.h"

class PATHZONE : public LO
{
	public: 
};

void OnPathzoneAdd(PATHZONE* ppathzone);
void LoadPathZoneFromBrx(PATHZONE* ppathzone, CBinaryInputStream* pbis);