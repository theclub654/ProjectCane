#include "crusher.h"

void InitCrfodb(CRFODB* pcrfodb)
{
	InitStepGuard(pcrfodb);
}

void OnCrfodAdd(CRFOD* pcrfod)
{
	OnStepguardAdd(pcrfod);
}

void InitCrbrain(CRBRAIN* pcrbrain)
{
	InitAlo(pcrbrain);
}
