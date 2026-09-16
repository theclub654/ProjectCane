#pragma once
#include "crv.h"

class SHAPE : public LO
{
	public:
	std::shared_ptr <CRV> pcrv;
	DLE dleShape;
};

SHAPE*NewShape();
void InitShape(SHAPE* pshape);
int  GetShapeSize();
void CloneShape(SHAPE* pshape, SHAPE* pshapeBase);
void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis);
void SetShapeParent(SHAPE* pshape, ALO* paloParent);
void DeleteShape(SHAPE* pshape);