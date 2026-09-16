#include "actseg.h"
#include "acg.h"
#include "acs.h"
#include "acr.h"
#include "acp.h"
#include "aseg.h"
#include "jt.h"
#include <cmath>

namespace
{
CHN* PchnFromActseg(ACTSEG* pactseg)
{
    if (pactseg == nullptr || pactseg->pasega == nullptr || pactseg->pasega->paseg == nullptr)
        return nullptr;

    ASEG* paseg = pactseg->pasega->paseg;

    if (pactseg->ichn >= 0 && pactseg->ichn < paseg->cchn)
        return &paseg->achn[pactseg->ichn];

    if (pactseg->ichn == paseg->cchn)
        return paseg->pchnStrip;

    return nullptr;
}
}

ACTSEG *PactsegNew(SW* psw, ALO* palo)
{
    std::shared_ptr <ACTSEG> pactseg = std::make_shared<ACTSEG>();

    pactseg->pvtactseg = &g_vtactseg;
    pactseg->pvtactseg->pfnInitAct(pactseg.get(), palo);

    // Keep ACTSEG alive while intrusive lists hold raw pointers.
    psw->actOwners.push_back(std::static_pointer_cast<ACT>(pactseg));

    return pactseg.get();
}

ACTSEG *PactsegNewClone(ACTSEG* pactsegBase, SW* psw, ALO* palo)
{
    ACTSEG *pactseg = PactsegNew(psw, palo);
    pactseg->pvtactseg->pfnCloneActseg(pactseg, pactsegBase);

    return pactseg;
}

void CloneActseg(ACTSEG* pactseg, ACTSEG* pactsegBase)
{
	// Clone the inherited ACT state while retaining this instance's ASEGA
	// linkage and intrusive-list entry, matching the original CloneActseg.
	CloneAct(pactseg, pactsegBase);

	pactseg->ichn = pactsegBase->ichn;
}

void RetractActseg(ACTSEG* pactseg, GRFRA grfra)
{
	ASEGA* pasega = pactseg->pasega;

	RemoveDlEntry(&pasega->dlActseg, pactseg);

	if (pasega->pactsegError == pactseg)
		pasega->pactsegError = nullptr;

	RetractAct(pactseg, grfra);
}

void GetActsegPositionGoal(ACTSEG* pactseg, float dtOffset, glm::vec3* ppos, glm::vec3* pv)
{
    ASEGA* pasega = pactseg->pasega;
    CHN* pchn = PchnFromActseg(pactseg);
    ACP* pacp = pchn != nullptr ? pchn->pacp.get() : nullptr;

    if (!pacp)
    {
        GetActPositionGoal((ACT*)pactseg, dtOffset, ppos, pv);
        return;
    }

    float tLocal = pasega->tLocal;

    if (dtOffset != 0.0f)
    {
        float svtLocal = pasega->svtLocal;
        tLocal += dtOffset * svtLocal;
        FWrapAsegaTime(pasega, &tLocal, &svtLocal);
    }

    if (pactseg->ackPos == ACK_Spline)
    {
        float dtMatch = (pactseg->tMatch - g_clock.t) - pasega->dtPause;

        if (dtMatch > 0.0f)
            tLocal += dtMatch * pasega->svtLocal;
    }

	pacp->pvtacp->pfnEvaluateAcp(pacp, pactseg->palo, tLocal, pasega->svtLocal, 0, ppos, pv);

}

void GetActsegRotationGoal(ACTSEG* pactseg, float dtOffset, glm::mat3* pmat, glm::vec3* pw)
{
    ASEGA* pasega = pactseg->pasega;
    CHN* pchn = PchnFromActseg(pactseg);
    ACR* pacr = pchn != nullptr ? pchn->pacr.get() : nullptr;

    if (!pacr)
    {
        GetActRotationGoal((ACT*)pactseg, dtOffset, pmat, pw);
        return;
    }

    float tLocal = pasega->tLocal;

    if (dtOffset != 0.0f)
    {
        float svtLocal = pasega->svtLocal;
        tLocal += dtOffset * svtLocal;
        FWrapAsegaTime(pasega, &tLocal, &svtLocal);
    }

    if (pactseg->ackRot == ACK_Spline)
    {
        float dtMatch = (pactseg->tMatch - g_clock.t) - pasega->dtPause;

        if (dtMatch > 0.0f)
            tLocal += dtMatch * pasega->svtLocal;
    }

    pacr->pvtacr->pfnEvaluateAcr(pacr, pactseg->palo, tLocal, pasega->svtLocal, 0, pmat, pw);

}

void GetActsegTwistGoal(ACTSEG* pactseg, float* pradTwist, float* pdradTwist)
{
    ASEGA* pasega  = pactseg->pasega;
    CHN* pchn = PchnFromActseg(pactseg);
    ACG* pacgTwist = pchn != nullptr ? pchn->pacgTwist.get() : nullptr;

    if (!pacgTwist)
    {
        GetActTwistGoal((ACT*)pactseg, pradTwist, pdradTwist);
        return;
    }

    pacgTwist->pvtacg->pfnEvaluateAcg(pacgTwist, pactseg->palo, pasega->tLocal, pasega->svtLocal, 0, pradTwist, pdradTwist);
}

void GetActsegScale(ACTSEG* pactseg, glm::mat3* pmat)
{
    ASEGA* pasega = pactseg->pasega;
    CHN* pchn = PchnFromActseg(pactseg);
    ACS* pacs = pchn != nullptr ? pchn->pacs.get() : nullptr;

    if (!pacs)
    {
        GetActScale((ACT*)pactseg, pmat);
        return;
    }

    pacs->pvtacs->pfnEvaluateAcs(pacs, pactseg->palo, pasega->tLocal, 0, pmat);
}

float GGetActsegPoseGoal(ACTSEG* pactseg, int ipose)
{
    ASEGA* pasega = pactseg->pasega;
    CHN* pchn = PchnFromActseg(pactseg);

    if (pchn == nullptr || ipose < 0 || ipose >= static_cast<int>(pchn->apacgPose.size()) || pchn->apacgPose[ipose] == nullptr)
        return GetActPoseGoal(static_cast<ACT*>(pactseg), ipose);

    ACG* pacgPose = pchn->apacgPose[ipose].get();

    float gPose = 0.0f;

    pacgPose->pvtacg->pfnEvaluateAcg(pacgPose, pactseg->palo, pasega->tLocal, pasega->svtLocal, 0, &gPose, nullptr);

    return gPose;
}
