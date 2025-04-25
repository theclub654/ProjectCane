#include "dsp.h"

DSP* NewDsp()
{
	return new DSP{};
}

int GetDspSize()
{
	return sizeof(DSP);
}

void CloneDsp(DSP* pdsp, DSP* pdspBase)
{
	CloneSo(pdsp, pdspBase);

    pdsp->psm = pdspBase->psm;
    pdsp->psma = pdspBase->psma;
    pdsp->oidDispense = pdspBase->oidDispense;
    pdsp->cpso = pdspBase->cpso;
    pdsp->apso = pdspBase->apso;
    pdsp->psoCur = pdspBase->psoCur;
    pdsp->ptargetCur = pdspBase->ptargetCur;
}

void DeleteDsp(DSP *pdsp)
{
	delete pdsp;
}
