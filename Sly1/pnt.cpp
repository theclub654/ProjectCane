#include "pnt.h"

PNT::PNT(CID cid, SW* psw, ALO* paloParent, OID oid, int isplice)
{
	this->cid = cid;
	this->psw = psw;
	this->paloParent;
	this->oid = oid;
}

void LoadPntFromBrx(PNT* pnt, CBinaryInputStream* pbis)
{
	pbis->ReadVector();
	LoadOptionFromBrx(pnt, pbis);
}
