#include "crusher.h"

CRFODB* NewCrfodb()
{
	return new CRFODB{};
}

void InitCrfodb(CRFODB* pcrfodb)
{
	InitStepGuard(pcrfodb);
}

int GetCrfodbSize()
{
	return sizeof(CRFODB);
}

void CloneCrfodb(CRFODB* pcrfodb, CRFODB* pcrfodbBase)
{
	CloneCrfod(pcrfodb, pcrfodbBase);

	pcrfodb->fDetectLatch = pcrfodbBase->fDetectLatch;
	pcrfodb->cpoint = pcrfodbBase->cpoint;
}

void DeleteCrfodb(CRFODB* pcrfodb)
{
	delete pcrfodb;
}

CRFOD* NewCrfod()
{
	return new CRFOD{};
}

void InitSwCrfodDl(SW* psw)
{
	InitDl(&psw->dlCrfod, offsetof(CRFOD, dleCrfod));
}

void OnCrfodAdd(CRFOD* pcrfod)
{
	OnStepguardAdd(pcrfod);
	AppendDlEntry(&pcrfod->psw->dlCrfod, pcrfod);
}

void OnCrfodRemove(CRFOD* pcrfod)
{
	OnStepguardRemove(pcrfod);
	RemoveDlEntry(&pcrfod->psw->dlCrfod, pcrfod);
}

void CloneCrfod(CRFOD* pcrfod, CRFOD* pcrfodBase)
{
	CloneStepguard(pcrfod, pcrfodBase);

	pcrfod->dleCrfod = pcrfodBase->dleCrfod;
}

int GetCrfodSize()
{
	return sizeof(CRFOD);
}

void DeleteCrfod(CRFOD* pcrfod)
{
	delete pcrfod;
}

CRFODK* NewCrfodk()
{
	return new CRFODK{};
}

int GetCrfodkSize()
{
	return sizeof(CRFODK);
}

void CloneCrfodk(CRFODK* pcrfodk, CRFODK* pcrfodkBase)
{
	CloneCrfod(pcrfodk, pcrfodkBase);
}

void DeleteCrfodk(CRFODK* pcrfodk)
{
	delete pcrfodk;
}

CRBRAIN* NewCrbrain()
{
	return new CRBRAIN{};
}

void InitCrbrain(CRBRAIN* pcrbrain)
{
	InitAlo(pcrbrain);
}

int GetCrbrainSize()
{
	return sizeof(CRBRAIN);
}

void CloneCrbrain(CRBRAIN* pcrbrain, CRBRAIN* pcrbrainBase)
{
	CloneAlo(pcrbrain, pcrbrainBase);

	pcrbrain->cpoint = pcrbrainBase->cpoint;
	pcrbrain->cpointMax = pcrbrainBase->cpointMax;
	pcrbrain->tContest = pcrbrainBase->tContest;
}

void DeleteCrbrain(CRBRAIN* pcrbrain)
{
	delete pcrbrain;
}