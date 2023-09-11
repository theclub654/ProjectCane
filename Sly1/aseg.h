#pragma once
#include "lo.h"

void LoadAcpcFromBrx(CBinaryInputStream* pbis);
void LoadAcpvFromBrx(CBinaryInputStream* pbis);
void LoadAkvbFromBrx(CBinaryInputStream* pbis);
void LoadAcrcFromBrx(CBinaryInputStream* pbis);
void LoadAcscFromBrx(CBinaryInputStream* pbis);
void LoadAcrbFromBrx(CBinaryInputStream* pbis);
void LoadApacgFromBrx(CBinaryInputStream* pbis);
void LoadAcgbFromBrx(CBinaryInputStream* pbis);
void LoadAcgbwFromBrx(CBinaryInputStream* pbis);
void LoadAcglFromBrx(CBinaryInputStream* pbis);
void LoadAcpbFromBrx(CBinaryInputStream* pbis);

class ASEG : public LO
{
	public:
};

static int LoadAsegaCount;
void LoadAsegFromBrx(ASEG* paseg, CBinaryInputStream* pbis);
void LoadAsegEventsFromBrx(CBinaryInputStream* pbis, int fFrame);