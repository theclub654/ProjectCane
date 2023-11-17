#include "rip.h"

void InitRipg(RIPG *pripg)
{
	std::cout << "RIPG Size: " << sizeof(RIPG) << "\n";
	InitSo(pripg);
	InitDl(&pripg->dlRip, 0x1C + 0x10 + 0xC);
	pripg->sMRD = 10000000000;
}

void OnRipgRemove(RIPG* pripg)
{

}