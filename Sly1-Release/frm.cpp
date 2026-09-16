#include "frm.h"
#include "blip.h"
#include <utility>

static DL s_dlBlipgPendingDeferred;
static DL s_dlBlipPendingDeferred;

void StartupFrame()
{
	// Removed BLIP/BLIPG objects remain alive until the next frame boundary.
	// These offsets are required before LoadSwFromBrx builds the reusable pools.
	InitDl(&g_dlBlipgPending, offsetof(BLIPG, dleBlipg));
	InitDl(&g_dlBlipPending, offsetof(BLIP, dle));
	InitDl(&s_dlBlipgPendingDeferred, offsetof(BLIPG, dleBlipg));
	InitDl(&s_dlBlipPendingDeferred, offsetof(BLIP, dle));
}

void OpenFrame()
{
	// Retail stores pending BLIPs on the currently open member of a two-frame
	// FRM ring. Reclaim the older frame first, then rotate this frame's pending
	// lists into the deferred slot. This keeps render submissions alive until
	// the frame that referenced them is no longer in flight.
	while (s_dlBlipgPendingDeferred.pvFirst != nullptr)
	{
		BLIPG* pblipg = s_dlBlipgPendingDeferred.pblipgFirst;
		RemoveDlEntry(&s_dlBlipgPendingDeferred, pblipg);
		AppendDlEntry(&pblipg->psw->dlBlipgFree, pblipg);
	}

	while (s_dlBlipPendingDeferred.pvFirst != nullptr)
	{
		BLIP* pblip = s_dlBlipPendingDeferred.pblipFirst;
		SW* psw = pblip->pblipg->psw;
		BLIPSP* pblipsp = pblip->pblipsp;

		RemoveDlEntry(&s_dlBlipPendingDeferred, pblip);

		if (pblipsp != nullptr)
		{
			for (auto it = psw->blipspOwners.begin(); it != psw->blipspOwners.end(); ++it)
			{
				if (it->get() == pblipsp)
				{
					psw->blipspOwners.erase(it);
					break;
				}
			}
		}

		for (auto it = psw->blipOwners.begin(); it != psw->blipOwners.end(); ++it)
		{
			if (it->get() == pblip)
			{
				psw->blipOwners.erase(it);
				break;
			}
		}
	}


	std::swap(g_dlBlipgPending, s_dlBlipgPendingDeferred);
	std::swap(g_dlBlipPending, s_dlBlipPendingDeferred);
}

void ClearAllPendingFrames()
{
	ClearDl(&g_dlBlipPending);
	ClearDl(&g_dlBlipgPending);
	ClearDl(&s_dlBlipPendingDeferred);
	ClearDl(&s_dlBlipgPendingDeferred);
}
