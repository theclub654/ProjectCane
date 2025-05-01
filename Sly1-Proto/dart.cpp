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

void SnipDartObjects(DART* pdart)
{
	SNIP& snip = s_asnip;
	uint32_t grffso = (snip.grfsnip & 0x1) ? 0x105 : 0x101;

	if ((snip.grfsnip & 0x20) == 0)
		grffso &= ~0x100; // Remove 0x100 if bit 0x20 is not set

	LO* plo = PloFindSwObject(pdart->psw, grffso, snip.oid, pdart);

	if (plo != nullptr)
	{
		if ((snip.grfsnip & 0x08) == 0)
		{
			// Store the pointer to the found object at a specific offset
			// NOTE: Replace this with proper field access if possible.
			pdart->pasegSticking = (ASEG*)plo;
		}

		if ((snip.grfsnip & 0x04) == 0)
			SnipLo(plo);

		if ((snip.grfsnip & 0x10) != 0)
			SubscribeLoObject(plo, pdart);
	}
}

void LoadDartFromBrx(DART* pdart, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pdart, pbis);
}

void DeleteDart(DART *pdart)
{
	delete pdart;
}

SNIP s_asnip = {2, OID_aseg_dart_sticking, offsetof(DART, pasegSticking)};