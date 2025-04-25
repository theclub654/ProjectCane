#include "cycle.h"

CYCLE* NewCycle()
{
	return new CYCLE{};
}

void InitCycle(CYCLE* pcycle)
{
	InitPo(pcycle);
}

int GetCycleSize()
{
	return sizeof(CYCLE);
}

void UpdateCycleXfWorld(CYCLE* pcycle)
{
	UpdateSoXfWorld(pcycle);
}

void CloneCycle(CYCLE* pcycle, CYCLE* pcycleBase)
{
    ClonePo(pcycle, pcycleBase);

    pcycle->sRadiusFrontWheel = pcycleBase->sRadiusFrontWheel;
    pcycle->sRadiusRearWheel = pcycleBase->sRadiusRearWheel;
    pcycle->svMax = pcycleBase->svMax;
    pcycle->radTarget = pcycleBase->radTarget;
    pcycle->svTarget = pcycleBase->svTarget;
    pcycle->radFront = pcycleBase->radFront;
    pcycle->radBankTarget = pcycleBase->radBankTarget;

    pcycle->xsxp = pcycleBase->xsxp;
    pcycle->asxp[0] = pcycleBase->asxp[0];
    pcycle->asxp[1] = pcycleBase->asxp[1];

    pcycle->pexplBoost = pcycleBase->pexplBoost;
    pcycle->pexplDirt = pcycleBase->pexplDirt;
    pcycle->pexplDust = pcycleBase->pexplDust;

    pcycle->cParticleBoost = pcycleBase->cParticleBoost;
    pcycle->cParticleDirt = pcycleBase->cParticleDirt;
    pcycle->cParticleDust = pcycleBase->cParticleDust;

    pcycle->tBoost = pcycleBase->tBoost;
    pcycle->cBoost = pcycleBase->cBoost;
    pcycle->tGround = pcycleBase->tGround;

    pcycle->cycles = pcycleBase->cycles;
    pcycle->tCycles = pcycleBase->tCycles;
}

void RenderCycleSelf(CYCLE* pcycle, CM* pcm, RO* pro)
{
	RenderSoSelf(pcycle, pcm, pro);
}

void DeleteCycle(CYCLE *pcycle)
{
	delete pcycle;
}
