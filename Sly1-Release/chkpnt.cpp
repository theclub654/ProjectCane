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
	LoadSoFromBrx(pchkpnt, pbis);
}

void CloneChkpnt(CHKPNT* pchkpnt, CHKPNT* pchkpntBase)
{
	CloneSo(pchkpnt, pchkpntBase);

	pchkpnt->coidAseg = pchkpntBase->coidAseg;
	for (int i = 0; i < 4; ++i)
		pchkpnt->aoidAseg[i] = pchkpntBase->aoidAseg[i];

	pchkpnt->cpaseg = pchkpntBase->cpaseg;
	for (int i = 0; i < 4; ++i)
		pchkpnt->apaseg[i] = pchkpntBase->apaseg[i];

	pchkpnt->coidVol = pchkpntBase->coidVol;
	for (int i = 0; i < 4; ++i)
		pchkpnt->aoidVol[i] = pchkpntBase->aoidVol[i];

	pchkpnt->cpvol = pchkpntBase->cpvol;
	for (int i = 0; i < 4; ++i)
		pchkpnt->apvol[i] = pchkpntBase->apvol[i];

	pchkpnt->oidWarp = pchkpntBase->oidWarp;
	pchkpnt->ichkTriggered = pchkpntBase->ichkTriggered;
	pchkpnt->ibitChkpnt = pchkpntBase->ibitChkpnt;
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
	pbis->ReadMatrix();
	pbis->ReadVector();
	LoadTbspFromBrx(pbis);
	LoadOptionsFromBrx(pvol, pbis);
}

void CloneVol(VOL* pvol, VOL* pvolBase)
{
	CloneXfm(pvol, pvolBase);

	pvol->ctsurf = pvolBase->ctsurf;
	pvol->atsurf = pvolBase->atsurf;
	pvol->ctbsp = pvolBase->ctbsp;
	pvol->atbsp = pvolBase->atbsp;
}

void DeleteVol(VOL* pvol)
{
	delete pvol;
}