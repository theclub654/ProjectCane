#include "map2d.h"

MAP* NewMap()
{
	return new MAP{};
}

void CloneMap(MAP* pmap, MAP* pmapBase)
{
    CloneLo(pmap, pmapBase);

    pmap->cpos = pmapBase->cpos;
    pmap->ctri = pmapBase->ctri;
    pmap->cmbsp = pmapBase->cmbsp;
    pmap->cbEach = pmapBase->cbEach;
    pmap->apos = pmapBase->apos;
    pmap->atri = pmapBase->atri;
    pmap->ambsp = pmapBase->ambsp;
    pmap->mpiposab = pmapBase->mpiposab;
}

glm::vec3* PvecFromMapIpos(MAP* pmap, int ipos)
{
    return reinterpret_cast<glm::vec3*>(PvFromMapIpos(pmap, ipos));
}

void* PvFromMapIpos(MAP* pmap, int ipos)
{
    return pmap->mpiposab + ipos * pmap->cbEach;
}

TRI* PtriFindMap(MAP* pmap, const glm::vec3* ppos)
{
    MBSP* pmbsp = pmap->ambsp;

    if (pmbsp == nullptr) {
        return nullptr;
    }

    while ((reinterpret_cast<uintptr_t>(pmbsp) & 1U) == 0) {
        float dot = glm::dot(*ppos, pmbsp->normal);

        if (dot - pmbsp->gDot < 0.0001f) {
            pmbsp = pmbsp->pmbspNeg;
        }
        else {
            pmbsp = pmbsp->pmbspPos;
        }
    }

    return reinterpret_cast<TRI*>(reinterpret_cast<uintptr_t>(pmbsp) & ~uintptr_t{ 1 });
}

int GetMapSize()
{
	return sizeof(MAP);
}

void DeleteMap(MAP* pmap)
{
	delete pmap;
}