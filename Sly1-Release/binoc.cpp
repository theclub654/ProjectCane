#include "binoc.h"

SCAN*NewScan()
{
	return new SCAN{};
}

int GetScanSize()
{
	return sizeof(SCAN);
}

void CloneScan(SCAN* pscan, SCAN* pscanBase)
{
	ClonePnt(pscan, pscanBase);

	pscan->tbidCaption = pscanBase->tbidCaption;
}

void LoadScanFromBrx(SCAN* pscan, CBinaryInputStream* pbis)
{
	LoadPntFromBrx(pscan, pbis);
	pbis->ReadStringSw();
}

void DeleteScan(SCAN* pscan)
{
	delete pscan;
}
