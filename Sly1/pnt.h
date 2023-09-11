#pragma once
#include "lo.h"

class PNT : public LO
{
	public:
		glm::vec3 posLocal;

		PNT(CID cid, SW* psw, ALO* paloParent, OID oid, int isplice);
};

void LoadPntFromBrx(PNT *pnt, CBinaryInputStream *pbis);