#include "mb.h"

MBG* NewMbg()
{
	return new MBG{};
}

void InitMbg(MBG *pmbg)
{
	InitStepGuard(pmbg);
}

int GetMbgSize()
{
	return sizeof(MBG);
}

void LoadMbgFromBrx(MBG* pmbg, CBinaryInputStream* pbis)
{
	LoadStepGuardFromBrx(pmbg, pbis);
}

void CloneMbg(MBG* pmbg, MBG* pmbgBase)
{
    CloneStepguard(pmbg, pmbgBase);

    // Clone scalar members
    pmbg->mbak = pmbgBase->mbak;
    pmbg->tMbsk = pmbgBase->tMbsk;
    pmbg->fAbandonExternal = pmbgBase->fAbandonExternal;
    pmbg->fLeftFootDown = pmbgBase->fLeftFootDown;
    pmbg->fRightFootDown = pmbgBase->fRightFootDown;

    // Clone the mpmbakpaseg array (pointers to ASEG*)
    for (int i = 0; i < 3; ++i)
    {
        pmbg->mpmbakpaseg[i] = pmbgBase->mpmbakpaseg[i]; // Direct pointer copy (if deep copy required, allocate new memory)
    }

    // Clone the mpmbakprwm array (pointers to RWM*)
    for (int i = 0; i < 3; ++i)
    {
        pmbg->mpmbakprwm[i] = pmbgBase->mpmbakprwm[i]; // Direct pointer copy (if deep copy required, allocate new memory)
    }

    // Clone mbsk (assumed to be a structure)
    pmbg->mbsk = pmbgBase->mbsk;

    // Clone the mpmbskpxfm array (pointers to XFM*)
    for (int i = 0; i < 4; ++i)
    {
        pmbg->mpmbskpxfm[i] = pmbgBase->mpmbskpxfm[i]; // Direct pointer copy (if deep copy required, allocate new memory)
    }

    // Clone the ablrun array (BLRUN structures)
    for (int i = 0; i < 6; ++i)
    {
        pmbg->ablrun[i] = pmbgBase->ablrun[i]; // Direct copy of BLRUN structures
    }

    // Clone the pasegblRun (pointer to ASEGBL)
    pmbg->pasegblRun = pmbgBase->pasegblRun;
}

void PostMbgLoad(MBG* pmbg)
{
    PostStepguardLoad(pmbg);
}

void DeleteMbg(MBG* pmbg)
{
	delete pmbg;
}

BHG* NewBhg()
{
	return new BHG{};
}

void InitBhg(BHG *pbhg)
{
	InitStepGuard(pbhg);
}

int GetBhgSize()
{
	return sizeof(BHG);
}

void CloneBhg(BHG* pbhg, BHG* pbhgBase)
{
    CloneStepguard(pbhg, pbhgBase);

    // Clone scalar members
    pbhg->oidScentmap = pbhgBase->oidScentmap;
    pbhg->tScent = pbhgBase->tScent;

    // Shallow copy of the pscentmap pointer
    pbhg->pscentmap = pbhgBase->pscentmap;
}

void PostBhgLoad(BHG* pbhg)
{
    PostStepguardLoad(pbhg);
}

void DeleteBhg(BHG* phg)
{
	delete phg;
}

SCENTMAP* NewScentmap()
{
	return new SCENTMAP{};
}

void InitScentmap(SCENTMAP* pscentmap)
{
	InitAlo(pscentmap);
}

int GetScentmapSize()
{
	return sizeof(SCENTMAP);
}

void CloneScentmap(SCENTMAP* pscentmap, SCENTMAP* pscentmapBase)
{
    CloneAlo(pscentmap, pscentmapBase);

    pscentmap->cxMax = pscentmapBase->cxMax;
    pscentmap->cyMax = pscentmapBase->cyMax;
    pscentmap->cscp = pscentmapBase->cscp;
    pscentmap->ascp = pscentmapBase->ascp;
    pscentmap->mpixiypscpFirst = pscentmapBase->mpixiypscpFirst;
    pscentmap->posMin = pscentmapBase->posMin;
    pscentmap->posMax = pscentmapBase->posMax;
}

void PostScentmapLoad(SCENTMAP* pscentmap)
{
    PostAloLoad(pscentmap);
}

void UpdateScentmap(SCENTMAP* pscentmap, float dt)
{
    UpdateAlo(pscentmap, dt);
}

void DeleteScentmap(SCENTMAP* pscentmap)
{
	delete pscentmap;
}