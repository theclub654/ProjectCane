#include "squish.h"

void* NewSquish()
{
	return new SQUISH;
}

void LoadSquishFromBrx(SQUISH* psquish, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(psquish, pbis);
}

void DeleteSquish(LO* plo)
{
	delete (SQUISH*)plo;
}
