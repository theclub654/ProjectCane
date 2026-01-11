#include "dart.h"

DART* NewDart()
{
	return new DART{};
}

void InitSwDartFreeDl(SW* psw)
{
	InitDl(&psw->dlRat, offsetof(DART, dleDartFree));
}

void InitDart(DART* pdart)
{
	InitSo(pdart);
}

int GetDartSize()
{
	return sizeof(DART);
}

void OnDartAdd(DART* pdart)
{
	OnSoAdd(pdart);
}

void RemoveDart(DART* pdart)
{
	RemoveLo(pdart);
	AppendDlEntry(&pdart->psw->dlDartFree, pdart);
}

void CloneDart(DART* pdart, DART* pdartBase)
{
    CloneSo(pdart, pdartBase);

    pdart->darts = pdartBase->darts;
    pdart->tDarts = pdartBase->tDarts;
    pdart->svDart = pdartBase->svDart;
    pdart->sRadiusFireCloud = pdartBase->sRadiusFireCloud;
    pdart->sRadiusStrikeCloud = pdartBase->sRadiusStrikeCloud;
    pdart->dtFade = pdartBase->dtFade;
    pdart->dtMaxStuck = pdartBase->dtMaxStuck;

    pdart->dleDartFree = pdartBase->dleDartFree;
    pdart->pasegSticking = pdartBase->pasegSticking;
    pdart->pasegaSticking = pdartBase->pasegaSticking;
    pdart->paloTarget = pdartBase->paloTarget;

    pdart->dtLaunchToTarget = pdartBase->dtLaunchToTarget;
    pdart->dzTarget = pdartBase->dzTarget;

    pdart->pdartgunFiredFrom = pdartBase->pdartgunFiredFrom;
    pdart->pexpl = pdartBase->pexpl;
}

void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pdart, pbis);
}

void PostDartLoad(DART* pdart)
{
    PostAloLoad(pdart);
}

void UpdateDart(DART* pdart, float dt)
{
    UpdateSo(pdart, dt);
}

void DeleteDart(DART *pdart)
{
	delete pdart;
}
