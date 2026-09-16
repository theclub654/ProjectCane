#include "target.h"

void StartupTarget()
{
	InitDl(&g_dlTarget, offsetof(TARGET, dleTarget));
}

void ResetTargetList()
{
	ClearDl(&g_dlTarget);
}

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

void* GetTargetGrftak(TARGET* ptarget)
{
	return &ptarget->grftak;
}

void SetTargetGrftak(TARGET* ptarget, GRFTAK grftak)
{
	ptarget->grftak = grftak;
}

void* GetTargetSRadiusTarget(TARGET* ptarget)
{
	return &ptarget->sRadiusTarget;
}

void SetTargetSRadiusTarget(TARGET* ptarget, float sRadiusTarget)
{
	ptarget->sRadiusTarget = sRadiusTarget;
}

void* GetTargetFHitTest(TARGET* ptarget)
{
	return &ptarget->fHitTest;
}

void SetTargetFHitTest(TARGET* ptarget, int fHitTest)
{
	ptarget->fHitTest = fHitTest;
}

int GetTargetSize()
{
	return sizeof(TARGET);
}

void OnTargetAdd(TARGET* ptarget)
{
	OnLoAdd(ptarget);
	AppendDlEntry(&g_dlTarget, ptarget);
}

void OnTargetRemove(TARGET* ptarget)
{
	OnLoRemove(ptarget);
	RemoveDlEntry(&g_dlTarget, ptarget);
}

void CloneTarget(TARGET* ptarget, TARGET* ptargetBase)
{
	DLE savedDle = ptarget->dleTarget;
	CloneXfm(ptarget, ptargetBase);

	ptarget->dleTarget = ptargetBase->dleTarget;
	ptarget->grftak = ptargetBase->grftak;
	ptarget->sRadiusTarget = ptargetBase->sRadiusTarget;
	ptarget->fHitTest = ptargetBase->fHitTest;

	ptarget->dleTarget = savedDle;
}

void DeleteTarget(TARGET* ptarget)
{
	delete ptarget;
}

DL g_dlTarget;