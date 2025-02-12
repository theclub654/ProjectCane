#include "chkpnt.h"

CHKPNT* NewChkpnt()
{
	return new CHKPNT{};
}

void InitChkpnt(CHKPNT* pchkpnt)
{
	InitAlo(pchkpnt);
}

int GetChkpntSize()
{
	return sizeof(CHKPNT);
}

void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pchkpnt, pbis);
}

void CloneChkpnt(CHKPNT* pchkpnt, CHKPNT* pchkpntBase)
{
	LO lo = *pchkpnt;
	*pchkpnt = *pchkpntBase;
	memcpy(pchkpnt, &lo, sizeof(LO));

	CloneLo(pchkpnt, pchkpntBase);

	ClearDl(&pchkpnt->dlChild);
}

void DeleteChkpnt(CHKPNT *pchkpnt)
{
	delete pchkpnt;
}

VOL* NewVol()
{
	return new VOL;
}

int GetVolSize()
{
	return sizeof(VOL);
}

void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis)
{
	pvol->matLocal = pbis->ReadMatrix();
	pvol->posLocal = pbis->ReadVector();
	LoadTbspFromBrx(pbis);
	LoadOptionsFromBrx(pvol, pbis);
}

void CloneVol(VOL* pvol, VOL* pvolBase)
{
	LO lo = *pvol;
	*pvol = *pvolBase;
	memcpy(pvol, &lo, sizeof(LO));

	CloneLo(pvol, pvolBase);
}

void DeleteVol(LO* plo)
{
	delete(VOL*)plo;
}
