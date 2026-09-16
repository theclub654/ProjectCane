#include "acp.h"
#include "acpb.h"
#include "acpc.h"
#include "acpbl.h"

std::shared_ptr<ACP> PacpNew(ACVK acvk)
{
    std::shared_ptr<ACP> pacp;

    switch (acvk)
    {
        case ACVK_Bezier:
        {
            auto pacpb = std::make_shared<ACPB>();
            pacpb->pvtacpb = &g_vtacpb;
            pacp = pacpb;
            break;
        }

        case ACVK_Component:
        {
            auto pacpc = std::make_shared<ACPC>();
            pacpc->pvtacpc = &g_vtacpc;
            pacp = pacpc;
            break;
        }

        case ACVK_Blend:
        {
            auto pacpbl = std::make_shared<ACPBL>();
            pacpbl->pvtacpbl = &g_vtacpbl;
            pacp = pacpbl;
            break;
        }

        default:
        return nullptr;
    }

    pacp->acvk = acvk;
    return pacp;
}

void EvaluateAcp(ACP* pacp, ALO* palo, float t, float svt, GRFEVAL grfeval, glm::vec3* ppos, glm::vec3* pv)
{

}

void GetAcpTimes(ACP* pacp, int* pct, std::vector<float>& pat)
{
    *pct = 0;
    pat.clear();
}

VTACP g_vtacp;