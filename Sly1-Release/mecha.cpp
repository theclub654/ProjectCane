#include "mecha.h"

MECHA* NewMecha()
{
	return new MECHA{};
}

void InitMecha(MECHA* pmecha)
{
	InitStep(pmecha);
}

int GetMechaSize()
{
	return sizeof(MECHA);
}

void CloneMecha(MECHA* pmecha, MECHA* pmechaBase)
{
    CloneStep(pmecha, pmechaBase);

    pmecha->mechas = pmechaBase->mechas;
    pmecha->tMechas = pmechaBase->tMechas;
    pmecha->pasegaCur = pmechaBase->pasegaCur;
    pmecha->psmaCur = pmechaBase->psmaCur;
    pmecha->fRetractAsega = pmechaBase->fRetractAsega;
    pmecha->psmIdle = pmechaBase->psmIdle;
    for (int i = 0; i < 6; ++i)
        pmecha->ablrunRun[i] = pmechaBase->ablrunRun[i];
    pmecha->pasegblRun = pmechaBase->pasegblRun;
    pmecha->sffRun = pmechaBase->sffRun;
    pmecha->pasegKick0 = pmechaBase->pasegKick0;
    pmecha->pasegKick1 = pmechaBase->pasegKick1;
    pmecha->psoKickEffect = pmechaBase->psoKickEffect;
    pmecha->pxpEffect = pmechaBase->pxpEffect;
    pmecha->cpaloIgnoreEffect = pmechaBase->cpaloIgnoreEffect;
    for (int i = 0; i < 16; ++i)
        pmecha->apaloIgnoreEffect[i] = pmechaBase->apaloIgnoreEffect[i];
    pmecha->psmGun = pmechaBase->psmGun;
    pmecha->psmaGun = pmechaBase->psmaGun;
    pmecha->pasegJump = pmechaBase->pasegJump;
    pmecha->vTargetCoast = pmechaBase->vTargetCoast;
}

void RenderMechaSelf(MECHA* pmecha, CM* pcm, RO* pro)
{

}

void DeleteMecha(MECHA *pmecha)
{
	delete pmecha;
}
