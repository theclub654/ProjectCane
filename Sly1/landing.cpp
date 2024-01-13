#include "landing.h"

void* NewLanding()
{
	return new LANDING{};
}

int GetLandingSize()
{
	return sizeof(LANDING);
}

void OnLandingAdd(LANDING* planding)
{
	OnLoAdd(planding);
	AppendDlEntry(&planding->psw->dlLanding, planding);
}

void CloneLanding(LANDING* planding, LANDING* plandingBase)
{
	LO lo = *planding;
	*planding = *plandingBase;
	memcpy(planding, &lo, sizeof(LO));

	CloneLo(planding, plandingBase);
}

void DeleteLanding(LO* plo)
{
	delete(LANDING*)plo;
}
