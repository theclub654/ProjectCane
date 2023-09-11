#include "rip.h"

void InitRipg(RIPG *pripg)
{
	InitSo(pripg);
	InitDl(&pripg->dlRip, 0x10 + 0xC);
	pripg->sMRD = 10000000000;
}

void OnRipgRemove(RIPG* pripg)
{

}