#include "jsg.h"

void InitJsg(JSG* pjsg)
{
	//std::cout << "JSG Size: " << sizeof(JSG) << "\n";
	InitLo(pjsg);
}

void DeleteJsg(LO* plo)
{
	delete(JSG*)plo;
}
