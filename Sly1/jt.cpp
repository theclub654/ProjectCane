#include "jt.h"

JT* NewJt()
{
	return new JT{};
}

void InitJt(JT* pjt)
{
	InitStep(pjt);
	pjt->jts = JTS_Nil;
	pjt->jtcs = JTCS_Nil;
	pjt->jtbs = JTBS_Nil;
	pjt->jtjk = JTJK_Nil;
}

void LoadJtFromBrx(JT* pjt, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pjt, pbis);
	pjt->tSweepPending = -1.0;
	pjt->tJumpPending = -1.0;
	g_pjt = pjt;
}

void CloneJt(JT* pjt, JT* pjtBase)
{
	LO lo = *pjt;
	*pjt = *pjtBase;
	memcpy(pjt, &lo, sizeof(LO));

	CloneLo(pjt, pjtBase);

	ClearDl(&pjt->dlChild);

	pjt->pxa = nullptr;
	pjt->grfpvaXpValid = 0;
	pjt->pstso = nullptr;
}

void RenderJtAll(JT* pjt, CM* pcm, RO* pro)
{
	
}

void RenderJtSelf(JT* pjt, CM* pcm, RO* pro)
{

}

int GetJtSize()
{
	return sizeof(JT);
}

void DeleteJt(LO* plo)
{
	delete (JT*)plo;
}
