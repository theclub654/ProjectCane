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
    glm::mat4 model;
    LoadMatrixFromPosRot(pms->xf.posWorld, pms->xf.matWorld, model);

    for (int i = 0; i < pms->globset.aglob.size(); i++)
    {
        if ((pms->globset.aglobi[i].grfzon & pcm->grfzon) == pcm->grfzon) // Check if submodel is in camera BSP zone
        {
            if (pms->globset.aglob[i].newDmat.size() == 0)
            {
                for (int a = 0; a < pms->globset.aglob[i].asubglob.size(); a++)
                {
                    rpl.ro.VAO = &pms->globset.aglob[i].asubglob[a].VAO;
                    rpl.ro.VBO = &pms->globset.aglob[i].asubglob[a].VBO;
                    rpl.ro.EBO = &pms->globset.aglob[i].asubglob[a].EBO;

                    rpl.ro.modelMatrix = model;

                    rpl.ro.grfglob = &pms->globset.aglob[i].grfglob;
                    rpl.ro.pshd = pms->globset.aglob[i].asubglob[a].pshd;

                    rpl.ro.unSelfIllum = &pms->globset.aglob[i].asubglob[a].unSelfIllum;
                    rpl.ro.cvtx = pms->globset.aglob[i].asubglob[a].indices.size();

                    rpl.z = pms->globset.aglob[i].gZOrder;

                    if (rpl.z == 3.402823e+38)
                        rpl.z = glm::dot(rpl.z, rpl.z);

                    rpl.rp = pms->globset.aglob[i].rp;

                    SubmitRpl(&rpl);
                }
            }

            else
            {
                // Renders Instance Model
                for (int b = 0; b < pms->globset.aglob[pms->globset.aglob[i].instanceIndex].asubglob.size(); b++)
                {
                    rpl.ro.VAO = &pms->globset.aglob[pms->globset.aglob[i].instanceIndex].asubglob[b].VAO;
                    rpl.ro.VBO = &pms->globset.aglob[pms->globset.aglob[i].instanceIndex].asubglob[b].VBO;
                    rpl.ro.EBO = &pms->globset.aglob[pms->globset.aglob[i].instanceIndex].asubglob[b].EBO;

                    rpl.ro.modelMatrix = model * pms->globset.aglob[i].newDmat[0];

                    rpl.ro.grfglob = &pms->globset.aglob[i].grfglob;
                    rpl.ro.pshd = pms->globset.aglob[pms->globset.aglob[i].instanceIndex].asubglob[b].pshd;

                    rpl.ro.unSelfIllum = &pms->globset.aglob[pms->globset.aglob[i].instanceIndex].asubglob[b].unSelfIllum;
                    rpl.ro.cvtx = pms->globset.aglob[pms->globset.aglob[i].instanceIndex].asubglob[b].indices.size();

                    rpl.z = pms->globset.aglob[i].gZOrder;

                    if (rpl.z == 3.402823e+38)
                        rpl.z = glm::dot(rpl.z, rpl.z);

                    rpl.rp = pms->globset.aglob[pms->globset.aglob[i].instanceIndex].rp;

                    SubmitRpl(&rpl);
                }
            }
        }
    }
}

void DeleteMs(MS *pms)
{
    delete pms;
}
