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

	glm::mat4 baseModelMatrix{};
	LoadMatrixFromPosRot(pms->xf.posWorld, pms->xf.matWorld, baseModelMatrix);
	rpl.ro.modelmatrix = baseModelMatrix;

	for (int i = 0; i < pms->globset.aglob.size(); ++i)
	{
		auto& glob = pms->globset.aglob[i];
		glm::vec3 posCenterWorld = glm::vec3(baseModelMatrix * glm::vec4(glob.posCenter, 1.0f));

		if (!SphereInFrustum(pcm->frustum, posCenterWorld, glob.sRadius))
			continue;

		if (!FInsideCmMrd(pcm, pcm->pos - posCenterWorld, glob.sRadius, glob.sMRD, rpl.ro.uAlpha))
			continue;

		for (auto& subglob : glob.asubglob)
		{
			rpl.ro.VAO = &subglob.VAO;

			if (g_fRenderCelBorders && subglob.fCelBorder == 1)
			{
				rpl.ro.celVAO = &subglob.celVAO;
				rpl.ro.celcvtx = subglob.celcvtx;
				rpl.ro.fCelBorder = 1;
			}
			else
			{
				rpl.ro.celVAO = nullptr;
				rpl.ro.fCelBorder = 0;
			}

			rpl.ro.fDynamic = glob.fDynamic;
			rpl.z = glob.gZOrder;
			/*if (rpl.z == 3.402823e+38)
				rpl.z = glm::length(rpl.ro.uAlpha);*/
			rpl.ro.uFog = glob.uFog;
			rpl.posCenter = glob.posCenter;
			rpl.ro.grfglob = &glob.grfglob;
			rpl.ro.pshd = subglob.pshd;
			rpl.ro.unSelfIllum = &subglob.unSelfIllum;
			rpl.ro.cvtx = subglob.cvtx;

			rpl.rp = glob.rp;

			if (rpl.ro.uAlpha < 1.0)
			{
				switch (rpl.rp)
				{
				case RP_Opaque:
				case RP_Cutout:
				case RP_OpaqueAfterProjVolume:
				case RP_CutoutAfterProjVolume:
					rpl.rp = RP_Translucent;
					break;
				case RP_CelBorder:
				case RP_CelBorderAfterProjVolume:
					rpl.rp = RP_TranslucentCelBorder;
					break;
				}
			}

			if (!glob.dmat.empty())
				rpl.ro.modelmatrix = baseModelMatrix * glob.dmat[0];
			else
				rpl.ro.modelmatrix = baseModelMatrix;

			SubmitRpl(&rpl);
		}
	}
}

void DeleteMs(MS *pms)
{
    delete pms;
}
