#include "jlo.h"

void InitJlovol(JLOVOL* pjlovol)
{
	//std::cout << "JLOVOL Size: " << sizeof(JLOVOL) << "\n";
	InitVolbtn(pjlovol);
}

void InitJlo(JLO* pjlo)
{
	//std::cout << "JLO Size: " << sizeof(JLO) << "\n";
	InitSo(pjlo);
}

void InitJloc(JLOC* pjloc)
{
	//std::cout << "JLOC Size: " << sizeof(JLOC) << "\n";
	InitAlo(pjloc);
}

void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pjlo, pbis);
}

void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjloc, pbis);
}
