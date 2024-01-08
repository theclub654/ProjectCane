#include "dsp.h"

void* NewDsp()
{
	return new DSP;
}

int GetDspSize()
{
	return sizeof(DSP);
}

void CloneDsp(DSP* pdsp, DSP* pdspBase)
{
	LO lo = *pdsp;
	*pdsp = *pdspBase;
	memcpy(pdsp, &lo, sizeof(LO));

	CloneLo(pdsp, pdspBase);

	ClearDl(&pdsp->dlChild);

	pdsp->pxa = nullptr;
	pdsp->grfpvaXpValid = 0;
	pdsp->pstso = nullptr;
}

void DeleteDsp(LO* plo)
{
	delete (DSP*)plo;
}
