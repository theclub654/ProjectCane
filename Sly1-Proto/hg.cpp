#include "hg.h"

HG* NewHg()
{
	return new HG{};
}

void InitHg(HG* phg)
{
	InitStep(phg);
}

int GetHgSize()
{
	return sizeof(HG);
}

void CloneHg(HG* phg, HG* phgBase)
{
    CloneStep(phg, phgBase);

    phg->hgs = phgBase->hgs;
    phg->tHgs = phgBase->tHgs;
    phg->pasegaCur = phgBase->pasegaCur;
    phg->swPan = phgBase->swPan;
    phg->radBank = phgBase->radBank;
    phg->radTilt = phgBase->radTilt;

    // Clone ablrunRun array
    for (int i = 0; i < 6; ++i)
    {
        phg->ablrunRun[i] = phgBase->ablrunRun[i];
    }

    phg->pasegblRun = phgBase->pasegblRun;
    phg->sffRun = phgBase->sffRun;
    phg->pasegGlide = phgBase->pasegGlide;
}

void RenderHgSelf(HG* phg, CM* pcm, RO* pro)
{
	RenderStepSelf(phg, pcm, pro);
}

void DeleteHg(HG *phg)
{
	delete phg;
}
