#include "jlo.h"

JLOVOL* NewJlovol()
{
	return new JLOVOL{};
}

void InitJlovol(JLOVOL* pjlovol)
{
	InitVolbtn(pjlovol);
}

int GetJlovolSize()
{
	return sizeof(JLOVOL);
}

void CloneJlovol(JLOVOL* pjlovol, JLOVOL* pjlovolBase)
{
	LO lo = *pjlovol;
	*pjlovol = *pjlovolBase;
	memcpy(pjlovol, &lo, sizeof(LO));

	CloneLo(pjlovol, pjlovolBase);

	ClearDl(&pjlovol->dlChild);

	pjlovol->pxa = nullptr;
	pjlovol->grfpvaXpValid = 0;
	pjlovol->pstso = nullptr;
}

void DeleteJlovol(JLOVOL* pjlovol)
{
	delete pjlovol;
}

JLO* NewJlo()
{
	return new JLO{};
}

void InitJlo(JLO* pjlo)
{
	InitSo(pjlo);
	InitDl(&pjlo->dlJlovol, offsetof(JLOVOL, dleJlo));
}

int GetJloSize()
{
	return sizeof(JLO);
}

void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pjlo, pbis);
}

void CloneJlo(JLO* pjlo, JLO* pjloBase)
{
	LO lo = *pjlo;
	*pjlo = *pjloBase;
	memcpy(pjlo, &lo, sizeof(LO));

	CloneLo(pjlo, pjloBase);

	ClearDl(&pjlo->dlChild);

	pjlo->pxa = nullptr;
	pjlo->grfpvaXpValid = 0;
	pjlo->pstso = nullptr;
}

void DeleteJlo(LO* plo)
{
	delete (JLO*)plo;
}

JLOC* NewJloc()
{
	return new JLOC{};
}

void InitJloc(JLOC* pjloc)
{
	InitAlo(pjloc);
}

int GetJlocSize()
{
	return sizeof(JLOC);
}

void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjloc, pbis);
}

void CloneJloc(JLOC* pjloc, JLOC* pjlocBase)
{
	LO lo = *pjloc;
	*pjloc = *pjlocBase;
	memcpy(pjloc, &lo, sizeof(LO));

	CloneLo(pjloc, pjlocBase);

	ClearDl(&pjloc->dlChild);
}

void DeleteJloc(LO* plo)
{
	delete(JLOC*)plo;
}
