#include "ms.h"
#include "wr.h"
#include "render.h"

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
	LoadMatrixFromPosRot(&pms->xf.posWorld, &pms->xf.matWorld, &baseModelMatrix);

	for (int i = 0; i < pms->globset.cglob; ++i)
	{
		auto& glob  = pms->globset.aglob[i];
		auto& globi = pms->globset.aglobi[i];

		if (g_fBsp != 0 && globi.grfzon != 0)
		{
			if ((globi.grfzon & pcm->grfzon) != pcm->grfzon)
				continue;
		}

		const glm::vec3 dpos3 = glob.posCenter - pcm->pos;
		const glm::vec4 dpos = glm::vec4(dpos3, 0.0f);

		if (!SphereInFrustum(pcm->frustum, glob.posCenter, glob.sRadius))
			continue;

		float dummy = 1.0f;

		if (!FInsideCmMrd(pcm, dpos, glob.sRadius, glob.sMRD, dummy))
			continue;

		rpl = {};

		rpl.palo = pms;
		rpl.pglob = &glob;

		rpl.ro.model = baseModelMatrix;
		rpl.ro.posCenter = glm::vec4(glob.posCenter, 1.0f);

		rpl.ro.uAlpha = 1.0f;
		rpl.ro.uAlphaCelBorder = 1.0f;

		if (glob.csubcel != 0)
		{
			if (glob.sCelBorderMRD < glob.sMRD)
			{
				float dummyCB = 1.0f;

				if (!FInsideCmMrd(pcm, dpos, glob.sRadius, glob.sCelBorderMRD, dummyCB))
					rpl.ro.uAlphaCelBorder = 0.0f;
			}
			else
				rpl.ro.uAlphaCelBorder = 1.0f;
		}
		else
			rpl.ro.uAlphaCelBorder = 0.0f;

		float target = (g_clock.tReal < globi.tUnfade) ? 0.5f : 1.0f;

		if (globi.uAlpha != target)
			globi.uAlpha = GSmooth(globi.uAlpha, target, g_clock.dt, &s_smpFade, nullptr);

		rpl.ro.uAlpha *= globi.uAlpha * g_uAlpha;

		if (rpl.ro.uAlpha <= 0.0f)
			continue;

		rpl.ro.uAlphaCelBorder *= rpl.ro.uAlpha;

		rpl.ro.uFog = glob.uFog;
		rpl.ro.darken = ((glob.grfglob & 4U) == 0) ? g_psw->rDarken : 1.0f;
		rpl.ro.fDynamic = glob.fDynamic;
		rpl.ro.sRadius = glob.sRadius;
		rpl.ro.grfglob = glob.grfglob;

		if (glob.pwarpGlob)
		{
			rpl.ro.warpType = glob.pwrbg->warpType;
			rpl.ro.warpCmat = glob.pwarpGlob->pwr->cmat;
			rpl.ro.warpCvtx = glob.pwarpGlob->vertexCount;

			const size_t count = static_cast<size_t>(glob.pwrbg->pwr->cmat);

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

				default:
				rpl.ro.warpType = WARP_NONE;
				break;
			}
		}
		else
			rpl.ro.warpType = WARP_NONE;

		if (glob.gZOrder != FLT_MAX)
			rpl.z = glob.gZOrder;
		else
			rpl.z = glm::dot(dpos3, dpos3);

		rpl.rp = glob.rp;

		if (rpl.ro.uAlpha < 1.0f)
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

				default:
				break;
			}
		}

		int sortT = 0;

		if (rpl.ro.uAlpha < 1.0f)
		{
			if (rpl.rp == RP_Translucent || rpl.rp == RP_TranslucentCelBorder)
				sortT = 1;
		}

		if (!sortT)
		{
			if (rpl.rp == RP_Background ||
				rpl.rp == RP_Cutout ||
				rpl.rp == RP_CutoutAfterProjVolume ||
				rpl.rp == RP_Translucent)
			{
				sortT = glob.fTransluscentSort;
			}
		}

		rpl.fTransluscentSort = sortT;

		if (glob.psaa != nullptr)
		{
			if (glob.psaa->pvtlooker != nullptr && glob.psaa->pvtlooker->pfnNotifyLookerRender != nullptr)
				glob.psaa->pvtlooker->pfnNotifyLookerRender((LOOKER*)glob.psaa, (ALO*)pms, &rpl);
		}

		if (glob.pdmat != nullptr)
			rpl.ro.model = baseModelMatrix * (*glob.pdmat);

		if (glob.rtck != RTCK_None)
			AdjustAloRtckMat(pms, pcm, glob.rtck, &glob.posCenter, rpl.ro.model);

		if (!allSwDynamicLights.empty() && glob.fThreeWay == 1)
			rpl.ro.fDynamicLight = FindSwDynamicLights(&glob.posCenter, glob.sRadius);
		else
			rpl.ro.fDynamicLight = 0;

		if (glob.fThreeWay == 1 && glob.fDynamic == 0 &&
			glob.pwarpGlob == nullptr &&
			globi.cframeStaticLights < g_cframeStaticLightsInvalid)
		{
			glob.trlk = TRLK_Relight;
		}

		if (glob.trlk == TRLK_Relight)
		{
			rpl.ro.trlk = TRLK_Relight;
			glob.trlk = TRLK_BakePending;
			globi.cframeStaticLights = g_cframe;
		}
		else if (glob.trlk == TRLK_BakePending)
		{
			// The cache-writing packet is still deferred.  Do not let another
			// packet read from the SSBO until DrawGlob completes that write.
			rpl.ro.trlk = TRLK_Dynamic;
		}
		else
		{
			rpl.ro.trlk = glob.trlk;
		}

		if (glob.fDynamic == 1 || glob.pwarpGlob != nullptr)
			rpl.ro.trlk = TRLK_Dynamic;

		SubmitRpl(&rpl);
	}
}

void DeleteMs(MS *pms)
{
	delete pms;
}
