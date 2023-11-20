#include "ub.h"

void InitUbg(UBG* pubg)
{
	//std::cout << "UBV Size: " << sizeof(UBP) << "\n";
	InitGomer(pubg);
}

void RenderUbpAll(UBP* pubp, CM* pcm, RO* pro)
{

}

void* CreateUbg()
{
	UBG ubg;
	return &ubg;
}

void DeleteUbg(LO* plo)
{
	delete (UBG*)plo;
}

void DeleteUbp(LO* plo)
{
	delete (UBP*)plo;
}

void DeleteUbv(LO* plo)
{
	delete (UBV*)plo;
}
