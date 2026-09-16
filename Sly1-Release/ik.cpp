#include "ik.h"

IKH* NewIkh()
{
	return new IKH{};
}

int GetIkhSize()
{
	return sizeof(IKH);
}

void CloneIkh(IKH* pikh, IKH* pikhBase)
{
	CloneSo(pikh, pikhBase);
}

void SetIkhTarget(IKH* pikh, ALO* ploTarget, const glm::vec3* pvTarget)
{
	if (ploTarget == nullptr || ploTarget == (ALO*)PpoCur()) 
	{
		pikh->fTarget = 0;
		pikh->ploTarget = nullptr;
		pikh->vTarget = g_vecZero;
		return;
	}

	pikh->ploTarget = ploTarget;
	pikh->fTarget = 1;
	pikh->vTarget = *pvTarget;
}

void RenderIkhSelf(IKH* pikh, CM* pcm, RO* pro)
{
	RenderSoSelf(pikh, pcm, pro);
}

void DeleteIkh(IKH *pikh)
{
	delete pikh;
}

LIKH* NewLikh()
{
	return new LIKH{};
}

int GetLikhSize()
{
	return sizeof(LIKH);
}

void CloneLikh(LIKH* plikh, LIKH* plikhBase)
{
	CloneAlo(plikh, plikhBase);
}

void RenderLikhSelf(LIKH* plikh, CM* pcm, RO* pro)
{
	RenderAloSelf(plikh, pcm, pro);
}

void DeleteLikh(LIKH *plikh)
{
	delete plikh;
}

void SolveAloIK(ALO* palo)
{
	if (palo == nullptr || palo->palox == nullptr)
		return;

	IKHALOX& ikh = palo->palox->ikh;

	ALO* paloShoulder = ikh.paloShoulder;
	ALO* paloElbow = ikh.paloElbow;
	ALO* paloCommon = ikh.paloCommon;

	if (paloShoulder == nullptr || paloElbow == nullptr || paloCommon == nullptr)
		return;

	const uint32_t grfik = static_cast<uint32_t>(ikh.grfik);

	auto NormalizeSafe = [](const glm::vec3& v, const glm::vec3& fallback)
	{
		const float length = glm::length(v);
		return length < 0.0001f ? fallback : v / length;
	};

	// Prevent the IK target from extending beyond the total limb length.
	if ((grfik & 2) != 0)
	{
		const float sUpper = glm::length(paloElbow->xf.pos);
		const float sLower = glm::length(ikh.posWrist);
		const float sReachMax = sUpper + sLower;

		glm::vec3 dpos = palo->xf.posWorld - paloShoulder->xf.posWorld;
		const float sTarget = glm::length(dpos);

		if (sTarget > sReachMax && sTarget >= 0.0001f)
		{
			glm::vec3 posIkh = paloShoulder->xf.posWorld + dpos * (sReachMax / sTarget);

			SetAloVelocityVec(palo, &g_vecZero);
			ConvertAloPos(nullptr, palo->paloParent, &posIkh, &posIkh);

			if (palo->pvtalo->pfnTranslateAloToPos != nullptr)
				palo->pvtalo->pfnTranslateAloToPos(palo, &posIkh);
		}
	}

	// Restore the shoulder's base local rotation.
	paloShoulder->xf.mat = paloShoulder->palox->matPostRotation * paloShoulder->palox->matPreRotation;

	// Restore the elbow's base local rotation.
	if ((paloElbow->palox->grfalox & 3) != 0)
		paloElbow->xf.mat = paloElbow->palox->matPostRotation * paloElbow->palox->matPreRotation;
	else
		paloElbow->xf.mat = glm::mat3(1.0f);

	if (paloShoulder->pvtlo->pfnUpdateLoXfWorld != nullptr)
		paloShoulder->pvtlo->pfnUpdateLoXfWorld(paloShoulder);

	glm::vec3 posIkh = ikh.posIkh;
	glm::vec3 vIkh = g_vecZero;

	CalculateAloTransformAdjust(palo, paloCommon, &posIkh, nullptr, &vIkh, nullptr);

	glm::vec3 aposOrig[4]{};
	glm::mat3 amatOrig[4]{ glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f), glm::mat3(1.0f) };
	glm::vec3 avOrig[4]{};
	glm::vec3 awOrig[4]{};

	CalculateAloTransform(paloElbow, paloCommon, 3, &aposOrig[1], &amatOrig[1], &avOrig[1], &awOrig[1]);

	const glm::vec3 posWrist = aposOrig[1] + amatOrig[1] * ikh.posWrist;

	glm::vec3 dposES = aposOrig[1] - aposOrig[2];
	glm::vec3 dposWE = posWrist - aposOrig[1];
	glm::vec3 dposIS = posIkh - aposOrig[2];

	const float sUpper = glm::length(dposES);
	const float sLower = glm::length(dposWE);
	const float sTarget = glm::length(dposIS);

	if (sUpper < 0.0001f || sLower < 0.0001f || sTarget < 0.0001f)
	{
		paloElbow->palox->ikj.fInvalid = 0;
		paloShoulder->palox->ikj.fInvalid = 0;
		return;
	}

	glm::vec3 normalElbow;
	const float radElbowCurrent = RadBetweenVectors(dposES, dposWE, &normalElbow);

	normalElbow = NormalizeSafe(normalElbow, g_normalX);

	// Law of cosines. This angle is the required angle between the two
	// segments using the engine's elbow-vector orientation.
	float cosElbow = ((sTarget * sTarget - sUpper * sUpper) - sLower * sLower) / (2.0f * sUpper * sLower);
	cosElbow = glm::clamp(cosElbow, -1.0f, 1.0f);

	const float radElbowTarget = std::acos(cosElbow);

	glm::mat3 matElbow;
	LoadRotateMatrixRad(radElbowTarget - radElbowCurrent, &normalElbow, &matElbow);

	dposWE = matElbow * dposWE;

	glm::vec3 dposWS = dposES + dposWE;

	glm::mat3 matShoulder;
	LoadRotateVectorMatrix(&dposWS, &dposIS, &matShoulder);

	const glm::vec3 normalIS = NormalizeSafe(dposIS, g_normalX);

	glm::mat3 matTwist;
	LoadRotateMatrixRad(ikh.radTwist, &normalIS, &matTwist);

	// Twist is applied after aligning the complete limb with the target.
	matShoulder = matTwist * matShoulder;

	const glm::mat3 matElbowCommon = matElbow * amatOrig[1];
	const glm::mat3 matShoulderCommon = matShoulder * amatOrig[2];

	// Match MatMulMatTransMat from the original game:
	// local = transpose(original parent/common space) * solved common space.
	paloElbow->xf.mat = glm::transpose(amatOrig[2]) * matElbowCommon;
	paloShoulder->xf.mat = glm::transpose(amatOrig[3]) * matShoulderCommon;

	if (paloShoulder->pvtlo->pfnUpdateLoXfWorld != nullptr)
		paloShoulder->pvtlo->pfnUpdateLoXfWorld(paloShoulder);

	if ((grfik & 1) == 0)
	{
		paloElbow->xf.w = g_vecZero;
		paloShoulder->xf.w = g_vecZero;
	}
	else
	{
		const glm::vec3 vIkhRelative = vIkh - avOrig[2];

		float dradElbow = 0.0f;
		const float sinElbowSq = 1.0f - cosElbow * cosElbow;

		if (sinElbowSq > 0.0001f)
		{
			dradElbow =
				(-1.0f / std::sqrt(sinElbowSq)) *
				(sTarget / (sLower * sUpper)) *
				glm::dot(vIkhRelative, normalIS);
		}

		const glm::vec3 wElbowRot = matShoulder * (normalElbow * dradElbow);
		const glm::vec3 dposWENew = matShoulder * dposWE;
		const glm::vec3 vElbowCommon = glm::cross(wElbowRot, dposWENew);
		const glm::vec3 vShoulderCommon = vIkhRelative - vElbowCommon;

		const glm::vec3 crossShoulder = glm::cross(dposIS, vShoulderCommon);
		const glm::vec3 normalShoulder = NormalizeSafe(crossShoulder, g_normalX);

		glm::vec3 wShoulderCommon = normalShoulder * (glm::length(vShoulderCommon) / sTarget);
		wShoulderCommon += normalIS * ikh.dradTwist;

		// Convert common-space angular velocities into each joint's
		// original parent coordinate system.
		paloElbow->xf.w = glm::transpose(amatOrig[2]) * wElbowRot;
		paloShoulder->xf.w = glm::transpose(amatOrig[3]) * (wShoulderCommon - awOrig[3]);
	}

	paloElbow->palox->ikj.fInvalid = 0;
	paloShoulder->palox->ikj.fInvalid = 0;
}
