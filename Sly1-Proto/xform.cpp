#include "xform.h"

XFM* NewXfm()
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
	LoadOptionsFromBrx(pxfm, pbis);
}

void CloneXfm(XFM* pxfm, XFM* pxfmBase)
{
	CloneLo(pxfm, pxfmBase);

	pxfm->posLocal = pxfmBase->posLocal;
	pxfm->matLocal = pxfmBase->matLocal;
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

void DeleteXfm(XFM* pxfm)
{
	delete pxfm;
}

WARP* NewWarp()
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

	LoadOptionsFromBrx(pwarp, pbis);

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
	CloneXfm(pwarp, pwarpBase);

	pwarp->widmenu = pwarpBase->widmenu;
	pwarp->v = pwarpBase->v;
	pwarp->fDefault = pwarpBase->fDefault;
	pwarp->radCmInit = pwarpBase->radCmInit;
	pwarp->cpaseg = pwarpBase->cpaseg;

	// Clone apaseg (array of void pointers)
	if (pwarp->apaseg != nullptr && pwarpBase->apaseg != nullptr) {
		for (int i = 0; i < pwarpBase->cpaseg; i++) {
			pwarp->apaseg[i] = pwarpBase->apaseg[i];
		}
	}

	pwarp->coidHide = pwarpBase->coidHide;

	// Clone aoidHide (vector of OIDs)
	pwarp->aoidHide = pwarpBase->aoidHide;

	pwarp->coidShowWhenDifficult = pwarpBase->coidShowWhenDifficult;

	// Clone aoidShowWhenDifficult (array of OIDs)
	for (int i = 0; i < 4; i++) {
		pwarp->aoidShowWhenDifficult[i] = pwarpBase->aoidShowWhenDifficult[i];
	}

	pwarp->oidAlias = pwarpBase->oidAlias;
	pwarp->crsmg = pwarpBase->crsmg;

	// Clone RSMG array
	for (int i = 0; i < 4; i++) {
		pwarp->arsmg[i] = pwarpBase->arsmg[i];
	}
}

void DeleteWarp(WARP* pwarp)
{
	delete pwarp;
}

EXIT* NewExit()
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

	LoadOptionsFromBrx(pexit, pbis);

	uint16_t numObjs = pbis->S16Read();

	for (int i = 0; i < numObjs; i++)
	{
		CID cid = (CID)pbis->S16Read();
		OID oid = (OID)pbis->S16Read();
		uint16_t isplice = pbis->S16Read();

		LO* plo = PloNew(cid, pexit->psw, pexit->paloParent, oid, isplice);
		plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);
		plo->pvtlo->pfnRemoveLo(plo);
	}

}

void CloneExit(EXIT* pexit, EXIT* pexitBase)
{
	CloneAlo(pexit, pexitBase);

	pexit->fDefault = pexitBase->fDefault;
	pexit->fKeyed = pexitBase->fKeyed;
	pexit->fFollowDefault = pexitBase->fFollowDefault;
	pexit->fTotals = pexitBase->fTotals;
	pexit->exits = pexitBase->exits;
	pexit->tExits = pexitBase->tExits;
	pexit->ctsurf = pexitBase->ctsurf;

	// Clone atsurf (pointer to surface data)
	pexit->atsurf = pexitBase->atsurf;

	pexit->ctbsp = pexitBase->ctbsp;

	// Clone atbsp (pointer to BSP data)
	pexit->atbsp = pexitBase->atbsp;

	pexit->widWarp = pexitBase->widWarp;
	pexit->oidWarp = pexitBase->oidWarp;
	pexit->edkAlt = pexitBase->edkAlt;
	pexit->widAlt = pexitBase->widAlt;
	pexit->oidAlt = pexitBase->oidAlt;
	pexit->cpaseg = pexitBase->cpaseg;

	// Clone apaseg (array of void pointers)
	if (pexit->apaseg != nullptr && pexitBase->apaseg != nullptr) {
		for (int i = 0; i < pexitBase->cpaseg; i++) {
			pexit->apaseg[i] = pexitBase->apaseg[i];
		}
	}

	pexit->wipek = pexitBase->wipek;
	pexit->tWipe = pexitBase->tWipe;
	pexit->dtUnblock = pexitBase->dtUnblock;
	pexit->dtTriggerWipe = pexitBase->dtTriggerWipe;
}

void UpdateExit(EXIT* pexit, float dt)
{
	
}

void DeleteExit(EXIT* pexit)
{
	delete pexit;
}

CAMERA* NewCamera()
{
	return new CAMERA{};
}

int GetCameraSize()
{
	return sizeof(CAMERA);
}

void CloneCamera(CAMERA* pcamera, CAMERA* pcameraBase)
{
	CloneAlo(pcamera, pcameraBase);

	pcamera->oidTarget = pcameraBase->oidTarget;
	pcamera->ppntTarget = pcameraBase->ppntTarget;
	pcamera->paloTarget = pcameraBase->paloTarget;
	pcamera->posEye = pcameraBase->posEye;
	pcamera->vecView = pcameraBase->vecView;
	pcamera->vecUp = pcameraBase->vecUp;
	pcamera->fSetCplcy = pcameraBase->fSetCplcy;
}

void InitCamera(CAMERA* pcamera)
{
	InitAlo(pcamera);
	pcamera->oidTarget = OID_Nil;
}

void DeleteCamera(CAMERA *pcamera)
{
	delete pcamera;
}
