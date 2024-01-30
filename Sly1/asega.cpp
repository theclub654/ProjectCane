#include "asega.h"

void InitSwAsegaDl(SW* psw)
{
	InitDl(&psw->dlAsega, offsetof(ASEGA, dleSw));
}

void InitSwAsegaRealClockDl(SW* psw)
{
	InitDl(&psw->dlAsegaRealClock, offsetof(ASEGA, dleSw));
}

void InitSwAsegaPending(SW* psw)
{
	InitDl(&psw->dlAsegaPending, offsetof(ASEGA, dleSw));
}

int GetAsegaSize()
{
	return sizeof(ASEGA);
}

void DeleteAsega(LO* plo)
{
	delete(ASEGA*)plo;
}
