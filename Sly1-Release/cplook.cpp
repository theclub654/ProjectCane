#include "cplook.h"
#include "jt.h"
#include "cm.h"
#include "binoc.h"
#include "tv.h"
#include "speaker.h"
#include "sound.h"
#include "turret.h"
#include "pnt.h"
#include "xform.h"

static bool FGetCplookTargetPos(LO* plo, glm::vec3* ppos)
{
	if (plo == nullptr)
		return false;

	if (FIsBasicDerivedFrom(plo, CID_ALO))
		*ppos = static_cast<ALO*>(plo)->xf.posWorld;
	else if (FIsBasicDerivedFrom(plo, CID_PNT))
		GetPntPos(static_cast<PNT*>(plo), ppos);
	else if (FIsBasicDerivedFrom(plo, CID_XFM))
		GetXfmPos(static_cast<XFM*>(plo), ppos);
	else
		return false;

	return true;
}

void InitCplook(CPLOOK* pcplook, CM* pcm)
{
	InitCplcy(pcplook, pcm);
	pcplook->clookk = 0;
	pcplook->rZoomMax = 10.0;
	PushCplookLookk(pcplook, LOOKK_User);
	pcplook->rScreenSniper = 0.5;
	pcplook->sRadiusSniper = -1.0;
	pcplook->paloFocusSniper = nullptr;
	pcplook->sNearClipRestore = pcm->sNearClip;
}

void PosCplookAnchor(CPLOOK* pcplook, glm::vec3* pposAnchor)
{
	if (pcplook->ppntAnchor != nullptr)
	{
		GetPntPos(pcplook->ppntAnchor, pposAnchor);
		return;
	}

	PO* ppo = PpoCur();

	if (ppo == nullptr)
	{
		*pposAnchor = g_pcm->pos;
		return;
	}

	*pposAnchor = ppo->xf.posWorld;
	pposAnchor->z += DZ_CplookAnchor;
}

void StartCplookSound(CPLOOK* pcplook)
{
	StartSound((SFXID)113, &pcplook->pambBinoc, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

	if (pcplook->pambBinoc != nullptr)
		PausePamb(pcplook->pambBinoc);

	StartSound((SFXID)112, &pcplook->pambAmbient, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

	pcplook->fSoundPaused = 1;
}

void PushCplookLookk(CPLOOK* pcplook, LOOKK lookk)
{
	if (pcplook->clookk >= 4)
		return;

	pcplook->alookk[pcplook->clookk++] = lookk;
}

void PosCplookEye(CPLOOK* pcplook, glm::vec3* pposEye)
{
	const glm::vec3& dposLook = (g_pjt != nullptr && g_pjt->jtbs == JTBS_Peek_Peek) ? s_dposLookNear : s_dposLookFar;

	glm::mat3 matTarget{};
	glm::vec3 posAnchor{};

	LoadRotateMatrixPanTilt(pcplook->radPan, pcplook->radTilt, &matTarget);
	PosCplookAnchor(pcplook, &posAnchor);

	*pposEye = posAnchor + matTarget * dposLook;
}

LOOKK LookkPopCplook(CPLOOK* pcplook)
{
	if (pcplook->clookk <= 0)
		return LOOKK_Nil;

	return pcplook->alookk[--pcplook->clookk];
}

LOOKK LookkCurCplook(CPLOOK* pcplook)
{
	if (pcplook->clookk <= 0)
		return LOOKK_Nil;

	return pcplook->alookk[pcplook->clookk - 1];
}

void ActivateCplook(CPLOOK* pcplook, void* pv)
{
	CM* pcm = pcplook->pcm;

	DecomposeRotateMatrixPanTilt(&pcm->mat, &pcplook->radPan, &pcplook->radTilt);

	pcplook->swPan = 0.0f;
	pcplook->radTilt = RAD_CplookTiltDefault;
	pcplook->swTilt = 0.0f;
	pcplook->uZoom = 0.0f;
	pcplook->svuZoom = 0.0f;

	pcplook->lmFOV.gMax = pcm->radFOVTarget;
	pcplook->lmFOV.gMin = pcm->radFOVTarget / pcplook->rZoomMax;

	glm::vec3 posAnchor{};
	PosCplookAnchor(pcplook, &posAnchor);
	ResetCmLookAtSmooth(pcm, &posAnchor);

	pcplook->sNearClipRestore = pcm->sNearClip;
	SetCmNearClip(pcm, 49.0f);
}

void UpdateCplook(CPLOOK* pcplook, CPDEFI* pcpdefi, JOY* pjoy, float dt)
{
	(void)pcpdefi;
	if (dt == 0.0f)
		return;

	CM* pcm = pcplook->pcm;
	glm::vec3 posAnchor{};
	PosCplookAnchor(pcplook, &posAnchor);

	const LOOKK lookk = LookkCurCplook(pcplook);

	// Update the zoom controller. Retail shares the sniper controller between
	// LOOKK_Sniper and LOOKK_SniperTrack, then gives Dialog its own controller.
	switch (lookk)
	{
		case LOOKK_User:
		{
			const float zoomRate = (s_clqUZoomToRsvu.g0 + pcplook->uZoom * (s_clqUZoomToRsvu.g1 + pcplook->uZoom * s_clqUZoomToRsvu.g2)) * pjoy->y2 * std::abs(pjoy->y2);
			pcplook->svuZoom = zoomRate;
			pcplook->uZoom = glm::clamp(pcplook->uZoom + zoomRate * dt, 0.0f, 1.0f);
			break;
		}

		case LOOKK_Sniper:
		case LOOKK_SniperTrack:
		{
			ALO* focus = pcplook->paloFocusSniper;
			bool stretch = false;
			float zoomTarget = 0.0f;

			if (focus != nullptr)
			{
				float radius = pcplook->sRadiusSniper;
				if (radius <= 0.0f)
					radius = focus->sRadiusRenderAll;

				const float distance = glm::length(g_pcm->pos - focus->xf.posWorld);
				float fovObject = pcplook->lmFOV.gMin;
				if (distance > 0.0001f && pcplook->rScreenSniper > 0.0001f)
					fovObject = 2.0f * std::atan(radius / (pcplook->rScreenSniper * distance));
				fovObject = glm::max(fovObject, pcplook->lmFOV.gMin);

				glm::vec3 predicted{};
				PredictAloTransform(focus, nullptr, 0.1f, &predicted, nullptr, nullptr, nullptr);
				const glm::vec3 movement = predicted - focus->xf.posWorld;
				const glm::vec3 center = focus->xf.posWorld + movement * 0.5f;
				const float expandedRadius = radius + glm::length(movement) * 0.5f;
				const glm::vec3 cameraDelta = center - g_pcm->pos;

				const float cameraX = glm::dot(g_pcm->mat[0], cameraDelta);
				const float cameraY = glm::dot(g_pcm->mat[1], cameraDelta);
				const float cameraZ = glm::dot(g_pcm->mat[2], cameraDelta);
				float fovFrame = pcplook->lmFOV.gMax;
				if (cameraX > 0.0001f)
				{
					const float fovHorizontal = 2.0f * std::atan((std::abs(cameraY) + expandedRadius) /
						(cameraX * g_pcm->rAspect));
					const float fovVertical = 2.0f * std::atan((std::abs(cameraZ) + expandedRadius) / cameraX);
					fovFrame = glm::min(glm::max(fovHorizontal, fovVertical), pcplook->lmFOV.gMax);
				}
				const float fovRequired = glm::max(fovObject, fovFrame);
				const float fovRange = pcplook->lmFOV.gMax - pcplook->lmFOV.gMin;
				zoomTarget = fovRange != 0.0f
					? glm::clamp(1.0f - (fovRequired - pcplook->lmFOV.gMin) / fovRange, 0.0f, 1.0f)
					: 0.0f;
				stretch = fovObject < fovFrame && zoomTarget < pcplook->uZoom;
			}

			bool testCoast = g_binoc.fTargeting == 1;
			if (!testCoast && !stretch)
			{
				testCoast = pcplook->uZoom <= zoomTarget ||
					(pcplook->svuZoom >= 0.0f && pcplook->uZoom - zoomTarget <= 0.025f);
			}

			const bool coast = testCoast &&
				(zoomTarget <= pcplook->uZoom ||
				 (pcplook->svuZoom <= 0.0f && zoomTarget - pcplook->uZoom <= 0.035f));

			if (coast)
			{
				pcplook->svuZoom = GSmooth(pcplook->svuZoom, 0.0f, dt, &s_smpSvu, nullptr);
				pcplook->uZoom = glm::clamp(pcplook->uZoom + pcplook->svuZoom * dt, 0.0f, 1.0f);
			}
			else
			{
				SMPA smpa = stretch ? s_smpaSniperStretch : s_smpaSniperZoom;
				pcplook->uZoom = GSmoothA(pcplook->uZoom, pcplook->svuZoom, zoomTarget,
					dt, &smpa, &pcplook->svuZoom);
			}
			break;
		}

		case LOOKK_Dialog:
		{
			const float zoomOld = pcplook->uZoom;
			pcplook->uZoom = glm::clamp(GSmooth(zoomOld, g_binoc.uZoom, dt, &s_smpDialogZoom, nullptr), 0.0f, 1.0f);
			pcplook->svuZoom = (pcplook->uZoom - zoomOld) / dt;
			break;
		}

		default:
		break;
	}

	if (pcplook->pambBinoc != nullptr)
	{
		if (pcplook->svuZoom == 0.0f)
		{
			if (pcplook->fSoundPaused == 0)
			{
				PausePamb(pcplook->pambBinoc);
				pcplook->fSoundPaused = 1;
			}
		}
		else
		{
			if (pcplook->fSoundPaused != 0)
			{
				ContinuePamb(pcplook->pambBinoc);
				pcplook->fSoundPaused = 0;
			}
			SetPambFrq(pcplook->pambBinoc, glm::clamp(std::abs(pcplook->svuZoom), -1.0f, 1.0f));
			SetPambVol(pcplook->pambBinoc, glm::clamp(std::abs(pcplook->svuZoom) * 0.5f + 0.5f, 0.0f, 1.0f));
		}
	}

	const float radFov = glm::mix(pcplook->lmFOV.gMax, pcplook->lmFOV.gMin, pcplook->uZoom);
	if (!FFloatsNear(pcm->radFOV, radFov, 0.0001f))
		SetCmFov(pcm, radFov);

	glm::vec3 posEye{};
	if (lookk == LOOKK_Confront)
	{
		TV* tv = g_tvLeft.tvs == TVS_Talk ? &g_tvLeft :
			(g_tvRight.tvs == TVS_Talk ? &g_tvRight : nullptr);
		if (tv == nullptr || tv->pspeaker == nullptr)
			return;

		ALO* target = g_binoc.paloLookat != nullptr ? g_binoc.paloLookat : tv->paloReplace;
		if (target == nullptr)
			return;

		SPEAKER* speaker = tv->pspeaker;
		const glm::vec3 center = target->xf.posWorld;
		const float screenX = speaker->uxTvConfront * 2.0f - 1.0f;
		glm::vec3 project{};
		if (pcplook->ppntAnchor == nullptr)
			project = glm::normalize(target->xf.matWorld[0] + target->xf.matWorld[1] * screenX);
		else
		{
			glm::vec3 anchor{};
			GetPntPos(pcplook->ppntAnchor, &anchor);
			project = glm::normalize(anchor - center);
		}

		glm::vec3 cameraCandidate = center + project * (speaker->sDistCm * (4.0f / 9.0f));
		cameraCandidate += speaker->dposTvConfrontCamera;

		float pan = 0.0f;
		float tilt = 0.0f;
		float distance = 0.0f;
		DecomposeSphere(&cameraCandidate, &center, &pan, &tilt, &distance);
		SetVectorSphere(&posEye, pan, tilt, distance);
		pcm->pos = center + posEye;

		const float cameraTilt = -tilt - (speaker->uyTvConfront * 2.0f - 1.0f) * pcm->radFOV * 0.5f;
		glm::mat3 matPan{}, matTilt{}, matRoll{};
		LoadRotateMatrixRad(pan + 3.1415927f + screenX * pcm->radFOV * pcm->rAspect * 0.5f, &g_normalZ, &matPan);
		LoadRotateMatrixRad(-cameraTilt, &g_normalY, &matTilt);
		LoadRotateMatrixRad(speaker->radTvConfrontRoll, &g_normalX, &matRoll);
		pcm->mat = matPan * matTilt * matRoll;
		UpdateCmMat4(pcm);
		pcm->posCenterPrev = center;
		return;
	}

	if (lookk == LOOKK_Dialog)
	{
		glm::vec3 posLookat{};
		if (!FGetCplookTargetPos(reinterpret_cast<LO*>(g_binoc.paloLookat), &posLookat))
			PosCplookEye(pcplook, &posEye);
		else
		{
			glm::vec3 dir = posLookat - posAnchor;
			const float length = glm::length(dir);
			if (length > 0.0001f)
			{
				dir /= length;
				pcplook->radPan = std::atan2(dir.y, dir.x);
				posEye = posAnchor - dir * glm::length(s_dposLookNear);
			}
			else
			{
				// A dialog may deliberately use the same PNT as both its focus
				// and anchor. There is no direction to derive in that case, so
				// retain the view established when binocular mode activated.
				PosCplookEye(pcplook, &posEye);
			}
		}

		// Original LOOKK_Dialog camera response: spring 20, damping -7.
		SetCmLookAtSmooth(pcm, 1, &posEye, &posAnchor, nullptr, 20.0f, -7.0f, 0.25f, 0.0f, 0.0f, dt);
		return;
	}

	if (lookk == LOOKK_SniperTrack)
	{
		TURRET* turret = static_cast<TURRET*>(PpoCur());
		glm::vec3 aim = pcplook->paloFocusSniper != nullptr
			? pcplook->paloFocusSniper->xf.posWorld
			: posAnchor + pcm->mat[0] * 100.0f;
		if (turret != nullptr && !turret->fTrack)
		{
			glm::vec3 projected = posAnchor + pcm->mat * glm::vec3(100.0f, 0.0f, 0.0f);
			glm::vec3 screen{};
			ConvertCmWorldToScreen(pcm, &projected, &screen);
			screen += glm::vec3(pjoy->x, ((g_pgsCur->grfgs & 1024U) == 0) ? -pjoy->y : pjoy->y, 0.0f) *
				(DT_CplookSniperTrack * dt);
			ConvertCmScreenToWorld(pcm, &screen, &aim);
		}

		glm::vec3 direction = aim - posAnchor;
		glm::mat3 look{};
		LoadLookAtMatrix(&direction, &look);
		DecomposeRotateMatrixPanTilt(&look, &pcplook->radPan, &pcplook->radTilt);
		PosCplookEye(pcplook, &posEye);
		SetCmLookAtSmooth(pcm, 1, &posEye, &posAnchor, nullptr, 20.0f, -7.0f, 1.0f, 0.0f, 0.0f, dt);
		return;
	}

	if (lookk == LOOKK_User || lookk == LOOKK_Sniper)
	{
		float inputPan = GLimitAbs(pjoy->x + pjoy->x2, 1.0f);
		float inputTilt = GLimitAbs(pjoy->y, 1.0f);
		if (g_pjt != nullptr && PpoCur() == g_pjt && g_pjt->jtbs != JTBS_Peek_Peek)
		{
			inputPan = 0.0f;
			inputTilt = 0.0f;
		}

		const float inputScale = lookk == LOOKK_User
			? 1.0f
			: glm::mix(0.5f, 1.0f, pcm->radFOV);
		float panAcceleration = -inputPan * inputPan * inputPan * inputScale * 10.0f;
		float tiltAcceleration = inputTilt * inputTilt * inputTilt * inputScale * 10.0f;
		if ((lookk == LOOKK_User && (g_pgsCur->grfgs & 512U) == 0) ||
			(lookk == LOOKK_Sniper && (g_pgsCur->grfgs & 1024U) == 0))
			tiltAcceleration = -tiltAcceleration;

		if (panAcceleration * pcplook->swPan <= 0.0f)
			pcplook->swPan = 0.0f;
		if (tiltAcceleration * pcplook->swTilt <= 0.0f)
			pcplook->swTilt = 0.0f;

		pcplook->swPan   = GSmooth(pcplook->swPan, 0.0f, dt, &s_smpSwCplook, nullptr);
		pcplook->swTilt  = GSmooth(pcplook->swTilt, 0.0f, dt, &s_smpSwCplook, nullptr);
		pcplook->swPan   = GLimitAbs(pcplook->swPan + panAcceleration * dt, pcm->radFOV * 1.3f * 0.9f);
		pcplook->swTilt  = GLimitAbs(pcplook->swTilt + tiltAcceleration * dt, pcm->radFOV * 1.3f * 0.8f);
		pcplook->radPan  = RadNormalize(pcplook->radPan + pcplook->swPan * dt);
		pcplook->radTilt = glm::clamp(pcplook->radTilt + pcplook->swTilt * dt, -1.0f, 1.0f);

		PosCplookEye(pcplook, &posEye);

		const float u = glm::clamp((g_clock.t - pcm->tActivateCplcy) * 2.0f, 0.0f, 1.0f);
		const bool entering = g_pjt == nullptr || g_pjt->jtbs != JTBS_Peek_Peek;
		const int grflas = entering ? 1 : 0;
		const float springTarget = entering ? 400.0f : 0.0f;
		const float dampingTarget = entering ? -50.0f : 0.0f;
		const float spring = glm::mix(20.0f, springTarget, u);
		const float damping = glm::mix(-7.0f, dampingTarget, u);
		const float limit = glm::mix(1.0f, 0.25f, u);

		SetCmLookAtSmooth(pcm, grflas, &posEye, &posAnchor, nullptr, spring, damping, limit, 0.0f, 0.0f, dt);
	}
}

void DeactivateCplook(CPLOOK* pcplook)
{
	glm::vec3 posAnchor;
	glm::vec3 posEye;

	PosCplookAnchor(pcplook, &posAnchor);
	PosCplookEye(pcplook, &posEye);

	CM* pcm = pcplook->pcm;

	StopSound(pcplook->pambBinoc, 0);
	StopSound(pcplook->pambAmbient, 500);

	SetCmLookAt(pcm, &posEye, &posAnchor);

	if (!FFloatsNear(pcm->radFOV, pcplook->lmFOV.gMax, 0.0001f))
		SetCmFov(pcm, pcplook->lmFOV.gMax);

	pcplook->ppntAnchor = nullptr;

	if (pcm->sNearClip == 49.0f)
		SetCmNearClip(pcm, pcplook->sNearClipRestore);
}

float DZ_CplookAnchor = 70.0;
glm::vec3 s_dposLookFar = {-150.0 , 0.0, 0.0};
glm::vec3 s_dposLookNear = {-25, 0.0, 0.0};
float RAD_CplookTiltDefault = 0.0;
CLQ s_clqUZoomToRsvu = {2.0, 0.0, -1.0};
SMP s_smpSvu = {0.5, 0.0, 0.1};
SMPA s_smpaSniperStretch = {0.5, 0.0, 0.4, 1.0};
SMPA s_smpaSniperZoom = {0.3, 0.0, 2.0, 0.5};
SMP s_smpSwCplook = {5.0, 0.0, 0.5};
SMP s_smpDialogZoom = {1.0, 0.4, 1.5};
float DT_CplookSniperTrack = 24.0;
