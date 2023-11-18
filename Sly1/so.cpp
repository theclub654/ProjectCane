#include "so.h"

void InitSo(SO* pso)
{
	InitDl(&pso->dlPhys, 0x440);
	InitAlo(pso);

	InitGeom(&pso->geomLocal);
}

void OnSoAdd(SO *pso)
{
	pso->psw->cpsoAll++;

	if (pso->paloParent == nullptr)
	{
		pso->psw->cpsoRoot++;
		AppendDlEntry(&pso->psw->dlRoot, pso);
	}

	OnAloAdd(pso);
}

void LoadSoFromBrx(SO* pso, CBinaryInputStream* pbis)
{
	pbis->U8Read();

	pbis->ReadGeom(&pso->geomLocal);
	pbis->ReadBspc();

	pso->m = pbis->F32Read();

	pso->momintLocal =  pbis->ReadMatrix();
	pso->posComLocal = pbis->ReadVector();

	pso->cnpg = pbis->U16Read();
	pso->anpg.resize(pso->cnpg);

	for (int i = 0; i < pso->cnpg; i++)
	{
		pso->anpg[i].cmk = pbis->S16Read();
		pso->anpg[i].ipglob = pbis->U16Read();
	}

	pso->mpibspinpg.resize(pbis->U32Read());

	for (int i = 0; i < pso->mpibspinpg.size(); i++)
		pso->mpibspinpg[i] = pbis->S16Read();

	pso->chsg = pbis->U32Read();
	pso->ahsg.resize(pso->chsg);

	for (int i = 0; i < pso->chsg; i++)
	{
		pso->ahsg[i].ipglob = pbis->S16Read();
		pso->ahsg[i].ipsubglob = pbis->S16Read();
	}

	pbis->ReadGeom(&pso->geomCameraLocal);
	pbis->ReadBspc();

	LoadAloFromBrx(pso, pbis);
}

void UpdateSo(SO* pso, float dt)
{

}

void RenderSoSelf(SO* pso, CM* pcm, RO* pro)
{
	RenderAloSelf(pso, pcm, pro);
}

void DeleteSo(LO* plo)
{
	delete(SO*)plo;
}
