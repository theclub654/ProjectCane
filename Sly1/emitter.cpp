#include "emitter.h"

void* NewExplo()
{
	return new EXPLO;
}

void InitExplo(EXPLO* pexplo)
{
	InitXfm(pexplo);
}

void DeleteExplo(LO* plo)
{
	delete(EXPLO*)plo;
}

void* NewEmitter()
{
	return new EMITTER;
}

void InitEmitter(EMITTER* pemitter)
{
	InitAlo(pemitter);
}

void DeleteEmitter(LO* plo)
{
	delete(EMITTER*)plo;
}

void* NewExpl()
{
	return new EXPL;
}

void DeleteExpl(LO* plo)
{
	delete(EXPL*)plo;
}

void* NewExpls()
{
	return new EXPLS;
}

void InitExpls(EXPLS* pexpls)
{
	InitExplo(pexpls);
}

void DeleteExpls(LO* plo)
{
	delete(EXPLS*)plo;
}

void LoadEmitMeshFromBrx(CBinaryInputStream* pbis)
{
	uint16_t cpos = pbis->U16Read();

	for (int i = 0; i < cpos; i++)
		pbis->ReadVector();

	uint16_t cemittri = pbis->U16Read();

	for (int i = 0; i < cemittri; i++)
	{
		for (int i = 0; i <= 2; i++)
			pbis->U16Read();

		pbis->F32Read();
	}

	pbis->ReadVector();
}

void LoadEmitblipColorsFromBrx(int crgba, CBinaryInputStream* pbis)
{
	byte colorRanges = pbis->U8Read();

	for (int i = 0; i < crgba; i++)
		pbis->U32Read();
}

void LoadEmitterFromBrx(EMITTER *pemitter, CBinaryInputStream *pbis)
{
	LoadAloFromBrx(pemitter, pbis);

	int8_t crvType = pbis->S8Read();

	if (crvType != -1)
	{
		switch (crvType)
		{
			case 0x0:
			LoadCrvlFromBrx(pbis);
			break;

			case 0x1:
			LoadCrvcFromBrx(pbis);
			break;
		}
	}

	if (loadEmitMesh == true)
	{
		loadEmitMesh = false;
		LoadEmitMeshFromBrx(pbis);
	}

	uint16_t crgba = pbis->U16Read();

	if (crgba != 0)
		LoadEmitblipColorsFromBrx(crgba, pbis);
}

void LoadExplgFromBrx(EXPLG *pexplg,CBinaryInputStream *pbis)
{
	LoadXfmFromBrx(pexplg, pbis);

	uint16_t numExplgObjs = pbis->S16Read();

	for (int i = 0; i < numExplgObjs; i++)
	{
		CID cid = (CID)pbis->S16Read();
		OID oid = (OID)pbis->S16Read();
		uint16_t isplice = pbis->S16Read();

		LO* plo = PloNew(cid, pexplg->psw, pexplg->paloParent, oid, isplice);
		plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);
	}
}

void* NewExplg()
{
	return new EXPLG;
}

void DeleteExplg(LO* plo)
{
	delete(EXPLG*)plo;
}

void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis)
{
	LoadXfmFromBrx(pexplo, pbis);
	int8_t crvType = pbis->S8Read();
	if (crvType != -1)
	{
		switch (crvType)
		{
			case 0x0:
			LoadCrvlFromBrx(pbis);
			break;

			case 0x1:
			LoadCrvcFromBrx(pbis);
			break;
		}
	}
	
	if (loadEmitMesh == true)
	{
		loadEmitMesh = false;
		LoadEmitMeshFromBrx(pbis);
	}

	uint16_t crgba = pbis->U16Read();

	if(crgba != 0)
		LoadEmitblipColorsFromBrx(crgba, pbis);
}

void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro)
{

}
