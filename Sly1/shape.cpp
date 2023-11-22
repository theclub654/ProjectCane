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
