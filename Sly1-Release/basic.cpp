#include "basic.h"

void GetBasicCid(BASIC* pbasic, CID* pcid)
{
	*pcid = pbasic->pvtbasic->cid;
}
