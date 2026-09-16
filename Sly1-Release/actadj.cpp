#include "actadj.h"
#include "jt.h"

ACTADJ* PactadjNew(SW* psw, ALO* palo)
{
    if (psw == nullptr || palo == nullptr)
        return nullptr;

    auto pactadjOwner = std::make_shared<ACTADJ>();
    ACTADJ* pactadj = pactadjOwner.get();

    pactadj->pvtactadj = &g_vtactadj;

    if (pactadj->pvtact != nullptr && pactadj->pvtact->pfnInitAct != nullptr)
        pactadj->pvtact->pfnInitAct(pactadj, palo);

    psw->actOwners.push_back(std::move(pactadjOwner));

    return pactadj;
}

ACTADJ* PactadjNewClone(ACTADJ* pactadjBase, SW* psw, ALO* palo)
{
    ACTADJ* pactadj = PactadjNew(psw, palo);

    if (pactadj == nullptr)
        return nullptr;

    if (pactadj->pvtactadj != nullptr && pactadj->pvtactadj->pfnCloneActadj != nullptr)
        pactadj->pvtactadj->pfnCloneActadj(pactadj, pactadjBase);

    return pactadj;
}

void InitActadj(ACTADJ* pactadj, ALO* palo)
{
	InitAct(pactadj, palo);

	pactadj->dmatGoal = g_matIdentity;
	pactadj->dmatScale = g_matIdentity;
}

void CloneActadj(ACTADJ* pactadj, ACTADJ* pactadjBase)
{
	CloneAct(pactadj, pactadjBase);

	pactadj->dposGoal = pactadjBase->dposGoal;
	pactadj->dvGoal = pactadjBase->dvGoal;
	pactadj->dmatGoal = pactadjBase->dmatGoal;
	pactadj->dwGoal = pactadjBase->dwGoal;
	pactadj->dradTwistGoal = pactadjBase->dradTwistGoal;
	pactadj->ddradTwistGoal = pactadjBase->ddradTwistGoal;
	pactadj->dmatScale = pactadjBase->dmatScale;
}

void GetActadjPositionGoal(ACTADJ* pactadj, float dtOffset, glm::vec3* ppos, glm::vec3* pv)
{
    ACT* act = static_cast<ACT*>(pactadj)->dleAlo.pactNext;

    while (act != nullptr && act->ackPos == ACK_Nil)
		act = act->dleAlo.pactNext;

    if (act != nullptr && act->ackPos != ACK_None)
		act->pvtact->pfnGetActPositionGoal(act, dtOffset, ppos, pv);
    else
        GetActPositionGoal(pactadj, dtOffset, ppos, pv);

    *ppos += pactadj->dposGoal;
    *pv += pactadj->dvGoal;
}

void GetActadjRotationGoal(ACTADJ* pactadj, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    ACT* act = static_cast<ACT*>(pactadj)->dleAlo.pactNext;

    while (act != nullptr && act->ackRot == ACK_Nil)
        act = act->dleAlo.pactNext;

    if (act != nullptr && act->ackRot != ACK_None)
        act->pvtact->pfnGetActRotationGoal(act, dtOffset, pmat, pw);
    else
        GetActRotationGoal(pactadj, dtOffset, pmat, pw);

    *pmat = pactadj->dmatGoal * (*pmat);
    *pw = pactadj->dmatGoal * (*pw) + pactadj->dwGoal;
}

void GetActadjTwistGoal(ACTADJ* pactadj, float* pradTwist, float* pdradTwist)
{
    ACT* act = static_cast<ACT*>(pactadj)->dleAlo.pactNext;

    while (act != nullptr && act->ackRot == ACK_Nil)
        act = act->dleAlo.pactNext;

    if (act != nullptr && act->ackRot != ACK_None)
    {
        act->pvtact->pfnGetActTwistGoal(act, pradTwist, pdradTwist);
        return;
    }

    GetActTwistGoal(pactadj, pradTwist, pdradTwist);
}

void GetActadjScale(ACTADJ* pactadj, glm::mat3* pmat)
{

}

float GGetActadjPoseGoal(ACTADJ* pactadj, int ipose)
{
    return 0.0f;
}
