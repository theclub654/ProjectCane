#include "emitter.h"

EXPLO* NewExplo()
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
	EMITB emitb{};

	pexplo->pemitb = std::make_shared <EMITB>(emitb);

	LoadXfmFromBrx(pexplo, pbis);
	int8_t crvk = pbis->S8Read();

	if (crvk != -1)
	{
		std::shared_ptr <CRV> pcrv = PcrvNew((CRVK)crvk);
		pcrv->pvtcrvl->pfnLoadCrvlFromBrx(std::static_pointer_cast <CRVL> (pcrv), pbis);
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

void BindExplo(EXPLO* pexplo)
{

}

void CloneExplo(EXPLO* pexplo, EXPLO* pexploBase)
{
	CloneExpl(pexplo, pexploBase);

	pexplo->pemitb = pexploBase->pemitb;
	pexplo->oidreference = pexploBase->oidreference;
	pexplo->oidShape = pexploBase->oidShape;
}

void* GetExploEmitok(EXPLO* pexplo)
{
	return &pexplo->pemitb->emito.emitok;
}

void DeleteExplo(EXPLO* pexplo)
{
	delete pexplo;
}

EMITTER* NewEmitter()
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
	EMITB emitb{};

	pemitter->pemitb = std::make_shared <EMITB>(emitb);

	LoadAloFromBrx(pemitter, pbis);

	int8_t crvk = pbis->S8Read();

	if (crvk != -1)
	{
		std::shared_ptr <CRV> pcrv = PcrvNew((CRVK)crvk);
		pcrv->pvtcrvl->pfnLoadCrvlFromBrx(std::static_pointer_cast <CRVL> (pcrv), pbis);
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

void BindEmitter(EMITTER* pemitter)
{
	BindAlo(pemitter);
}

void CloneEmitter(EMITTER* pemitter, EMITTER* pemitterBase)
{
	CloneAlo(pemitter, pemitterBase);

	pemitter->pemitb = pemitterBase->pemitb;
	pemitter->emitrk = pemitterBase->emitrk;
	pemitter->cParticle = pemitterBase->cParticle;
	pemitter->lmSvcParticle = pemitterBase->lmSvcParticle;
	pemitter->fCountIsDensity = pemitterBase->fCountIsDensity;
	pemitter->uPauseProb = pemitterBase->uPauseProb;
	pemitter->lmDtPause = pemitterBase->lmDtPause;
	pemitter->cParticleConstant = pemitterBase->cParticleConstant;
	pemitter->oidReference = pemitterBase->oidReference;
	pemitter->oidRender = pemitterBase->oidRender;
	pemitter->oidTouch = pemitterBase->oidTouch;
	pemitter->oidNextRender = pemitterBase->oidNextRender;
	pemitter->fAutoPause = pemitterBase->fAutoPause;
	pemitter->oidShape = pemitterBase->oidShape;
	pemitter->oidGroup = pemitterBase->oidGroup;
	pemitter->dlGroup = pemitterBase->dlGroup;
	pemitter->dleGroup = pemitterBase->dleGroup;
	pemitter->svcParticle = pemitterBase->svcParticle;
	pemitter->dtRecalcSvc = pemitterBase->dtRecalcSvc;
	pemitter->tRecalcSvc = pemitterBase->tRecalcSvc;
	pemitter->rDensity = pemitterBase->rDensity;
	pemitter->sBoxRadius = pemitterBase->sBoxRadius;
	pemitter->uParticle = pemitterBase->uParticle;
	pemitter->tUnpause = pemitterBase->tUnpause;
	pemitter->pripg = pemitterBase->pripg;
	pemitter->pblipg = pemitterBase->pblipg;
	pemitter->fValuesChanged = pemitterBase->fValuesChanged;
}

EMITB* PemitbEnsureEmitter(EMITTER* pemitter, ENSK ensk)
{
	return pemitter->pemitb.get();
}

void* GetEmitterEmitok(EMITTER* pemitter)
{
	return &pemitter->pemitb->emito.emitok;
}

void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro)
{

}

void DeleteEmitter(EMITTER *pemitter)
{
	delete pemitter;
}

EXPL* NewExpl()
{
	return new EXPL{};
}

int GetExplSize()
{
	return sizeof(EXPL);
}

void CloneExpl(EXPL* pexpl, EXPL* pexplBase)
{
	CloneXfm(pexpl, pexplBase);
	pexpl->pexplgParent = pexplBase->pexplgParent;
}

void DeleteExpl(EXPL* pexpl)
{
	delete pexpl;
}

EXPLS* NewExpls()
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
	CloneExplo(pexpls, pexplsBase);

	pexpls->psfx = pexplsBase->psfx;
	pexpls->lmcParticle = pexplsBase->lmcParticle;
	pexpls->oidRender = pexplsBase->oidRender;
	pexpls->oidNextRender = pexplsBase->oidNextRender;
	pexpls->oidTouch = pexplsBase->oidTouch;
	pexpls->dtDelay = pexplsBase->dtDelay;
	pexpls->fGrouped = pexplsBase->fGrouped;
	pexpls->pripg = pexplsBase->pripg;
	pexpls->pblipg = pexplsBase->pblipg;
	pexpls->tExplodeNext = pexplsBase->tExplodeNext;
	pexpls->fExplodeSiblings = pexplsBase->fExplodeSiblings;
}

void* GetExplsEmitok(EXPLS* pexpls)
{
	return &pexpls->pemitb->emito.emitok;
}

void BindExpls(EXPLS* pexpls)
{

}

void DeleteExpls(EXPLS* pexpls)
{
	delete pexpls;
}

EXPLG* NewExplg()
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

void BindExplg(EXPLG* pexplg)
{
	
}

void CloneExplg(EXPLG* pexplg, EXPLG* pexplgBase)
{
	CloneExpl(pexplg, pexplgBase);

	pexplg->cpexpl = pexplgBase->cpexpl;
	pexplg->apexpl = pexplgBase->apexpl;
}

void DeleteExplg(EXPLG* pexplg)
{
	delete pexplg;
}