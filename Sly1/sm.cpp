#include "sm.h"

void* NewSm()
{
	return new SM{};
}

int GetSmSize()
{
	return sizeof(SM);
}

void LoadSmFromBrx(SM* psm, CBinaryInputStream* pbis)
{
	InitDl(&psm->dlSma, offsetof(SMA, dleSm));

	psm->csms = pbis->U8Read();
	psm->asms.resize(psm->csms);

	for (int i = 0; i < psm->csms; i++)
	{
		psm->asms[i].oid = (OID)pbis->S16Read();
		psm->asms[i].oidNext = (OID)pbis->S16Read();
	}

	psm->csmt = pbis->U8Read();
	psm->asmt.resize(psm->csmt);

	for (int i = 0; i < psm->csmt; i++)
	{
		pbis->S8Read();
		pbis->S16Read();
		pbis->S16Read();
		pbis->S32Read();
		pbis->F32Read();
	}

	LoadOptionFromBrx(psm, pbis);

	int16_t numSmObjs = pbis->S16Read();

	for (int i = 0; i < numSmObjs; i++)
	{
		CID cid = (CID)pbis->S16Read();
		OID oid = (OID)pbis->S16Read();
		uint16_t isplice = pbis->S16Read();

		LO* plo = PloNew(cid, psm->psw, psm->paloParent, oid, isplice);
		plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);
	}
}

void DeleteSm(LO* plo)
{
	delete(SM*)plo;
}

void* NewSma()
{
	return new SMA{};
}

void InitSwSmaDl(SW* psw)
{
	InitDl(&psw->dlSma, offsetof(SMA, dleSw));
}

int GetSmaSize()
{
	return sizeof(SMA);
}

void DeleteSma(LO* plo)
{
	delete(SMA*)plo;
}