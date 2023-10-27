#include "so.h"

void InitSo(SO* pso)
{
	//CREF cref;
	InitDl(&pso->dlPhys, 0x2f4);
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

	uint16_t vertexCount = pbis->ReadGeom();
	pbis->ReadBspc();

	pbis->F32Read();

	pbis->ReadMatrix();
	pbis->ReadVector();

	uint16_t unk1 = pbis->U16Read();

	for (int i = 0; i < unk1; i++)
	{
		pbis->S16Read();
		pbis->U16Read();
	}

	uint32_t unk2 = pbis->U32Read();

	for (int i = 0; i < unk2; i++)
		pbis->S16Read();

	uint32_t unk3 = pbis->U32Read();

	for (int i = 0; i < unk3; i++)
	{
		pbis->S16Read();
		pbis->S16Read();
	}

	pbis->ReadGeom();
	pbis->ReadBspc();

	LoadAloFromBrx(pso, pbis);
}

void UpdateSo(SO* pso, float dt)
{

}
