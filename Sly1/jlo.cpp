#include "jlo.h"

void InitJlovol(JLOVOL* pjlovol)
{
	//std::cout << "JLOVOL Size: " << sizeof(JLOVOL) << "\n";
	InitVolbtn(pjlovol);
}

void DeleteJlovol(LO* plo)
{
	delete(JLOVOL*)plo;
}

void InitJlo(JLO* pjlo)
{
	//std::cout << "JLO Size: " << sizeof(JLO) << "\n";
	InitSo(pjlo);
}

void DeleteJlo(LO* plo)
{
	delete (JLO*)plo;
}

void InitJloc(JLOC* pjloc)
{
	//std::cout << "JLOC Size: " << sizeof(JLOC) << "\n";
	InitAlo(pjloc);
}

void DeleteJloc(LO* plo)
{
	delete(JLOC*)plo;
}

void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pjlo, pbis);
}

void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjloc, pbis);
}
