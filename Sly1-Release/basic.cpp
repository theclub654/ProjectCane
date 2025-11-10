#include "basic.h"

void GetBasicCid(BASIC* pbasic, CID* pcid)
{
	*pcid = pbasic->pvtbasic->cid;
}

bool FIsBasicDerivedFrom(const BASIC* basic, CID cid)
{
    if (!basic) return false;

    VTBASIC *pvt = basic->pvtbasic;

    while (pvt) {
        if (pvt->cid == cid) 
            return true;

        pvt = (VTBASIC*)pvt->pvtSuper;
    }
    return false;
}

