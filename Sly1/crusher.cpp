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
	LO lo = *pcrfodb;
	*pcrfodb = *pcrfodbBase;
	memcpy(pcrfodb, &lo, sizeof(LO));

	CloneLo(pcrfodb, pcrfodbBase);

	ClearDl(&pcrfodb->dlChild);

	pcrfodb->pxa = nullptr;
	pcrfodb->grfpvaXpValid = 0;
	pcrfodb->pstso = nullptr;
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
	LO lo = *pcrfod;
	*pcrfod = *pcrfodBase;
	memcpy(pcrfod, &lo, sizeof(LO));

	CloneLo(pcrfod, pcrfodBase);

	ClearDl(&pcrfod->dlChild);

	pcrfod->pxa = nullptr;
	pcrfod->grfpvaXpValid = 0;
	pcrfod->pstso = nullptr;
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
	LO lo = *pcrfodk;
	*pcrfodk = *pcrfodkBase;
	memcpy(pcrfodk, &lo, sizeof(LO));

	CloneLo(pcrfodk, pcrfodkBase);

	ClearDl(&pcrfodk->dlChild);

	pcrfodk->pxa = nullptr;
	pcrfodk->grfpvaXpValid = 0;
	pcrfodk->pstso = nullptr;
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
	LO lo = *pcrbrain;
	*pcrbrain = *pcrbrainBase;
	memcpy(pcrbrain, &lo, sizeof(LO));

	CloneLo(pcrbrain, pcrbrainBase);
}

void DeleteCrbrain(LO* plo)
{
	delete(CRBRAIN*)plo;
}