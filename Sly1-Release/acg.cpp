#include "acg.h"
#include "acgb.h"
#include "acgbw.h"
#include "acgl.h"
#include "acgblt.h"
#include "acgblp.h"

std::shared_ptr<ACG> PacgNew(ACGK acgk)
{
    std::shared_ptr<ACG> pacg;

    switch (acgk)
    {
        case ACGK_Bezier:
        {
            auto pacgb = std::make_shared<ACGB>();
            pacgb->pvtacgb = &g_vtacgb;
            pacg = pacgb;
            break;
        }

        case ACGK_BezierWeighted:
        {
            auto pacgbw = std::make_shared<ACGBW>();
            pacgbw->pvtacgbw = &g_vtacgbw;
            pacg = pacgbw;
            break;
        }

        case ACGK_Linear:
        {
            auto pacgl = std::make_shared<ACGL>();
            pacgl->pvtacgl = &g_vtacgl;
            pacg = pacgl;
            break;
        }

        case ACGK_BlendTwist:
        {
            auto pacgblt = std::make_shared<ACGBLT>();
            pacgblt->pvtacgblt = &g_vtacgblt;
            pacg = pacgblt;
            break;
        }

        case ACGK_BlendPose:
        {
            auto pacgblp = std::make_shared<ACGBLP>();
            pacgblp->pvtacgblp = &g_vtacgblp;
            pacg = pacgblp;
            break;
        }

        default:
        return nullptr;
    }

    pacg->acgk = acgk;
    return pacg;
}

void EvaluateAcg(ACG* pacg, ALO* palo, float t, float svt, GRFEVAL grfeval, float* pg, float* pdg)
{

}

void GetAcgTimes(ACG* pacg, int* pct, std::vector <float> &pat)
{
    *pct = 0;
    pat.clear();
}

VTACG g_vtacg;