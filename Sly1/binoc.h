#pragma once
#include "pnt.h"

class SCAN : public PNT
{
	public:
		TBID tbidCaption;
};

void* NewScan();
void DeleteScan(LO* plo);