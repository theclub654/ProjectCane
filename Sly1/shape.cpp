#include "shape.h"

void* NewShape()
{
    return new SHAPE;
}

void InitShape(SHAPE* pshape)
{
    InitLo(pshape);
    AppendDlEntry(&pshape->psw->dlShape, pshape);
}

int GetShapeSize()
{
    return sizeof(SHAPE);
}

void CloneShape(SHAPE* pshape, SHAPE* pshapeBase)
{
    LO lo = *pshape;
    *pshape = *pshapeBase;
    memcpy(pshape, &lo, sizeof(LO));

    CloneLo(pshape, pshapeBase);
}

void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis)
{
    byte unk_0 = pbis->U8Read();

    switch (unk_0)
    {
        case 0x0:
        LoadCrvlFromBrx(pbis);
        break;
        case 0x1:
        LoadCrvcFromBrx(pbis);
        break;
    }

    LoadOptionFromBrx(pshape, pbis);
}

void DeleteShape(LO* plo)
{
    delete(SHAPE*)plo;
}
