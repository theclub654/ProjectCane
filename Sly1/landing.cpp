#include "landing.h"

void OnLandingAdd(LANDING* planding)
{
	OnLoAdd(planding);
	AppendDlEntry(&planding->psw->dlLanding, planding);
}
