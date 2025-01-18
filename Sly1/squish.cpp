#include "squish.h"

SQUISH* NewSquish()
{
	return new SQUISH{};
}

int GetSquishSize()
{
	return sizeof(SQUISH);
}

void LoadSquishFromBrx(SQUISH* psquish, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(psquish, pbis);
}

void CloneSquish(SQUISH* psquish, SQUISH* psquishBase)
{
	LO lo = *psquish;
	*psquish = *psquishBase;
	memcpy(psquish, &lo, sizeof(LO));

	CloneLo(psquish, psquishBase);

	ClearDl(&psquish->dlChild);

	psquish->pxa = nullptr;
	psquish->grfpvaXpValid = 0;
	psquish->pstso = nullptr;
}

void RenderSquishSelf(SQUISH* psquish, CM* pcm, RO* pro)
{
	RenderSoSelf(psquish, pcm, pro);
}

void DeleteSquish(SQUISH *psquish)
{
	delete psquish;
}
