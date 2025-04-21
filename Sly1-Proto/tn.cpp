#include "tn.h"

TN* NewTn()
{
    return new TN{};
}

void InitTn(TN* ptn)
{
    InitAlo(ptn);
}

int GetTnSize()
{
    return sizeof(TN);
}

void OnTnRemove(TN* ptn)
{
    OnAloRemove(ptn);
}

void LoadTnFromBrx(TN* ptn, CBinaryInputStream* pbis)
{
    ptn->xf.mat = pbis->ReadMatrix();
    ptn->xf.pos = pbis->ReadVector();

    ptn->pvtalo->pfnUpdateAloXfWorld(ptn);

    LoadTbspFromBrx(pbis);

    int8_t crvk = pbis->S8Read();

    if (crvk == -1)
    {
        ptn->matXfm = pbis->ReadMatrix();
        ptn->posXfm = pbis->ReadVector();
        ptn->sTotal = 0.0;
    }

    else
    {
        ptn->pcrv = PcrvNew((CRVK)crvk);
        ptn->pcrv->pvtcrvl->pfnLoadCrvlFromBrx(std::static_pointer_cast<CRVL> (ptn->pcrv), pbis);
    }

    LoadOptionsFromBrx(ptn, pbis);
}

void CloneTn(TN* ptn, TN* ptnBase)
{
    LO lo = *ptn;
    *ptn = *ptnBase;
    memcpy(ptn, &lo, sizeof(LO));

    CloneLo(ptn, ptnBase);

    ClearDl(&ptn->dlChild);
}

void LoadTbspFromBrx(CBinaryInputStream* pbis)
{
    uint16_t unk_0 = pbis->U16Read();

    for (int i = 0; i < unk_0; i++)
    {
        pbis->F32Read();
        pbis->ReadVector();
    }

    uint16_t unk_1 = pbis->U16Read();

    for (int i = 0; i < unk_1; i++)
    {
        pbis->U16Read();
        pbis->S16Read();
        pbis->S16Read();
    }
}

void RenderTnSelf(TN* ptn, CM* pcm, RO* pro)
{
    RenderAloSelf(ptn, pcm, pro);
}

void DeleteTn(TN *ptn)
{
    delete ptn;
}
