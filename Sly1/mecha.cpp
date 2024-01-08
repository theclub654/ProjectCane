#include "mecha.h"

void* NewMecha()
{
	return new MECHA;
}

void InitMecha(MECHA* pmecha)
{
	InitStep(pmecha);
}

int GetMechaSize()
{
	return sizeof(MECHA);
}

void CloneMecha(MECHA* pmecha, MECHA* pmechaBase)
{
	LO lo = *pmecha;
	*pmecha = *pmechaBase;
	memcpy(pmecha, &lo, sizeof(LO));

	CloneLo(pmecha, pmechaBase);

	ClearDl(&pmecha->dlChild);

	pmecha->pxa = nullptr;
	pmecha->grfpvaXpValid = 0;
	pmecha->pstso = nullptr;
}

void DeleteMecha(LO* plo)
{
	delete (MECHA*)plo;
}
