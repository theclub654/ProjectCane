#include "bbmark.h"
#include "mark.h"

void RecalcSwOxfFilterForObject(SW* psw, SO* pso)
{
    pso->grfpvaXpValid = 0;

    for (OXA* otherOxa = psw->poxaAlloc; otherOxa != nullptr; otherOxa = otherOxa->poxaNext)
    {
        SO* other = otherOxa->pso;

        if (other == pso)
            continue;

        const bool ignoreIntersection = pso->pvtso->pfnFIgnoreSoIntersection(pso, other) ||
        other->pvtso->pfnFIgnoreSoIntersection(other, pso);

        constexpr OXF OXF_IgnoreIntersection = 0x08;

        const OXF oxfAdd    = ignoreIntersection ? 0x00 : OXF_IgnoreIntersection;
        const OXF oxfRemove = ignoreIntersection ? OXF_IgnoreIntersection : 0x00;

        UpdateSwPox(psw, pso->poxa, otherOxa, oxfAdd, oxfRemove);
    }
}

void UpdateSwPox(SW* psw, OXA* poxa0, OXA* poxa1, OXF oxfAdd, OXF oxfRemove)
{
    constexpr int MaxOxa = 256;

    const int i0 = static_cast<int>(poxa0 - psw->aoxa.data());
    const int i1 = static_cast<int>(poxa1 - psw->aoxa.data());

    uint8_t& flags01 = psw->aaoxf[i0 * MaxOxa + i1];
    uint8_t& flags10 = psw->aaoxf[i1 * MaxOxa + i0];
    uint8_t flags = static_cast<uint8_t>((flags01 | oxfAdd) & ~oxfRemove);

    flags01 = flags;
    flags10 = flags;

    const bool shouldHavePair = ((flags & 0x0f) == 0x0f) || ((flags & 0x40) != 0);
    const bool hasPair = (flags & 0x80) != 0;

    if (shouldHavePair && !hasPair)
    {
        flags = static_cast<uint8_t>(flags | 0x80);

        flags01 = flags;
        flags10 = flags;

        OX* pox0 = PoxAddSw(psw, poxa0, poxa1);
        OX* pox1 = PoxAddSw(psw, poxa1, poxa0);

        pox0->poxOther = pox1;
        pox1->poxOther = pox0;
    }
    else if (!shouldHavePair && hasPair)
    {
        flags = static_cast<uint8_t>(flags & ~0x80);

        flags01 = flags;
        flags10 = flags;

        OX* pox0 = PoxRemoveSw(psw, poxa0, poxa1);
        OX* pox1 = PoxRemoveSw(psw, poxa1, poxa0);

        if (pox0 != nullptr && pox0->pxp != nullptr)
        {
            FreeSwXpList(psw, pox0->pxp);

            if (!pox0->psoOther->fLockedAll)
                ResolveAlo(pox0->psoOther);

            if (pox1 != nullptr && !pox1->psoOther->fLockedAll)
                ResolveAlo(pox1->psoOther);
        }

        FreeSwOx(psw, pox0);
        FreeSwOx(psw, pox1);
    }
}

OX* PoxAddSw(SW* psw, OXA* poxa, OXA* poxaOther)
{
    OX* pox = nullptr;

    if (!psw->apoxFree.empty())
    {
        pox = psw->apoxFree.back();
        psw->apoxFree.pop_back();
        *pox = OX{};
    }
    else
    {
        psw->aox.emplace_back();
        pox = &psw->aox.back();
    }

    pox->pxp = nullptr;
    pox->psoOther = poxaOther->pso;
    pox->poxOther = nullptr;
    pox->poxNext = poxa->pox;

    poxa->pox = pox;
    return pox;
}

OX* PoxRemoveSw(SW* psw, OXA* poxa, OXA* poxaOther)
{
    OX** ppox = &poxa->pox;

    while (*ppox != nullptr)
    {
        OX* pox = *ppox;

        if (pox->psoOther == poxaOther->pso)
        {
            *ppox = pox->poxNext;
            pox->poxNext = nullptr;
            return pox;
        }

        ppox = &pox->poxNext;
    }

    return nullptr;
}

void FreeSwOx(SW* psw, OX* pox)
{
    if (pox == nullptr)
        return;

    *pox = OX{};
    psw->apoxFree.push_back(pox);
}

int IoxaFromPoxa(SW* psw, OXA* poxa)
{
    OXA* first = psw->aoxa.data();
    OXA* last = first + psw->aoxa.size();

    assert(poxa >= first);
    assert(poxa < last);

    return static_cast<int>(poxa - first);
}

void UpdateSwAaox(SW* psw)
{
    if (psw->cpsoRoot == 0)
        return;

    if (psw->fAaoxValid)
        return;

    OXF oxfAxis = 1;

    for (int axis = 0; axis < AXIS_COUNT; axis++)
    {
        OBR* aobr = psw->aaobr.data() + axis * BOUNDS_PER_AXIS;
        const int cobr = psw->cpsoRoot * 2;

        for (int iobr = 1; iobr < cobr; iobr++)
        {
            OBR obrMoving = aobr[iobr];
            const float sMoving = *obrMoving.ps;

            int iPrev = iobr - 1;

            while (iPrev >= 0 && sMoving < *aobr[iPrev].ps)
            {
                OBR& obrCrossed = aobr[iPrev];

                if (obrMoving.obrf != obrCrossed.obrf)
                {
                    if (obrMoving.obrf == OBRF_Min)
                    {
                        // Moving min left across another object's max: overlap starts on this axis.
                        UpdateSwPox(psw, obrMoving.pso->poxa, obrCrossed.pso->poxa, oxfAxis, 0);
                    }
                    else
                    {
                        // Moving max left across another object's min: overlap ends on this axis.
                        UpdateSwPox(psw, obrMoving.pso->poxa, obrCrossed.pso->poxa, 0, oxfAxis);
                    }
                }

                aobr[iPrev + 1] = aobr[iPrev];
                iPrev--;
            }

            aobr[iPrev + 1] = obrMoving;
        }

        oxfAxis = static_cast<OXF>(oxfAxis << 1);
    }

    psw->fAaoxValid = 1;
}

void RecalcSwXpAll(SW* psw, int fCenterXp)
{
    UpdateSwAaox(psw);

    for (SO* pso = psw->dlBusySo.psoFirst; pso != nullptr; pso = pso->dleBusySo.psoNext)
    {
        if (pso->grfpvaXpValid == 7)
            continue;

        if (pso->fNoXpsAll)
            continue;

        pso->fRecalcSwXpAll = 1;

        for (OX* pox = pso->poxa->pox; pox != nullptr; pox = pox->poxNext)
        {
            SO* apso[2];
            apso[0] = pso;
            apso[1] = pox->psoOther;

            SO* psoOther = apso[1];

            if (psoOther < pso && psoOther->grfpvaXpValid != 7 && psoOther->fBusy)
                continue;

            if (psoOther->fNoXpsAll)
                continue;

            psoOther->fRecalcSwXpAll = 1;

            if ((pso->grfpvaXpValid & 1) != 0)
            {
                if ((psoOther->grfpvaXpValid & 1) == 0)
                {
                    XP* pxpFirst = pox->pxp;

                    if (pxpFirst != nullptr)
                        FreeSwXpList(psw, pxpFirst);

                    XP* pxpNew = PxpMarkSwContactsRoot(psw, apso, fCenterXp);


                    pox->pxp = pxpNew;
                    pox->poxOther->pxp = pxpNew;

                    if ((pxpFirst == nullptr) != (pxpNew == nullptr))
                    {
                        if (!pso->fLockedAll)
                            ResolveAlo(pso);

                        if (!psoOther->fLockedAll)
                            ResolveAlo(psoOther);
                    }
                }
                else
                {
                    for (XP* pxp = pox->pxp; pxp != nullptr; pxp = pxp->pxpNext)
                        RecalcSwXpMovement(psw, 1, &pxp);
                }
            }
            else
            {
                XP* pxpFirst = pox->pxp;

                if (pxpFirst != nullptr)
                    FreeSwXpList(psw, pxpFirst);

                XP* pxpNew = PxpMarkSwContactsRoot(psw, apso, fCenterXp);


                pox->pxp = pxpNew;
                pox->poxOther->pxp = pxpNew;

                if ((pxpFirst == nullptr) != (pxpNew == nullptr))
                {
                    if (!pso->fLockedAll)
                        ResolveAlo(pso);

                    if (!psoOther->fLockedAll)
                        ResolveAlo(psoOther);
                }
            }
        }
    }

    for (SO* pso = psw->dlBusySo.psoFirst; pso != nullptr; pso = pso->dleBusySo.psoNext)
    {
        if ((pso->grfpvaXpValid & 4) != 0)
            continue;

        for (SO* psoPhys = pso->dlPhys.psoFirst; psoPhys != nullptr; psoPhys = psoPhys->dlePhys.psoNext)
        {
            auto pfnUpdate = psoPhys->pvtso->pfnUpdateSoInternalXps;

            if (pfnUpdate != nullptr)
                psoPhys->pvtso->pfnUpdateSoInternalXps(psoPhys);
        }

        pso->fRecalcSwXpAll = 1;
    }

    for (SO* pso = psw->dlBusySo.psoFirst; pso != nullptr; pso = pso->dleBusySo.psoNext)
    {
        if (pso->fRecalcSwXpAll)
        {
            for (SO* psoPhys = pso->dlPhys.psoFirst; psoPhys != nullptr; psoPhys = psoPhys->dlePhys.psoNext)
            {
                auto pfnAdjust = psoPhys->pvtso->pfnAdjustSoXps;

                if (pfnAdjust != nullptr)
                    psoPhys->pvtso->pfnAdjustSoXps(psoPhys);
            }

            pso->fRecalcSwXpAll = 0;
        }

        pso->grfpvaXpValid = 7;
    }

}

void AddSwAaobrObject(SW* psw, SO* pso)
{
    int objectBoundIndex = psw->cpsoRoot * BOUNDS_PER_OBJECT;

    for (int axis = 0; axis < AXIS_COUNT; axis++)
    {
        int axisBase = axis * BOUNDS_PER_AXIS;
        OBR* bounds = psw->aaobr.data() + axisBase + objectBoundIndex;

        bounds[0].obrf = OBRF_Min; // min
        bounds[0].pso = pso;
        bounds[0].ps = &pso->posMin[axis];

        bounds[1].obrf = OBRF_Max; // max
        bounds[1].pso = pso;
        bounds[1].ps = &pso->posMax[axis];
    }

    InvalidateSwAaox(psw);
}

void RemoveSwAaobrObject(SW* psw, SO* pso)
{
    const int cobrf = psw->cpsoRoot * 2;

    for (int axis = 0; axis < AXIS_COUNT; axis++)
    {
        OBR* aobr = psw->aaobr.data() + axis * BOUNDS_PER_AXIS;

        int iMin = -1;
        int iMax = -1;

        for (int i = 0; i < cobrf; i++)
        {
            if (aobr[i].pso != pso)
                continue;

            if (aobr[i].obrf == OBRF_Min)
            {
                iMin = i;
            }
            else
            {
                iMax = i;
                break;
            }
        }

        if (iMin < 0 || iMax < 0)
            continue;

        for (int i = iMin; i < iMax - 1; i++)
            aobr[i] = aobr[i + 1];

        for (int i = iMax - 1; i < cobrf - 2; i++)
            aobr[i] = aobr[i + 2];

        aobr[cobrf - 2] = {};
        aobr[cobrf - 1] = {};
    }
}

void InvalidateSwAaox(SW* psw)
{
    psw->fAaoxValid = 0;
}

void InvalidateSwXpForObject(SW* psw, SO* pso, GRFPVA grfpvaInvalid)
{
    SO* root = (SO*)pso->paloRoot;

    if (root != nullptr)
        root->grfpvaXpValid &= ~grfpvaInvalid;
}

OX* PoxFromSoSo(SO* pso, SO* psoOther)
{
    SO* psoRoot = (SO*)pso->paloRoot;
    OX* pox = psoRoot->poxa->pox;

    while (pox)
    {
        if (pox->psoOther == psoOther->paloRoot)
            return pox;

        pox = pox->poxNext;
    }

    return nullptr;
}

XP* PxpFirstFromSoSo(SO* pso, SO* psoOther)
{
    OX* pox = PoxFromSoSo(pso, psoOther);

    if (pox)
        return pox->pxp;

    return nullptr;
}
