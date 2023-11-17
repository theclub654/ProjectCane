#include "chkpnt.h"

void InitChkpnt(CHKPNT* pchkpnt)
{
	//std::cout << "VOL Size: " << sizeof(VOL) << "\n";
	InitAlo(pchkpnt);
}

void LoadChkpntFromBrx(CHKPNT* pchkpnt, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pchkpnt, pbis);
}

void LoadVolFromBrx(VOL* pvol, CBinaryInputStream* pbis)
{
	pbis->ReadMatrix();
	pbis->ReadVector();
	LoadTbspFromBrx(pbis);
	LoadOptionFromBrx(pvol, pbis);
}
