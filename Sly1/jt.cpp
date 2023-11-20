#include "jt.h"

void* CreateJt()
{
	JT jt;
	return &jt;
}

void InitJt(JT* pjt)
{
	InitStep(pjt);
}

void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pjt, pbis);
	g_pjt = pjt;
}

void RenderJtAll(JT* pjt, CM* pcm, RO* pro)
{
	
}

void RenderJtSelf(JT* pjt, CM* pcm, RO* pro)
{

}

void DeleteJt(LO* plo)
{
	delete (JT*)plo;
}
