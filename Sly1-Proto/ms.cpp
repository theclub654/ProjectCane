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
		//glm::vec3 posCenterWorld = glm::vec3(rpl.ro.modelmatrix * glm::vec4(pms->globset.aglob[i].posCenter, 1.0));

		if (SphereInFrustum(pcm->frustum, pms->globset.aglob[i].posCenter, pms->globset.aglob[i].sRadius) == 1)
		{
			for (int a = 0; a < pms->globset.aglob[i].asubglob.size(); a++)
			{
				rpl.ro.VAO = &pms->globset.aglob[i].asubglob[a].VAO;

				if (g_fRenderCelBorders == true)
				{
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
				}
				else
				{
					rpl.ro.celVAO = nullptr;
					rpl.ro.fCelBorder = 0;
				}

				rpl.posCenter = pms->globset.aglob[i].posCenter;

				rpl.ro.fDynamic = pms->globset.aglob[i].fDynamic;

				rpl.ro.uFog = pms->globset.aglob[i].uFog;

				rpl.ro.grfglob = &pms->globset.aglob[i].grfglob;

				rpl.ro.pshd = pms->globset.aglob[i].asubglob[a].pshd;
				rpl.ro.unSelfIllum = &pms->globset.aglob[i].asubglob[a].unSelfIllum;

				rpl.ro.cvtx = pms->globset.aglob[i].asubglob[a].cvtx;

				rpl.z = pms->globset.aglob[i].gZOrder;

				if (rpl.z == 3.402823e+38)
					rpl.z = glm::dot(rpl.z, rpl.z);

				rpl.rp = pms->globset.aglob[i].rp;

				if (pms->globset.aglob[i].dmat.size() != 0)
					rpl.ro.modelmatrix = modelmatrix * pms->globset.aglob[i].dmat[0];

				//if (palo->globset.aglob[i].rtck != RTCK_None)
					//AdjustAloRtckMat(palo, pcm, palo->globset.aglob[i].rtck, &palo->globset.aglob[i].posCenter, rpl.ro.modelmatrix);

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
