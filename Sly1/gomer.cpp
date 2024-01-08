#include "gomer.h"

void* NewGomer()
{
	return new GOMER;
}

void InitGomer(GOMER* pgomer)
{
	InitStepGuard(pgomer);
}

int GetGomerSize()
{
	return sizeof(GOMER);
}

void CloneGomer(GOMER* pgomer, GOMER* pgomerBase)
{
	LO lo = *pgomer;
	*pgomer = *pgomerBase;
	memcpy(pgomer, &lo, sizeof(LO));

	CloneLo(pgomer, pgomerBase);

	ClearDl(&pgomer->dlChild);

	pgomer->pxa = nullptr;
	pgomer->grfpvaXpValid = 0;
	pgomer->pstso = nullptr;
}

void DeleteGomer(LO* plo)
{
	delete (GOMER*)plo;
}
