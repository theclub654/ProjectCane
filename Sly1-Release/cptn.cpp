#include "cptn.h"
#include "cm.h"
#include "tn.h"
#include "jt.h"
#include "wm.h"
#include "mark.h"

void ActivateCptn(CPTN* pcptn, TN *ptn)
{
    CM* pcm = pcptn->pcm;

    pcptn->fActivate = 1;
    pcptn->ptn = ptn;

    SO* psoFocus = pcm->acpr[0].psoFocus;
    TNFN* ptnfn = PtnfnFromTn(ptn);

    CPDEFI cpdefi{};
    GetCmCpdefi(pcm, psoFocus, 0.0, &cpdefi);

    glm::vec3 posTarget(0.0f);
    GetTnfnNose(ptnfn, &cpdefi, &posTarget, nullptr);

    if (ptn != nullptr)
    {
        if (ptn->pcrv != nullptr)
        {
            glm::vec3 posCurveLocal(0.0f);
            ConvertAloPos(nullptr, ptn, &posTarget, &posCurveLocal);

            if (ptn->pcrv->pvtcrv->pfnFindCrvClosestPointAll != nullptr)
                ptn->pcrv->pvtcrv->pfnFindCrvClosestPointAll(ptn->pcrv.get(), &posCurveLocal, nullptr, nullptr, nullptr, nullptr, &ptn->sClosest);
        }
        else if (ptn->fFakeCylinder != 0)
            UpdateCptnClosestPoint(pcptn, &cpdefi);

        GetTnfnNose(ptnfn, &cpdefi, &posTarget, ptn);
    }

    const glm::vec3 posFocus = posTarget + pcm->dposFocus;

    DecomposeCylind(&pcm->pos, &posFocus, &pcptn->radManual, &pcptn->xyManual, &pcptn->zManual);

    const float distance = std::sqrt(pcptn->xyManual * pcptn->xyManual + pcptn->zManual * pcptn->zManual);
    const float nearClipScale = distance > 0.0001f
        ? (distance - pcm->sNearClip) / distance
        : 0.0f;

    pcptn->xyManual *= nearClipScale;
    pcptn->zManual *= nearClipScale;

    glm::vec3 vEyeOffset(0.0f);
    SetVectorCylind(&vEyeOffset, pcptn->radManual, pcptn->xyManual, pcptn->zManual);

    pcptn->ftnd = static_cast<FTND>(~FTND_Nil);
    pcptn->posEyePrev = posFocus + vEyeOffset;

    if (ptn != nullptr && ptn->fCutOnActivate != 0)
        SetCmCut(pcm, nullptr);

    pcptn->tActivate = g_clock.t;
    pcptn->tMoveLast = g_clock.t;
    pcptn->fPanOnEntryDone = 0;
    pcptn->tLastOrbit = g_clock.t - 10.0f;

    if (ptn != nullptr && ptn->radFOVOverride != 0.0f)
    {
        ptn->radFOVPrev = pcm->radFOVTarget;
        pcm->radFOVTarget = ptn->radFOVOverride;
    }

    AdaptCm(pcm, &posFocus, ptnfn->xScreenPref, ptnfn->yScreenPref);
}

void DeativateCptn(CPTN* pcptn)
{
    TN* ptn = pcptn->ptn;

    if (ptn != nullptr && ptn->radFOVOverride != 0.0f)
        g_pcm->radFOVTarget = ptn->radFOVPrev;

    pcptn->ptn = nullptr;
}

void UpdateCptn(CPTN* pcptn, CPDEFI* pcpdefi, JOY* pjoy, float dt)
{
	CM* pcm = pcptn->pcm;
	TN* ptn = pcptn->ptn;
	JT* psoFocus = reinterpret_cast<JT*>(pcm->acpr[0].psoFocus);

	const int fPanOnEntry = (ptn != nullptr) ? ptn->fPanOnEntry : 0;
	const int fPanOnIdle = (ptn != nullptr) ? ptn->fPanOnIdle : 0;
	const FTND grftnd = (ptn != nullptr) ? (FTND)ptn->grftnd : FTND_Manual;

	const float dtInput = (g_wmc.fActive != 0) ? g_clock.dt : g_clock.dtReal;

	UpdateCptnClosestPoint(pcptn, pcpdefi);

	TNFN* ptnfn = PtnfnFromTn(ptn);

	glm::vec3 posNose(0.0f);
	GetTnfnNose(ptnfn, pcpdefi, &posNose, ptn);

	if (psoFocus == g_pjt && psoFocus->jtfock == JTFOCK_Focus)
	{
		const glm::vec3 dposFocusTarget = psoFocus->posFocus - posNose;
		SpringCm(g_pcm, &g_pcm->dposFocus, &g_pcm->vFocus, &dposFocusTarget, 8.0f, -6.0f, dt);
	}

	glm::vec3 posFocus = posNose + g_pcm->dposFocus;

	float uFollowTarget = 0.0f;

	if ((pcpdefi->grfcpd & 1) == 0)
		uFollowTarget = ptnfn->uFollow;

	pcptn->uFollowCur = GSmooth(pcptn->uFollowCur, uFollowTarget, dt, &s_smpUFollow, nullptr);
	pcptn->tMoveLast = std::max(pcptn->tMoveLast, pcpdefi->tMoveLast);

	glm::vec3 dposBaseCrv(0.0f);
	glm::vec3 vecBase(0.0f);

	float radFwd = 0.0f;
	float xyFwd = 0.0f;
	float zFwd = 0.0f;

	float radRev = 0.0f;
	float xyRev = 0.0f;
	float zRev = 0.0f;

	if (ptn != nullptr && (grftnd & (FTND_Forward | FTND_Reverse)) != 0)
	{
		CalculateTnCrv(ptn, &posNose, &dposBaseCrv, &vecBase);

		if (ptn->fCylinder != 0)
		{
			float radBase = 0.0f;
			float xyBase = 0.0f;
			float zBase = 0.0f;

			DecomposeCylind(&dposBaseCrv, &g_vecZero, &radBase, &xyBase, &zBase);

			if (pcptn->fActivate == 0)
			{
				const float drad = RadNormalize(pcptn->radCur - radBase);
				radBase += GLimitAbs(drad, ptn->dradSlack);
			}

			// Retail leaves CalculateTnCrv's result decomposed here.  The TN
			// camera CLQs consume (angle, radius, height), and CalculateTnPos
			// converts their evaluated result back to Cartesian.  Reconstructing
			// Cartesian coordinates at this point makes the CLQs treat a world-
			// scale X/Y offset as a camera radius (for example 450 -> ~3200).
			dposBaseCrv = glm::vec3(radBase, xyBase, zBase);
			pcptn->radCur = radBase;
		}

		if ((grftnd & FTND_Forward) != 0)
		{
			glm::vec3 posCamFwd(0.0f);
			CalculateTnPos(ptn, &dposBaseCrv, ptn->sClosest, ptn->tnfn.dsCam, ptn->tnfn.aclqCam, ptn->tnfn.almCam, FTND_Forward, &posCamFwd);
			DecomposeCylind(&posCamFwd, &posFocus, &radFwd, &xyFwd, &zFwd);
		}

		if ((grftnd & FTND_Reverse) != 0)
		{
			glm::vec3 posCamRev(0.0f);
			CalculateTnPos(ptn, &dposBaseCrv, ptn->sClosest, ptn->tnfn.dsCam, ptn->tnfn.aclqCam, ptn->tnfn.almCam, FTND_Reverse, &posCamRev);
			DecomposeCylind(&posCamRev, &posFocus, &radRev, &xyRev, &zRev);
		}
	}

	if (pcptn->fActivate != 0)
		pcptn->ftnd = FTND_Manual;

	if (pcm->fCut != 0)
	{
		if (pcm->fRadCut != 0)
		{
			pcptn->ftnd = FTND_Manual;
			pcptn->radManual = pcm->radCut;
		}
		else if ((grftnd & (FTND_Forward | FTND_Reverse)) == 0)
		{
			pcptn->ftnd = FTND_Manual;
			pcptn->radManual = pcpdefi->radHome;
		}
		else if ((grftnd & FTND_Reverse) == 0)
			pcptn->ftnd = FTND_Forward;
		else if ((grftnd & FTND_Forward) == 0)
			pcptn->ftnd = FTND_Reverse;
		else
		{
			const float dradFwd = RadNormalize(radFwd - pcpdefi->radHome);
			const float dradRev = RadNormalize(radRev - pcpdefi->radHome);

			pcptn->ftnd = (std::abs(dradFwd) < std::abs(dradRev)) ? FTND_Forward : FTND_Reverse;
		}

		pcptn->tMoveLast = g_clock.t;
		pcptn->tLastOrbit = g_clock.t - 10.0f;
	}

	if (pcptn->ftnd > FTND_Nil && pcptn->ftnd < FTND_Manual && (pcptn->ftnd & grftnd) == 0)
		pcptn->ftnd = FTND_Manual;

	glm::vec3 vFollow = pcpdefi->vBase;

	if (ptn != nullptr && ptn->paloTarget != nullptr)
	{
		const glm::vec3 dposTarget = ptn->paloTarget->xf.posWorld - pcpdefi->posBase;
		const float distance = glm::length(dposTarget);

		if (distance < 0.0001f)
		{
			vFollow = glm::vec3(0.0f);
		}
		else
		{
			float scale = ptn->clqFollowTarget.g0 + distance * (ptn->clqFollowTarget.g1 + distance * ptn->clqFollowTarget.g2);
			scale = std::clamp(scale, ptn->lmFollowTarget.gMin, ptn->lmFollowTarget.gMax);

			vFollow = (dposTarget / distance) * scale;
		}
	}

	const float radManualPrev = pcptn->radManual;
	float radManual = radManualPrev;

	if (pjoy->IsPressed(BTN_R3) && (grftnd & FTND_Manual) != 0)
	{
		StartSound((SFXID)3, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

		pcptn->fHome = 1;
		pcptn->radHome = RadNormalize(pcpdefi->radHome);
		pcptn->fPanOnEntryDone = 1;
		pcptn->tLastOrbit = g_clock.t;
	}
	else if ((grftnd & FTND_Manual) != 0 && (pjoy->uDeflect2 > 0.0f || pcptn->swOrbit != 0.0f || pcptn->fHome != 0))
	{
		if (pcptn->ftnd != FTND_Manual)
			StartSound((SFXID)3, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

		if (pjoy->uDeflect2 > 0.0f)
			pcptn->fHome = 0;

		if (pcptn->fHome == 0)
		{
			const float swTarget = pjoy->x2 * std::abs(pjoy->x2) * SW_CptnOrbitMax;
			const float swMin = pcptn->swOrbit - DSW_CptnOrbit * dtInput;
			const float swMax = pcptn->swOrbit + DSW_CptnOrbit * dtInput;

			pcptn->swOrbit = std::clamp(swTarget, swMin, swMax);
			radManual = RadNormalize(pcptn->radManual + pcptn->swOrbit * dtInput);
		}
		else
		{
			radManual = RadSmoothA(pcptn->radManual, pcptn->swOrbit, pcptn->radHome, dtInput, &s_smpaHome, &pcptn->swOrbit);

			if (radManual == pcptn->radHome)
				pcptn->fHome = 0;
		}

		pcptn->ftnd = FTND_Manual;
		pcptn->fPanOnEntryDone = 1;
		pcptn->tLastOrbit = g_clock.t;
	}

	if (pcptn->ftnd == FTND_Manual &&
		!(pjoy->IsPressed(BTN_R3) && (grftnd & FTND_Manual) != 0) &&
		!((grftnd & FTND_Manual) != 0 &&
		  (pjoy->uDeflect2 > 0.0f || pcptn->swOrbit != 0.0f || pcptn->fHome != 0)))
	{
		glm::vec2 joy(pjoy->x, pjoy->y);
		const float joyLength = glm::length(joy);
		joy = (joyLength < 0.0001f) ? glm::vec2(1.0f, 0.0f) : joy / joyLength;

		glm::vec3 vCamera = posFocus - pcm->pos;
		const float cameraDistance = glm::length(vCamera);
		vCamera = (cameraDistance < 0.0001f) ? g_normalX : vCamera / cameraDistance;

		CFK cfk = (ptn != nullptr && ptn->cfk != CFK_Nil) ? ptn->cfk : pcpdefi->cfk;
		glm::vec3 vMeasure(0.0f);

		if (cfk == CFK_Behind)
			vMeasure = glm::cross(g_normalZ, vCamera);
		else if (cfk == CFK_Side)
			vMeasure = vCamera;

		float swFollow = 0.0f;
		if (cameraDistance >= 0.0001f)
			swFollow = GLimitAbs(glm::dot(vFollow, vMeasure) / cameraDistance, 2.5f);

		// Retail s_clqYJoyToUSpin and its [0, 1] output limit.
		const float uSpin = std::clamp(1.0f + joy.y * (-1.0f + joy.y * 0.0f), 0.0f, 1.0f);
		const float dtOrbit = g_clock.t - pcptn->tLastOrbit;
		const float uOrbitFollow = std::clamp(dtOrbit * 0.1f, 0.0f, 1.0f);

		radManual = RadNormalize(radManual + swFollow * pcptn->uFollowCur * uSpin * uOrbitFollow * dt);

		if ((grftnd & (FTND_Forward | FTND_Reverse)) != 0 && ptn != nullptr)
		{
			float radAuto = radFwd;
			float uDirection = 1.0f;

			if (ptn->pcrv == nullptr && ptn->fFakeCylinder == 0)
			{
				uDirection = 1.0f;
			}
			else if (psoFocus == g_pjt && ptn == psoFocus->ptnHideDynamic)
			{
				uDirection = 3.0f;
			}
			else if (pcptn->ftnd == FTND_Reverse)
			{
				radAuto = radRev;
				uDirection = 1.0f;
			}
			else if (pcptn->ftnd != FTND_Forward)
			{
				glm::vec3 vFacing;
				if (psoFocus == g_pjt && psoFocus->jts == JTS_Sidestep)
					vFacing = psoFocus->xf.v;
				else
					vFacing = psoFocus->xf.matWorld[0];

				vFacing.z = 0.0f;
				if (glm::length(vFacing) < 0.0001f)
					vFacing = g_normalX;
				else
					vFacing = glm::normalize(vFacing);

				glm::vec3 vCurveWorld(0.0f);
				ConvertAloVec(ptn, nullptr, &vecBase, &vCurveWorld);
				const float dot = glm::dot(vFacing, vCurveWorld);
				uDirection = dot * dot * (3.0f - std::abs(dot));
				radAuto = ((grftnd & FTND_Reverse) != 0 && dot <= 0.0f) ? radRev : radFwd;
			}

			float uLock = 0.0f;
			if (g_pjt != nullptr)
			{
				const float speed = glm::length(vFollow);
				uLock = std::min(1.0f, speed *
					((g_pjt->jts == JTS_Hide && g_pjt->jthk == JTHK_Nil) ? 0.005f : 0.0016666667f));
			}
			uLock *= ptnfn->uLockOn;

			if (fPanOnIdle != 0)
			{
				float uIdle = 0.0f;
				const float dtMove = g_clock.t - (pcptn->tMoveLast + 0.75f);
				const float dtIdleOrbit = g_clock.t - (pcptn->tLastOrbit + 10.0f);

				if (ptn->dtPanOnIdle > 0.0f)
					uIdle = std::clamp(dtMove / ptn->dtPanOnIdle, 0.0f, 1.0f) *
						std::clamp(dtIdleOrbit / ptn->dtPanOnIdle, 0.0f, 1.0f);
				else if (dtMove > 0.0f && dtIdleOrbit > 0.0f)
					uIdle = 1.0f;

				if (uLock * uDirection < uIdle * ptn->rswPanOnIdle)
				{
					uDirection = 1.0f;
					uLock = uIdle * ptn->rswPanOnIdle;
				}
			}

			if (fPanOnEntry != 0 && pcptn->fPanOnEntryDone == 0)
			{
				const float uEntry = (ptn->dtPanOnEntry > 0.0f)
					? std::clamp((g_clock.t - ptn->tTns) / ptn->dtPanOnEntry, 0.0f, 1.0f)
					: 1.0f;
				const float entryLock = uEntry * ptn->rswPanOnEntry;

				if (uLock * uDirection < entryLock)
				{
					uDirection = 1.0f;
					uLock = entryLock;
				}
			}

			SMP smpAuto = {4.0f * uLock, s_sffRun.au[0] * uLock, 0.5f};

			// Retail smooths from the angle at the start of the frame, then picks
			// whichever result (follow rotation or auto rotation) is closer to the
			// authored TN direction.  Using the already-followed angle as the base
			// applies the follow delta twice and can turn a correction into a
			// self-sustaining orbit.
			const float radAutoTarget = RadNormalize(
				radManualPrev + RadNormalize(radAuto - radManualPrev));
			float radSmoothed = RadSmooth(
				radManualPrev, radAutoTarget, dt, &smpAuto, nullptr);

			if (std::abs(RadNormalize(radAuto - radManual)) <
				std::abs(RadNormalize(radAuto - radSmoothed)))
			{
				radSmoothed = radManual;
			}

			const float dradFollow = RadNormalize(radManual - radManualPrev);
			const float dradSmooth = RadNormalize(radSmoothed - radManualPrev);

			radManual = RadNormalize(radManualPrev +
				(1.0f - uDirection) * dradFollow +
				uDirection * dradSmooth);

			if (pcptn->fActivate == 0)
			{
				if ((grftnd & FTND_Forward) != 0)
				{
					const float dradPrev = RadNormalize(pcptn->radManual - pcptn->radFwdPrev);
					const float dradNow = RadNormalize(radManual - radFwd);
					if ((dradPrev * dradNow <= 0.0f || std::abs(dradNow) < 0.0001f) && std::abs(dradNow) < glm::half_pi<float>())
					{
						pcptn->fPanOnEntryDone = 1;
						pcptn->ftnd = FTND_Forward;
					}
				}

				if (pcptn->ftnd == FTND_Manual && (grftnd & FTND_Reverse) != 0)
				{
					const float dradPrev = RadNormalize(pcptn->radManual - pcptn->radRevPrev);
					const float dradNow = RadNormalize(radManual - radRev);
					if ((dradPrev * dradNow <= 0.0f || std::abs(dradNow) < 0.0001f) && std::abs(dradNow) < glm::half_pi<float>())
					{
						pcptn->fPanOnEntryDone = 1;
						pcptn->ftnd = FTND_Reverse;
					}
				}
			}
		}
	}

	float rad = radManual;
	float xy = 0.0f;
	float z = 0.0f;
	float xScreen = ptnfn->xScreenPref;
	float yScreen = ptnfn->yScreenPref;

	switch (pcptn->ftnd)
	{
		case FTND_Forward:
		rad = radFwd;
		xy = xyFwd;
		z = zFwd;
		break;

		case FTND_Reverse:
		rad = radRev;
		xy = xyRev;
		z = zRev;
		xScreen = -ptnfn->xScreenPref;
		break;

		case FTND_Manual:
		rad = radManual;

		if ((grftnd & (FTND_Forward | FTND_Reverse)) == 0)
		{
			xy = std::abs(ptnfn->dsCam);
			z = ptnfn->aclqCam[2].g0;
		}
		else
		{
			float uFwd;

			if ((grftnd & FTND_Reverse) == 0)
			{
				uFwd = 1.0f;
			}
			else if ((grftnd & FTND_Forward) == 0)
			{
				uFwd = 0.0f;
			}
			else
			{
				float radFwdAdjusted = radFwd;
				float radRevAdjusted = radRev;

				if (radManual < radFwdAdjusted)
				{
					if (radRevAdjusted > radManual)
					{
						if (radRevAdjusted < radFwdAdjusted)
							radFwdAdjusted -= glm::two_pi<float>();
						else
							radRevAdjusted -= glm::two_pi<float>();
					}
				}
				else if (radManual > radFwdAdjusted && radManual > radRevAdjusted)
				{
					if (radRevAdjusted <= radFwdAdjusted)
						radRevAdjusted += glm::two_pi<float>();
					else
						radFwdAdjusted += glm::two_pi<float>();
				}

				const float dradRange = radFwdAdjusted - radRevAdjusted;

				// At a TN seam (or on a symmetric tunnel) the authored forward and
				// reverse eye positions can have the same azimuth. Dividing by that
				// zero-width range poisons the camera state with NaNs.
				if (std::abs(dradRange) < 0.0001f)
				{
					const float dradToFwd = std::abs(RadNormalize(radManual - radFwd));
					const float dradToRev = std::abs(RadNormalize(radManual - radRev));
					uFwd = (dradToFwd <= dradToRev) ? 1.0f : 0.0f;
				}
				else
				{
					uFwd = (radManual - radRevAdjusted) / dradRange;
					uFwd = uFwd * uFwd * (3.0f - 2.0f * uFwd);
				}
			}

			const float uRev = 1.0f - uFwd;

			xy = uFwd * xyFwd + uRev * xyRev;
			z = uFwd * zFwd + uRev * zRev;
			xScreen = uFwd * ptnfn->xScreenPref - uRev * ptnfn->xScreenPref;
		}
		break;

		default:
		break;
	}

	if (pcm->fCut == 0)
	{
		pcptn->vxy = (1.0f + ptnfn->springDamping * DAMP_CptnPosition * dt) * pcptn->vxy + (xy - pcptn->xyManual) * ptnfn->springStrength * SPRING_CptnPosition * dt;
		xy = pcptn->xyManual + pcptn->vxy * dt;

		pcptn->vz = (1.0f + ptnfn->springDamping * DAMP_CptnPosition * dt) * pcptn->vz + (z - pcptn->zManual) * ptnfn->springStrength * SPRING_CptnPosition * dt;
		z = pcptn->zManual + pcptn->vz * dt;
	}
	else
	{
		pcptn->vxy = 0.0f;
		pcptn->vz = 0.0f;
	}

	glm::vec3 vecEye(0.0f);
	SetVectorCylind(&vecEye, rad, xy, z);

	glm::vec3 posEye = posFocus + vecEye;

	if (pcm->fCut == 0 && g_fDisableSquish == 0 &&
		(ptn == nullptr || ptn->fNoSquish == 0))
	{
		ClipCmEye(pcm, &pcptn->posEyePrev, &posEye, &posEye);
	}

	DecomposeCylind(&posEye, &posFocus, &rad, &xy, &z);

	pcptn->posEyePrev = posEye;

	SetVectorCylind(&vecEye, rad, xy, z);

	const float eyeDistance = glm::length(vecEye);

	if (eyeDistance > 0.0001f)
		posEye = posFocus + vecEye * ((eyeDistance + pcm->sNearClip) / eyeDistance);

	SetCm(pcm, &posFocus, &posEye, xScreen, yScreen);

	pcptn->radManual = rad;
	pcptn->xyManual = xy;
	pcptn->zManual = z;
	pcptn->radFwdPrev = radFwd;
	pcptn->radRevPrev = radRev;

	UpdateCmFade(pcm);

	pcptn->fActivate = 0;
}

void UpdateCptnClosestPoint(CPTN* pcptn, CPDEFI* pcpdefi)
{
    TN* ptn = pcptn->ptn;

    if (ptn == nullptr)
        return;

    if (ptn->pcrv == nullptr && ptn->fFakeCylinder == 0)
        return;

    TNFN* ptnfn = PtnfnFromTn(ptn);

    glm::vec3 posNoseWorld(0.0f);
    glm::vec3 posNoseLocal(0.0f);

    GetTnfnNose(ptnfn, pcpdefi, &posNoseWorld, nullptr);
    ConvertAloPos(nullptr, ptn, &posNoseWorld, &posNoseLocal);

    if (ptn->pcrv == nullptr)
    {
        const glm::vec3 dpos = posNoseLocal - ptn->posFake;

        float rad = std::atan2(dpos.y, dpos.x);

        if (rad < 0.0f)
            rad += glm::two_pi<float>();
		
        ptn->sClosest = rad * ptn->sRadFake;
        return;
    }

    if (ptn->pcrv->pvtcrv->pfnFindCrvClosestPointFromS != nullptr)
        ptn->pcrv->pvtcrv->pfnFindCrvClosestPointFromS(ptn->pcrv.get(), &posNoseLocal, ptn->sClosest, nullptr, nullptr, nullptr, nullptr, &ptn->sClosest);
}

void UpdateCptnSquishCut(CPTN* pcptn)
{
	CM* pcm = pcptn->pcm;
	SO* psoFocus = pcm->acpr[0].psoFocus;

	if ((g_grfusr & 2) != 0)
	{
		pcptn->tActivate = g_clock.t;
		return;
	}

	if (pcptn->ptn != nullptr && pcptn->ptn->fNoSquish != 0)
		return;

	if (g_fDisableSquish != 0)
		return;

	if (g_clock.t - pcptn->tActivate < DT_CptnSquishCheckDelay)
		return;

	const glm::vec3 up = g_normalZ;
	glm::vec3 right = glm::cross(up, psoFocus->xf.posWorld - pcm->pos);

	if (glm::length(right) < 0.0001f)
		right = g_normalX;
	else
		right = glm::normalize(right);

	const glm::vec3 forward = glm::cross(right, up);

	glm::vec3 aposProbe[5];

	glm::vec3 posMin = pcm->pos;
	glm::vec3 posMax = pcm->pos;

	for (int i = 0; i < 5; ++i)
	{
		const glm::vec3& offset = s_avecCptnSquishProbe[i];

		aposProbe[i] =
			psoFocus->xf.posWorld +
			forward * offset.x +
			right * offset.y +
			up * offset.z;

		posMin = glm::min(posMin, aposProbe[i]);
		posMax = glm::max(posMax, aposProbe[i]);
	}

	std::vector<SO*> apso;

	IntersectSwBoundingBox(g_psw, nullptr, &posMin, &posMax, (PFNFILTER)FFilterCamera, nullptr, apso);

	for (int i = 0; i < 5; ++i)
	{
		glm::vec3 posNearScreen(0.0f);
		glm::vec3 posNearWorld(0.0f);

		ConvertCmWorldToScreen(pcm, &aposProbe[i], &posNearScreen);

		posNearScreen.z = pcm->sNearClip;

		ConvertCmScreenToWorld(pcm, &posNearScreen, &posNearWorld);

		float sClearStart = 0.0f;
		float sClearEnd = 0.0f;

		const int fClear = FFindCmClearInterval(0.0f, static_cast<int>(apso.size()), apso.data(), &aposProbe[i], &posNearWorld, &sClearStart, &sClearEnd);

		if (fClear != 0 && sClearEnd >= 1.0f)
		{
			pcptn->tActivate = g_clock.t;
			break;
		}
	}

	if (g_clock.t - pcptn->tActivate > DT_CptnSquishCutDelay)
		SetCmCutNext(pcm);
}

float FtndCptnSquishConstraint(void* pvContext, float sRadius, const glm::vec3* pposEye, const SBI* psbi)
{
	(void)pvContext;
	(void)sRadius;
	(void)pposEye;

	// Ghidra labels this overlapping PS2 offset as CPTN::ftnd, but the
	// callback's fourth argument is an SBI and the value is SBI::gDist.
	return psbi->gDist;
}

void FindCptnClearEyePosition(CPTN* pcptn, const glm::vec3* pposCenter, glm::vec3* pposEye, glm::vec3* pposEyeClear)
{
	CM* pcm = pcptn->pcm;
	SO* psoFocus = pcm->acpr[0].psoFocus;

	std::vector<SO*> apso;

	const glm::vec3 posMin = glm::min(glm::min(*pposCenter, *pposEye), psoFocus->xf.posWorld);
	const glm::vec3 posMax = glm::max(glm::max(*pposCenter, *pposEye), psoFocus->xf.posWorld);

	IntersectSwBoundingBox(g_psw, nullptr, &posMin, &posMax, (PFNFILTER)FFilterCamera, nullptr, apso);

	glm::vec3 posClosestClear;
	FindCmClosestClearPos(500.0f, static_cast<int>(apso.size()), apso.data(), &psoFocus->xf.posWorld, &posClosestClear);

	float sClearStart = 0.0f;
	float sClearEnd = 0.0f;

	FFindCmClearInterval(0.0f, static_cast<int>(apso.size()), apso.data(), &posClosestClear, pposEye, &sClearStart, &sClearEnd);

	glm::vec3 posClear;

	if (sClearEnd < 1.0f)
	{
		pcptn->ftnd = FTND_Manual;

		float radEye = 0.0f;
		float sXY = 0.0f;
		float sZ = 0.0f;

		DecomposeCylind(pposEye, pposCenter, &radEye, &sXY, &sZ);

		float drad = 0.0f;
		float sPrevious = 0.0f;
		float bestScore = FLT_MAX;

		do
		{
			glm::vec3 vecCandidate;
			SetVectorCylind(&vecCandidate, radEye + drad, sXY, sZ);

			glm::vec3 posCandidate = *pposCenter + vecCandidate;

			FFindCmClearInterval(sPrevious, static_cast<int>(apso.size()), apso.data(), &posClosestClear, &posCandidate, &sClearStart, &sClearEnd);

			const float score = (1.25f - sClearEnd) * (std::abs(drad) + 0.47123894f);

			if (score < bestScore)
			{
				posClear = glm::mix(posClosestClear, posCandidate, sClearEnd);
				bestScore = score;
			}

			const bool fAdvance = drad <= sPrevious;

			sPrevious = drad;
			drad = -drad;

			if (fAdvance)
				drad += 0.28559935f;
		} while ((std::abs(drad) + 0.47123894f) * 0.25f <= bestScore && drad < glm::pi<float>());
	}
	else
	{
		posClear = glm::mix(posClosestClear, *pposEye, sClearEnd);
	}

	SquishCmEye(&posClear, pcm->sRadiusNearClip, pcm, static_cast<int>(apso.size()), apso.data(), FtndCptnSquishConstraint, pcm->sRadiusNearClip, &posClear, pposEyeClear);

	FFindCmClearInterval(0.0f, static_cast<int>(apso.size()), apso.data(), &posClear, pposEyeClear, &sClearStart, &sClearEnd);

	if (sClearEnd != 1.0f)
	{
		const float sMidpoint = (sClearStart + sClearEnd) * 0.5f;
		*pposEyeClear = glm::mix(posClear, *pposEyeClear, sMidpoint);
	}
}

SMP s_smpUFollow = {4.0, 0.0, 0.25};
int g_fDisableSquish = 0;
float DAMP_CptnPosition = -8.0;
SMPA s_smpaHome = {2.5, 0.0, 0.34999999, 15.0};
float DSW_CptnOrbit = 15.0;
float SW_CptnOrbitMax = 2.5;
float DT_CptnSquishCheckDelay = 0.5;
float DT_CptnSquishCutDelay = 2.0;
float SPRING_CptnPosition = 15.0;
