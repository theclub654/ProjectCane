#include "zap.h"

void InitTzp(TZP* ptzp)
{
	// VOLZP is the same size as TZP
	//std::cout << "TZP Size: " << sizeof(TZP) << "\n";
	InitSo(ptzp);
}

void DeleteTzp(LO* plo)
{
	delete (TZP*)plo;
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
