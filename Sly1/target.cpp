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
	LO lo = *ptarget;
	*ptarget = *ptargetBase;
	memcpy(ptarget, &lo, sizeof(LO));

	CloneLo(ptarget, ptargetBase);
}

void DeleteTarget(LO* plo)
{
	delete(TARGET*)plo;
}
