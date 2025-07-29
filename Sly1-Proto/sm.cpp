#include "sm.h"

SM* NewSm()
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

	LoadOptionsFromBrx(psm, pbis);

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

OID OidFromSmIsms(SM* psm, int isms)
{
	return psm->asms[isms].oid;
}

void CloneSm(SM* psm, SM* psmBase)
{
	CloneLo(psm, psmBase);

	psm->csms = psmBase->csms;
	psm->asms = psmBase->asms;
	psm->csmt = psmBase->csmt;
	psm->asmt = psmBase->asmt;
	psm->fDefault = psmBase->fDefault;
	psm->dlSma = psmBase->dlSma;
}

void DeleteSm(SM* psm)
{
	delete psm;
}

SMA* NewSma()
{
	return new SMA{};
}

void InitSwSmaDl(SW* psw)
{
	InitDl(&psw->dlSma, offsetof(SMA, dleSw));
}

void CloneSma(SMA* psma, SMA* psmaBase)
{
	psma->dleSm = psmaBase->dleSm;
	psma->dleSw = psmaBase->dleSw;  
	psma->psm = psmaBase->psm;      
	psma->paloRoot = psmaBase->paloRoot;  
	psma->grfapl = psmaBase->grfapl;  
	psma->pasegaCur = psmaBase->pasegaCur;  
	psma->ismsCur = psmaBase->ismsCur;  
	psma->ismsNext = psmaBase->ismsNext;  
	psma->ismsGoal = psmaBase->ismsGoal;  
	psma->psmtCur = psmaBase->psmtCur;  
	psma->svtLocal = psmaBase->svtLocal;
	psma->pmqFirst = psmaBase->pmqFirst;
}

int GetSmaSize()
{
	return sizeof(SMA);
}

void DeleteSma(LO* plo)
{
	delete(SMA*)plo;
}