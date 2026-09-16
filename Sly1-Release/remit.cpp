#include "remit.h"
#include "expls.h"

void PostRemitEmit(REMIT* premit, EMITB* pemitb)
{
    premit->pexpls = static_cast<EXPLS*>(PloFindSwObject(g_psw, CID_EXPLS, pemitb->emitx.remit.oidExpls, pemitb->emito.paloReference));
    premit->svcParticle = pemitb->emitx.remit.svcParticle;
}

void UpdateRemit(REMIT* premit, float dt)
{
    UpdateRip(premit, dt);

    if (premit->pexpls == nullptr)
        return;

    premit->cParticle += dt * premit->svcParticle;

    while (premit->cParticle >= 1.0f)
    {
        EXPLSO explso{};
        explso.grfexplso = 4;
        explso.posOrigin = premit->pos;

        premit->pexpls->pvtexpls->pfnExplodeExplsExplso(premit->pexpls, &explso);
        premit->cParticle -= 1.0f;
    }
}

VTREMIT g_vtremit;