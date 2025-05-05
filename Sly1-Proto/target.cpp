#include "target.h"

TARGET* NewTarget()
{
	return new TARGET{};
}

void InitTarget(TARGET* ptarget)
{
	InitXfm(ptarget);
	ptarget->grftak = 0x1e;
	ptarget->sRadiusTarget = 25.0;
}

int GetTargetSize()
{
	return sizeof(TARGET);
}

void OnTargetAdd(TARGET* ptarget)
{
	OnLoAdd(ptarget);
}

void OnTargetRemove(TARGET* ptarget)
{
	OnLoRemove(ptarget);
	//RemoveDlEntry(&g_dlTarget, ptarget);
}

void CloneTarget(TARGET* ptarget, TARGET* ptargetBase)
{
	CloneXfm(ptarget, ptargetBase);

	ptarget->dleTarget = ptargetBase->dleTarget;
	ptarget->grftak = ptargetBase->grftak;
	ptarget->sRadiusTarget = ptargetBase->sRadiusTarget;
	ptarget->fHitTest = ptargetBase->fHitTest;
}

void RenderTarget(TARGET* ptarget, CM* pcm)
{
	LO *ptarget1 = ptarget->psw->aploStock[0xf];

	if (ptarget != nullptr)
	{
		RO ro;

		ro.model = glm::identity <glm::mat4>();
		ro.uAlpha = 1.0;
		ptarget1->pvtalo->pfnRenderAloAll((ALO*)ptarget1, pcm, &ro);
	}
}

TARGET* PtargetEnsureAlo(ALO* palo)
{
	TARGET *ptarget = (TARGET*)PloFindSwObject(palo->psw, 0x102, OID_ensured_target, palo);

	if (ptarget == nullptr) 
		ptarget = (TARGET*)PloNew(CID_TARGET, palo->psw, palo, OID_ensured_target, -1);

	return ptarget;
}

void SetAloTargetHitTest(ALO* palo, int fHitTest)
{
	TARGET *ptarget = PtargetEnsureAlo(palo);
	ptarget->fHitTest = fHitTest;
}

void DeleteTarget(TARGET* ptarget)
{
	delete ptarget;
}
