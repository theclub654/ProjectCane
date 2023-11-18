#include "dsp.h"

void DeleteDsp(LO* plo)
{
	delete (DSP*)plo;
}
