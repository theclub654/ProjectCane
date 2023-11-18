#include "rail.h"

void OnRailAdd(RAIL* prail)
{
	std::cout << "RAIL Size: " << sizeof(RAIL) << "\n";
	OnLoAdd(prail);
	AppendDlEntry(&prail->psw->dlRail, prail);
}

void DeleteRail(LO* plo)
{
	delete(RAIL*)plo;
}
