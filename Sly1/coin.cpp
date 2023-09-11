#include "coin.h"

void InitDprize(DPRIZE *pdprize)
{
	pdprize->dprizesInit = DPRIZES_Normal;
	pdprize->dprizes = DPRIZES_Nil;
	pdprize->svcAttract = 30.0;
	pdprize->oidInitialState = OID_Nil;
	InitAlo(pdprize);
	AppendDlEntry(&pdprize->psw->dlDprize, pdprize);
	
}

void LoadDprizeFromBrx(DPRIZE *pdprize, CBinaryInputStream *pbis)
{
	LoadAloFromBrx((ALO*)pdprize, pbis);
}
