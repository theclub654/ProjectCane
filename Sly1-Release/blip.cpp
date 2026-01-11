#include "blip.h"

BLIPG* NewBlipg()
{
	return new BLIPG{};
}

void InitSwBlipgDl(SW* psw)
{
	InitDl(&psw->dlBlipg, offsetof(BLIPG, dleBlipg));
}

void InitSwBlipgFreeDl(SW* psw)
{
	InitDl(&psw->dlBlipgFree, offsetof(BLIPG, dleBlipg));
}

void InitBlipg(BLIPG* pblipg)
{
    AppendDlEntry(&pblipg->psw->dlBlipgFree, pblipg);
    InitAlo(pblipg);
    InitDl(&pblipg->dlBlip, offsetof(BLIP, dle));
    pblipg->sMRD = 1e+10;
    *(unsigned long*)&pblipg->bitfield = *(unsigned long*)&pblipg->bitfield & 0xffffffffcfffffff | 0x10020000000;
}

int GetBlipgSize()
{
	return sizeof(BLIPG);
}

void CloneBlipg(BLIPG* pblipg, BLIPG* pblipgBase)
{
    CloneAlo(pblipg, pblipgBase);

    pblipg->pemitter = pblipgBase->pemitter;
    pblipg->clqScale = pblipgBase->clqScale;
    pblipg->clqAlpha = pblipgBase->clqAlpha;
    pblipg->clqTexture = pblipgBase->clqTexture;
    pblipg->clqColor = pblipgBase->clqColor;
    pblipg->blipmk = pblipgBase->blipmk;
    pblipg->blipgm = pblipgBase->blipgm;
    pblipg->pshd = pblipgBase->pshd;
    pblipg->crgba = pblipgBase->crgba;
    for (int i = 0; i < 32; ++i)
        pblipg->argba[i] = pblipgBase->argba[i];
    pblipg->fColorRanges = pblipgBase->fColorRanges;
    pblipg->blipok = pblipgBase->blipok;
    pblipg->rSFlying = pblipgBase->rSFlying;
    pblipg->cblipe = pblipgBase->cblipe;
    pblipg->dlBlip = pblipgBase->dlBlip;
    pblipg->dleBlipg = pblipgBase->dleBlipg;
}

void OnBlipgAdd(BLIPG* pblipg)
{
    RemoveDlEntry(&pblipg->psw->dlBlipgFree, pblipg);
    AppendDlEntry(&pblipg->psw->dlBlipg, pblipg);
    OnAloAdd(pblipg);
}

void OnBlipgRemove(BLIPG* pblipg)
{
    OnAloRemove(pblipg);
}

void SetBlipgShader(BLIPG *pblipg, OID oid)
{
    pblipg->pshd = PshdFindShader(oid);

    if (pblipg->pshd == nullptr) {
        pblipg->pshd = &g_ashd[0];
    }

    if (pblipg->crgba == 0)
        pblipg->argba[0] = pblipg->pshd->rgba;

    //PropagateBlipgShader(pblipg);
}

void UpdateBlipg(BLIPG* pblipg, float dt)
{
    UpdateAlo(pblipg, dt);
    ResolveAlo(pblipg);

    GRFZON objZoneMask = pblipg->pshd->grfzon;

    bool inCameraZone = (objZoneMask & 0x10000000u) || ((g_pcm->grfzon & objZoneMask) == g_pcm->grfzon);

    if (!inCameraZone)
        pblipg->pvtlo->pfnRemoveLo(pblipg);
}

void RenderBlipgSelf(BLIPG* pblipg, CM* pcm, RO* pro)
{
    RPL rpl{};
    rpl.rp = RP_Blip;
    rpl.ro.uAlpha = 1.0;
    rpl.z = 0.0;

    if (pro != nullptr)
        rpl.ro.uAlpha = pro->uAlpha;

    SubmitRpl(&rpl);
}

void DeleteBlipg(BLIPG* pblipg)
{
	delete pblipg;
}
