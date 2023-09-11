#include "mb.h"

void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis)
{
	LoadStepGuardFromBrx(pmbg, pbis);
}
