#include "jlo.h"

void* NewJlovol()
{
	return new JLOVOL;
}

void InitJlovol(JLOVOL* pjlovol)
{
	InitVolbtn(pjlovol);
}

void DeleteJlovol(LO* plo)
{
	delete(JLOVOL*)plo;
}

void* NewJlo()
{
	return new JLO;
}

void InitJlo(JLO* pjlo)
{
	InitSo(pjlo);
}

void DeleteJlo(LO* plo)
{
	delete (JLO*)plo;
}

void* NewJloc()
{
	return new JLOC;
}

void InitJloc(JLOC* pjloc)
{
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
