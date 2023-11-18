#include "squish.h"

void LoadSquishFromBrx(SQUISH* psquish, CBinaryInputStream* pbis)
{
	std::cout << "SQUISH Size: " << sizeof(SQUISH) << "\n";
	LoadSoFromBrx(psquish, pbis);
}

void DeleteSquish(LO* plo)
{
	delete (SQUISH*)plo;
}
