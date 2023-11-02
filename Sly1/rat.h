#pragma once
#include "so.h"
#include "chkpnt.h"

class RAT : public SO
{
	public:
};

class RATHOLE : public VOL
{
	public:
};

void InitRat(RAT* prat);
void OnRatAdd(RAT* prat);
void OnRatholeAdd(RATHOLE* prathole);
void LoadRatFromBrx(RAT* prat, CBinaryInputStream* pbis);

