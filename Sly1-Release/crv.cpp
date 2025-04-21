#include "crv.h"

std::shared_ptr <CRV> PcrvNew(CRVK crvk)
{
	std::shared_ptr <CRV> pcrv;

	if (crvk == CRVK_Linear)
	{
		CRVL crvl{};
		crvl.pvtcrvl = &g_vtcrvl;
		pcrv = std::make_shared <CRVL>(crvl);
	}

	else
	{
		if (crvk == CRVK_Cubic)
		{
			CRVC crvc{};
			crvc.pvtcrvc = &g_vtcrvc;
			pcrv = std::make_shared <CRVC>(crvc);
		}
	}

	if (pcrv != nullptr)
		pcrv->crvk = crvk;

	return pcrv;
}

void LoadCrvlFromBrx(std::shared_ptr <CRVL> pcrvl, CBinaryInputStream* pbis)
{
	pcrvl->fClosed = pbis->U8Read();
	pcrvl->ccv = pbis->U8Read();

	pcrvl->mpicvu.resize(pcrvl->ccv);
	pcrvl->mpicvs.resize(pcrvl->ccv);
	pcrvl->mpicvpos.resize(pcrvl->ccv);

	for (int i = 0; i < pcrvl->ccv; i++)
	{
		pcrvl->mpicvu[i] = pbis->F32Read();
		pcrvl->mpicvpos[i] = pbis->ReadVector();
	}
}

void LoadCrvcFromBrx(std::shared_ptr <CRVC> pcrvc, CBinaryInputStream* pbis)
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
		pcrvc->mpicvu[i] = pbis->F32Read();
		pcrvc->mpicvpos[i] = pbis->ReadVector();
		pcrvc->mpicvdposIn[i] = pbis->ReadVector();
		pcrvc->mpicvdposOut[i] = pbis->ReadVector();
	}
}

void DeletePcrv(CRVK crvk, CRV* pcrv)
{

}
