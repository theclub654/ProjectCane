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
    CloneLo(pshape, pshapeBase);

    pshape->dleShape = pshapeBase->dleShape;
    pshape->pcrv = pshapeBase->pcrv;
}

void SetShapeParent(SHAPE* pshape, ALO* paloParent)
{
    glm::mat4 matSrc{};
    glm::mat4 matDst{};

    if (pshape->paloParent == nullptr)
        matSrc = glm::identity <glm::mat4>();
    else
        LoadMatrixFromPosRot(pshape->paloParent->xf.posWorld, pshape->paloParent->xf.matWorld, matSrc);

    if (paloParent == nullptr)
        matDst = glm::identity <glm::mat4>();
    else
        LoadMatrixFromPosRot(paloParent->xf.posWorld, paloParent->xf.matWorld, matDst);

    SetLoParent(pshape, paloParent);
}

void LoadShapeFromBrx(SHAPE* pshape, CBinaryInputStream* pbis)
{
    byte crvk = pbis->U8Read();

    pshape->pcrv = PcrvNew((CRVK)crvk);

    pshape->pcrv->pvtcrvl->pfnLoadCrvlFromBrx(std::static_pointer_cast <CRVL> (pshape->pcrv), pbis);

    LoadOptionsFromBrx(pshape, pbis);
}

void DeleteShape(SHAPE* pshape)
{
    delete pshape;
}
