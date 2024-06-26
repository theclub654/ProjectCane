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

SHAPE*NewShape();
void InitSwShapeDl(SW* psw);
void InitShape(SHAPE* pshape);
int  GetShapeSize();
void CloneShape(SHAPE* pshape, SHAPE* pshapeBase);
void SetShapeParent(SHAPE* pshape, ALO* paloParent);
void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis);
void DeleteShape(LO* plo);