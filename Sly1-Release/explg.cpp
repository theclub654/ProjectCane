#include "explg.h"

EXPLG* NewExplg()
{
	return new EXPLG{};
}

int GetExplgSize()
{
	return sizeof(EXPLG);
}

void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis)
{
    LoadXfmFromBrx(static_cast<XFM*>(pexplg), pbis);

    const int clo = pbis->S16Read();

    for (int ilo = 0; ilo < clo; ++ilo)
    {
        const CID cid = static_cast<CID>(pbis->S16Read());
        const OID oid = static_cast<OID>(pbis->S16Read());
        const OID oidContext = static_cast<OID>(pbis->S16Read());

        LO* plo = PloNew(cid, pexplg->psw, pexplg->paloParent, oid, oidContext);
        plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);

        std::array<LO*, 8> alo{};
        int cloProcess = 0;

        if ((plo->pvtlo->grfcid & 0x100) == 0)
            alo[cloProcess++] = plo;
        else
        {
            ALO* palo = static_cast<ALO*>(plo);
            
            for (LO* ploChild = palo->dlChild.ploFirst; ploChild != nullptr && cloProcess < static_cast<int>(alo.size()); ploChild = ploChild->dleChild.ploNext)
                alo[cloProcess++] = ploChild;
        }

        for (int iloProcess = 0; iloProcess < cloProcess; ++iloProcess)
        {
            LO* ploExpl = alo[iloProcess];

            const bool fExplg = FIsBasicDerivedFrom(ploExpl, CID_EXPLG) != 0;
            const bool fExpls = FIsBasicDerivedFrom(ploExpl, CID_EXPLS) != 0;

            if (!fExplg && !fExpls)
                continue;

            ploExpl->pvtlo->pfnRemoveLo(ploExpl);

            if (pexplg->cpexpl >= 8)
                continue;

            EXPL* pexpl = static_cast<EXPL*>(ploExpl);

            pexplg->apexpl[pexplg->cpexpl++] = pexpl;
            pexpl->pexplgParent = pexplg;
        }
    }
}

void CloneExplg(EXPLG* pexplg, EXPLG* pexplgBase)
{
    CloneExpl(pexplg, pexplgBase);

    pexplg->cpexpl = pexplgBase->cpexpl;

    for (int ipexpl = 0; ipexpl < pexplg->cpexpl; ipexpl++)
    {
        EXPL* pexplBase = pexplgBase->apexpl[ipexpl];

        EXPL* pexpl = (EXPL*)PloCloneLo((LO*)pexplBase, pexplg->psw, pexplg->paloParent);

        pexpl->pvtlo->pfnRemoveLo((LO*)pexpl);

        pexplg->apexpl[ipexpl] = pexpl;
        pexpl->pexplgParent = pexplg;
    }

    for (int ipexpl = pexplg->cpexpl; ipexpl < 8; ipexpl++)
        pexplg->apexpl[ipexpl] = nullptr;
}

void BindExplg(EXPLG* pexplg)
{
    for (int ipexpl = 0; ipexpl < pexplg->cpexpl; ++ipexpl)
    {
        EXPL* pexpl = pexplg->apexpl[ipexpl];

        if (pexpl->pvtlo->pfnBindLo != nullptr)
            pexpl->pvtlo->pfnBindLo(pexpl);
    }
}

void ExplodeExplgExplso(EXPLG* pexplg, EXPLSO* pexplso)
{
    for (int ipexpl = 0; ipexpl < pexplg->cpexpl; ++ipexpl)
    {
        EXPL* pexpl = pexplg->apexpl[ipexpl];
        pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, pexplso);
    }
}

void DeleteExplg(EXPLG* pexplg)
{
	delete pexplg;
}
