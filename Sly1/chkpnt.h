#pragma once
#include "xform.h"

void LoadTbspFromBrx(CBinaryInputStream* pbis);

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

void InitChkpnt(CHKPNT* pchkpnt);
void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis);
void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis);