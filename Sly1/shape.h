#pragma once
#include "lo.h"

void LoadCrvlFromBrx(CBinaryInputStream* pbis);
void LoadCrvcFromBrx(CBinaryInputStream* pbis);

class SHAPE : public LO
{
	public:
};

void InitShape(SHAPE* pshape);
void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis);