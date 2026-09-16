#include "cpalign.h"
#include "cm.h"
#include "so.h"

void InitCpalign(CPALIGN* pcpalign, CM* pcm)
{
	InitCplcy(pcpalign, pcm);
	pcpalign->matLocal = glm::identity<glm::mat3>();
}

void ActivateCpalign(CPALIGN* pcpalign, void* pv)
{
	ResetCmLookAtSmooth(pcpalign->pcm, &pcpalign->pcm->acpr[0].psoFocus->posWorldPrev);
}

void UpdateCpalign(CPALIGN* pcpalign, CPDEFI* pcpdefi, JOY* pjoy, float dt)
{
	/*(void)pcpdefi;
	(void)pjoy;*/

	CM* pcm = pcpalign->pcm;
	SO* psoFocus = pcm->acpr[0].psoFocus;

	if (psoFocus == nullptr)
		return;

	// Transform the alignment orientation and position by the focus object.
	const glm::mat3 matAdjust = psoFocus->xf.matWorld * pcpalign->matLocal;
	const glm::vec3 dpos = psoFocus->xf.matWorld * pcpalign->posLocal;

	const glm::vec3 posEye = psoFocus->xf.posWorld + dpos;
	const glm::vec3 posCenter = posEye + matAdjust[0] * 10.0f;

	const float u = std::clamp(g_clock.t - pcm->tActivateCplcy, 0.0f, 1.0f);
	const float uInv = 1.0f - u;

	const float sRadius = uInv * 20.0f + u * 100.0f;
	const float dz = uInv * -7.0f + u * -15.0f;
	const float smooth = uInv + u * 0.25f;

	SetCmLookAtSmooth(pcm, 1, &posEye, &posCenter, nullptr, sRadius, dz, smooth, 0.0f, 0.0f, dt);
}
