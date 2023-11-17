#include "zap.h"

void InitTzp(TZP* ptzp)
{
	// VOLZP is the same size as TZP
	//std::cout << "TZP Size: " << sizeof(TZP) << "\n";
	InitSo(ptzp);
}

void InitVolzp(VOLZP* pvolzp)
{
	InitTzp(pvolzp);
}

void RenderTzpAll(TZP* ptzp, CM* pcm, RO* pro)
{

}
