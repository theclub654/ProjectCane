#include "shdanim.h"
#include "glob.h"
#include "loop.h"
#include "pingpong.h"
#include "shuffle.h"
#include "hologram.h"
#include "eyes.h"
#include "scroller.h"
#include "circler.h"
#include "looker.h"

std::shared_ptr<SAA> NewSaa(SAAK saak)
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

std::shared_ptr<SAA> PsaaLoadFromBrx(CBinaryInputStream *pbis)
{
    SAAK saakType = (SAAK)pbis->U16Read();
    //std::cout << saakType << " ";
    std::shared_ptr<SAA> psaa = NewSaa(saakType);

    if (psaa)
    {
        g_apsaaSw.push_back(psaa);
        SAA* const rawPsaa = psaa.get();
        rawPsaa->saak = saakType;
        rawPsaa->pvtsaa = PvtsaaFromSaak(saakType);
        SAAF saaf{};
        saaf.oid = pbis->S16Read();
        saaf.fInstanced = pbis->U16Read();
        rawPsaa->pvtscroller->pfnLoadScrollerFromBrx((SCROLLER*)rawPsaa, pbis);
        rawPsaa->pvtsaa->pfnInitSaa(rawPsaa, &saaf);
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
    // SAA instances are owned by g_apsaaSw.  This legacy raw-pointer
    // callback must not delete an object managed by a shared_ptr.
    (void)psaa;
}
