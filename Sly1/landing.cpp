#include "landing.h"

void* NewLanding()
{
	return new LANDING;
}

void OnLandingAdd(LANDING* planding)
{
	OnLoAdd(planding);
	AppendDlEntry(&planding->psw->dlLanding, planding);
}

void DeleteLanding(LO* plo)
{
	delete(LANDING*)plo;
}
