#include "cplcy.h"

void InitCplcy(CPLCY* pcplcy, CM* pcm)
{
	pcplcy->pcm = pcm;
}

int FActiveCplcy(CPLCY* pcplcy)
{
	return (int)(pcplcy->pcm->acpr[0].pcplcy == pcplcy);
}