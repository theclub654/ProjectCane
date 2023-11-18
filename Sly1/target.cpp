#include "target.h"

void InitTarget(TARGET* ptarget)
{
	//std::cout << "TARGET Size: " << sizeof(TARGET) << "\n";
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
