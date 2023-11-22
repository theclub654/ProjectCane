#include "ub.h"

void* NewUbg()
{
	return new UBG;
}

void InitUbg(UBG* pubg)
{
	InitGomer(pubg);
}

void RenderUbpAll(UBP* pubp, CM* pcm, RO* pro)
{

}

void DeleteUbg(LO* plo)
{
	delete (UBG*)plo;
}

void* NewUbp()
{
	return new UBP;
}

void DeleteUbp(LO* plo)
{
	delete (UBP*)plo;
}

void* NewUbv()
{
	return new UBV;
}

void DeleteUbv(LO* plo)
{
	delete (UBV*)plo;
}
