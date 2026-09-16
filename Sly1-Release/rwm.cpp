#include "rwm.h"
#include "bomb.h"
#include "break.h"
#include "target.h"
#include "gomer.h"
#include "missile.h"
#include "pnt.h"

RWM* NewRwm()
{
	return new RWM{};
}

void InitRwm(RWM* prwm)
{
	InitLo(prwm);

    prwm->oidAim = OID_Nil;
    prwm->oidFire = OID_Nil;
    prwm->oidTarget = OID_Nil;
    prwm->oidAimXfm = -1;

    memset(&prwm->rwfi, 0, sizeof(prwm->rwfi));
    prwm->rwfi.mat = g_matIdentity;

    memset(&prwm->rwti, 0, sizeof(prwm->rwti));
    prwm->rwti.clqDistance.g0 = 0.0f;
    prwm->rwti.clqDistance.g1 = 1.0f;
    prwm->rwti.clqDistance.g2 = 0.0f;
    prwm->rwti.lmDistance.gMin = 0.0f;
    prwm->rwti.lmDistance.gMax = 20000.0f;

    memset(&prwm->rwac, 0, sizeof(prwm->rwac));

    prwm->fPostLoad = 0;
    prwm->rwfi.tMax = 5.0f;
    prwm->rwti.lmDtPredict.gMax = FLT_MAX;

    PostSwCallback(prwm->psw, (PFNMQ)BindRwmAmmo, prwm, MSGID_callback, nullptr);
}

bool* PrwmcloneEnsureRwmFSharedAmmo(RWM* prwm, ENSK ensk)
{
    return &PrwmcloneEnsureRwm(prwm, ensk)->fSharedAmmo;
}

void* GetRwmCspk(RWM* prwm)
{
    return &prwm->cspk;
}

void SetRwmCspk(RWM* prwm, CSPK cspk)
{
    prwm->cspk = cspk;
}

void* GetRwmIrwc(RWM* prwm)
{
    return &prwm->irwc;
}

void SetRwmIrwc(RWM* prwm, int irwc)
{
    prwm->irwc = irwc;
}

void* GetRwmPsoAmmo(RWM* prwm)
{
    return &prwm->psoAmmo;
}

void SetRwmPsoAmmo(RWM* prwm, SO* psoAmmo)
{
    prwm->psoAmmo = psoAmmo;
}

void* GetRwmOidFire(RWM* prwm)
{
    return &prwm->oidFire;
}

void SetRwmOidFire(RWM* prwm, int oidFire)
{
    prwm->oidFire = (OID)oidFire;
}

void* GetRwmRwfiPpnt(RWM* prwm)
{
    return &prwm->rwfi.ppnt;
}

void SetRwmRwfiPpnt(RWM* prwm, PNT* ppnt)
{
    prwm->rwfi.ppnt = ppnt;
}

void* GetRwmRwfiPxfm(RWM* prwm)
{
    return &prwm->rwfi.pxfm;
}

void SetRwmRwfiPxfm(RWM* prwm, XFM* pxfm)
{
    prwm->rwfi.pxfm = pxfm;
}

void* GetRwmRwfiPalo(RWM* prwm)
{
    return &prwm->rwfi.palo;
}

void SetRwmRwfiPalo(RWM* prwm, ALO* palo)
{
    prwm->rwfi.palo = palo;
}

void* GetRwmRwfiPos(RWM* prwm)
{
    return &prwm->rwfi.pos;
}

void SetRwmRwfiPos(RWM* prwm, glm::vec3 pos)
{
    prwm->rwfi.pos = pos;
}

void* GetRwmRwfiMat(RWM* prwm)
{
    return &prwm->rwfi.mat;
}

void SetRwmRwfiMat(RWM* prwm, glm::mat3 mat)
{
    prwm->rwfi.mat = mat;
}

void* GetRwmRwfiV(RWM* prwm)
{
    return &prwm->rwfi.v;
}

void SetRwmRwfiV(RWM* prwm, glm::vec3 v)
{
    prwm->rwfi.v = v;
}

void* GetRwmRwfiW(RWM* prwm)
{
    return &prwm->rwfi.w;
}

void SetRwmRwfiW(RWM* prwm, glm::vec3 w)
{
    prwm->rwfi.w = w;
}

void* GetRwmRwfiLmSw(RWM* prwm)
{
    return &prwm->rwfi.lmSw;
}

void SetRwmRwfiLmSw(RWM* prwm, LM lmSw)
{
    prwm->rwfi.lmSw = lmSw;
}

void* GetRwmRwfiTMax(RWM* prwm)
{
    return &prwm->rwfi.tMax;
}

void SetRwmRwfiTMax(RWM* prwm, float tMax)
{
    prwm->rwfi.tMax = tMax;
}

void* GetRwmOidTarget(RWM* prwm)
{
    return &prwm->oidTarget;
}

void SetRwmOidTarget(RWM* prwm, int oidTarget)
{
    prwm->oidTarget = (OID)oidTarget;
}

void* GetRwmRwtiPalo(RWM* prwm)
{
    return &prwm->rwti.palo;
}

void SetRwmRwtiPalo(RWM* prwm, ALO* palo)
{
    prwm->rwti.palo = palo;
}

void* GetRwmRwtiPtarget(RWM* prwm)
{
    return &prwm->rwti.ptarget;
}

void SetRwmRwtiPtarget(RWM* prwm, TARGET* ptarget)
{
    prwm->rwti.ptarget = ptarget;
}

void* GetRwmRwtiPos(RWM* prwm)
{
    return &prwm->rwti.pos;
}

void SetRwmRwtiPos(RWM* prwm, glm::vec3 pos)
{
    prwm->rwti.pos = pos;
}

void* GetRwmRwtiRwmtk(RWM* prwm)
{
    return &prwm->rwti.rwmtk;
}

void SetRwmRwtiRwmtk(RWM* prwm, RWMTK rwmtk)
{
    prwm->rwti.rwmtk = rwmtk;
}

void* GetRwmRwtiClqDistance(RWM* prwm)
{
    return &prwm->rwti.clqDistance;
}

void SetRwmRwtiClqDistance(RWM* prwm, CLQ clqDistance)
{
    prwm->rwti.clqDistance = clqDistance;
}

void* GetRwmRwtiLmDistance(RWM* prwm)
{
    return &prwm->rwti.lmDistance;
}

void SetRwmRwtiLmDistance(RWM* prwm, LM lmDistance)
{
    prwm->rwti.lmDistance = lmDistance;
}

void* GetRwmRwtiClqSToDtPredict(RWM* prwm)
{
    return &prwm->rwti.clqSToDtPredict;
}

void SetRwmRwtiClqSToDtPredict(RWM* prwm, CLQ clqSToDtPredict)
{
    prwm->rwti.clqSToDtPredict = clqSToDtPredict;
}

void* GetRwmRwtiLmDtPredict(RWM* prwm)
{
    return &prwm->rwti.lmDtPredict;
}

void SetRwmRwtiLmDtPredict(RWM* prwm, LM lmDtPredict)
{
    prwm->rwti.lmDtPredict = lmDtPredict;
}

void* GetRwmRwacGrfask(RWM* prwm)
{
    return &prwm->rwac.grfask;
}

void SetRwmRwacGrfask(RWM* prwm, GRFASK grfask)
{
    prwm->rwac.grfask = grfask;
}

void* GetRwmOidAimXfm(RWM* prwm)
{
    return &prwm->oidAimXfm;
}

void SetRwmOidAimXfm(RWM* prwm, int oidAimXfm)
{
    prwm->oidAimXfm = oidAimXfm;
}

void* GetRwmRwacPxfm(RWM* prwm)
{
    return &prwm->rwac.pxfm;
}

void SetRwmRwacPxfm(RWM* prwm, XFM* pxfm)
{
    prwm->rwac.pxfm = pxfm;
}

void* GetRwmRwacRadTilt(RWM* prwm)
{
    return &prwm->rwac.radTilt;
}

void SetRwmRwacRadTilt(RWM* prwm, float radTilt)
{
    prwm->rwac.radTilt = radTilt;
}

void* GetRwmRwacRadPan(RWM* prwm)
{
    return &prwm->rwac.radPan;
}

void SetRwmRwacRadPan(RWM* prwm, float radPan)
{
    prwm->rwac.radPan = radPan;
}

void* GetRwmRwacSv(RWM* prwm)
{
    return &prwm->rwac.sv;
}

void SetRwmRwacSv(RWM* prwm, float sv)
{
    prwm->rwac.sv = sv;
}

void* GetRwmOidAim(RWM* prwm)
{
    return &prwm->oidAim;
}

void SetRwmOidAim(RWM* prwm, int oidAim)
{
    prwm->oidAim = (OID)oidAim;
}

void* GetRwmPaloFireContext(RWM* prwm)
{
    return &prwm->paloFireContext;
}

void SetRwmPaloFireContext(RWM* prwm, ALO* paloContext)
{
    prwm->paloFireContext = paloContext;
}

void OnRwmRemove(RWM* prwm)
{
    ClearRwmAmmo(prwm);
}

RWMCLONE* PrwmcloneEnsureRwm(RWM* prwm, ENSK ensk)
{
    if (!prwm->prwmclone)
    {
        prwm->prwmclone = std::make_shared<RWMCLONE>();
        prwm->prwmclone->prwm = prwm;
    }

    return prwm->prwmclone.get();
}

void AddRwmCache(RWM* prwm, OID oid, int cpso, bool fReuse, bool fEnabled)
{
    constexpr std::size_t crwcMax = 4;

    if (prwm->arwc.size() >= crwcMax)
        return;

    RWC& rwc = prwm->arwc.emplace_back();

    rwc.oid = oid;
    rwc.cpsoMax = cpso;
    rwc.cpso = 0;
    rwc.grfrwc = (fReuse ? 1u : 0u) | (fEnabled ? 2u : 0u);
    rwc.apso.reserve(cpso);
    prwm->crwc = static_cast<int>(prwm->arwc.size());

}

void BindRwmAmmo(RWM* prwm)
{
    RWMCLONE* const prwmclone = prwm->prwmclone.get();
    const bool fSharedAmmo = prwmclone != nullptr && prwmclone->fSharedAmmo;

    std::vector<RWC>* parwc = &prwm->arwc;

    if (fSharedAmmo && prwmclone->clonek == RWMCLONEK_Clone)
    {
        if (prwmclone->prwmBase == nullptr)
            return;

        parwc = &prwmclone->prwmBase->arwc;
    }

    // Retail allocates one transform record per cache for every shared-ammo
    // RWM, including RWMCLONEK_Clone instances.  Each clone needs the origin
    // of the ammo template found in its own hierarchy.
    if (fSharedAmmo)
    {
        prwmclone->arwmammoxf.clear();
        prwmclone->arwmammoxf.resize(parwc->size());
        prwmclone->irwmammoxfCur = 0;
    }

    for (std::size_t irwc = 0; irwc < parwc->size(); ++irwc)
    {
        RWC& rwc = (*parwc)[irwc];

        LO* const ploTemplate = PloFindSwObject(prwm->psw, 4, (OID)rwc.oid, prwm);

        if (ploTemplate == nullptr)
        {
            if (!fSharedAmmo || prwmclone->clonek != RWMCLONEK_Clone)
            {
                rwc.cpso = 0;
                rwc.apso.clear();
            }

            continue;
        }

        if (fSharedAmmo && prwmclone->clonek == RWMCLONEK_Clone)
        {
            rwc.apso.push_back(static_cast<SO*>(ploTemplate));
            rwc.cpso = static_cast<int>(rwc.apso.size());

            if (ploTemplate->pvtlo->pfnBindLo != nullptr)
                ploTemplate->pvtlo->pfnBindLo(ploTemplate);

            ploTemplate->pvtlo->pfnPostLoLoad(ploTemplate);
            ploTemplate->pvtlo->pfnSetLoParent(ploTemplate, nullptr);
            ploTemplate->pvtlo->pfnRemoveLo(ploTemplate);

            if (irwc < prwmclone->arwmammoxf.size())
            {
                ALO* const paloTemplate = static_cast<ALO*>(ploTemplate);
                RWMAMMOXF& rwmammoxf = prwmclone->arwmammoxf[irwc];

                rwmammoxf.posOrig = paloTemplate->posOrig;
                rwmammoxf.eulOrig = paloTemplate->eulOrig;
            }

            continue;
        }

        int cpsoAllocate = rwc.cpsoMax;
        int cpsoClone = cpsoAllocate - 1;

        if (fSharedAmmo)
        {
            if (prwmclone->clonek == RWMCLONEK_Nil)
            {
                prwmclone->clonek = RWMCLONEK_Base;
                InitDl(&prwmclone->dlClone, offsetof(RWMCLONE, dleClone));
            }

            if (prwmclone->clonek == RWMCLONEK_Base)
            {
                const int crwmClone = CPvDl(&prwmclone->dlClone);

                cpsoAllocate = std::max(rwc.cpsoMax, crwmClone + 1);
                cpsoClone = cpsoAllocate - crwmClone - 1;
            }
        }

        rwc.apso.clear();
        rwc.apso.reserve(cpsoAllocate);
        rwc.apso.push_back(static_cast<SO*>(ploTemplate));

        for (int ipso = 0; ipso < cpsoClone; ++ipso)
        {
            LO* const ploClone = PloCloneLo(ploTemplate, ploTemplate->psw, ploTemplate->paloParent);

            if (ploClone == nullptr)
                break;

            rwc.apso.push_back(static_cast<SO*>(ploClone));

            if (ploClone->pvtlo->pfnBindLo != nullptr)
                ploClone->pvtlo->pfnBindLo(ploClone);

            ploClone->pvtlo->pfnPostLoLoad(ploClone);
            ploClone->pvtlo->pfnSetLoParent(ploClone, nullptr);
            ploClone->pvtlo->pfnRemoveLo(ploClone);
        }

        if (ploTemplate->pvtlo->pfnBindLo != nullptr)
            ploTemplate->pvtlo->pfnBindLo(ploTemplate);

        ploTemplate->pvtlo->pfnPostLoLoad(ploTemplate);
        ploTemplate->pvtlo->pfnSetLoParent(ploTemplate, nullptr);
        ploTemplate->pvtlo->pfnRemoveLo(ploTemplate);

        rwc.cpso = static_cast<int>(rwc.apso.size());

        if (fSharedAmmo)
        {
            ALO* const paloTemplate = static_cast<ALO*>(ploTemplate);
            RWMAMMOXF& rwmammoxf = prwmclone->arwmammoxf[irwc];

            rwmammoxf.posOrig = paloTemplate->posOrig;
            rwmammoxf.eulOrig = paloTemplate->eulOrig;
        }
    }
}

void CloneRwm(RWM* prwm, RWM* prwmBase)
{
    CloneLo(prwm, prwmBase); // Assuming CloneLo handles copying the base class (LO) members

    // Shallow copy of value members
    prwm->cspk = prwmBase->cspk;
    prwm->irwc = prwmBase->irwc;
    prwm->oidFire = prwmBase->oidFire;
    prwm->oidTarget = prwmBase->oidTarget;
    prwm->oidAim = prwmBase->oidAim;
    prwm->rwfi = prwmBase->rwfi;
    prwm->rwti = prwmBase->rwti;
    prwm->rwac = prwmBase->rwac;
    prwm->fPostLoad = prwmBase->fPostLoad;

    // The retail object contains an inline cache array.  In this port it is a
    // vector, so copying only crwc leaves an empty vector with a nonzero count
    // and causes FIsRwmAmmo to index out of range.  Vector assignment preserves
    // the intended value-copy of each RWC while retaining its SO pointer list.
    prwm->arwc = prwmBase->arwc;
    prwm->crwc = static_cast<int>(prwm->arwc.size());

    // Shallow copy of the pointer member
    prwm->psoAmmo = prwmBase->psoAmmo;

    //////////////////////////////////////////////////////////////////////

    RWMCLONE* const pcloneBase = prwmBase->prwmclone.get();

    if (pcloneBase != nullptr && pcloneBase->fSharedAmmo)
    {
        if (pcloneBase->clonek == RWMCLONEK_Nil)
        {
            pcloneBase->clonek = RWMCLONEK_Base;
            InitDl(&pcloneBase->dlClone, offsetof(RWMCLONE, dleClone));
        }

        prwm->prwmclone.reset();

        RWMCLONE* const pclone = PrwmcloneEnsureRwm(prwm, ENSK_Set);

        pclone->fSharedAmmo = true;
        pclone->clonek = RWMCLONEK_Clone;
        pclone->prwmBase = prwmBase;

        AppendDlEntry(&pcloneBase->dlClone, pclone);
        return;
    }

    prwm->arwc = prwmBase->arwc;

    for (RWC& rwc : prwm->arwc)
        rwc.apso.clear();
}

void PostRwmLoad(RWM* prwm)
{
    PostLoLoad(prwm);

    if (prwm->fPostLoad)
        return;

    prwm->fPostLoad = true;

    if (prwm->oidFire != OID_Nil)
    {
        LO* const ploFire = PloFindSwNearest(prwm->psw, (OID)prwm->oidFire, prwm);

        if (ploFire != nullptr)
        {
            if (FIsBasicDerivedFrom(ploFire, CID_PNT))
                prwm->rwfi.ppnt = (PNT*)ploFire;
            else if (FIsBasicDerivedFrom(ploFire, CID_XFM))
                prwm->rwfi.pxfm = (XFM*)ploFire;
            else if (FIsBasicDerivedFrom(ploFire, CID_ALO))
                prwm->rwfi.palo = (ALO*)ploFire;
        }
    }

    if (prwm->oidTarget != OID_Nil)
    {
        LO* const ploTarget = PloFindSwNearest(prwm->psw, (OID)prwm->oidTarget, prwm);

        if (ploTarget != nullptr)
        {
            if (FIsBasicDerivedFrom(ploTarget, CID_ALO))
                prwm->rwti.palo = static_cast<ALO*>(ploTarget);
            else if (FIsBasicDerivedFrom(ploTarget, CID_TARGET))
                prwm->rwti.ptarget = (TARGET*)ploTarget;
        }
    }

    if (prwm->oidAimXfm != OID_Nil)
    {
        LO* const ploAimXfm = PloFindSwNearest(prwm->psw, (OID)prwm->oidAimXfm, prwm);

        if (ploAimXfm != nullptr)
        {
            if (FIsBasicDerivedFrom(ploAimXfm, CID_XFM))
                prwm->rwac.pxfm = (XFM*)ploAimXfm;
            else if (FIsBasicDerivedFrom(ploAimXfm, CID_ALO))
                prwm->rwac.palo = static_cast<ALO*>(ploAimXfm);
        }
    }

    if (prwm->oidAim != OID_Nil)
        prwm->paloFireContext = static_cast<ALO*>(PloFindSwNearest(prwm->psw, (OID)prwm->oidAim, prwm));

}

void FreezeRwm(RWM* prwm, int fFreeze)
{
    if (fFreeze != 0)
        ClearRwmAmmo(prwm);
}

RWC* PrwcFindRwmCache(RWM* prwm, OID oidCache)
{
    for (RWC& rwc : prwm->arwc)
    {
        if (rwc.oid == oidCache)
            return &rwc;
    }

    return nullptr;
}

void EnableRwmCache(RWM* prwm, OID oidCache)
{
    RWC* const prwc = PrwcFindRwmCache(prwm, oidCache);

    if (prwc != nullptr)
        prwc->grfrwc |= 2u;
}

void DisableRwmCache(RWM* prwm, OID oidCache)
{
    RWC* const prwc = PrwcFindRwmCache(prwm, oidCache);

    if (prwc != nullptr)
        prwc->grfrwc &= ~2u;
}

void ResizeRwmCache(RWM* prwm, OID oidCache, int cpso)
{
    RWC* const prwc = PrwcFindRwmCache(prwm, oidCache);

    if (prwc != nullptr)
        prwc->cpso = std::min(cpso, static_cast<int>(prwc->apso.size()));
}

int FIsRwmAmmo(RWM* prwm, SO* psoAmmo)
{
    if (prwm == nullptr || psoAmmo == nullptr)
        return 0;

    const int crwc = std::min(prwm->crwc, static_cast<int>(prwm->arwc.size()));

    for (int irwc = 0; irwc < crwc; irwc++)
    {
        RWC* prwc = &prwm->arwc[irwc];
		const int cpso = std::min(prwc->cpso, static_cast<int>(prwc->apso.size()));

        if (cpso <= 0 || prwc->apso[0] == nullptr)
            continue;

        if (prwc->apso[0]->oid != psoAmmo->oid)
            continue;

        for (int ipso = 0; ipso < cpso; ipso++)
        {
            if (prwc->apso[ipso] == psoAmmo)
                return 1;
        }
    }

    return 0;
}

bool FAvailableRwmAmmo(RWM* prwm, SO* psoAmmo)
{
    if (psoAmmo == nullptr)
        return false;

    RWMCLONE* pclone = prwm->prwmclone.get();

    if (pclone != nullptr && pclone->fSharedAmmo)
    {
        RWM* prwmBase = prwm;

        if (pclone->clonek == RWMCLONEK_Clone)
        {
            prwmBase = pclone->prwmBase;

            if (prwmBase == nullptr)
                return false;

            if (prwmBase->psoAmmo == psoAmmo)
                return false;
        }

        RWMCLONE* pbaseClone = prwmBase->prwmclone.get();

        if (pbaseClone != nullptr)
        {
            RWMCLONE* pentry = static_cast<RWMCLONE*>(pbaseClone->dlClone.pvFirst);

            while (pentry != nullptr)
            {
                RWM* prwmOther = pentry->prwm;

                if (prwmOther != nullptr && prwmOther != prwm && prwmOther->psoAmmo == psoAmmo)
                    return false;

                pentry = static_cast<RWMCLONE*>(pentry->dleClone.pvNext);
            }
        }
    }

    if (FIsLoInWorld(psoAmmo))
        return false;

    if (FIsBasicDerivedFrom(psoAmmo, CID_BOMB))
    {
        BOMB* pbomb = static_cast<BOMB*>(psoAmmo);

        if (!pbomb->fReclaim)
            return false;
    }

    if (FIsBasicDerivedFrom(psoAmmo, CID_BRK))
    {
        BRK* pbrk = static_cast<BRK*>(psoAmmo);

        if (pbrk->psfx != nullptr)
            return false;
    }

    return true;
}

void CycleRwmCache(RWM* prwm)
{
    const std::size_t crwc = prwm->arwc.size();

    if (crwc <= 1)
        return;

    const std::size_t irwcCurrent = static_cast<std::size_t>(prwm->irwc) % crwc;

    for (std::size_t offset = 1; offset < crwc; ++offset)
    {
        const std::size_t irwcCandidate = (irwcCurrent + offset) % crwc;
        RWC& rwc = prwm->arwc[irwcCandidate];

        if ((rwc.grfrwc & 2u) == 0)
            continue;

        const int cpsoActive = std::min(rwc.cpso, static_cast<int>(rwc.apso.size()));

        for (int ipso = 0; ipso < cpsoActive; ++ipso)
        {
            if (FAvailableRwmAmmo(prwm, rwc.apso[ipso]))
            {
                prwm->irwc = static_cast<int>(irwcCandidate);
                return;
            }
        }
    }
}

void ReloadRwm(RWM* prwm)
{
    prwm->psoAmmo = nullptr;

    RWMCLONE* const pclone = prwm->prwmclone.get();

    if (pclone != nullptr && pclone->fSharedAmmo)
        pclone->irwmammoxfCur = -1;

    std::vector<RWC>* parwc = &prwm->arwc;

    if (pclone != nullptr && pclone->fSharedAmmo && pclone->clonek == RWMCLONEK_Clone)
    {
        if (pclone->prwmBase == nullptr)
            return;

        parwc = &pclone->prwmBase->arwc;
    }

    if (parwc->empty())
        return;

    if (prwm->cspk == CSPK_AutoCycle)
        CycleRwmCache(prwm);

    if (prwm->irwc < 0 || static_cast<std::size_t>(prwm->irwc) >= parwc->size())
        return;

    RWC& rwc = (*parwc)[prwm->irwc];

    if ((rwc.grfrwc & 2u) == 0)
        return;

    const int cpsoActive = std::clamp(rwc.cpso, 0, static_cast<int>(rwc.apso.size()));

    for (int ipso = 0; ipso < cpsoActive; ++ipso)
    {
        SO* const psoAmmo = rwc.apso[ipso];

        if (!FAvailableRwmAmmo(prwm, psoAmmo))
            continue;

        prwm->psoAmmo = psoAmmo;

        if (pclone != nullptr && pclone->fSharedAmmo)
        {
            const std::size_t irwc = static_cast<std::size_t>(prwm->irwc);

            if (irwc < pclone->arwmammoxf.size())
                pclone->irwmammoxfCur = irwc;
        }

        if ((rwc.grfrwc & 1u) == 0)
        {
            rwc.apso.erase(rwc.apso.begin() + ipso);
            --rwc.cpso;
        }

        return;
    }
}

int FEnsureRwmLoaded(RWM* prwm)
{
    if (prwm->psoAmmo == nullptr)
        ReloadRwm(prwm);

    return prwm->psoAmmo != nullptr;
}

int FFireRwm(RWM* prwm, bool fAim)
{
    if (prwm->psoAmmo == nullptr)
    {
        ReloadRwm(prwm);

        if (prwm->psoAmmo == nullptr)
            return false;
    }

    SO* psoAmmo = prwm->psoAmmo;
    ALO* paloAmmo = static_cast<ALO*>(psoAmmo);
    ALO* paloOwner = prwm->paloParent;

    while (paloOwner != nullptr && paloOwner->paloParent != nullptr)
        paloOwner = paloOwner->paloParent;

    if (paloOwner != nullptr && paloOwner->oid == static_cast<OID>(1948))
    {
        if (psoAmmo->oid == static_cast<OID>(2092) && FIsBasicDerivedFrom(psoAmmo, CID_MISSILE))
        {
            MISSILE* pmissile = static_cast<MISSILE*>(psoAmmo);
            constexpr OID oidGomerSnowballSibling = static_cast<OID>(2093);
            bool fAlreadyIgnored = false;

            for (int ioid = 0; ioid < pmissile->coidIgnore; ++ioid)
            {
                if (pmissile->aoidIgnore[ioid] == static_cast<short>(oidGomerSnowballSibling))
                {
                    fAlreadyIgnored = true;
                    break;
                }
            }

            // OID 2093 is the separate root-level bomb placed beside Gomer's
            // launch point. Without this retail ignore relationship, the
            // freshly launched OID 2092 missile detonates against it at once.
            if (!fAlreadyIgnored)
                AddMissileIgnoreObject(pmissile, oidGomerSnowballSibling);
        }

    }

    glm::vec3 posFire{};
    glm::mat3 matFire{ 1.0f };
    glm::vec3 posTarget{};

    GetRwfiPosMat(&prwm->rwfi, &posFire, &matFire, prwm->paloFireContext);

    GetRwtiPos(&prwm->rwti, &posFire, &posTarget, prwm->paloFireContext);

    glm::vec3 dposTarget = posTarget - posFire;
    const float distance = glm::length(dposTarget);

    if (distance < 0.0001f)
        dposTarget = g_normalX;
    else
        dposTarget /= distance;

    const float adjustedDistance = std::clamp(prwm->rwti.clqDistance.g0 + distance * (prwm->rwti.clqDistance.g1 + distance * prwm->rwti.clqDistance.g2), prwm->rwti.lmDistance.gMin, prwm->rwti.lmDistance.gMax);

    posTarget = posFire + dposTarget * adjustedDistance;

    if (prwm->paloParent != nullptr && prwm->paloParent->paloRoot != nullptr)
    {
        ALO* paloRoot = prwm->paloParent->paloRoot;

        if ((paloRoot->pvtalo->grfcid & 2) != 0 && FIsBasicDerivedFrom(psoAmmo, CID_MISSILE))
            static_cast<MISSILE*>(psoAmmo)->psoIgnore = static_cast<SO*>(paloRoot);
    }

    if (prwm->paloFireContext != nullptr)
    {
        if (paloAmmo->palox == nullptr)
            paloAmmo->palox = std::make_shared <ALOX>();

        paloAmmo->palox->grfalox |= 0x0c;
        paloAmmo->palox->foster.paloParent = prwm->paloFireContext;
    }

    psoAmmo->pvtlo->pfnAddLo(psoAmmo);
    SetSoConstraints(psoAmmo, CT_Free, nullptr, CT_Free, nullptr);
    paloAmmo->pvtalo->pfnTranslateAloToPos(paloAmmo, &posFire);
    paloAmmo->pvtalo->pfnRotateAloToMat(paloAmmo, &matFire);

    RWMCLONE* pclone = prwm->prwmclone.get();

    if (pclone != nullptr && pclone->fSharedAmmo)
    {
        const int index = pclone->irwmammoxfCur;

        if (index >= 0 && static_cast<std::size_t>(index) < pclone->arwmammoxf.size())
        {
            const RWMAMMOXF& ammoXf = pclone->arwmammoxf[index];
            psoAmmo->posOrig = ammoXf.posOrig;
            psoAmmo->eulOrig = ammoXf.eulOrig;
        }
    }

    if (prwm->rwfi.palo != nullptr)
    {
        ALO* paloFire = prwm->rwfi.palo;
        glm::vec3 velocity{};
        glm::vec3 angularVelocity{};

        paloAmmo->pvtalo->pfnRotateAloToMat(paloAmmo, &paloFire->xf.matWorld);
        CalculateAloMovement(paloFire, nullptr, paloFire->xf.posWorld, &velocity, &angularVelocity, nullptr, nullptr);
        paloAmmo->pvtalo->pfnSetAloVelocityVec(paloAmmo, &velocity);
        paloAmmo->pvtalo->pfnSetAloAngularVelocityVec(paloAmmo, &angularVelocity);

    }
    else
    {
        if (fAim)
        {
            RWAC* prwac = &prwm->rwac;
            const bool fDirectAim = FIsBasicDerivedFrom(psoAmmo, CID_TARMISS) || FIsBasicDerivedFrom(psoAmmo, CID_ACCMISS);

            if (fDirectAim)
            {
                const glm::vec3 dpos = posTarget - posFire;
                float radPan = 0.0f;
                float radTilt = 0.0f;

                if ((prwac->grfask & 2) == 0)
                    radPan = std::atan2(dpos.y, dpos.x);
                else
                    GetRwacPan(prwac, &radPan, prwm->paloFireContext);

                if ((prwac->grfask & 1) == 0)
                    radTilt = std::atan2(dpos.z, glm::length(glm::vec2(dpos.x, dpos.y)));
                else
                    GetRwacTilt(prwac, &radTilt, prwm->paloFireContext);

                MISSILE* pmissile = (MISSILE*)psoAmmo;
                const float speed = (prwac->grfask & 4) != 0 ? prwac->sv : pmissile->svMissileLaunch;

                glm::mat3 matAim{ 1.0f };

                LoadRotateMatrixPanTilt(radPan, radTilt, &matAim);
                prwm->rwfi.v = matAim[0] * speed;
            }
            else
            {
                const glm::vec3 dpos = posTarget - posFire;

                if ((prwac->grfask & 2) != 0)
                {
                    const float calculatedPan = RadNormalize(std::atan2(dpos.y, dpos.x));
                    float configuredPan = 0.0f;

                    GetRwacPan(prwac, &configuredPan, prwm->paloFireContext);
                    configuredPan = RadNormalize(configuredPan);
                    FFloatsNear(calculatedPan, configuredPan, 0.0001f);
                }

                if ((prwac->grfask & 1) == 0)
                {
                    if ((prwac->grfask & 4) == 0)
                        FCalculateMuzzleVelocity(posFire, posTarget, 0.5f, prwm->rwfi.v, psoAmmo);
                    else
                        FCalculateMuzzleVelocitySpeed(posFire, posTarget, prwac->sv, prwm->rwfi.v, psoAmmo);
                }
                else
                {
                    float radTilt = 0.0f;

                    GetRwacTilt(prwac, &radTilt, prwm->paloFireContext);
                    FCalculateMuzzleVelocityAngle(posFire, posTarget, radTilt, prwm->rwfi.v, psoAmmo);
                }
            }
        }

        paloAmmo->pvtalo->pfnSetAloVelocityVec(paloAmmo, &prwm->rwfi.v);

        if (prwm->rwfi.lmSw.gMax > 0.0f)
        {
            const float angularSpeed = GRandInRange(prwm->rwfi.lmSw.gMin, prwm->rwfi.lmSw.gMax);
            glm::vec3 angularVelocity(GRandInRange(-1.0f, 1.0f), GRandInRange(-1.0f, 1.0f), GRandInRange(-1.0f, 1.0f));
            const float angularLength = glm::length(angularVelocity);

            if (angularLength < 0.0001f)
                angularVelocity = g_normalX;
            else
                angularVelocity /= angularLength;

            angularVelocity *= angularSpeed;
            paloAmmo->pvtalo->pfnSetAloAngularVelocityVec(paloAmmo, &angularVelocity);
        }
        else
            paloAmmo->pvtalo->pfnSetAloAngularVelocityVec(paloAmmo, &prwm->rwfi.w);

    }

    if (prwm->rwfi.tMax > 0.0f && FIsBasicDerivedFrom(psoAmmo, CID_BOMB))
        static_cast<BOMB*>(psoAmmo)->tDetonate = g_clock.t + prwm->rwfi.tMax;

    if (FIsBasicDerivedFrom(psoAmmo, CID_MISSILE))
    {
        if (prwm->rwti.rwmtk == RWMTK_Homing)
        {
            ALO* paloTarget = prwm->rwti.palo;
            glm::vec3* pposTarget = &prwm->rwti.pos;

            if (paloTarget == nullptr && prwm->rwti.ptarget != nullptr)
            {
                paloTarget = prwm->rwti.ptarget->paloParent;
                pposTarget = &prwm->rwti.ptarget->posLocal;
            }

            psoAmmo->pvtmissile->pfnFireMissile((MISSILE*)psoAmmo, paloTarget, pposTarget);
        }
        else
            psoAmmo->pvtmissile->pfnFireMissile((MISSILE*)psoAmmo, nullptr, &posTarget);

    }

    if (FIsBasicDerivedFrom(psoAmmo, CID_STEPGUARD))
    {
        STEPGUARD* pstepguard = static_cast<STEPGUARD*>(psoAmmo);

        SetStepguardSgs(pstepguard, SGS_Stun, nullptr);

        if (FIsBasicDerivedFrom(psoAmmo, CID_GOMER))
            static_cast<GOMER*>(psoAmmo)->fDetectLatchExternal = true;
    }

    // Object event arguments are passed as addresses of object-pointer
    // variables.  Passing psoAmmo itself made the argument marshaler read the
    // object's first field (its vtable) as though it were the object pointer.
    void* pvArgs = &psoAmmo;

    HandleLoSpliceEvent(prwm, 13, 1, &pvArgs);

    ClearRwmAmmo(prwm);

    return true;
}

void ClearRwmAmmo(RWM* prwm)
{
    prwm->psoAmmo = nullptr;

    if (prwm->prwmclone && prwm->prwmclone->fSharedAmmo)
        prwm->prwmclone->irwmammoxfCur = -1;
}

void ClearRwmFireInfo(RWM* prwm)
{
    prwm->rwfi = RWFI{};
    prwm->rwfi.mat = glm::mat3(1.0f);
}

void ClearRwmTargetInfo(RWM* prwm)
{
    prwm->rwti = RWTI{};

    prwm->rwti.clqDistance.g0 = 0.0f;
    prwm->rwti.clqDistance.g1 = 1.0f;
    prwm->rwti.clqDistance.g2 = 0.0f;

    prwm->rwti.lmDistance.gMin = 0.0f;
    prwm->rwti.lmDistance.gMax = 20000.0f;
}

void ClearRwmAimControl(RWM* prwm)
{
    prwm->rwac = RWAC{};
}

void GetRwfiPosMat(const RWFI* prwfi, glm::vec3* ppos, glm::mat3* pmat, ALO* palo)
{
    if (prwfi->ppnt != nullptr)
    {
        GetPntPos(prwfi->ppnt, ppos);
        *pmat = prwfi->mat;
    }
    else if (prwfi->pxfm != nullptr)
    {
        GetXfmPos(prwfi->pxfm, ppos);
        GetXfmMat(prwfi->pxfm, pmat);
    }
    else if (prwfi->palo != nullptr)
    {
        *ppos = prwfi->palo->xf.posWorld;
        *pmat = prwfi->palo->xf.matWorld;
    }
    else
    {
        *ppos = prwfi->pos;
        *pmat = prwfi->mat;
    }

    if (palo != nullptr)
    {
        ConvertAloPos(nullptr, palo, ppos, ppos);
        ConvertAloMat(nullptr, palo, pmat, pmat);
    }
}

void GetRwtiPos(RWTI* prwti, const glm::vec3* pposSource, glm::vec3* ppos, ALO* paloReference)
{
    ALO* paloTarget = prwti->palo;
    glm::vec3* pposTarget = &prwti->pos;

    if (paloTarget == nullptr && prwti->ptarget != nullptr)
    {
        paloTarget = prwti->ptarget->paloParent;
        pposTarget = &prwti->ptarget->posLocal;
    }

    if (prwti->rwmtk == RWMTK_Current)
    {
        ConvertAloPos(paloTarget, paloReference, pposTarget, ppos);
        return;
    }

    if (prwti->rwmtk <= RWMTK_Nil || prwti->rwmtk >= RWMTK_Max)
        return;

    glm::vec3 posTargetWorld{};
    ConvertAloPos(paloTarget, nullptr, pposTarget, &posTargetWorld);

    const float distance = glm::length(posTargetWorld - *pposSource);
    const float dtCalculated = prwti->clqSToDtPredict.g0 + distance * (prwti->clqSToDtPredict.g1 + distance * prwti->clqSToDtPredict.g2);
    const float dtPredict = std::clamp(dtCalculated, prwti->lmDtPredict.gMin, prwti->lmDtPredict.gMax);

    glm::vec3 posPredicted{};
    glm::mat3 matPredicted{ 1.0f };

    PredictAloTransform(paloTarget, nullptr, dtPredict, &posPredicted, &matPredicted, nullptr, nullptr);
    *ppos = posPredicted + matPredicted * *pposTarget;

    if (paloReference != nullptr)
    {
        glm::vec3 posReference{};
        glm::mat3 matReference{ 1.0f };

        PredictAloTransform(paloReference, nullptr, dtPredict, &posReference, &matReference, nullptr, nullptr);
        *ppos = glm::transpose(matReference) * (*ppos - posReference);
    }
}

void GetRwacPan(const RWAC* prwac, float* pradPan, ALO* paloReference)
{
    if (prwac->pxfm != nullptr)
    {
        glm::mat3 mat{};
        GetXfmMat(prwac->pxfm, &mat);
        ConvertAloMat(nullptr, paloReference, &mat, &mat);
        *pradPan = std::atan2(mat[0].y, mat[0].x);
    }
    else if (prwac->palo != nullptr)
    {
        glm::mat3 mat{};
        ConvertAloMat(nullptr, paloReference, &prwac->palo->xf.matWorld, &mat);
        *pradPan = std::atan2(mat[0].y, mat[0].x);
    }
    else
        *pradPan = prwac->radPan;
}

void GetRwacTilt(const RWAC* prwac, float* pradTilt, ALO* paloReference)
{
    if (prwac->pxfm == nullptr)
    {
        *pradTilt = prwac->radTilt;
        return;
    }

    glm::mat3 mat{};
    GetXfmMat(prwac->pxfm, &mat);
    ConvertAloMat(nullptr, paloReference, &mat, &mat);

    const glm::vec3 forward = mat[0];
    const float horizontalLength = glm::length(glm::vec2(forward.x, forward.y));
    *pradTilt = std::atan2(forward.z, horizontalLength);
}

int GetRwmSize()
{
	return sizeof(RWM);
}

void DeleteRwm(RWM* prwm)
{
	delete prwm;
}
