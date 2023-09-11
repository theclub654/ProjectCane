#include "cm.h"

CM::CM(CID cid, SW* psw, ALO* paloParent, OID oid)
{
	this->cid = cid;
	this->psw = psw;
	this->paloParent = paloParent;
	this->oid = oid;
}

void InitCm(CM* pcm)
{
	InitLo((LO*)pcm);

	pcm->radFOV = 1.0;
	pcm->rAspect = 1.298701;
	pcm->sNearClip = 100.0;
	pcm->sFarClip = 20000.0;
	pcm->sNearFog = 500.0;
	pcm->sFarFog = 20000.0;
	pcm->uFogMax = 0.5;
	pcm->rMRD = 1.0;
}