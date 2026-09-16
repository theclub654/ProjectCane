#include "actadd.h"
#include "actseg.h"

std::shared_ptr<ACTADD> PactaddNew(SW* psw, ALO* palo)
{
    auto pactadd = std::make_shared<ACTADD>();

    pactadd.get()->pvtactadd = &g_vtactadd;
    pactadd->pvtact->pfnInitAct(pactadd.get(), palo);

    psw->actOwners.push_back(pactadd);

    return pactadd;
}

void CloneActadd(ACTADD* pactadd, ACTADD* pactaddBase)
{
    CloneAct(pactadd, pactaddBase);
    pactadd->cpactAdd = pactaddBase->cpactAdd;
}

void GetActaddPositionGoal(ACTADD* pactadd, float dtOffset, glm::vec3* ppos, glm::vec3* pv)
{
    if (pactadd == nullptr || pactadd->palo == nullptr || ppos == nullptr || pv == nullptr)
        return;

    ALO* palo = pactadd->palo;

    *ppos = palo->posOrig;
    *pv = g_vecZero;

    int cpactRemaining = pactadd->cpactAdd;
    ACT* pact = pactadd->dleAlo.pactNext;

    while (cpactRemaining > 0 && pact != nullptr)
    {
        ACT* pactNext = pact->dleAlo.pactNext;

        if (static_cast<uint8_t>(pact->ackPos + ACK_Spring) > 1)
        {
            if (pact->pvtact != nullptr && pact->pvtact != &g_vtact && pact->pvtact->pfnGetActPositionGoal != nullptr)
            {
                glm::vec3 pos(0.0f);
                glm::vec3 v(0.0f);

                pact->pvtact->pfnGetActPositionGoal(pact, dtOffset, &pos, &v);

                const glm::vec3 dpos = pos - palo->posOrig;

                *ppos += dpos;
                *pv += v;
            }

            --cpactRemaining;
        }

        pact = pactNext;
    }
}

void GetActaddRotationGoal(ACTADD* pactadd, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    if (pactadd == nullptr || pactadd->palo == nullptr || pmat == nullptr || pw == nullptr)
        return;

    ALO* palo = pactadd->palo;

    *pmat = palo->matOrig;
    *pw = g_vecZero;

    if (palo->pvtalo != nullptr && palo->pvtalo->pfnAdjustAloRotation != nullptr)
        palo->pvtalo->pfnAdjustAloRotation(palo, pmat, nullptr);

    const glm::mat3 matBase = *pmat;

    int cpactRemaining = pactadd->cpactAdd;
    ACTSEG* pactseg = pactadd->dleAlo.pactsegNext;

    while (cpactRemaining > 0 && pactseg != nullptr)
    {
        ACTSEG* pactsegNext = pactseg->dleAlo.pactsegNext;

        if (static_cast<uint8_t>(pactseg->ackRot + ACK_Spring) > 1)
        {
            if (pactseg->pvtact != nullptr && pactseg->pvtact != &g_vtact && pactseg->pvtact->pfnGetActRotationGoal != nullptr)
            {
                glm::mat3 mat(1.0f);
                glm::mat3 dmat(1.0f);
                glm::vec3 w(0.0f);

                pactseg->pvtact->pfnGetActRotationGoal(pactseg, dtOffset, &mat, &w);

                CalculateDmat(&matBase, &mat, &dmat);

                *pmat = dmat * (*pmat);
                *pw = dmat * (*pw) + w;
            }

            --cpactRemaining;
        }

        pactseg = pactsegNext;
    }

    
}
