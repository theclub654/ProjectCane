#include "zap.h"

TZP* NewTzp()
{
	return new TZP{};
}

void InitTzp(TZP* ptzp)
{
	InitSo(ptzp);
}

int GetTzpSize()
{
	return sizeof(TZP);
}

void CloneTzp(TZP* ptzp, TZP* ptzpBase)
{
	LO lo = *ptzp;
	*ptzp = *ptzpBase;
	memcpy(ptzp, &lo, sizeof(LO));

	CloneLo(ptzp, ptzpBase);

	ClearDl(&ptzp->dlChild);

	ptzp->pxa = nullptr;
	ptzp->grfpvaXpValid = 0;
	ptzp->pstso = nullptr;
}

void DeleteTzp(LO* plo)
{
	delete (TZP*)plo;
}

VOLZP* NewVolzp()
{
	return new VOLZP{};
}

void InitVolzp(VOLZP* pvolzp)
{
	InitTzp(pvolzp);
}

int GetVolzpSize()
{
	return sizeof(VOLZP);
}

void CloneVolzp(VOLZP* pvolzp, VOLZP* pvolzpBase)
{
	LO lo = *pvolzp;
	*pvolzp = *pvolzpBase;
	memcpy(pvolzp, &lo, sizeof(LO));

	CloneLo(pvolzp, pvolzpBase);

	ClearDl(&pvolzp->dlChild);

	pvolzp->pxa = nullptr;
	pvolzp->grfpvaXpValid = 0;
	pvolzp->pstso = nullptr;
}

void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro)
{

}

void DeleteVolzp(LO* plo)
{
	delete (VOLZP*)plo;
}
