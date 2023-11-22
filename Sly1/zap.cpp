#include "zap.h"

void* NewTzp()
{
	return new TZP;
}

void InitTzp(TZP* ptzp)
{
	InitSo(ptzp);
}

void DeleteTzp(LO* plo)
{
	delete (TZP*)plo;
}

void* NewVolzp()
{
	return new VOLZP;
}

void InitVolzp(VOLZP* pvolzp)
{
	InitTzp(pvolzp);
}

void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro)
{

}

void DeleteVolzp(LO* plo)
{
	delete (VOLZP*)plo;
}
