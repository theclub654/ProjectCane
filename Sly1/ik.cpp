#include "ik.h"

IKH* NewIkh()
{
	return new IKH{};
}

int GetIkhSize()
{
	return sizeof(IKH);
}

void CloneIkh(IKH* pikh, IKH* pikhBase)
{
	LO lo = *pikh;
	*pikh = *pikhBase;
	memcpy(pikh, &lo, sizeof(LO));

	CloneLo(pikh, pikhBase);

	ClearDl(&pikh->dlChild);

	pikh->pxa = nullptr;
	pikh->grfpvaXpValid = 0;
	pikh->pstso = nullptr;
}

void DeleteIkh(LO* plo)
{
	delete(IKH*)plo;
}

LIKH* NewLikh()
{
	return new LIKH{};
}

int GetLikhSize()
{
	return sizeof(LIKH);
}

void CloneLikh(LIKH* plikh, LIKH* plikhBase)
{
	LO lo = *plikh;
	*plikh = *plikhBase;
	memcpy(plikh, &lo, sizeof(LO));

	CloneLo(plikh, plikhBase);

	ClearDl(&plikh->dlChild);
}

void DeleteLikh(LO* plo)
{
	delete(LIKH*)plo;
}
