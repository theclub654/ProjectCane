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
    CloneAlo(ptn, ptnBase);

    ptn->ppo = ptnBase->ppo;
    ptn->ctsurf = ptnBase->ctsurf;
    ptn->atsurf = ptnBase->atsurf;
    ptn->ctbsp = ptnBase->ctbsp;
    ptn->atbsp = ptnBase->atbsp;
    ptn->pcrv = ptnBase->pcrv;
    std::memcpy(ptn->TNFN, ptnBase->TNFN, sizeof(ptn->TNFN));
    ptn->fCylinder = ptnBase->fCylinder;
    ptn->fSwitchInAir = ptnBase->fSwitchInAir;
    ptn->priCamera = ptnBase->priCamera;
    ptn->fFakeCylinder = ptnBase->fFakeCylinder;
    ptn->dradSlack = ptnBase->dradSlack;
    ptn->tns = ptnBase->tns;
    ptn->tTns = ptnBase->tTns;
    ptn->sClosest = ptnBase->sClosest;
    ptn->matXfm = ptnBase->matXfm;
    ptn->posXfm = ptnBase->posXfm;
    ptn->sTotal = ptnBase->sTotal;
    ptn->posFake = ptnBase->posFake;
    ptn->sRadFake = ptnBase->sRadFake;
    ptn->grftnd = ptnBase->grftnd;
    ptn->fPanOnEntry = ptnBase->fPanOnEntry;
    ptn->fPanOnIdle = ptnBase->fPanOnIdle;
    ptn->dtPanOnEntry = ptnBase->dtPanOnEntry;
    ptn->rswPanOnEntry = ptnBase->rswPanOnEntry;
    ptn->dtPanOnIdle = ptnBase->dtPanOnIdle;
    ptn->rswPanOnIdle = ptnBase->rswPanOnIdle;
    ptn->fUseVolume = ptnBase->fUseVolume;
    ptn->cfk = ptnBase->cfk;
    ptn->fCutOnEntry = ptnBase->fCutOnEntry;
    ptn->radFOV = ptnBase->radFOV;
    ptn->fNoSquish = ptnBase->fNoSquish;
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
