#pragma once
#include "xform.h"

class CHKPNT : public ALO
{
	public:
        int coidAseg;
        OID aoidAseg[4];
        int cpaseg;
        struct ASEG* apaseg[4];
        int coidVol;
        OID aoidVol[4];
        int cpvol;
        struct VOL* apvol[4];
        OID oidWarp;
        int ichkTriggered;
        int ibitChkpnt;
};

class VOL : public XFM
{
	public:
        int ctsurf;
        struct TSURF* atsurf;
        int ctbsp;
        struct TBSP* atbsp;
};

CHKPNT* NewChkpnt();
void InitChkpnt(CHKPNT* pchkpnt);
int  GetChkpntSize();
void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis);
void BindChkpnt(CHKPNT* pchkpnt);
void CloneChkpnt(CHKPNT* pchkpnt, CHKPNT* pchkpntBase);
void PostChkpntLoad(CHKPNT* pchkpnt);
void DeleteChkpnt(CHKPNT *pchkpnt);

VOL* NewVol();
int  GetVolSize();
void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis);
void CloneVol(VOL* pvol, VOL* pvolBase);
void DeleteVol(VOL* pvol);