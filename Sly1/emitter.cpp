#include "emitter.h"

void* NewExplo()
{
	return new EXPLO{};
}

void InitExplo(EXPLO* pexplo)
{
	InitXfm(pexplo);
}

int GetExploSize()
{
	return sizeof(EXPLO);
}

void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis)
{
	/*LoadExploCount++;

	std::cout << "LoadExploFromBrx Count: " << LoadExploCount << "\n";*/

	EMITB emitb{};

	pexplo->pemitb = new EMITB{};
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

	if (crgba != 0)
		LoadEmitblipColorsFromBrx(crgba, pbis);
}

void CloneExplo(EXPLO* pexplo, EXPLO* pexploBase)
{
	LO lo = *pexplo;
	*pexplo = *pexploBase;
	memcpy(pexplo, &lo, sizeof(LO));

	CloneLo(pexplo, pexploBase);
	pexplo->pemitb->cref++;
}

void DeleteExplo(LO* plo)
{
	EXPLO* explo = (EXPLO*)plo;

	delete explo;
}

void* NewEmitter()
{
	return new EMITTER{};
}

void InitEmitter(EMITTER* pemitter)
{
	InitAlo(pemitter);
	InitDl(&pemitter->dlGroup, offsetof(EMITTER, dleGroup));
}

int GetEmitterSize()
{
	return sizeof(EMITTER);
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

void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis)
{
	/*LoadEmitterCount++;

	std::cout << "LoadEmitterFromBrx Count: " << LoadEmitterCount << "\n";*/

	pemitter->pemitb = new EMITB{};
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

void CloneEmitter(EMITTER* pemitter, EMITTER* pemitterBase)
{
	LO lo = *pemitter;
	*pemitter = *pemitterBase;
	memcpy(pemitter, &lo, sizeof(LO));

	CloneLo(pemitter, pemitterBase);

	ClearDl(&pemitter->dlChild);

	pemitter->pemitb->cref++;
}

EMITB* PemitbEnsureEmitter(EMITTER* pemitter, ENSK ensk)
{
	return pemitter->pemitb;
}

void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro)
{

}

void DeleteEmitter(LO* plo)
{
	EMITTER *emitter = (EMITTER*)plo;

	delete emitter;
}

void* NewExpl()
{
	return new EXPL{};
}

int GetExplSize()
{
	return sizeof(EXPL);
}

void CloneExpl(EXPL* pexpl, EXPL* pexplBase)
{
	LO lo = *pexpl;
	*pexpl = *pexplBase;
	memcpy(pexpl, &lo, sizeof(LO));

	CloneLo(pexpl, pexplBase);
}

void DeleteExpl(LO* plo)
{
	delete(EXPL*)plo;
}

void* NewExpls()
{
	return new EXPLS{};
}

void InitExpls(EXPLS* pexpls)
{
	InitExplo(pexpls);
}

int GetExplsSize()
{
	return sizeof(EXPLS);
}

void CloneExpls(EXPLS* pexpls, EXPLS* pexplsBase)
{
	LO lo = *pexpls;
	*pexpls = *pexplsBase;
	memcpy(pexpls, &lo, sizeof(LO));

	CloneLo(pexpls, pexplsBase);

}

void DeleteExpls(LO* plo)
{
	delete(EXPLS*)plo;
}

void* NewExplg()
{
	return new EXPLG{};
}

int GetExplgSize()
{
	return sizeof(EXPLG);
}

void LoadExplgFromBrx(EXPLG* pexplg, CBinaryInputStream* pbis)
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

void CloneExplg(EXPLG* pexplg, EXPLG* pexplgBase)
{
	CloneLo(pexplg, pexplgBase);
}

void DeleteExplg(LO* plo)
{
	delete(EXPLG*)plo;
}