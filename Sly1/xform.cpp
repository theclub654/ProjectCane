#include "xform.h"

void* NewXfm()
{
	return new XFM{};
}

void InitXfm(XFM* pxfm)
{
	InitLo(pxfm);
}

int GetXfmSize()
{
	return sizeof(XFM);
}

void LoadXfmFromBrx(XFM *pxfm, CBinaryInputStream* pbis)
{
	pxfm->matLocal = pbis->ReadMatrix();
	pxfm->posLocal = pbis->ReadVector();
	LoadOptionFromBrx(pxfm, pbis);
}

void CloneXfm(XFM* pxfm, XFM* pxfmBase)
{
	LO lo = *pxfm;
	*pxfm = *pxfmBase;
	memcpy(pxfm, &lo, sizeof(LO));

	CloneLo(pxfm, pxfmBase);
}

void SetXfmParent(XFM* pxfm, ALO* paloParent)
{
	ConvertAloPos(pxfm->paloParent, paloParent, pxfm->posLocal, pxfm->posLocal);
	ConvertAloMat(pxfm->paloParent, paloParent, pxfm->matLocal, pxfm->matLocal);
	SetLoParent(pxfm, paloParent);
}

void ApplyXfmProxy(XFM* pxfm, PROXY* pproxyApply)
{
	ConvertAloPos((ALO*)pproxyApply, nullptr, pxfm->posLocal, pxfm->posLocal);
	ConvertAloMat((ALO*)pproxyApply, nullptr, pxfm->matLocal, pxfm->matLocal);
}

void DeleteXfm(LO* plo)
{
	delete(XFM*)plo;
}

void* NewWarp()
{
	return new WARP{};
}

int GetWarpSize()
{
	return sizeof(WARP);
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

		LO* plo = PloNew(cid, pwarp->psw, nullptr, oid, isplice);
		plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);
	}

	pwarp->widmenu = (WID)pbis->S16Read();

	pwarp->coidHide = pbis->S16Read();
	pwarp->aoidHide.resize(pwarp->coidHide);

	for (int i = 0; i < pwarp->coidHide; i++)
		pwarp->aoidHide[i] = (OID)pbis->S16Read();
}

void CloneWarp(WARP* pwarp, WARP* pwarpBase)
{
	LO lo = *pwarp;
	*pwarp = *pwarpBase;
	memcpy(pwarp, &lo, sizeof(LO));

	CloneLo(pwarp, pwarpBase);
}

void DeleteWarp(LO* plo)
{
	delete(WARP*)plo;
}

void* NewExit()
{
	return new EXIT{};
}

int GetExitSize()
{
	return sizeof(EXIT);
}

void LoadExitFromBrx(EXIT* pexit, CBinaryInputStream* pbis)
{
	pexit->xf.mat = pbis->ReadMatrix();
	pexit->xf.pos = pbis->ReadVector();

	pexit->pvtalo->pfnUpdateAloXfWorld(pexit);

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

void CloneExit(EXIT* pexit, EXIT* pexitBase)
{
	LO lo = *pexit;
	*pexit = *pexitBase;
	memcpy(pexit, &lo, sizeof(LO));

	CloneLo(pexit, pexitBase);

	ClearDl(&pexit->dlChild);
}

void DeleteExit(LO* plo)
{
	delete(EXIT*)plo;
}

void* NewCamera()
{
	return new CAMERA{};
}

int GetCameraSize()
{
	return sizeof(CAMERA);
}

void CloneCamera(CAMERA* pcamera, CAMERA* pcameraBase)
{
	LO lo = *pcamera;
	*pcamera = *pcameraBase;
	memcpy(pcamera, &lo, sizeof(LO));

	CloneLo(pcamera, pcameraBase);

	ClearDl(&pcamera->dlChild);
}

void InitCamera(CAMERA* pcamera)
{
	InitAlo(pcamera);
	pcamera->oidTarget = OID_Nil;
}

void DeleteCamera(LO* plo)
{
	delete(CAMERA*)plo;
}
