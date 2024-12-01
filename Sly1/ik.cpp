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

void RenderIkhSelf(IKH* pikh, CM* pcm, RO* pro)
{
	RenderSoSelf(pikh, pcm, pro);
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

void RenderLikhSelf(LIKH* plikh, CM* pcm, RO* pro)
{
	RenderAloSelf(plikh, pcm, pro);
}

void DeleteLikh(LO* plo)
{
	delete(LIKH*)plo;
}
