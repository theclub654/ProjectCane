#include "flying.h"
#include "stuck.h"

void PostFlyingEmit(FLYING* pflying, EMITB* pemitb)
{
	pflying->paloStuck = (pemitb->emitp).emitrip.paloNextRender;
}

void RenderFlying(FLYING* pflying, CM* pcm)
{
	glm::mat3 mat;
	BuildOrthonormalMatrixZ(pflying->v, g_normalZ, mat);
	FRenderRipPosMat(pflying, pcm, &pflying->pos, &mat);
}

int FBounceFlying(FLYING* pflying, SO* psoOther, glm::vec3* ppos, glm::vec3* pnormal)
{
	if (pflying == nullptr || psoOther == nullptr || ppos == nullptr || pnormal == nullptr)
		return 0;

	// FLYING particles with a next-render object become a surface splat on
	// impact.  The retail VTFLYING dispatches here instead of using the generic
	// rigid bounce response.
	if (pflying->paloStuck != nullptr)
	{
		CreateStuck(pflying, pflying->paloStuck, psoOther, ppos, pnormal, nullptr);
		RemoveRip(pflying);
		return 1;
	}

	return FBounceRip(pflying, psoOther, ppos, pnormal);
}

VTFLYING g_vtflying;
