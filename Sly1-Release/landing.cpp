#include "landing.h"

LANDING* NewLanding()
{
	return new LANDING{};
}

void InitSwLandingDl(SW* psw)
{
	InitDl(&psw->dlLanding, offsetof(LANDING, dleLanding));
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

void OnLandingRemove(LANDING* planding)
{
	OnLoRemove(planding);
	RemoveDlEntry(&planding->psw->dlLanding, planding);
}

void CloneLanding(LANDING* planding, LANDING* plandingBase)
{
	CloneShape(planding, plandingBase);

	planding->dleLanding = plandingBase->dleLanding;
}

void DeleteLanding(LANDING* planding)
{
	delete planding;
}