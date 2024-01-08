#pragma once
#include "lo.h"

void LoadCrvlFromBrx(CBinaryInputStream* pbis);
void LoadCrvcFromBrx(CBinaryInputStream* pbis);

class SHAPE : public LO
{
	public:
		struct CRV* pcrv;
		DLE dleShape;
};

void*NewShape();
void InitShape(SHAPE* pshape);
int  GetShapeSize();
void CloneShape(SHAPE* pshape, SHAPE* pshapeBase);
void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis);
void DeleteShape(LO* plo);