#include "rwm.h"

RWM* NewRwm()
{
	return new RWM{};
}

void InitRwm(RWM* prwm)
{
	InitLo(prwm);
}

void PostRwmLoad(RWM* prwm)
{
    PostLoLoad(prwm);
}

void CloneRwm(RWM* prwm, RWM* prwmBase)
{
    CloneLo(prwm, prwmBase); // Assuming CloneLo handles copying the base class (LO) members

    // Shallow copy of value members
    prwm->crwc = prwmBase->crwc;
    prwm->cspk = prwmBase->cspk;
    prwm->irwc = prwmBase->irwc;
    prwm->oidFire = prwmBase->oidFire;
    prwm->oidTarget = prwmBase->oidTarget;
    prwm->oidAim = prwmBase->oidAim;
    prwm->rwfi = prwmBase->rwfi;
    prwm->rwti = prwmBase->rwti;
    prwm->rwac = prwmBase->rwac;
    prwm->fPostLoad = prwmBase->fPostLoad;

    // Shallow copy of array of RWC objects (assuming RWC is a struct)
    for (int i = 0; i < 4; ++i)
    {
        prwm->arwc[i] = prwmBase->arwc[i];
    }

    // Shallow copy of the pointer member
    prwm->psoAmmo = prwmBase->psoAmmo;
}

int GetRwmSize()
{
	return sizeof(RWM);
}

void DeleteRwm(RWM* prwm)
{
	delete prwm;
}
