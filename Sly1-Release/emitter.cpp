#include "emitter.h"

EXPLO* NewExplo()
{
	return new EXPLO{};
}

void InitExplo(EXPLO* pexplo)
{
	InitXfm(pexplo);
	pexplo->oidShape = OID_Nil;
	pexplo->oidReference = OID_Nil;
}

int GetExploSize()
{
	return sizeof(EXPLO);
}

void LoadExploFromBrx(EXPLO* pexplo, CBinaryInputStream* pbis)
{
	EMITB emitb{};

	pexplo->pemitb = std::make_shared<EMITB>();

	LoadXfmFromBrx(pexplo, pbis);
	int8_t crvk = pbis->S8Read();

	if (crvk != -1)
	{
		std::shared_ptr <CRV> pcrv = PcrvNew((CRVK)crvk);
		pcrv->pvtcrvl->pfnLoadCrvlFromBrx(std::static_pointer_cast<CRVL>(pcrv), pbis);
	}

	if (pexplo->pemitb->emito.emitok == EMITOK_Mesh)
		LoadEmitMeshFromBrx(&pexplo->pemitb->emito.emitmeshOrigin, pbis);

	uint16_t crgba = pbis->U16Read();

	if (crgba != 0)
	{
		LoadEmitblipColorsFromBrx(nullptr, crgba, pbis);
	}
}

void CloneExplo(EXPLO* pexplo, EXPLO* pexploBase)
{
	CloneExpl(pexplo, pexploBase);

	pexplo->pemitb = pexploBase->pemitb;
	pexplo->oidReference = pexploBase->oidReference;
	pexplo->oidShape = pexploBase->oidShape;
}

EMITOK* PemitbEnsureExploEmitok(EXPLO* pexplo, ENSK ensk)
{
	return &pexplo->pemitb.get()->emito.emitok;;
}

void BindExplo(EXPLO* pexplo)
{

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
	pemitter->lmSvcParticle.gMin = 10.0;
	pemitter->tUnpause = -1.0;
	pemitter->oidGroup = OID_Nil;
	pemitter->cParticle = -1;
	pemitter->lmSvcParticle.gMax = 10.0;
	pemitter->oidReference = OID_Nil;
	pemitter->oidRender = OID_Nil;
	pemitter->oidNextRender = OID_Nil;
	pemitter->oidTouch = OID_Nil;
	pemitter->oidShape = OID_Nil;
	pemitter->emitrk = EMITRK_Nil;
	InitDl(&pemitter->dlGroup, offsetof(EMITTER, dleGroup));
}

int GetEmitterSize()
{
	return sizeof(EMITTER);
}

void LoadEmitMeshFromBrx(EMITMESH* pemitmesh, CBinaryInputStream* pbis)
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

void LoadEmitblipColorsFromBrx(EMITBLIP *pemitblip, int crgba, CBinaryInputStream* pbis)
{
	if (pemitblip != nullptr)
	{
		const int storeCount = std::min<int>(crgba, 0x20);

		pemitblip->crgba = storeCount;
		pemitblip->argba.resize(storeCount);

		pemitblip->fColorRanges = static_cast<int>(static_cast<int8_t>(pbis->U8Read()));

		constexpr float inv255 = 1.0f / 255.0f;

		for (int i = 0; i < crgba; ++i)
		{
			const u32 packed = pbis->U32Read();
			
			if (i < storeCount)
			{
				const float r = ((packed >> 0)  & 0xFF) * inv255;
				const float g = ((packed >> 8)  & 0xFF) * inv255;
				const float b = ((packed >> 16) & 0xFF) * inv255;
				const float a = ((packed >> 24) & 0xFF) * inv255;

				pemitblip->argba[i] = glm::vec4(r, g, b, a);
			}
		}
	}
	else
	{
		byte colorRanges = pbis->U8Read();

		for (int i = 0; i < crgba; i++)
			pbis->U32Read();
	}
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
		pemitter->pemitb->emito.emitcrvOrigin.pcrv = pcrv;
		pcrv->pvtcrvl->pfnLoadCrvlFromBrx(std::static_pointer_cast <CRVL> (pcrv), pbis);
	}

	if (pemitter->pemitb->emito.emitok == EMITOK_Mesh)
		LoadEmitMeshFromBrx(&pemitter->pemitb->emito.emitmeshOrigin, pbis);

	uint16_t crgba = pbis->U16Read();

	if (crgba != 0)
	{
		EMITB *pemitb = PemitbEnsureEmitter(pemitter, ENSK_Set);
		LoadEmitblipColorsFromBrx(&pemitb->emitp.emitblip, crgba, pbis);
	}
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

void UnpauseEmitter(EMITTER* pemitter)
{
	pemitter->tUnpause = -1.0;
}

int FPausedEmitter(EMITTER* emitter)
{
	if (emitter->tUnpause > g_clock.t)
		return 1;

	const int isActiveNonBurst = (EMITRK_Nil < emitter->emitrk) && (emitter->emitrk < EMITRK_Burst);

	return (isActiveNonBurst && emitter->cParticle == 0) ? 1 : 0;
}

EMITTER* PemitterEnsureEmitter(EMITTER* pemitter, ENSK ensk)
{
	if (ensk == ENSK_Set) {
		pemitter->fValuesChanged = 1;
	}

	return pemitter;
}

EMITB* PemitbEnsureEmitter(EMITTER* pemitter, ENSK ensk)
{
	/*if (ensk == ENSK_Set) 
	{
		pemitter->fValuesChanged = 1;
		pemitter->pemitb = PemitbCopyOnWrite(pemitter->pemitb);
		pemitter->pemitb->pchzName = PchzFromLo(pemitter);
	}*/

	return pemitter->pemitb.get();
}

EMITOK* PemitbEnsureEmitterEmitok(EMITTER* pemitter, ENSK ensk)
{

	return &pemitter->pemitb.get()->emito.emitok;
}

glm::vec3* PemitbEnsureEmitterEmitokVec(EMITTER* pemitter, ENSK ensk)
{
	return &pemitter->pemitb.get()->emito.posOrigin;
}

EMITRK *PemitbEnsureEmitterEmitrk(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->emitrk;
}

LM* PemitbEnsureEmitterlmSvcParticle(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->lmSvcParticle;
}

float* PemitbEnsureEmittercParticleConstant(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->cParticleConstant;
}

float* PemitEnsureEmitteruPauseProb(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->uPauseProb;
}

LM* PemitbEnsureEmitterlmDtPause(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->lmDtPause;
}

void GetEmitterEnabled(EMITTER* pemitter, int* pfEnabled)
{
	bool isPaused = FPausedEmitter(pemitter);
	*pfEnabled = !isPaused;
}

void SetEmitterEnabled(EMITTER* pemitter, int fEnabled)
{
	if (fEnabled == 0) {
		PauseEmitterIndefinite(pemitter);
	}
	else {
		UnpauseEmitter(pemitter);
	}
}

int* GetEmitterfCountIsDensity(EMITTER* pemitter)
{
	return &pemitter->fCountIsDensity;
}

void SetEmitterfCountIsDensity(EMITTER* pemitter, bool fCountDensity)
{
	pemitter->fCountIsDensity = fCountDensity;
}

void SetEmitterOidReference(EMITTER* pemitter, OID oidReference)
{
	pemitter->oidReference = oidReference;
}

OID* GetEmitterOidReference(EMITTER* pemitter)
{
	return &pemitter->oidReference;
}

void* GetEmitterOidRender(EMITTER* pemitter)
{
	return &pemitter->oidRender;
}

void SetEmitterOidRender(EMITTER* pemitter, OID oidRender)
{
	pemitter->oidRender = oidRender;
}

void* GetEmitterOidTouch(EMITTER* pemitter)
{
	return &pemitter->oidTouch;
}

void SetEmitterOidTouch(EMITTER* pemitter, OID oidTouch)
{
	pemitter->oidTouch = oidTouch;
}

void* GetEmitterOidNextRender(EMITTER* pemitter)
{
	return &pemitter->oidNextRender;
}

void SetEmitterOidNextRender(EMITTER* pemitter, OID oidNextRender)
{
	pemitter->oidNextRender = oidNextRender;
}

void* GetEmitterOidGroup(EMITTER* pemitter)
{
	return &pemitter->oidGroup;
}

void SetEmitterOidGroup(EMITTER* pemitter, OID oidGroup)
{
	pemitter->oidGroup = oidGroup;
}

void PauseEmitter(EMITTER* pemitter, float dtPause)
{
	pemitter->tUnpause = g_clock.t + dtPause;
}

void GetEmitterPaused(EMITTER* pemitter, int* pfPaused)
{
	*pfPaused = FPausedEmitter(pemitter);
}

void* GetEmitterOidShape(EMITTER* pemitter)
{
	return &pemitter->oidShape;
}

void SetEmitterOidShape(EMITTER* pemitter, OID oidShape)
{
	pemitter->oidShape = oidShape;
}

EMITNK* PemitbEnsureEmitterEmitnk(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->pemitb->emito.emitnk;
}

glm::vec3* PemitbEnsureEmitterEmitoVec(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->pemitb->emito.vec;
}

LM* PemitbEnsureEmitterlmSOffset(EMITTER* pemitter)
{
	return &PemitterEnsureEmitter(pemitter, ENSK_Set)->pemitb->emito.lmSOffset;
}

void SetEmitterParticleCount(EMITTER* pemitter, int cParticle)
{
	if (pemitter->emitrk == EMITRK_Nil) {
		pemitter->cParticle = cParticle;
	}
	else if (pemitter->emitrk == EMITRK_Continuous) {
		if (cParticle == 0) {
			PauseEmitterIndefinite(pemitter);
		}
		else if (cParticle == -1) {
			UnpauseEmitter(pemitter);
		}
		else {
			pemitter->cParticle = cParticle;
		}
	}
	pemitter->fValuesChanged = 1;
}

void SetEmitterAutoPause(EMITTER* pemitter, int fAutoPause)
{
	pemitter->fAutoPause = fAutoPause;
	pemitter->fValuesChanged = 1;
}

void PauseEmitterIndefinite(EMITTER* pemitter)
{
	pemitter->tUnpause = 3.402823e+38;
}

void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro)
{

}

void BindEmitter(EMITTER* pemitter)
{
	BindAlo(pemitter);
}

void PostEmitterLoad(EMITTER* pemitter)
{
	PostAloLoad(pemitter);
}

void UpdateEmitter(EMITTER* pemitter, float dt)
{
	UpdateAlo(pemitter, dt);
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

void PostExplLoad(EXPL* pexpl)
{
	PostLoLoad(pexpl);
	pexpl->pvtlo->pfnRemoveLo(pexpl);
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
	pexpls->oidTouch = OID_Nil;
	pexpls->oidRender = OID_Nil;
	pexpls->oidNextRender = OID_Nil;
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

void BindExpls(EXPLS* pexpls)
{
	BindExplo(pexpls);
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

void CloneExplg(EXPLG* pexplg, EXPLG* pexplgBase)
{
	CloneExpl(pexplg, pexplgBase);

	pexplg->cpexpl = pexplgBase->cpexpl;
	pexplg->apexpl = pexplgBase->apexpl;
}

void BindExplg(EXPLG* pexplg)
{

}

void DeleteExplg(EXPLG* pexplg)
{
	delete pexplg;
}