#include "rog.h"

void LoadRovFromBrx(ROV* prov, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prov, pbis);
}

void LoadRohFromBrx(ROH* proh, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(proh, pbis);
}

void LoadRocFromBrx(ROC* proc, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(proc, pbis);
}

void LoadRostFromBrx(ROST* prost, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prost, pbis);
}

void LoadRopFromBrx(ROP* prop, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(prop, pbis);
}
