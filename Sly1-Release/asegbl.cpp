#include "asegbl.h"

ASEGBL* NewAsegbl()
{
    return NewWorldObject<ASEGBL>();
}

void CloneAsegbl(ASEGBL* pasegbl, ASEGBL* pasegblBase)
{
    CloneLo(pasegbl, pasegblBase);

    pasegbl->cbBl = pasegblBase->cbBl;
    pasegbl->cbl = pasegblBase->cbl;
    pasegbl->abl = pasegblBase->abl;
    pasegbl->cmrsgc = pasegblBase->cmrsgc;
    pasegbl->amrsgc = pasegblBase->amrsgc;
}

void DeleteAsegbl(ASEGBL* pasegbl)
{
    ReleaseWorldObject(pasegbl);
}
