#pragma once
#include "lo.h"

void LoadCrvlFromBrx(CBinaryInputStream* pbis);
void LoadCrvcFromBrx(CBinaryInputStream* pbis);

class SHAPE : public LO
{
	public:
};

void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis);