#include "shdanim.h"
#include "glob.h"

void* NewSaa(SAAK saak)
{
    switch (saak)
    {
        case SAAK_Loop:
        return NewLoop();

        case SAAK_PingPong:
        return NewPingPong();

        case SAAK_Shuffle:
        return NewShuffle();

        case SAAK_Hologram:
        return NewHologram();

        case SAAK_Eyes:
        return NewEyes();

        case SAAK_Scroller:
        return NewScroller();

        case SAAK_Circler:
        return NewCircler();

        case SAAK_Looker:
        return NewLooker();

        default:
        return nullptr;
    }
}

SAA* PsaaLoadFromBrx(CBinaryInputStream *pbis)
{
    SAAK saakType = (SAAK)pbis->U16Read();
    //std::cout << saakType << " ";
    SAA *psaa{};
    psaa = (SAA*)NewSaa(saakType);

    if (psaa != nullptr)
    {
        g_apsaaSw.push_back(psaa);
        psaa->saak = saakType;
        psaa->pvtsaa = PvtsaaFromSaak(saakType);
        SAAF saaf{};
        saaf.oid = pbis->S16Read();
        saaf.fInstanced = pbis->U16Read();
        psaa->pvtscroller->pfnLoadScrollerFromBrx((SCROLLER*)psaa, pbis);
        psaa->pvtsaa->pfnInitSaa(psaa, &saaf);
    }

    return psaa;
}

VTSAA* PvtsaaFromSaak(SAAK saak)
{
    switch (saak) 
    {
        case SAAK_Loop:
        return (VTSAA*)&g_vtloop;

        case SAAK_PingPong:
        return (VTSAA*)&g_vtpingpong;

        case SAAK_Shuffle:
        return (VTSAA*)&g_vtshuffle;

        case SAAK_Hologram:
        return (VTSAA*)&g_vthologram;

        case SAAK_Eyes:
        return (VTSAA*)&g_vteyes;

        case SAAK_Scroller:
        return (VTSAA*)&g_vtscroller;

        case SAAK_Circler:
        return (VTSAA*)&g_vtcircler;

        case SAAK_Looker:
        return (VTSAA*)&g_vtlooker;

        default:
        return (VTSAA*)nullptr;
    }
}

void InitSaa(SAA* psaa, SAAF* psaaf)
{
    psaa->oid = (OID)psaaf->oid;
    psaa->sai.grfsai |= 0x01;

    if (psaaf->fInstanced != 0)
        psaa->sai.grfsai |= 0x04;
}

void PostSaaLoad(SAA* psaa)
{
    if (psaa->sai.pshd == nullptr) 
        psaa->sai.pshd = PshdFindShader(psaa->oid);
}

float UCompleteSaa(SAA* psaa)
{
    return 0.0f;
}

SAI* PsaiFromSaaShd(SAA* psaa, SHD* pshd)
{
    if (pshd->oid == psaa->oid) 
        return &psaa->sai;

    return nullptr;
}

int FUpdatableSaa(SAA* psaa)
{
    if (psaa->tUpdated != g_clock.t) 
    {
        psaa->tUpdated = g_clock.t;
        return 1;
    }

    return 0;
}

void SetSaiDuDv(SAI* psai, float du, float dv)
{
    // If nothing changed, do nothing.
    if (psai->tcx.du == du && psai->tcx.dv == dv)
        return;

    psai->tcx.du = du;
    psai->tcx.dv = dv;
}

void DeleteSaa(SAA* psaa)
{
    delete psaa;
}