#include "target.h"

void* NewTarget()
{
	return new TARGET;
}

void InitTarget(TARGET* ptarget)
{
	InitXfm(ptarget);
}

int GetTargetSize()
{
	return sizeof(TARGET);
}

void OnTargetAdd(TARGET* ptarget)
{
	OnLoAdd(ptarget);
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
