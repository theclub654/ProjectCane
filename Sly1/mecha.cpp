#include "mecha.h"

void* NewMecha()
{
	return new MECHA;
}

void InitMecha(MECHA* pmecha)
{
	InitStep(pmecha);
}

void DeleteMecha(LO* plo)
{
	delete (MECHA*)plo;
}
