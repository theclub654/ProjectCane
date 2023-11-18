#include "mecha.h"

void InitMecha(MECHA* pmecha)
{
	std::cout << "MECHA Size: " << sizeof(MECHA) << "\n";
	InitStep(pmecha);
}

void DeleteMecha(LO* plo)
{
	delete (MECHA*)plo;
}
