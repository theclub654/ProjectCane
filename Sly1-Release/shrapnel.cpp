#include "shrapnel.h"

void PostShrapnelEmit(SHRAPNEL* pshrapnel, EMITB* pemitb)
{
    const float lifetimeScale = GRandInRange(s_lmRdtLifetime.gMin, s_lmRdtLifetime.gMax);
    const float speed = glm::length(pshrapnel->v);

    pshrapnel->dtLifetime *= lifetimeScale;

    // EMITX is a union.  Reading the overlapping integer bullet fields and
    // numerically casting them converts the IEEE-754 bit patterns (for
    // example 25.0f -> 1103626240) into enormous float values.  Shrapnel
    // properties are authored and stored as floats, so access that member.
    pshrapnel->sRadius = pemitb->emitx.shrapnel.sRadius;
    pshrapnel->elas = pemitb->emitx.shrapnel.elas;
    pshrapnel->mu = pemitb->emitx.shrapnel.mu;

    const float angularScaleRaw = s_clqSvToU.g0 + speed * (s_clqSvToU.g1 + speed * s_clqSvToU.g2);
    const float angularScale = glm::clamp(angularScaleRaw, 0.0f, 1.0f);
    const float angularVelocityMax = angularScale * 10.0f;

    pshrapnel->w.x = GRandInRange(-angularVelocityMax, angularVelocityMax);
    pshrapnel->w.y = GRandInRange(-angularVelocityMax, angularVelocityMax);
    pshrapnel->w.z = GRandInRange(-angularVelocityMax, angularVelocityMax);
}

int FBounceShrapnel(SHRAPNEL* pshrapnel, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal)
{
    if (psoOther == nullptr || psoOther->paloRoot == nullptr)
        return 0;

    if (FIsBasicDerivedFrom(psoOther->paloRoot, CID_STEP))
        return 0;

    return FBounceRip(pshrapnel, psoOther, ppos, pnormal);
}

void RenderShrapnel(SHRAPNEL* pshrapnel, CM* pcm)
{
    glm::vec3 posRender = pshrapnel->pos + g_normalZ * pshrapnel->sRadius;
    FRenderRipPosMat(pshrapnel, pcm, &posRender, &pshrapnel->mat);
}

LM s_lmRdtLifetime{ 0.75, 1.25};
CLQ s_clqSvToU{ 0.0, 0.001, 0.0 };
VTSHRAPNEL g_vtshrapnel;
