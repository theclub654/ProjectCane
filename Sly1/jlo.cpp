#include "jlo.h"

void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis)
{
	//std::cout << std::hex << pbis->file.tellg() <<"\n";
	LoadSoFromBrx(pjlo, pbis);
}

void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjloc, pbis);
}
