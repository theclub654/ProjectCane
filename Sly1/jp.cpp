#include "jp.h"

void* NewJp()
{
	return new JP;
}

void InitJp(JP* pjp)
{
	InitStep(pjp);
}

int GetJpSize()
{
	return sizeof(JP);
}

void CloneJp(JP* pjp, JP* pjpBase)
{
	LO lo = *pjp;
	*pjp = *pjpBase;
	memcpy(pjp, &lo, sizeof(LO));

	CloneLo(pjp, pjpBase);

	ClearDl(&pjp->dlChild);

	pjp->pxa = nullptr;
	pjp->grfpvaXpValid = 0;
	pjp->pstso = nullptr;
}

void RenderJpAll(JP* pjp, CM* pcm, RO* pro)
{

}

void DeleteJp(LO* plo)
{
	delete (JP*)plo;
}
