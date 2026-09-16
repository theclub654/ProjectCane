#include "actval.h"

std::shared_ptr <ACTVAL> PactvalNew(SW* psw, ALO* palo)
{
    auto pactval = std::make_shared<ACTVAL>();

    pactval->pvtactval = &g_vtactval;
    pactval->pvtactval->pfnInitActval(pactval.get(), palo);

    psw->actOwners.push_back(pactval);

    return pactval;
}

void InitActval(ACTVAL* pactval, ALO* palo)
{
    InitAct(pactval, palo);

    pactval->posGoal = palo->posOrig;
    pactval->matGoal = palo->matOrig;

    if (palo->palox && (palo->palox->grfalox & 0x20))
        pactval->radTwistGoal = palo->palox->ikh.radTwistOrig;

    pactval->matScale = glm::mat3(1.0f);

    pactval->cgPoses = palo->globset.cpose;
    pactval->agPoses = palo->globset.agPosesOrig.data();
}

void CloneActval(ACTVAL* pactval, ACTVAL* pactvalBase)
{
    CloneAct(pactval, pactvalBase);

    pactval->posGoal = pactvalBase->posGoal;
    pactval->vGoal = pactvalBase->vGoal;
    pactval->matGoal = pactvalBase->matGoal;
    pactval->wGoal = pactvalBase->wGoal;
    pactval->radTwistGoal = pactvalBase->radTwistGoal;
    pactval->dradTwistGoal = pactvalBase->dradTwistGoal;
    pactval->matScale = pactvalBase->matScale;
    pactval->cgPoses = pactvalBase->cgPoses;
    pactval->agPoses = pactvalBase->agPoses;
}

void GetActvalPositionGoal(ACTVAL* pactval, float dtOffset, glm::vec3* ppos, glm::vec3* pv)
{
    *ppos = pactval->posGoal;
    *pv = pactval->vGoal;

    if (pactval->palo != nullptr && pactval->palo->pvtalo != nullptr &&
        pactval->palo->pvtalo->pfnAdjustAloPosition != nullptr)
    {
        pactval->palo->pvtalo->pfnAdjustAloPosition(pactval->palo, ppos, pv);
    }
}

void GetActvalRotationGoal(ACTVAL* pactval, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    *pmat = pactval->matGoal;
    *pw = pactval->wGoal;

    if (pactval->palo != nullptr && pactval->palo->pvtalo != nullptr &&
        pactval->palo->pvtalo->pfnAdjustAloRotation != nullptr)
    {
        pactval->palo->pvtalo->pfnAdjustAloRotation(pactval->palo, pmat, pw);
    }
}

void GetActvalTwistGoal(ACTVAL* pactval, float* pradTwist, float* pdradTwist)
{
    *pradTwist = pactval->radTwistGoal;
    *pdradTwist = pactval->dradTwistGoal;
}

void GetActvalScale(ACTVAL* pactval, glm::mat3* pmat)
{
    *pmat = pactval->matScale;
}

float GetActvalPoseGoal(ACTVAL* pactval, int ipose)
{
    return pactval->agPoses[ipose];
}
