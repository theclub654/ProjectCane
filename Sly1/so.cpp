#include "so.h"

void SO::so(CID cid, SW* psw, ALO* paloParent, OID oid)
{
	ALocalObject.cid = cid;
	ALocalObject.psw = psw;
	ALocalObject.paloParent = paloParent;
	ALocalObject.oid = oid;
}

void InitSo(SO* pso)
{
	// Setting base offset to dlPhys
	InitDl(&pso->dlPhys, 0x2f4);
	// Initializing static object ALO
	InitAlo(&pso->ALocalObject);
}