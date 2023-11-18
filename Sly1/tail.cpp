#include "tail.h"

void InitTail(TAIL* ptail)
{
	//std::cout << "TAIL Size: " << sizeof(TAIL) << "\n";
	InitAlo(ptail);
}

void DeleteTail(LO* plo)
{
	delete(TAIL*)plo;
}
