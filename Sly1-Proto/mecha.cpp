#include "mecha.h"

MECHA* NewMecha()
{
	return new MECHA{};
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

void RenderMechaSelf(MECHA* pmecha, CM* pcm, RO* pro)
{
	RenderStepSelf(pmecha, pcm, pro);
}

void DeleteMecha(MECHA *pmecha)
{
	delete pmecha;
}
