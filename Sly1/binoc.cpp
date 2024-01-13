#include "binoc.h"

void* NewScan()
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

void DeleteScan(LO* plo)
{
	delete(SCAN*)plo;
}
