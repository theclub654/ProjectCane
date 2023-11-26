#include "chkpnt.h"

void* NewChkpnt()
{
	return new CHKPNT;
}

void InitChkpnt(CHKPNT* pchkpnt)
{
	InitAlo(pchkpnt);
}

void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pchkpnt, pbis);
}

void DeleteChkpnt(LO* plo)
{
	delete(CHKPNT*)plo;
}

void* NewVol()
{
	return new VOL;
}

void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis)
{
	pvol->matLocal = pbis->ReadMatrix();
	pvol->posLocal = pbis->ReadVector();
	LoadTbspFromBrx(pbis);
	LoadOptionFromBrx(pvol, pbis);
}

void DeleteVol(LO* plo)
{
	delete(VOL*)plo;
}
