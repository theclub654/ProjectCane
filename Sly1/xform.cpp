#include "xform.h"

void LoadXfmFromBrx(XFM *pxfm, CBinaryInputStream* pbis)
{
	pbis->ReadMatrix();
	pbis->ReadVector();
	LoadOptionFromBrx(pxfm, pbis);
}

void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis)
{
	pbis->ReadMatrix();
	pbis->ReadVector();

	LoadOptionFromBrx(pwarp, pbis);

	int numObjs = pbis->S16Read();

	for (int i = 0; i < numObjs; i++)
	{
		CID cid = (CID)pbis->S16Read();
		OID oid = (OID)pbis->S16Read();
		uint16_t isplice = pbis->S16Read();

		LO* plo = PloNew(cid, pwarp->psw, 0, oid, isplice);
		plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);
	}

	pbis->S16Read();

	uint16_t coidHide = pbis->S16Read();

	for (int i = 0; i < coidHide; i++)
		pbis->S16Read();
}

void LoadExitFromBrx(EXIT* pexit, CBinaryInputStream* pbis)
{
	pbis->ReadMatrix();
	pbis->ReadVector();

	LoadTbspFromBrx(pbis);

	LoadOptionFromBrx(pexit, pbis);

	uint16_t numObjs = pbis->S16Read();

	for (int i = 0; i < numObjs; i++)
	{
		CID cid = (CID)pbis->S16Read();
		OID oid = (OID)pbis->S16Read();
		uint16_t isplice = pbis->S16Read();

		LO* plo = PloNew(cid, pexit->psw, pexit->paloParent, oid, isplice);
		plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);
	}

}
