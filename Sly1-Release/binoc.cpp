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
	LO lo = *pscan;
	*pscan = *pscanBase;
	memcpy(pscan, &lo, sizeof(LO));

	CloneLo(pscan, pscanBase);
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
