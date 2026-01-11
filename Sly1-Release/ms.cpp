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

		float dummy = 1.0f;
		if (!FInsideCmMrd(pcm, glm::vec4(pcm->pos, 1.0) - posCenterWorld, glob.sRadius, glob.sMRD, dummy))
			continue;

		// globi unfade smoothing
		float target = (g_clock.tReal < globi.tUnfade) ? 0.5f : 1.0f;

		if (globi.uAlpha != target)
			globi.uAlpha = GSmooth(globi.uAlpha, target, g_clock.dt, &s_smpFade, nullptr);

		float alpha = 1.0f * globi.uAlpha * g_uAlpha;

		if (alpha <= 0.0f) 
			continue;

		rpl.ro.uAlpha = alpha;
		rpl.ro.uFog = glob.uFog;

		if ((glob.grfglob & 4U) == 0)
			rpl.ro.darken = g_psw->rDarken;
		else
			rpl.ro.darken = 1.0;

		if (glob.pdmat != nullptr)
			rpl.ro.model = baseModelMatrix * *glob.pdmat;
		else
			rpl.ro.model = baseModelMatrix;

		if (glob.pwrbg != nullptr && glob.pwrbg->pwr != nullptr)
		{
			rpl.ro.warpType = glob.pwrbg->warpType;
			rpl.ro.warpCmat = glob.pwrbg->pwr->cmat;

			const size_t count = glob.pwrbg->cmat;

			switch (rpl.ro.warpType)
			{
				case WARP_POS:
				std::memcpy(rpl.ro.amatDpos, glob.pwrbg->pwr->amatDpos, count * sizeof(*rpl.ro.amatDpos));
				break;

				case WARP_UV:
				std::memcpy(rpl.ro.amatDuv, glob.pwrbg->pwr->amatDuv, count * sizeof(*rpl.ro.amatDuv));
				break;

				case WARP_BOTH:
				std::memcpy(rpl.ro.amatDpos, glob.pwrbg->pwr->amatDpos, count * sizeof(*rpl.ro.amatDpos));
				std::memcpy(rpl.ro.amatDuv,  glob.pwrbg->pwr->amatDuv,  count * sizeof(*rpl.ro.amatDuv));
				break;
			}

		}
		else
			rpl.ro.warpType = WARP_NONE;

		if (glob.rtck != RTCK_None)
			AdjustAloRtckMat(pms, pcm, glob.rtck, (glm::vec3*)&posCenterWorld, rpl.ro.model);

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
			}
		}

		switch (rpl.rp)
		{
			case RP_Background:
			rpl.z = -glm::length(pcm->pos - glm::vec3(rpl.ro.model * glm::vec4(glob.posCenter, 1.0f)));
			break;
			case RP_Cutout:
			case RP_CutoutAfterProjVolume:
			case RP_Translucent:
			rpl.z = glm::length(pcm->pos - glm::vec3(rpl.ro.model * glm::vec4(glob.posCenter, 1.0f)));
			break;
		}

		for (auto& subglob : glob.asubglob)
		{
			rpl.VAO  = subglob.VAO;
			rpl.cvtx = subglob.cvtx;

			rpl.pshd = subglob.pshd;

			if (subglob.pshd->shdk == SHDK_ThreeWay)
			{
				rpl.PFNBIND = BindThreeWay;
				rpl.ro.rko = RKO_ThreeWay;
				rpl.ro.fDynamic = glob.fDynamic;
				rpl.ro.posCenter = posCenterWorld;
				rpl.ro.sRadius = glob.sRadius;
				rpl.ro.unSelfIllum = subglob.unSelfIllum;
			}
			else
			{
				rpl.PFNBIND = BindOneWay;
				rpl.ro.rko = RKO_OneWay;
			}

			if (subglob.usesUvAnim && subglob.uvSai) 
			{
				rpl.ro.uvOffsets.x = subglob.uvSai->tcx.du;
				rpl.ro.uvOffsets.y = subglob.uvSai->tcx.dv;
			}
			else
			{
				rpl.ro.uvOffsets.x = 0.0;
				rpl.ro.uvOffsets.y = 0.0;
			}

			// ---- enable warp if this subglob is wired ----
			if (rpl.ro.warpType != WARP_NONE)
			{
				rpl.ro.warpCvtx   = subglob.pwarp->vertexCount;
				rpl.ssboWarpState = subglob.pwarp->ssboState;
			}

			SubmitRpl(&rpl);
		}
	}
}

void DeleteMs(MS* pms)
{
	delete pms;
}