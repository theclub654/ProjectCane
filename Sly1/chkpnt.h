#pragma once
#include "xform.h"

void LoadTbspFromBrx(CBinaryInputStream* pbis);

class CHKPNT : public ALO
{
	public:
};

class VOL : public XFM
{
	public:
};

void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis);
void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis);