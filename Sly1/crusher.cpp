#include "crusher.h"

void* NewCrfodb()
{
	return new CRFODB;
}

void InitCrfodb(CRFODB* pcrfodb)
{
	InitStepGuard(pcrfodb);
}

void DeleteCrfodb(LO* plo)
{
	delete (CRFODB*)plo;
}

void* NewCrfod()
{
	return new CRFOD;
}

void OnCrfodAdd(CRFOD* pcrfod)
{
	OnStepguardAdd(pcrfod);
	AppendDlEntry(&pcrfod->psw->dlCrfod, pcrfod);
}

void DeleteCrfod(LO* plo)
{
	delete (CRFOD*)plo;
}

void* NewCrfodk()
{
	return new CRFODK;
}

void DeleteCrfodk(LO* plo)
{
	delete (CRFODK*)plo;
}

void* NewCrbrain()
{
	return new CRBRAIN;
}

void InitCrbrain(CRBRAIN* pcrbrain)
{
	InitAlo(pcrbrain);
}

void DeleteCrbrain(LO* plo)
{
	delete(CRBRAIN*)plo;
}
