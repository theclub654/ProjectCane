#include "ms.h"

MS* NewMs()
{
    return new MS{};
}

int GetMsSize()
{
    return sizeof(MS);
}

void RenderMsGlobset(MS *pms, CM *pcm, RO *pro)
{
    RPL rpl{};
    rpl.PFNDRAW = DrawGlob;
    
    glm::mat4 modelmatrix{};
    LoadMatrixFromPosRot(pms->xf.posWorld, pms->xf.matWorld, modelmatrix);

    rpl.ro.modelmatrix = modelmatrix;

    for (int i = 0; i < pms->globset.aglob.size(); i++)
    {
        if ((pms->globset.aglobi[i].grfzon & pcm->grfzon) == pcm->grfzon) // Check if submodel is in camera BSP zone
        {
            for (int a = 0; a < pms->globset.aglob[i].asubglob.size(); a++)
            {
                rpl.ro.VAO = &pms->globset.aglob[i].asubglob[a].VAO;

                if (pms->globset.aglob[i].asubglob[a].fCelBorder == 1)
                {
                    rpl.ro.celVAO = &pms->globset.aglob[i].asubglob[a].celVAO;
                    rpl.ro.celcvtx = pms->globset.aglob[i].asubglob[a].celcvtx;
                    rpl.ro.fCelBorder = 1;
                }
                else
                {
                    rpl.ro.celVAO = nullptr;
                    rpl.ro.fCelBorder = 0;
                }

                rpl.ro.grfglob = &pms->globset.aglob[i].grfglob;
                rpl.ro.pshd = pms->globset.aglob[i].asubglob[a].pshd;

                rpl.ro.fThreeWay = pms->globset.aglob[i].asubglob[a].fThreeWay;

                rpl.ro.unSelfIllum = &pms->globset.aglob[i].asubglob[a].unSelfIllum;

                rpl.ro.cvtx = pms->globset.aglob[i].asubglob[a].cvtx;
                
                rpl.z = pms->globset.aglob[i].gZOrder;

                if (rpl.z == 3.402823e+38)
                    rpl.z = glm::dot(rpl.z, rpl.z);

                rpl.ro.fDynamic = pms->globset.aglob[i].fDynamic;

                rpl.posCenter = pms->globset.aglob[i].posCenter;

                rpl.rp = pms->globset.aglob[i].rp;

                if (pms->globset.aglob[i].dmat.size() != 0)
                    rpl.ro.modelmatrix = pms->globset.aglob[i].dmat[0] * modelmatrix;

                SubmitRpl(&rpl);

                if (pms->globset.aglob[i].dmat.size() != 0)
                    rpl.ro.modelmatrix = modelmatrix;
            }
        }
    }
}

void DeleteMs(MS *pms)
{
    delete pms;
}
