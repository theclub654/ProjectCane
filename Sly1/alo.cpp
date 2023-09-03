#include "alo.h"

ALO::ALO(CID cid, SW* psw, ALO* paloParent, OID oid)
{
	this->cid = cid;
	this->psw = psw;
	this->paloParent;
	this->oid = oid;
}

void RemoveAloHierarchy(ALO* palo)
{
	DLI plo;

	// Loading objects child list
	plo.m_pdl = &palo->dlChild;
	// Loading base offset to object child
	plo.m_ibDle = palo->dlChild.ibDle;
	plo.m_pdliNext = s_pdliFirst;

	// Storing parent object in global parent list
	s_pdliFirst = &plo;
	plo.m_ppv = (void**)plo.m_pdl;

	// Loading object header from RIPG object
	LO *LocalObject = (LO*)palo;

	while (true)
	{
		switch (LocalObject->cid)
		{

		}

		break;
	}
}

void InitAlo(ALO* palo)
{
	InitDl(&palo->dlChild, 0x1C);
	InitDl(&palo->dlFreeze, 0x64);
}

void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis)
{

}
