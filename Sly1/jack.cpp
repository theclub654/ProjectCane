#include "jack.h"

void* NewJack()
{
	return new JACK{};
}

void InitJack(JACK* pjack)
{
	InitPo(pjack);
}

int GetJackSize()
{
	return sizeof(JACK);
}

void CloneJack(JACK* pjack, JACK* pjackBase)
{
	LO lo = *pjack;
	*pjack = *pjackBase;
	memcpy(pjack, &lo, sizeof(LO));

	CloneLo(pjack, pjackBase);

	ClearDl(&pjack->dlChild);

	pjack->pxa = nullptr;
	pjack->grfpvaXpValid = 0;
	pjack->pstso = nullptr;
}

void DeleteJack(LO* plo)
{
	delete (JACK*)plo;
}

void* NewJackb()
{
	return new JACKB{};
}

void InitJackb(JACKB* pjackb)
{
	InitAlo(pjackb);
}

int GetJackbSize()
{
	return sizeof(JACKB);
}

void CloneJackb(JACKB* pjackb, JACKB* pjackbBase)
{
	LO lo = *pjackb;
	*pjackb = *pjackbBase;
	memcpy(pjackb, &lo, sizeof(LO));

	CloneLo(pjackb, pjackbBase);

	ClearDl(&pjackb->dlChild);
}

void DeleteJackb(LO* plo)
{
	delete(JACKB*)plo;
}

void* NewJackn()
{
	return new JACKN{};
}

void InitJackn(JACKN* pjackn)
{
	InitAlo(pjackn);
}

int GetJacknSize()
{
	return sizeof(JACKN);
}

void LoadJacknFromBrx(JACKN* pjackn, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjackn, pbis);
}

void CloneJackn(JACKN* pjackn, JACKN* pjacknBase)
{
	LO lo = *pjackn;
	*pjackn = *pjacknBase;
	memcpy(pjackn, &lo, sizeof(LO));

	CloneLo(pjackn, pjacknBase);

	ClearDl(&pjackn->dlChild);
}

void UpdateJackn(JACKN* pjackn, float dt)
{
	pjackn->pvtalo->pfnTranslateAloToPos(pjackn, pjackn->xf.pos);
}

void DeleteJackn(LO* plo)
{
	delete(JACKN*)plo;
}

void* NewJackf()
{
	return new JACKF{};
}

void InitJackf(JACKF* pjackf)
{
	InitAlo(pjackf);
}

int GetJackfSize()
{
	return sizeof(JACKF);
}

void CloneJackf(JACKF* pjackf, JACKF* pjackfBase)
{
	LO lo = *pjackf;
	*pjackf = *pjackfBase;
	memcpy(pjackf, &lo, sizeof(LO));

	CloneLo(pjackf, pjackfBase);

	ClearDl(&pjackf->dlChild);
}

void DeleteJackf(LO* plo)
{
	delete(JACKF*)plo;
}