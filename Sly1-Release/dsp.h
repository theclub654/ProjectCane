#pragma once
#include "so.h"

class DSP : public SO
{
	public:
    struct SM* psm;
    struct SMA* psma;
    OID oidDispense;
    int cpso;
    class SO** apso;
    class SO* psoCur;
    struct TARGET* ptargetCur;
};

DSP* NewDsp();
int  GetDspSize();
void CloneDsp(DSP* pdsp, DSP* pdspBase);
void DeleteDsp(DSP *pdsp);