#include "xform.h"

void InitXfm(XFM* pxfm)
{
	//std::cout << "XFM Size: " << sizeof(XFM) << "\n";
	//std::cout << "WARP Size: " << sizeof(WARP) << "\n";
	InitLo(pxfm);
}

void LoadXfmFromBrx(XFM *pxfm, CBinaryInputStream* pbis)
{
	pxfm->matLocal = pbis->ReadMatrix();
	pxfm->posLocal = pbis->ReadVector();
	LoadOptionFromBrx(pxfm, pbis);
}

void DeleteXfm(LO* plo)
{
	delete(XFM*)plo;
}

void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis)
{
	pwarp->matLocal = pbis->ReadMatrix();
	pwarp->posLocal = pbis->ReadVector();

	LoadOptionFromBrx(pwarp, pbis);

	pwarp->cpaseg = pbis->S16Read();

	for (int i = 0; i < pwarp->cpaseg; i++)
	{
		CID cid = (CID)pbis->S16Read();
		OID oid = (OID)pbis->S16Read();
		uint16_t isplice = pbis->S16Read();

		LO* plo = PloNew(cid, pwarp->psw, 0, oid, isplice);
		plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);
	}

	pwarp->widmenu = (WID)pbis->S16Read();

	pwarp->coidHide = pbis->S16Read();
	pwarp->aoidHide.resize(pwarp->coidHide);

	for (int i = 0; i < pwarp->coidHide; i++)
		pwarp->aoidHide[i] = (OID)pbis->S16Read();
}

void DeleteWarp(LO* plo)
{
	delete(WARP*)plo;
}

void LoadExitFromBrx(EXIT* pexit, CBinaryInputStream* pbis)
{
	//std::cout << "EXIT Size: " << sizeof(EXIT) << "\n";
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

void DeleteExit(LO* plo)
{
	delete(EXIT*)plo;
}

void InitCamera(CAMERA* pcamera)
{
	//std::cout << "CAMERA Size: " << sizeof(CAMERA) << "\n";
	InitAlo(pcamera);
	pcamera->oidTarget = OID_Nil;
}

void DeleteCamera(LO* plo)
{
	delete(CAMERA*)plo;
}
