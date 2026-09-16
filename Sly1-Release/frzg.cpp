#include "frzg.h"

FRZG* NewFrzg()
{
	return new FRZG{};
}

int GetFrzgSize()
{
	return sizeof(FRZG);
}

void CloneFrzg(FRZG* pfrzg, FRZG* pfrzgBase)
{
	CloneLo(pfrzg, pfrzgBase);

    pfrzg->coid = pfrzgBase->coid;

    for (int i = 0; i < 8; i++)
        pfrzg->aoid[i] = pfrzgBase->aoid[i];

    pfrzg->mrg = pfrzgBase->mrg;
}

void PostFrzgLoad(FRZG* pfrzg)
{
    PostLoLoad(pfrzg);

    SW* psw = pfrzg->psw;
    MRG& mrg = pfrzg->mrg;

    mrg.apalo.clear();
    mrg.apalo.reserve(pfrzg->coid);

    for (int i = 0; i < pfrzg->coid; ++i)
    {
        ALO* palo = static_cast<ALO*>(PloFindSwChild(psw, (OID)pfrzg->aoid[i], pfrzg->paloParent));

        if (palo != nullptr)
            mrg.apalo.push_back(palo);
    }

    AddSwMergeGroup(psw, &mrg);
}

void AddFrzgObject(FRZG* pfrzg, OID oid)
{
    pfrzg->aoid[pfrzg->coid++] = oid;
}

void DeleteFrzg(FRZG* pfrzg)
{
	delete pfrzg;
}
