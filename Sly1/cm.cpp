#include "cm.h"

CM::CM(CID cid, SW* psw, ALO* paloParent, OID oid)
{
	this->cid = cid;
	this->psw = psw;
	this->paloParent;
	this->oid = oid;
}

void InitCm(CM *pcm)
{
	InitLo((LO*)pcm);
}