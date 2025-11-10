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
	glm::mat4 baseModelMatrix{};
	LoadMatrixFromPosRot(pms->xf.posWorld, pms->xf.matWorld, baseModelMatrix);

	float uAlpha{};

	for (int i = 0; i < pms->globset.aglob.size(); ++i)
	{
		if (g_fBsp != 0)
		{
			if ((pms->globset.aglobi[i].grfzon & pcm->grfzon) != pcm->grfzon)
				continue;
		}

		auto& glob  = pms->globset.aglob[i];
		auto& globi = pms->globset.aglobi[i];

		glm::vec4 posCenterWorld = baseModelMatrix * glm::vec4(glob.posCenter, 1.0f);

		if (!SphereInFrustum(pcm->frustum, posCenterWorld, glob.sRadius))
			continue;

		if (!FInsideCmMrd(pcm, glm::vec4(pcm->pos, 1.0) - posCenterWorld, glob.sRadius, glob.sMRD, uAlpha))
			continue;

		for (auto& subglob : glob.asubglob)
		{
			subglob.rpl.ro.uAlpha = uAlpha;

			subglob.rpl.ro.posCenter = posCenterWorld;

			if ((glob.grfglob & 4U) == 0)
				subglob.rpl.ro.darken = g_psw->rDarken;
			else
				subglob.rpl.ro.darken = 1.0;

			subglob.rpl.rp = glob.rp;
			subglob.rpl.ro.uAlpha = subglob.rpl.ro.uAlpha * g_uAlpha;

			if (subglob.rpl.ro.uAlpha != 1.0)
			{
				switch (subglob.rpl.rp)
				{
					case RP_Opaque:
					case RP_Cutout:
					case RP_OpaqueAfterProjVolume:
					case RP_CutoutAfterProjVolume:
					subglob.rpl.rp = RP_Translucent;
					break;
				}
			}

			if (glob.pdmat != nullptr)
				subglob.rpl.ro.model = baseModelMatrix * *glob.pdmat;
			else
				subglob.rpl.ro.model = baseModelMatrix;

			switch (subglob.rpl.rp)
			{
				case RP_Background:
				subglob.rpl.z = -glm::length2(pcm->pos - glm::vec3(subglob.rpl.ro.model * glm::vec4(subglob.posCenter, 1.0f)));
				break;
				case RP_Cutout:
				case RP_CutoutAfterProjVolume:
				case RP_Translucent:
				subglob.rpl.z = glm::length2(pcm->pos - glm::vec3(subglob.rpl.ro.model * glm::vec4(subglob.posCenter, 1.0f)));
				break;
			}

			if (glob.rtck != RTCK_None)
				AdjustAloRtckMat(pms, pcm, glob.rtck, (glm::vec3*)&posCenterWorld, subglob.rpl.ro.model);

			SubmitRpl(&subglob.rpl);
		}
	}
}


void DeleteMs(MS* pms)
{
	delete pms;
}
