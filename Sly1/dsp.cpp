#include "dsp.h"

void* NewDsp()
{
	return new DSP;
}

void DeleteDsp(LO* plo)
{
	delete (DSP*)plo;
}
