#include "shape.h"

SHAPE* NewShape()
{
    return new SHAPE{};
}

void InitSwShapeDl(SW* psw)
{
    InitDl(&psw->dlShape, offsetof(SHAPE, dleShape));
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

void SetShapeParent(SHAPE* pshape, ALO* paloParent)
{
    SetLoParent(pshape, paloParent);
}

void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis)
{
    byte crvk = pbis->U8Read();

    pshape->pcrv = PcrvNew((CRVK)crvk);

    pshape->pcrv->pvtcrvl->pfnLoadCrvlFromBrx((CRVL*)pshape->pcrv, pbis);

    LoadOptionsFromBrx(pshape, pbis);
}

void DeleteShape(SHAPE* pshape)
{
    if (pshape->pcrv != nullptr)
        DeletePcrv(pshape->pcrv->crvk, pshape->pcrv);

    delete pshape;
}
