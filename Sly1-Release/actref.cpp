#include "actref.h"
#include "alo.h"

std::shared_ptr <ACTREF> PactrefNew(SW* psw, ALO* palo)
{
    auto pactref = std::make_shared<ACTREF>();

    pactref->pvtactref = &g_vtactref;
    pactref->pvtactref->pfnInitActref(pactref.get(), palo);

    psw->actOwners.push_back(std::static_pointer_cast<ACT>(pactref));

    return pactref;
}

std::shared_ptr <ACTREF> PactNewClone(ACTREF* pactrefBase, SW* psw, ALO* palo)
{
    auto pactref = PactrefNew(psw, palo);
    pactref->pvtactref->pfnCloneActref(pactref.get(), pactrefBase);

    return pactref;
}

void InitActref(ACTREF* pactref, ALO* palo)
{
    InitAct(pactref, palo);

    pactref->pposGoal = &palo->posOrig;
    pactref->pmatGoal = &palo->matOrig;

    pactref->pcgPoses = &palo->globset.cpose;
    pactref->agPoses  = palo->globset.agPosesOrig.data();

    pactref->pvGoal = &g_vecZero;
    pactref->pwGoal = &g_vecZero;

    if (palo->palox && (palo->palox->grfalox & 0x20))
    {
        pactref->pdradTwistGoal = &g_vecZero.x;
        pactref->pradTwistGoal = &palo->palox->ikh.radTwistOrig;
    }

    pactref->pmatScale = &g_matIdentity;
}

void CloneActref(ACTREF* pactref, ACTREF* pactrefBase)
{
    CloneAct(pactref, pactrefBase);

    pactref->pposGoal = pactrefBase->pposGoal;
    pactref->pvGoal = pactrefBase->pvGoal;
    pactref->pmatGoal = pactrefBase->pmatGoal;
    pactref->pwGoal = pactrefBase->pwGoal;
    pactref->pradTwistGoal = pactrefBase->pradTwistGoal;
    pactref->pdradTwistGoal = pactrefBase->pdradTwistGoal;
    pactref->pmatScale = pactrefBase->pmatScale;
    pactref->pcgPoses = pactrefBase->pcgPoses;
    pactref->agPoses = pactrefBase->agPoses;
}

void GetActrefPositionGoal(ACTREF* pactref, float dtOffset, glm::vec3* ppos, glm::vec3* pv)
{
    *ppos = *pactref->pposGoal;
    *pv = *pactref->pvGoal;

    /*if (pactref->palo->pvtalo->pfnAdjustAloPosition)
        pactref->palo->pvtalo->pfnAdjustAloPosition(pactref->palo, ppos);*/
}

void GetActrefRotationGoal(ACTREF* pactref, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    *pmat = *pactref->pmatGoal;
    *pw = *pactref->pwGoal;

    /*if (pactref->palo->pvtalo->pfnAdjustAloRotation)
        pactref->palo->pvtalo->pfnAdjustAloRotation(pactref->palo, pmat, 0);*/
}

void GetActrefTwistGoal(ACTREF* pactref, float* pradTwist, float* pdradTwist)
{
    *pradTwist = *pactref->pradTwistGoal;
    *pdradTwist = *pactref->pdradTwistGoal;
}

void GetActrefScale(ACTREF* pactref, glm::mat3* pmat)
{
    *pmat = *pactref->pmatScale;
}

float GGetActrefPoseGoal(ACTREF* pactref, int ipose)
{
    return pactref->agPoses[ipose];
}
