#include "cpaseg.h"
#include "xform.h"
#include "hubblot.h"
#include "hubsel.h"
#include "sw.h"

void ActivateCpaseg(CPASEG* pcpaseg, void* pv)
{
	pcpaseg->pcamera = (CAMERA*)pv;
}

void DeactivateCpaseg(CPASEG* pcpaseg, void* pv)
{
	pcpaseg->pcamera = nullptr;
}

void UpdateCpaseg(CPASEG* pcpaseg, CPDEFI* pcpdefi, JOY* pjoy, float dt)
{
	CAMERA* pcamera = pcpaseg->pcamera;

	if (pcamera == nullptr)
		return;

	CM* pcm = pcpaseg->pcm;

	glm::vec3 posEyeWorld{};
	glm::mat3 matCamera(1.0f);

	/*
	 * The original transforms CAMERA::posEye into world space.
	 *
	 * This position is used to calculate the look direction, but the
	 * final camera position passed to SetCmPosMat is still xf.posWorld.
	 */
	ConvertAloPos(pcamera, nullptr, &pcamera->posEye, &posEyeWorld);

	if (pcamera->paloTarget != nullptr)
	{
		const glm::vec3 posTarget = pcamera->paloTarget->xf.posWorld;
		glm::vec3 direction = posTarget - posEyeWorld;

		LoadLookAtMatrix(&direction, &matCamera);
	}
	else if (pcamera->ppntTarget != nullptr)
	{
		glm::vec3 posTarget{};
		GetPntPos(pcamera->ppntTarget, &posTarget);

		glm::vec3 direction = posTarget - posEyeWorld;

		LoadLookAtMatrix(&direction, &matCamera);
	}
	else
	{
		glm::mat3 matLocal(1.0f);

		BuildOrthonormalMatrixZ(pcamera->vecView, pcamera->vecUp, matLocal);

		matCamera = pcamera->xf.matWorld * matLocal;
	}

	SetCmPosMat(pcm, &pcamera->xf.posWorld, &matCamera);
}
