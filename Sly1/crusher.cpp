#include "crusher.h"

void InitCrfodb(CRFODB* pcrfodb)
{
	std::cout << "CRFODB Size: " << sizeof(CRFODB) << "\n";
	std::cout << "CRBRAIN Size: " << sizeof(CRBRAIN) << "\n";
	InitStepGuard(pcrfodb);
}

void OnCrfodAdd(CRFOD* pcrfod)
{
	OnStepguardAdd(pcrfod);
	AppendDlEntry(&pcrfod->psw->dlCrfod, pcrfod);
}

void InitCrbrain(CRBRAIN* pcrbrain)
{
	InitAlo(pcrbrain);
}
