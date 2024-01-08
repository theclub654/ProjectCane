#pragma once
#include "pnt.h"

class SCAN : public PNT
{
	public:
		TBID tbidCaption;
};

void*NewScan();
int  GetScanSize();
void CloneScan(SCAN* pscan, SCAN* pscanBase);
void DeleteScan(LO* plo);