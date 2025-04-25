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

void DeleteTarget(TARGET* ptarget)
{
	delete ptarget;
}
