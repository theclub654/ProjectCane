#include "rip.h"

void InitRipg(RIPG* pripg)
{
	InitSo(&pripg->ripg);
	InitDl(&pripg->dlRip, 0x10);
	pripg->ripg.ALocalObject.sMRD = 10000000000;
}

void OnRipgRemove(RIPG* pripg)
{

}