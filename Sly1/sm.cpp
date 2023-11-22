#include "sm.h"

void* NewSm()
{
	return new SM;
}

void LoadSmFromBrx(SM* psm, CBinaryInputStream* pbis)
{

	byte unk0 = pbis->U8Read();

	for (int i = 0; i < unk0; i++)
	{
		pbis->S16Read();
		pbis->S16Read();
	}

	byte unk1 = pbis->U8Read();

	for (int i = 0; i < unk1; i++)
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
	return new SMA;
}

void DeleteSma(LO* plo)
{
	delete(SMA*)plo;
}
