#include "crv.h"

CRV* PcrvNew(CRVK crvk)
{
	if (crvk == CRVK_Linear)
	{
		CRVL* pcrvl = new CRVL{};
		pcrvl->pvtcrvl = &g_vtcrvl;
		pcrvl->crvk = crvk;

		return pcrvl;
	}

	else
	{
		if (crvk == CRVK_Cubic)
		{
			CRVC* pcrvc = new CRVC{};
			pcrvc->pvtcrvc = &g_vtcrvc;
			pcrvc->crvk - crvk;

			return pcrvc;
		}
	}
}

void LoadCrvlFromBrx(CRVL* pcrvl, CBinaryInputStream* pbis)
{
	pcrvl->fClosed = pbis->U8Read();
    pcrvl->ccv = pbis->U8Read();

	pcrvl->mpicvu.resize(pcrvl->ccv);
	pcrvl->mpicvs.resize(pcrvl->ccv);
	pcrvl->mpicvpos.resize(pcrvl->ccv);

	for (int i = 0; i < pcrvl->ccv; i++)
	{
		pcrvl->mpicvu[i]   = pbis->F32Read();
		pcrvl->mpicvpos[i] = pbis->ReadVector();
	}
}

void LoadCrvcFromBrx(CRVC* pcrvc, CBinaryInputStream* pbis)
{
	pcrvc->fClosed = pbis->U8Read();
	pcrvc->ccv = pbis->U8Read();

	pcrvc->mpicvu.resize(pcrvc->ccv);
	pcrvc->mpicvs.resize(pcrvc->ccv);
	pcrvc->mpicvpos.resize(pcrvc->ccv);
	pcrvc->mpicvdposIn.resize(pcrvc->ccv);
	pcrvc->mpicvdposOut.resize(pcrvc->ccv);


	for (int i = 0; i < pcrvc->ccv; i++)
	{
		pcrvc->mpicvu[i]       = pbis->F32Read();
		pcrvc->mpicvpos[i]     = pbis->ReadVector();
		pcrvc->mpicvdposIn[i]  = pbis->ReadVector();
		pcrvc->mpicvdposOut[i] = pbis->ReadVector();
	}
}

void DeletePcrv(CRVK crvk, CRV *pcrv)
{
	if (crvk == CRVK_Linear)
		delete (CRVL*)pcrv;

	else
	{
		if (crvk == CRVK_Cubic)
			delete (CRVC*)pcrv;
	}
}
