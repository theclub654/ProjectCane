#include "jlo.h"

void InitJlovol(JLOVOL* pjlovol)
{
	InitVolbtn(pjlovol);
}

void InitJlo(JLO* pjlo)
{
	InitSo(pjlo);
}

void InitJloc(JLOC* pjloc)
{
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
