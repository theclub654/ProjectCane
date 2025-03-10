#pragma once
#include "pnt.h"

class SCAN : public PNT
{
	public:
		TBID tbidCaption;
};

SCAN*NewScan();
int  GetScanSize();
void CloneScan(SCAN* pscan, SCAN* pscanBase);
void LoadScanFromBrx(SCAN *pscan, CBinaryInputStream *pbis);
void DeleteScan(SCAN* pscan);