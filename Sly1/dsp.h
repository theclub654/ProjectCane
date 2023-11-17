#pragma once
#include "so.h"

class DSP : public SO
{
	public:
        struct SM* psm;
        struct SMA* psma;
        OID oidDispense;
        int cpso;
        struct SO** apso;
        struct SO* psoCur;
        struct TARGET* ptargetCur;
};

