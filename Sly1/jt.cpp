#include "jt.h"

void* NewJt()
{
	return new JT;
}

void InitJt(JT* pjt)
{
	InitStep(pjt);
}

void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pjt, pbis);
	pjt->tSweepPending = -1.0;
	pjt->tJumpPending = -1.0;
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
