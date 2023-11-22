#include "target.h"

void* NewTarget()
{
	return new TARGET;
}

void InitTarget(TARGET* ptarget)
{
	InitXfm(ptarget);
}

void OnTargetAdd(TARGET* ptarget)
{
	OnLoAdd(ptarget);
}

void DeleteTarget(LO* plo)
{
	delete(TARGET*)plo;
}
