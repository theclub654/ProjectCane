#include "target.h"

void InitTarget(TARGET* ptarget)
{
	InitXfm(ptarget);
}

void OnTargetAdd(TARGET* ptarget)
{
	OnLoAdd(ptarget);
}
