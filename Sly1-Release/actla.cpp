#include "actla.h"


std::shared_ptr <ACTLA> PactlaNew(SW* psw, ALO* palo, VTACTLA* pvtact)
{
    auto pactla = std::make_shared <ACTLA>();

    pactla->pvtactla = pvtact;
    pactla->pvtact->pfnInitAct(pactla.get(), palo);

    return pactla;
}

std::shared_ptr<ACTLA> PactlaNewClone(ACT* pactBase, SW* psw, ALO* palo)
{
    auto pactla = PactlaNew(psw, palo, &g_vtactla);
    pactla->pvtact->pfnCloneAct(pactla.get(), pactBase);

    return pactla;
}

void InitActla(ACTLA* pactla, ALO* palo)
{
    InitAct(pactla, palo);

    pactla->sIgnore = 800.0f;

    pactla->clqPanToPan.g0 = 0.0;
    pactla->clqPanToPan.g1 = 1.0;
    pactla->clqPanToPan.g2 = 0.0;

    pactla->lmPan.gMin = -3.4028235E+38;
    pactla->lmPan.gMax = 3.4028235E+38;

    pactla->clqTiltToTilt.g0 = 0.0;
    pactla->clqTiltToTilt.g1 = 1.0;
    pactla->clqTiltToTilt.g2 = 0.0;

    pactla->lmTilt.gMin = -1.0;
    pactla->lmTilt.gMax = 1.0;

    pactla->nPriorityDisabled = -1;
    pactla->nPriorityEnabled = 1;
}

void CloneActla(ACTLA* pactla, ACTLA* pactlaBase)
{
    CloneAct(pactla, pactlaBase);

    pactla->fUseTarget = pactlaBase->fUseTarget;
    pactla->paloTarget = pactlaBase->paloTarget;
    pactla->posTargetLocal = pactlaBase->posTargetLocal;
    pactla->sIgnore = pactlaBase->sIgnore;
    pactla->nPriorityEnabled = pactlaBase->nPriorityEnabled;
    pactla->nPriorityDisabled = pactlaBase->nPriorityDisabled;
    pactla->fPaused = pactlaBase->fPaused;
    pactla->clqPanToPan = pactlaBase->clqPanToPan;
    pactla->lmPan = pactlaBase->lmPan;
    pactla->clqTiltToTilt = pactlaBase->clqTiltToTilt;
    pactla->lmTilt = pactlaBase->lmTilt;
}

void UpdateActla(ACTLA* pactla, float dt)
{
    ALO* palo = pactla->palo;
    ALO* paloTarget = PaloGetActlaTarget(pactla);

    if (paloTarget == nullptr)
        return;

    // The retail game gates the controller using the two object origins in
    // world space. posTargetLocal is only applied while calculating the
    // rotation goal below.
    glm::vec3 dpos = palo->xf.posWorld - paloTarget->xf.posWorld;

    float distSq = glm::dot(dpos, dpos);

    int priority;

    if (distSq > pactla->sIgnore * pactla->sIgnore)
    {
        priority = pactla->nPriorityDisabled;
    }
    else
    {
        priority = pactla->nPriorityEnabled;
        ResolveAlo(palo);
    }

    if (priority != pactla->nPriority)
    {
        pactla->nPriority = priority;
        ResortAloActList(palo);
    }
}

ALO* PaloGetActlaTarget(ACTLA* pactla)
{
    if (pactla->fUseTarget)
        return pactla->paloTarget;

    return reinterpret_cast<ALO*>(PpoCur());
}

void GetActlaRotationGoal(ACTLA* pactla, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    ALO* palo = pactla->palo;

    *pw = glm::vec3(0.0f);

    if (pactla->fPaused)
    {
        *pmat = palo->xf.mat;
        return;
    }

    ALO* paloTarget = PaloGetActlaTarget(pactla);

    glm::vec3 pos;
    glm::mat3 mat;

    PredictAloTransform(paloTarget, nullptr, dtOffset, &pos, &mat, nullptr, nullptr);

    pos = pos + mat * pactla->posTargetLocal;

    ConvertAloPos(nullptr, palo->paloParent, &pos, &pos);

    glm::vec3 dpos = pos - palo->xf.pos;
    float dist = glm::length(dpos);

    if (dist > pactla->sIgnore)
    {
        *pmat = palo->matOrig;
        return;
    }

    float radPan;
    float radTilt;

    CalculateVectorPanTilt(dpos, &radPan, &radTilt);

    const float radPanRaw = radPan;
    const float radTiltRaw = radTilt;

    radPan  = pactla->clqPanToPan.g0   + radPan  * (pactla->clqPanToPan.g1   + radPan  * pactla->clqPanToPan.g2);
    radTilt = pactla->clqTiltToTilt.g0 + radTilt * (pactla->clqTiltToTilt.g1 + radTilt * pactla->clqTiltToTilt.g2);

    radPan  = glm::clamp(radPan,  pactla->lmPan.gMin,  pactla->lmPan.gMax);
    radTilt = glm::clamp(radTilt, pactla->lmTilt.gMin, pactla->lmTilt.gMax);

    LoadRotateMatrixPanTilt(radPan, radTilt, pmat);
}

void SetActlaTarget(ACTLA* pactla, ALO* paloTarget, glm::vec3* pposLocal)
{
    pactla->paloTarget = paloTarget;
    pactla->fUseTarget = 1;
    pactla->posTargetLocal = *pposLocal;
}
