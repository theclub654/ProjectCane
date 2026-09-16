#include "actbank.h"
#include "so.h"

std::shared_ptr<ACTBANK> PactbankNew(SW* psw, ALO* palo, VTACTBANK* pvtactbank)
{
    auto pactbank = std::make_shared<ACTBANK>();

    pactbank->pvtactbank = pvtactbank;
    pactbank->pvtactbank->pfnInitActbank(pactbank.get(), palo);

    return pactbank;
}

std::shared_ptr<ACTBANK> PactbankNewClone(ACTBANK* pactbankBase, SW* psw, ALO* palo)
{
    auto pactbank = PactbankNew(psw, palo, &g_vtactbank);
    pactbank->pvtactbank->pfnCloneActbank(pactbank.get(), pactbankBase);

    return pactbank;

}

void InitActbank(ACTBANK* pactbank, ALO* palo)
{
	InitAct(pactbank, palo);
	pactbank->uBank = 1.0;
	pactbank->dtPredict = 0.25;
}

void CloneActbank(ACTBANK* pactbank, ACTBANK* pactbankBase)
{
	CloneAct(pactbank, pactbankBase);

	pactbank->uBank = pactbankBase->uBank;
	pactbank->dtPredict = pactbankBase->dtPredict;
}

void GetActbankRotationGoal(ACTBANK* pactbank, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    SO* pso = static_cast<SO*>(pactbank->palo);

    ACT* pactPos = pso->pactPos;

    glm::vec3 dv;

    if ((pso->pvtlo->grfcid & 2) == 0)
        dv = -pso->psw->dvGravity;
    else
        dv = -pso->dvGravity;

    if (pactPos != nullptr &&
        pactbank->dtPredict != 0.0f &&
        pactbank->uBank != 0.0f)
    {
        glm::vec3 vPredict;

        pactPos->pvtact->pfnGetActPositionGoal(pactPos, 0.0f, nullptr, &vPredict);

        glm::vec3 dvPredict =
            (vPredict - pso->xf.v) / pactbank->dtPredict;

        dv += dvPredict * pactbank->uBank;
    }

    BuildOrthonormalMatrixZ(&pso->xf.v, &dv, pmat);

    *pw = glm::vec3(0.0f);
}

