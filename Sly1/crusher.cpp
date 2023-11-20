#include "crusher.h"

void* CreateCrfodb()
{
	CRFODB crfodb;
	return &crfodb;
}

void InitCrfodb(CRFODB* pcrfodb)
{
	std::cout << "CRFODB Size: " << sizeof(CRFODB) << "\n";
	std::cout << "CRBRAIN Size: " << sizeof(CRBRAIN) << "\n";
	InitStepGuard(pcrfodb);
}

void DeleteCrfodb(LO* plo)
{
	delete (CRFODB*)plo;
}

void* CreateCrfod()
{
	CRFOD crfod;
	return &crfod;
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

void* CreateCrfodk()
{
	CRFODK crfodk;
	return &crfodk;
}

void DeleteCrfodk(LO* plo)
{
	delete (CRFODK*)plo;
}

void InitCrbrain(CRBRAIN* pcrbrain)
{
	InitAlo(pcrbrain);
}

void DeleteCrbrain(LO* plo)
{
	delete(CRBRAIN*)plo;
}
