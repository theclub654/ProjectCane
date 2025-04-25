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
	CloneSo(ptzp, ptzpBase);

	ptzp->zpd = ptzpBase->zpd;

	// Clone array of RZPR elements
	ptzp->crzpr = ptzpBase->crzpr;
	for (int i = 0; i < ptzp->crzpr; ++i)
	{
		ptzp->arzpr[i] = ptzpBase->arzpr[i];
	}
}

void DeleteTzp(TZP *ptzp)
{
	delete ptzp;
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
	CloneTzp(pvolzp, pvolzpBase);
}

void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro)
{
	RenderAloAll(ptzp, pcm, pro);
}

void DeleteVolzp(VOLZP *pvolzp)
{
	delete pvolzp;
}
