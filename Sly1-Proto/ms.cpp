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

                rpl.ro.grfglob = &pms->globset.aglob[i].grfglob;
                rpl.ro.pshd = pms->globset.aglob[i].asubglob[a].pshd;

                rpl.ro.unSelfIllum = &pms->globset.aglob[i].asubglob[a].unSelfIllum;

                rpl.ro.cvtx = pms->globset.aglob[i].asubglob[a].indices.size() * sizeof(INDICE);
                
                rpl.z = pms->globset.aglob[i].gZOrder;

                if (rpl.z == 3.402823e+38)
                    rpl.z = glm::dot(rpl.z, rpl.z);

                rpl.ro.fDynamic = pms->globset.aglob[i].fDynamic;

                rpl.posCenter = pms->globset.aglob[i].posCenter;
                
                rpl.rp = pms->globset.aglob[i].rp;

                if (pms->globset.aglob[i].dmat.size() != 0)
                    rpl.ro.modelmatrix = pms->globset.aglob[i].dmat[0] * modelmatrix;

                /*if (pms->globset.aglob[i].rtck != RTCK_None)
                    AdjustAloRtckMat(pms, pcm, pms->globset.aglob[i].rtck, &pms->globset.aglob[i].posCenter, &rpl.ro.modelmatrix);*/

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
