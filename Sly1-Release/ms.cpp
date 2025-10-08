#include "ms.h"

MS* NewMs()
{
	return new MS{};
}

int GetMsSize()
{
	return sizeof(MS);
}

void RenderMsGlobset(MS* pms, CM* pcm, RO* pro)
{
	RPL rpl{};

	glm::mat4 baseModelMatrix{};
	LoadMatrixFromPosRot(pms->xf.posWorld, pms->xf.matWorld, baseModelMatrix);

	rpl.ro.model = baseModelMatrix;

	for (int i = 0; i < pms->globset.aglob.size(); ++i)
	{
		if (g_fBsp != 0)
		{
			if ((pms->globset.aglobi[i].grfzon & pcm->grfzon) != pcm->grfzon)
				continue;
		}

		auto& glob  = pms->globset.aglob[i];
		auto& globi = pms->globset.aglobi[i];

		glm::vec3 posCenterWorld = glm::vec3(baseModelMatrix * glm::vec4(glob.posCenter, 1.0f));

		if (!SphereInFrustum(pcm->frustum, posCenterWorld, glob.sRadius))
			continue;

		if (!FInsideCmMrd(pcm, pcm->pos - posCenterWorld, glob.sRadius, glob.sMRD, rpl.ro.uAlpha))
			continue;

		for (auto& subglob : glob.asubglob)
		{
			rpl.ro.VAO = &subglob.VAO;
			rpl.ro.fDynamic = glob.fDynamic;
			rpl.ro.uFog = glob.uFog;
			rpl.ro.posCenter = posCenterWorld;
			rpl.sRadius = glob.sRadius;

			if ((glob.grfglob & 4U) == 0)
				rpl.ro.darken = g_psw->rDarken;
			else
				rpl.ro.darken = 1.0;
			
			rpl.ro.pshd = subglob.pshd;
			rpl.grfshd = subglob.pshd->grfshd;
			rpl.ro.unSelfIllum = subglob.unSelfIllum;
			rpl.ro.cvtx = subglob.cvtx;
			rpl.rp = glob.rp;
			rpl.ro.uAlpha = rpl.ro.uAlpha * g_uAlpha;

			if (rpl.ro.uAlpha != 1.0)
			{
				switch (rpl.rp)
				{
					case RP_Opaque:
					case RP_Cutout:
					case RP_OpaqueAfterProjVolume:
					case RP_CutoutAfterProjVolume:
					rpl.rp = RP_Translucent;
					break;
				}
			}

			if (glob.pdmat != nullptr)
				rpl.ro.model = baseModelMatrix * *glob.pdmat;
			else
				rpl.ro.model = baseModelMatrix;

			switch (rpl.rp)
			{
				case RP_Background:
				rpl.z = glob.gZOrder;
				break;
				case RP_Cutout:
				case RP_CutoutAfterProjVolume:
				case RP_Translucent:
				rpl.z = glm::length2(pcm->pos - glm::vec3(rpl.ro.model * glm::vec4(subglob.posCenter, 1.0f)));
				break;
			}

			if (glob.rtck != RTCK_None)
				AdjustAloRtckMat(pms, pcm, glob.rtck, &glob.posCenter, rpl.ro.model);

			SubmitRpl(&rpl);

			rpl.ro.model = baseModelMatrix;
		}
	}
}

void DeleteMs(MS* pms)
{
	delete pms;
}
