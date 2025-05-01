#include "hide.h"

HBSK* NewHbsk()
{
	return new HBSK{};
}

void InitHbsk(HBSK* phbsk)
{
	InitSo(phbsk);
}

int GetHbskSize()
{
	return sizeof(HBSK);
}

void OnHbskAdd(HBSK* phbsk)
{
	OnSoAdd(phbsk);
}

void OnHbskRemove(HBSK* phbsk)
{
	OnSoRemove(phbsk);
	//RemoveDlEntry(&g_dlHbsk, phbsk);
}

void LoadHbskFromBrx(HBSK* phbsk, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(phbsk, pbis);
}

void CloneHbsk(HBSK* phbsk, HBSK* phbskBase)
{
	CloneSo(phbsk, phbskBase);

	phbsk->hbsks = phbskBase->hbsks;
	phbsk->tHbsks = phbskBase->tHbsks;
	phbsk->dleHbsk = phbskBase->dleHbsk;
	phbsk->sFlattenRadius = phbskBase->sFlattenRadius;
	phbsk->cMaxDartsStuck = phbskBase->cMaxDartsStuck;
}

void DeleteHbsk(HBSK *phbsk)
{
	delete phbsk;
}

HSHAPE* NewHshape()
{
	return new HSHAPE{};
}

void InitHshape(HSHAPE* phshape)
{
	InitShape(phshape);
}

int GetHshapeSize()
{
	return sizeof(HSHAPE);
}

void OnHshapeAdd(HSHAPE* phshape)
{
	OnLoAdd(phshape);
}

void OnHshapeRemove(HSHAPE* phshape)
{
	OnLoRemove(phshape);
	//RemoveDlEntry(&g_dlHshape, phshape);
}

void CloneHshape(HSHAPE* phshape, HSHAPE* phshapeBase)
{
	CloneShape(phshape, phshapeBase);

	phshape->dleHshape = phshapeBase->dleHshape;
	phshape->jthk = phshapeBase->jthk;
	phshape->oidTnHide = phshapeBase->oidTnHide;
	phshape->ptnHide = phshapeBase->ptnHide;
	phshape->fTunnel = phshapeBase->fTunnel;
	phshape->grfhp = phshapeBase->grfhp;
	phshape->fDetect = phshapeBase->fDetect;
}

void BindHshape(HSHAPE* phshape)
{

}

void DeleteHshape(HSHAPE* phshape)
{
	delete phshape;
}

HPNT* NewHpnt()
{
	return new HPNT{};
}

void InitHpnt(HPNT* phpnt)
{
	InitLo(phpnt);
}

int GetHpntSize()
{
	return sizeof(HPNT);
}

void OnHpntAdd(HPNT* phpnt)
{
	OnLoAdd(phpnt);
}

void OnHpntRemove(HPNT* phpnt)
{
	OnLoRemove(phpnt);
	//RemoveDlEntry(&g_dlHpnt, phpnt);
}

void CloneHpnt(HPNT* phpnt, HPNT* phpntBase)
{
	ClonePnt(phpnt, phpntBase);

	phpnt->dleHpnt = phpntBase->dleHpnt;
	phpnt->jthk = phpntBase->jthk;
	phpnt->sFlattenRadius = phpntBase->sFlattenRadius;
	phpnt->oidTnHide = phpntBase->oidTnHide;
	phpnt->ptnHide = phpntBase->ptnHide;
	phpnt->fTunnel = phpntBase->fTunnel;
	phpnt->fDetect = phpntBase->fDetect;
	phpnt->dzIgnore = phpntBase->dzIgnore;
}

void BindHpnt(HPNT* phpnt)
{

}

void DeleteHpnt(HPNT* phpnt)
{
	delete phpnt;
}