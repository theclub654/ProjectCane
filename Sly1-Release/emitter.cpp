#include "emitter.h"
#include "blip.h"
#include "shape.h"
#include "expl.h"
#include "explg.h"
#include "expls.h"
#include "explo.h"
#include "rip.h"
static void EnsureEmitterGroup(EMITTER* pemitter);


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
	pemitmesh->cpos = pbis->U16Read();
	pemitmesh->apos.resize(pemitmesh->cpos);
	pemitmesh->anormal.assign(pemitmesh->cpos, glm::vec3(0.0f));

	for (int ipos = 0; ipos < pemitmesh->cpos; ++ipos)
		pemitmesh->apos[ipos] = pbis->ReadVector();

	pemitmesh->cemittri = pbis->U16Read();
	pemitmesh->aemittri.resize(pemitmesh->cemittri);
	pemitmesh->sTotalArea = 0.0f;

	for (int iemittri = 0; iemittri < pemitmesh->cemittri; ++iemittri) 
	{
		EMITTRI& emittri = pemitmesh->aemittri[iemittri];

		for (int i = 0; i < 3; ++i)
			emittri.aipos[i] = pbis->U16Read();

		emittri.sArea = pbis->F32Read();
		pemitmesh->sTotalArea += emittri.sArea;

		const glm::vec3& pos0 = pemitmesh->apos[emittri.aipos[0]];
		const glm::vec3& pos1 = pemitmesh->apos[emittri.aipos[1]];
		const glm::vec3& pos2 = pemitmesh->apos[emittri.aipos[2]];

		glm::vec3 normal = glm::cross(pos1 - pos0, pos2 - pos0);

		pemitmesh->anormal[emittri.aipos[0]] += normal;
		pemitmesh->anormal[emittri.aipos[1]] += normal;
		pemitmesh->anormal[emittri.aipos[2]] += normal;
	}

	pemitmesh->posCenter = pbis->ReadVector();

	for (glm::vec3& normal : pemitmesh->anormal) 
	{
		float length = glm::length(normal);

		if (length < 0.0001f)
			normal = g_normalZ;
		else
			normal /= length;
	}
}

void LoadEmitblipColorsFromBrx(EMITBLIP *pemitblip, int crgba, CBinaryInputStream* pbis)
{
	pemitblip->crgba = std::min(crgba, 32);
	pemitblip->argba.resize(pemitblip->crgba);

	pemitblip->fColorRanges = static_cast<int8_t>(pbis->U8Read());

	for (int irgba = 0; irgba < crgba; ++irgba) 
	{
		uint32_t rgba = pbis->U32Read();

		if (irgba < pemitblip->crgba)
		{
			float r = static_cast<float>((rgba >> 0) & 0xff) / 255.0f;
			float g = static_cast<float>((rgba >> 8) & 0xff) / 255.0f;
			float b = static_cast<float>((rgba >> 16) & 0xff) / 255.0f;
			// SetBlipgEmitb halves the source byte before the PS2 GS interprets
			// 0x80 as full alpha.  Converting the original byte through 255 here
			// is the equivalent normalized OpenGL value.
			float a = static_cast<float>((rgba >> 24) & 0xff) / 255.0f;

			pemitblip->argba[irgba] = glm::vec4(r, g, b, a);
		}
	}
}

void LoadEmitterFromBrx(EMITTER* pemitter, CBinaryInputStream* pbis)
{
	EMITB emitb{};
	pemitter->pemitb = std::make_shared <EMITB>(emitb);

	InitEmitb(pemitter->pemitb.get());
	LoadAloFromBrx(pemitter, pbis);

	int8_t crvk = pbis->S8Read();

	if (crvk != -1)
	{
		std::shared_ptr <CRV> pcrv = PcrvNew((CRVK)crvk);
		pemitter->pemitb->emito.emitcrvOrigin.pcrv = pcrv;
		pcrv->pvtcrv->pfnLoadCrvFromBrx(pcrv.get(), pbis);
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
	// These are intrusive runtime links, not cloneable emitter properties.
	// Copying them aliases the base emitter's group list and can create parent/list
	// cycles when PostEmitterLoad appends the clone to its actual group.
	InitDl(&pemitter->dlGroup, offsetof(EMITTER, dleGroup));
	ClearDl(&pemitter->dlGroup);
	ClearDle(&pemitter->dleGroup);
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

void SetEmitterAutoPause(EMITTER* pemitter, int fAutoPause)
{
	pemitter->fAutoPause = fAutoPause;
	pemitter->fValuesChanged = 1;
}

void* GetEmitterFAutoPause(EMITTER* pemitter)
{
	return &pemitter->fAutoPause;
}

EMITTER* PemitterEnsureEmitter(EMITTER* pemitter, ENSK ensk)
{
	if (ensk == ENSK_Set)
		pemitter->fValuesChanged = 1;

	return pemitter;
}

void AddEmitterSkeleton(EMITTER* pemitter, OID oid, OID oidOther, float sRadius, float gDensity, float sRadiusOther, float gDensityOther)
{
	EMITB* pemitb = PemitbEnsureEmitter(pemitter, ENSK_Set);
	AddEmitoSkeleton(&pemitb->emito, oid, oidOther, sRadius, gDensity, sRadiusOther, gDensityOther, pemitter);
}

EMITB* PemitbEnsureEmitter(EMITTER* pemitter, ENSK ensk)
{
	if (pemitter == nullptr || pemitter->pemitb == nullptr)
		return nullptr;

	if (ensk == ENSK_Set)
	{
		pemitter->fValuesChanged = 1;

		// CloneEmitter intentionally shares EMITB until an instance needs to
		// mutate it.  The original cref/PemitbEnsure path then performs a
		// copy-on-write.  Without this step, binding one cloned emitter's
		// paloReference overwrites every clone that shares the same EMITB.
		if (pemitter->pemitb.use_count() > 1)
		{
			auto pemitbUnique = std::make_shared<EMITB>(*pemitter->pemitb);
			pemitbUnique->cref = 1;
			pemitter->pemitb = std::move(pemitbUnique);
		}

		// PchzFromLo returns the object's effective LO name in the original.
		// This port stores that name directly on LO.
		pemitter->pemitb->pchzName = pemitter->pchzName;
	}

	return pemitter->pemitb.get();
}

EMITRK* PemitbEnsureEmitterEmitrk(EMITTER* pemitter, ENSK ensk)
{
	return &PemitterEnsureEmitter(pemitter, ensk)->emitrk;
}

LM* PemitbEnsureEmitterlmSvcParticle(EMITTER* pemitter, ENSK ensk)
{
	return &PemitterEnsureEmitter(pemitter, ensk)->lmSvcParticle;
}

float* PemitbEnsureEmittercParticleConstant(EMITTER* pemitter, ENSK ensk)
{
	return &PemitterEnsureEmitter(pemitter, ensk)->cParticleConstant;
}

float* PemitEnsureEmitteruPauseProb(EMITTER* pemitter, ENSK ensk)
{
	return &PemitterEnsureEmitter(pemitter, ensk)->uPauseProb;
}

LM* PemitbEnsureEmitterlmDtPause(EMITTER* pemitter, ENSK ensk)
{
	return &PemitterEnsureEmitter(pemitter, ensk)->lmDtPause;
}

void GetEmitterEnabled(EMITTER* pemitter, int* pfEnabled)
{
	*pfEnabled = !FPausedEmitter(pemitter);
}

void SetEmitterEnabled(EMITTER* pemitter, int fEnabled)
{
	if (fEnabled)
	{
		UnpauseEmitter(pemitter);
	}
	else
	{
		PauseEmitterIndefinite(pemitter);
	}
}

void SetEmitterFCountIsDensity(EMITTER* pemitter, int fCountIsDensity)
{
	pemitter->fCountIsDensity = fCountIsDensity;
}

void* GetEmitterFCountIsDensity(EMITTER* pemitter)
{
	return &pemitter->fCountIsDensity;
}

void SetEmitterOidReference(EMITTER* pemitter, OID oidReference)
{
	pemitter->oidReference = oidReference;
}

void* GetEmitterOidReference(EMITTER* pemitter)
{
	return &pemitter->oidReference;
}

void SetEmitterOidRender(EMITTER* pemitter, OID oidRender)
{
	pemitter->oidRender = oidRender;
}

void* GetEmitterOidRender(EMITTER* pemitter)
{
	return &pemitter->oidRender;
}

void SetEmitterOidTouch(EMITTER* pemitter, OID oidTouch)
{
	pemitter->oidTouch = oidTouch;
}

void* GetEmitterOidTouch(EMITTER* pemitter)
{
	return &pemitter->oidTouch;
}

void SetEmitterOidNextRender(EMITTER* pemitter, OID oidNextRender)
{
	pemitter->oidNextRender = oidNextRender;
}

void* GetEmitterOidNextRender(EMITTER* pemitter)
{
	return &pemitter->oidNextRender;
}

void SetEmitterOidGroup(EMITTER* pemitter, OID oidGroup)
{
	pemitter->oidGroup = oidGroup;
}

void* GetEmitterOidGroup(EMITTER* pemitter)
{
	return &pemitter->oidGroup;
}

void GetEmitterPaused(EMITTER* pemitter, int* pfPaused)
{
	*pfPaused = FPausedEmitter(pemitter);
}

EMITOK* PemitbEnsureEmitterEmitok(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emito.emitok;
}

glm::vec3* PemitbEnsureEmitterPosOrigin(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emito.posOrigin;
}

glm::vec3* PemitbEnsureEmitterBoxOriginPosMin(EMITTER* pemitter, ENSK ensk)
{
	// The PS2 EMITO origin variants overlap.  A box's minimum corner is the
	// same storage as posOrigin, so keep that canonical in the safe C++ port.
	return &PemitbEnsureEmitter(pemitter, ensk)->emito.posOrigin;
}

glm::vec3* PemitbEnsureEmitterBoxOriginPosMax(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emito.boxOrigin.posMax;
}

void SetEmitterOidShape(EMITTER* pemitter, OID oidShape)
{
	pemitter->oidShape = oidShape;
}

void* GetEmitterOidShape(EMITTER* pemitter)
{
	return &pemitter->oidShape;
}

EMITNK* PemitbEnsureEmitterEmitnk(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emito.emitnk;
}

glm::vec3* PemitbEnsureEmitterEmitoVec(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emito.vec;
}

LM* PemitbEnsureEmitterlmSOffset(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emito.lmSOffset;
}

void* PemitbEnsureEmitterEmitvk(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emito.emitvk;
}

LM* PemitbEnsureEmitterLmSv(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.lmSv;
}

float* PemitbEnsureEmitterRSvz(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.rSvz;
}

glm::vec3* PemitbEnsureEmitterDv(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.dv;
}

float* PemitbEnsureEmitterRvDamping(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.rvDamping;
}

float* PemitbEnsureEmitterSwCurl(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.swCurl;
}

glm::vec3* PemitbEnsureEmitterNormalCurl(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.normalCurl;
}

void* PemitbEnsureEmitterEmitcnk(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.emitcnk;
}

LM* PemitbEnsureEmitterLmTilt(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.lmTilt;
}

int* PemitbEnsureEmitterCParticlePerRing(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.cParticlePerRing;
}

float* PemitbEnsureEmitterURandomRad(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.uRandomRad;
}

float* PemitbEnsureEmitterDtLifetime(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.dtLifetime;
}

LM* PemitbEnsureEmitterLmDtSkip(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.lmDtSkip;
}

LM* PemitbEnsureEmitterLmDtBirth(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.lmDtBirth;
}

CLQ* PemitbEnsureEmitterClqAlpha(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitv.clqAlpha;
}

void* PemitbEnsureEmitterEmitpk(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitpk;
}

void* PemitbEnsureEmitterRiptTrail(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitrip.riptTrail;
}

LM* PemitbEnsureEmitterRipLmGScale(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitrip.lmGScale;
}

CLQ* PemitbEnsureEmitterRipClqScale(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitrip.clqScale;
}

void* PemitbEnsureEmitterRipgt(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitrip.ripgt;
}

float* PemitbEnsureEmitterSExpand(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitrip.sExpand;
}

OID* PemitbEnsureEmitterOidShader(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.oidShader;
}

CLQ* PemitbEnsureEmitterBlipClqScale(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.clqScale;
}

CLQ* PemitbEnsureEmitterBlipClqTexture(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.clqTexture;
}

CLQ* PemitbEnsureEmitterBlipClqColor(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.clqColor;
}

int* PemitbEnsureEmitterBlipFShaderSpan(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.fShaderSpan;
}

float* PemitbEnsureEmitterBlipDtShaderLoop(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.dtShaderLoop;
}

int* PemitbEnsureEmitterBlipFRandomFrame(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.fRandomFrame;
}

void* PemitbEnsureEmitterBlipmk(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.blipmk;
}

void* PemitbEnsureEmitterBlipok(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.blipok;
}

float* PemitbEnsureEmitterBlipRSFlying(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.rSFlying;
}

void* PemitbEnsureEmitterBliprk(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.bliprk;
}

LM* PemitbEnsureEmitterBlipLmSw(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.lmSw;
}

int* PemitbEnsureEmitterBlipFRandomRoll(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitp.emitblip.fRandomRoll;
}

int* PemitbEnsureEmitterBulletFDamage(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitx.bullet.fDamage;
}

int* PemitbEnsureEmitterBulletFRichochet(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitx.bullet.fRichochet;
}

OID* PemitbEnsureEmitterRemitOidExpls(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitx.remit.oidExpls;
}

float* PemitbEnsureEmitterRemitSvcParticle(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitx.remit.svcParticle;
}

float* PemitbEnsureEmitterShrapnelSRadius(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitx.shrapnel.sRadius;
}

float* PemitbEnsureEmitterShrapnelElas(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitx.shrapnel.elas;
}

float* PemitbEnsureEmitterShrapnelMu(EMITTER* pemitter, ENSK ensk)
{
	return &PemitbEnsureEmitter(pemitter, ensk)->emitx.shrapnel.mu;
}

void PauseEmitter(EMITTER* pemitter, float dtPause)
{
	pemitter->tUnpause = g_clock.t + dtPause;
}

void UnpauseEmitter(EMITTER* pemitter)
{
	pemitter->tUnpause = -1.0;
}

int FPausedEmitter(EMITTER* pemitter)
{
	if (g_clock.t < pemitter->tUnpause)
		return true;

	if (pemitter->emitrk > EMITRK_Nil && pemitter->emitrk < EMITRK_Burst && pemitter->cParticle == 0)
		return true;

	return false;
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

void* GetEmitterParticleCount(EMITTER* pemitter)
{
	return &pemitter->cParticle;
}

void PauseEmitterIndefinite(EMITTER* pemitter)
{
	pemitter->tUnpause = 3.402823e+38;
}

void RenderEmitterSelf(EMITTER* pemitter, CM* pcm, RO* pro)
{
	
}

void BindEmitterCallback(EMITTER* pemitter, MSGID msgid, void* pvData)
{
	pemitter->pvtalo->pfnSetAloParent(pemitter, pemitter->pemitb->emito.paloReference);
}

void BindEmitter(EMITTER* pemitter)
{
	BindAlo(pemitter);

	const bool needsSpecialBinding =
		pemitter->oidReference != OID_Nil ||
		pemitter->oidRender != OID_Nil ||
		pemitter->oidNextRender != OID_Nil ||
		pemitter->oidShape != OID_Nil ||
		pemitter->oidTouch != OID_Nil ||
		pemitter->pemitb->emito.emitok == EMITOK_Skeleton ||
		(pemitter->pemitb->emitp.emitpk == EMITPK_Blip &&
			pemitter->pemitb->emitp.emitblip.oidSplineTarget != OID_Nil);

	EMITB* pemitb = pemitter->pemitb.get();

	if (needsSpecialBinding)
	{
		pemitb = PemitbEnsureEmitter(pemitter, ENSK_Set);

		if (pemitter->oidReference != OID_Nil)
		{
			auto* paloReference = reinterpret_cast<ALO*>(PloFindSwObject(pemitter->psw, 0x104, (OID)pemitter->oidReference, pemitter));

			pemitb->emito.paloReference = paloReference;

			if (paloReference != nullptr)
				PostSwCallback(pemitter->psw, (PFNMQ)BindEmitterCallback, pemitter, MSGID_callback, nullptr);
		}

		if (pemitter->oidRender != OID_Nil)
			pemitb->emitp.emitrip.paloRender = reinterpret_cast<ALO*>(PloFindSwObject(pemitter->psw, 0x104, (OID)pemitter->oidRender, pemitter));

		if (pemitter->oidNextRender != OID_Nil)
			pemitb->emitp.emitrip.paloNextRender = reinterpret_cast<ALO*>(PloFindSwObject(pemitter->psw, 0x104, (OID)pemitter->oidNextRender, pemitter));

		if (pemitter->oidTouch != OID_Nil)
		{
			auto* psoTouch = reinterpret_cast<SO*>(PloFindSwObject(pemitter->psw, 0x104, (OID)pemitter->oidTouch, pemitter));

			if (FIsBasicDerivedFrom(reinterpret_cast<BASIC*>(psoTouch), CID_SO) != 0)
				pemitb->emitp.emitrip.psoTouch = psoTouch;
		}

		if (pemitter->oidShape != OID_Nil)
		{
			SHAPE* pshape = (SHAPE*)PloFindSwObject(pemitter->psw, 0x104, (OID)pemitter->oidShape, pemitter);

			if (pshape != nullptr)
			{
				// Original code copies the VECTOR at pshape[1].field0_0x0
				pemitb->emito.emitcrvOrigin.pcrv = pshape->pcrv;
				pemitb->emito.paloReference = pshape->paloParent;

			}
		}
	}

	BindEmitb(pemitb, pemitter);
}

void AddEmitoSkeleton(EMITO* pemito, OID oid, OID oidOther, float sRadius, float gDensity, float sRadiusOther, float gDensityOther, LO* ploContext)
{
	pemito->emitok = EMITOK_Skeleton;

	auto& askelp = pemito->skelOrigin.askelp;

	if (askelp.size() >= 32)
		return;

	SKELP& skelp = askelp.emplace_back();
	pemito->skelOrigin.cskelp = static_cast<int>(askelp.size());

	skelp.aoid[0] = oid;
	skelp.aoid[1] = oidOther;

	skelp.agDensity[0] = gDensity;
	skelp.agDensity[1] = gDensityOther >= 0.0f ? gDensityOther : gDensity;

	skelp.asRadius[0] = sRadius;
	skelp.asRadius[1] = sRadiusOther >= 0.0f ? sRadiusOther : sRadius;
}

void InitEmitb(EMITB* pemitb)
{
	if (pemitb == nullptr)
		return;

	// These are the values copied from s_emitoInit/s_emitvInit by the
	// original game.  The old port had decoded the packed EMITV initializer
	// as individual assignments to the wrong members (for example, 750 was
	// being written into damping/curl instead of the speed range).
	pemitb->cref = 1;

	pemitb->emito.emitok = EMITOK_Point;
	pemitb->emito.posOrigin = g_vecZero;
	pemitb->emito.emitnk = EMITNK_Normal;
	pemitb->emito.vec = g_normalX;
	pemitb->emito.lmSOffset = LM(0.0f, 0.0f);
	pemitb->emito.pemitolxf = nullptr;
	pemitb->emito.paloReference = nullptr;
	pemitb->emito.emitvk = EMITVK_Relative;
	pemitb->emito.vLocal = g_vecZero;

	pemitb->emitv.lmSv = LM(750.0f, 750.0f);
	pemitb->emitv.rSvz = 1.0f;
	pemitb->emitv.dv = glm::vec3(0.0f, 0.0f, -1500.0f);
	pemitb->emitv.rvDamping = -1.0f;
	pemitb->emitv.swCurl = 0.0f;
	pemitb->emitv.normalCurl = g_normalZ;
	pemitb->emitv.emitcnk = EMITCNK_World;
	pemitb->emitv.lmTilt = LM(0.0f, 0.0f);
	pemitb->emitv.cParticlePerRing = 200;
	pemitb->emitv.uRandomRad = 1.0f;
	pemitb->emitv.dtLifetime = 1.0f;
	pemitb->emitv.lmDtSkip = LM(0.0f, 0.0f);
	pemitb->emitv.lmDtBirth = LM(0.0f, 0.0f);
	pemitb->emitv.clqAlpha = { 1.0f, 0.0f, -1.0f, 0.0f };

	pemitb->emitp.emitpk = EMITPK_Blip;
	pemitb->emitp.emitblip.blipmk = BLIPMK_Accel;
	pemitb->emitp.emitblip.clqScale = { 1.0f, 0.0f, 0.0f, 0.0f };
	// RIP emitters multiply their render object by a randomly selected value
	// from this range.  A cleared EMITB otherwise produces zero-sized RIPs.
	pemitb->emitp.emitrip.lmGScale = LM(1.0f, 1.0f);
	pemitb->emitp.emitrip.ript = RIPT_Nil;
	pemitb->emitp.emitrip.riptTrail = RIPT_Nil;
	pemitb->pchzName = const_cast<char*>("Unknown Emitter");
}

void SetEmitbRipt(EMITB* pemitb, RIPT ript)
{
	pemitb->emitp.emitrip.ript = ript;
	pemitb->emitx = {};

	switch (ript)
	{
		case RIPT_Shrapnel:
		pemitb->emitx.shrapnel.sRadius = 25.0f;
		pemitb->emitx.shrapnel.elas = 0.8f;
		pemitb->emitx.shrapnel.mu = 0.8f;
		break;

		case RIPT_Emitter:
		pemitb->emitx.remit.oidExpls = OID_Nil;
		pemitb->emitx.remit.svcParticle = 10.0f;
		break;

		default:
		break;
	}
}

void SetEmitterRipt(EMITTER* pemitter, RIPT ript)
{
	EMITB* pemitb = PemitbEnsureEmitter(pemitter, ENSK_Set);
	SetEmitbRipt(pemitb, ript);
}

void BindEmitb(EMITB* pemitb, LO* ploContext)
{
	if (pemitb->emitp.emitpk == EMITPK_Blip)
	{
		OID oidSplineTarget = (OID)pemitb->emitp.emitblip.oidSplineTarget;

		if (oidSplineTarget != OID_Nil)
			pemitb->emitp.emitblip.pexploSplineTarget = reinterpret_cast<EXPLO*>(PloFindSwObject(ploContext->psw, 0x104, oidSplineTarget, ploContext));
	}

	if (pemitb->emito.emitok != EMITOK_Skeleton)
		return;

	auto& skel = pemitb->emito.skelOrigin;
	float totalWeight = 0.0f;

	for (int i = 0; i < skel.cskelp; ++i)
	{
		SKELP& skelp = skel.askelp[i];

		for (int j = 0; j < 2; ++j)
			skelp.apalo[j] = reinterpret_cast<ALO*>(PloFindSwObject(ploContext->psw, 0x104, (OID)skelp.aoid[j], ploContext));

		if (skelp.apalo[0] == nullptr || skelp.apalo[1] == nullptr)
		{
			skelp.s = 0.0f;
			skelp.gWeight = 0.0f;
			continue;
		}

		const glm::vec3 p0(skelp.apalo[0]->xf.posWorld.x, skelp.apalo[0]->xf.posWorld.y, skelp.apalo[0]->xf.posWorld.z);
		const glm::vec3 p1(skelp.apalo[1]->xf.posWorld.x, skelp.apalo[1]->xf.posWorld.y, skelp.apalo[1]->xf.posWorld.z);

		const float segmentLength = glm::length(p0 - p1);
		skelp.s = segmentLength;

		const float avgDensity = 0.5f * (skelp.agDensity[0] + skelp.agDensity[1]);
		skelp.gWeight = avgDensity * segmentLength;

		totalWeight += skelp.gWeight;
	}

	pemitb->emito.posOrigin.z = totalWeight;
	skel.gTotalWeight = totalWeight;
}

void SetEmitdvEmitb(EMITDV* pemitdv, EMITB* pemitb)
{
	if (pemitdv == nullptr || pemitb == nullptr)
		return;

	auto& emitv = pemitb->emitv;
	auto& emito = pemitb->emito;

	pemitdv->rvDamping = emitv.rvDamping;
	pemitdv->dv = emitv.dv;
	pemitdv->swCurl = emitv.swCurl;

	const float len = glm::length(emitv.normalCurl);
	pemitdv->normalCurl = (len < 0.0001f)
		? g_normalZ
		: (emitv.normalCurl / len);

	pemitdv->emitcnk = emitv.emitcnk;
	pemitdv->paloCurlRef = emito.paloReference;
}

void CalculateEmitdvMatrix(EMITDV* pemitdv, float dt, glm::mat4* pmat4Dv)
{
	glm::mat4 dampingAcceleration(1.0f);

	dampingAcceleration[0][0] = pemitdv->rvDamping;
	dampingAcceleration[1][1] = pemitdv->rvDamping;
	dampingAcceleration[2][2] = pemitdv->rvDamping;

	dampingAcceleration[3][0] = pemitdv->dv.x;
	dampingAcceleration[3][1] = pemitdv->dv.y;
	dampingAcceleration[3][2] = pemitdv->dv.z;

	glm::mat4 rotation(1.0f);

	if (pemitdv->swCurl != 0.0f)
	{
		glm::vec3 normal = pemitdv->normalCurl;

		if (pemitdv->emitcnk == EMITCNK_Local && pemitdv->paloCurlRef != nullptr)
			ConvertAloVec(pemitdv->paloCurlRef, nullptr, &pemitdv->normalCurl, &normal);

		const float normalLength = glm::length(normal);

		if (normalLength > 0.0001f)
			rotation = glm::rotate(glm::mat4(1.0f), pemitdv->swCurl * dt, normal / normalLength);
	}

	const glm::mat4 derivative = rotation * dampingAcceleration;
	*pmat4Dv = rotation + derivative * dt;
}

static void EnsureEmitterGroup(EMITTER* pemitter)
{
	if (pemitter == nullptr)
		return;
	if (pemitter->pemitb == nullptr)
		return;
	if (pemitter->oidGroup == OID_Nil)
		return;

	EMITTER* pGroupEmitter = reinterpret_cast<EMITTER*>(
		PloFindSwChild(pemitter->psw, static_cast<OID>(pemitter->oidGroup), nullptr));

	if (pGroupEmitter == nullptr)
	{
		pGroupEmitter = reinterpret_cast<EMITTER*>(
			PloNew(CID_EMITTER, pemitter->psw, nullptr,
				static_cast<OID>(pemitter->oidGroup), -1));
		if (pGroupEmitter == nullptr)
			return;

		pGroupEmitter->pemitb = std::make_shared<EMITB>(*pemitter->pemitb);
		pGroupEmitter->pemitb->emito.paloReference = nullptr;
		pGroupEmitter->emitrk = pemitter->emitrk;
		pGroupEmitter->cParticle = pemitter->cParticle;
		pGroupEmitter->lmSvcParticle = pemitter->lmSvcParticle;
		pGroupEmitter->fCountIsDensity = pemitter->fCountIsDensity;
		pGroupEmitter->uPauseProb = pemitter->uPauseProb;
		pGroupEmitter->lmDtPause = pemitter->lmDtPause;
		pGroupEmitter->cParticleConstant = pemitter->cParticleConstant;
		pGroupEmitter->fAutoPause = pemitter->fAutoPause;
		pGroupEmitter->svcParticle = pemitter->svcParticle;
		pGroupEmitter->dtRecalcSvc = pemitter->dtRecalcSvc;
		pGroupEmitter->tRecalcSvc = pemitter->tRecalcSvc;
		pGroupEmitter->rDensity = pemitter->rDensity;
		pGroupEmitter->fFixedPhys = 1;
		pGroupEmitter->fNoFreeze = 1;
		pGroupEmitter->xf.matWorld = pemitter->xf.mat;
		pGroupEmitter->xf.mat = pGroupEmitter->xf.matWorld;

		if (VTEMITTER* pvtemitter = pGroupEmitter->pvtemitter;
			pvtemitter != nullptr && pvtemitter->pfnBindEmitter != nullptr)
			pvtemitter->pfnBindEmitter(pGroupEmitter);
	}
	else if (pemitter->oid == 1526 && pGroupEmitter->pemitb != nullptr)
	{
		// This emitter group contains several contextual OID 1527 shapes.  The
		// water-tower member owns the closed curve, while earlier members can own
		// short open curves.  Object traversal order differs from the PS2 build,
		// so "first member wins" can select an open line for the shared particle
		// buffer.  Prefer the closed curve when it becomes available.
		CRV* const pcrvMember = pemitter->pemitb->emito.emitcrvOrigin.pcrv.get();
		CRV* const pcrvMaster =
			pGroupEmitter->pemitb->emito.emitcrvOrigin.pcrv.get();
		if (pcrvMember != nullptr && pcrvMember->fClosed != 0 &&
			(pcrvMaster == nullptr || pcrvMaster->fClosed == 0))
		{
			// The origin and velocity configuration are a matched template.  Copying
			// only EMITO leaves the straight-curve member's EMITV active and drives
			// particles above and below the otherwise horizontal closed ring.
			pGroupEmitter->pemitb = std::make_shared<EMITB>(*pemitter->pemitb);
			pGroupEmitter->pemitb->emito.paloReference = nullptr;
			pGroupEmitter->emitrk = pemitter->emitrk;
			pGroupEmitter->cParticle = pemitter->cParticle;
			pGroupEmitter->lmSvcParticle = pemitter->lmSvcParticle;
			pGroupEmitter->fCountIsDensity = pemitter->fCountIsDensity;
			pGroupEmitter->uPauseProb = pemitter->uPauseProb;
			pGroupEmitter->lmDtPause = pemitter->lmDtPause;
			pGroupEmitter->cParticleConstant = pemitter->cParticleConstant;
			pGroupEmitter->fAutoPause = pemitter->fAutoPause;
			pGroupEmitter->svcParticle = pemitter->svcParticle;
			pGroupEmitter->dtRecalcSvc = pemitter->dtRecalcSvc;
			pGroupEmitter->tRecalcSvc = pemitter->tRecalcSvc;
			pGroupEmitter->rDensity = pemitter->rDensity;
			// The PS2's traversal creates this synthetic group from the water-tower
			// member, so the group owns both its closed EMITB template and its local
			// transform.  When our different traversal promotes that member later,
			// promote the transform as well.  Otherwise ConvertEmitoPosVec uses the
			// first (straight-line) member's frame for the circular particles.
			pGroupEmitter->xf.matWorld = pemitter->xf.mat;
			pGroupEmitter->xf.mat = pGroupEmitter->xf.matWorld;
		}
	}

	if (!FFindDlEntry(&pGroupEmitter->dlGroup, pemitter))
		AppendDlEntry(&pGroupEmitter->dlGroup, pemitter);
	InheritEmitterGrfzon(pemitter);
	pemitter->fValuesChanged = 0;

}

void PostEmitterLoad(EMITTER* pemitter)
{
	PostAloLoad(pemitter);

	if (pemitter->pemitb->emitp.emitrip.ript != RIPT_Nil)
		pemitter->pemitb->emitp.emitpk = EMITPK_Rip;

	if (pemitter->emitrk == EMITRK_Nil)
	{
		if (pemitter->uPauseProb == 0.0f && pemitter->fAutoPause == 0)
			pemitter->emitrk = (pemitter->cParticleConstant == 0.0f) ? EMITRK_Continuous : EMITRK_ConstantCount;
		else
			pemitter->emitrk = EMITRK_BurstOld;
	}

	switch (pemitter->emitrk)
	{
		case EMITRK_Burst:
		{
			pemitter->tRecalcSvc = FLT_MAX;
			pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);

			if (pemitter->fAutoPause != 0)
				PauseEmitterIndefinite(pemitter);
			else if (GRandInRange(0.0f, 1.0f) <= pemitter->uPauseProb)
			{
				const float dtPause = GRandInRange(pemitter->lmDtPause.gMin, pemitter->lmDtPause.gMax);
				PauseEmitter(pemitter, dtPause);
			}
			break;
		}

		case EMITRK_Continuous:
		case EMITRK_ConstantCount:
		{
			pemitter->dtRecalcSvc = 1.0f;
			pemitter->tRecalcSvc  = 1.0f;
			pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);
			break;
		}

		case EMITRK_BurstOld:
		{
			pemitter->tRecalcSvc = FLT_MAX;
			pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax) * 60.0f;

			if (pemitter->fAutoPause != 0)
				PauseEmitterIndefinite(pemitter);
			else if (GRandInRange(0.0f, 1.0f) <= pemitter->uPauseProb)
			{
				const float dtPause = GRandInRange(pemitter->lmDtPause.gMin, pemitter->lmDtPause.gMax);
				PauseEmitter(pemitter, dtPause);
			}
			break;
		}

		default:
		break;
	}

	pemitter->rDensity = 1.0f;

	EMITB* pemitb = pemitter->pemitb.get();

	if (pemitb->emito.emitok == EMITOK_Box)
	{
		const BOX& box = pemitb->emito.boxOrigin;

		const glm::vec3 bmin = pemitb->emito.posOrigin;
		const glm::vec3 bmax(box.posMax.x, box.posMax.y, box.posMax.z);
		const glm::vec3 size = glm::abs(bmax - bmin);

		if (pemitter->oidReference != OID_Nil)
			pemitter->sBoxRadius = 0.5f * glm::compMax(size);

		if (pemitter->fCountIsDensity != 0)
		{
			if (pemitter->cParticleConstant == 0.0f)
				pemitter->rDensity = size.y * size.z * 0.0001f;
			else
				pemitter->rDensity = size.x * size.y * size.z * 1.0e-6f;
		}
	}

	// Do not snapshot a grouped emitter here.  Object-reference/shape binding
	// happens after post-load in ProjectCane, so its EMITB can still contain the
	// serialized placeholder curve at this point.  UpdateEmitter calls
	// EnsureEmitterGroup after binding has completed, matching the effective
	// retail ordering and letting the synthetic master copy the resolved shape.
}

void HandleEmitterMessage(EMITTER* pemitter, MSGID msgid, void* pv)
{
	if (msgid != MSGID_removed)
		return;

	if (pv == pemitter->pripg)
	{
		RIPG* pripg = static_cast<RIPG*>(pv);
		pripg->pvtlo->pfnUnsubscribeLoObject(pripg, pemitter);
		pemitter->pripg = nullptr;
	}
	else if (pv == pemitter->pblipg)
	{
		BLIPG* pblipg = static_cast<BLIPG*>(pv);
		pblipg->pvtlo->pfnUnsubscribeLoObject(pblipg, pemitter);
		pemitter->pblipg = nullptr;
	}
}

void OnEmitterValuesChanged(EMITTER* pemitter)
{
	if (pemitter == nullptr)
		return;

	pemitter->fValuesChanged = 0;

	if (pemitter->pblipg != nullptr)
		SetBlipgEmitb(pemitter->pblipg, pemitter->pemitb.get());

	if (pemitter->pripg != nullptr)
		SetRipgEmitb(pemitter->pripg, pemitter->pemitb.get());

	float svc = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);

	if (pemitter->emitrk == EMITRK_BurstOld)
		svc *= 60.0f;

	pemitter->svcParticle = svc;
}

void EmitParticles(int cParticle, EMITB* pemitb, EMITG* pemitg)
{
	if (cParticle == 0)
		return;

	EMITGEN emitgen{};
	EMITGEN emitgenTarget{};

	emitgen.fConvertPosVec = 1;
	OriginateParticles(cParticle, pemitb, &emitgen);

	
	if (pemitb->emitp.emitpk == EMITPK_Rip)
		EmitRips(pemitb, pemitg, cParticle, emitgen.apos.data(), emitgen.av.data(), emitgen.atCreated.data(), emitgen.atDestroy.data());
	else 
	{
		if (pemitb->emitp.emitblip.blipmk == BLIPMK_Spline)
			OriginateSplineSinkParticles(cParticle, pemitb, &emitgen, &emitgenTarget);

		EmitBlips(pemitb, pemitg, cParticle, emitgen.apos.data(), emitgen.av.data(), emitgen.atCreated.data(), emitgen.atDestroy.data(), emitgenTarget.apos.data(), emitgenTarget.av.data());
	}
}

void EmitRipsSphere(glm::vec3* ppos, glm::vec3* pnormal, int crip, EMITRIP* pemitrip, EMITV* pemitv, RIPG** ppripg, LO* ploSubscribe)
{
	// This is a temporary emitter with no local transform. Value-initialize it
	// so emito.pemitolxf and the other optional pointers start as nullptr.
	EMITB emitb{};
	InitEmitb(&emitb);

	EMITG emitg{};

	emitb.emito.posOrigin = *ppos;
	emitb.emito.vec = *pnormal;
	emitb.emitv = *pemitv;

	emitb.emitp.emitpk = EMITPK_Rip;
	emitb.emitp.emitrip = *pemitrip;

	emitg.ppripg = ppripg;
	emitg.ploSubscribe = ploSubscribe;

	EmitParticles(crip, &emitb, &emitg);
}

void ModifyEmitterParticles(EMITTER* pemitter)
{
	if (!pemitter || !pemitter->pemitb)
		return;

	const float radius = pemitter->sBoxRadius;

	if (radius <= 0.0001f)
		return;

	if (pemitter->pemitb->emitp.emitpk != EMITPK_Blip || !pemitter->pblipg)
		return;

	const ALO* paloReference = pemitter->pemitb->emito.paloReference;
	const glm::vec3 center = paloReference ? paloReference->xf.posWorld : glm::vec3(0.0f);
	const glm::vec3 boxMin = center - glm::vec3(radius);
	const glm::vec3 boxMax = center + glm::vec3(radius);
	const glm::vec3 boxSize(radius * 2.0f);

	for (BLIP* pblip = pemitter->pblipg->dlBlip.pblipFirst; pblip; pblip = pblip->dle.pblipNext)
	{
		BLIPF& blipf = pblip->ablipf[pblip->iblipfLatest];

		for (int iblip = 0; iblip < pblip->cblipe; ++iblip)
		{
			BLIPP& blipp = blipf.ablipp[iblip];

			if (blipp.x < boxMin.x || blipp.x > boxMax.x)
				blipp.x = boxMin.x + GModPositive(blipp.x - boxMin.x, boxSize.x);

			if (blipp.y < boxMin.y || blipp.y > boxMax.y)
				blipp.y = boxMin.y + GModPositive(blipp.y - boxMin.y, boxSize.y);

			if (blipp.z < boxMin.z || blipp.z > boxMax.z)
				blipp.z = boxMin.z + GModPositive(blipp.z - boxMin.z, boxSize.z);
		}
	}
}

void OriginateParticles(int cParticle, EMITB* pemitb, EMITGEN* pemitgen)
{
	pemitgen->apos.resize(cParticle);
	pemitgen->av.resize(cParticle);
	pemitgen->atCreated.resize(cParticle);
	pemitgen->atDestroy.resize(cParticle);

	EMITVX emitvx{};
	CalculateEmitvx(pemitb->emitv.cParticlePerRing, &pemitb->emitv.lmTilt, cParticle, &emitvx);

	if (cParticle <= 0) {
		return;
	}

	EMITO* emito = &pemitb->emito;

	for (int i = 0; i < cParticle; ++i)
	{
		glm::vec3 normal{};

		// 1. Choose spawn position + surface normal
		ChooseEmitoPos(emito, i, cParticle, &pemitgen->apos[i], &normal);

		// 2. Choose velocity + lifetime
		ChooseEmitvVelocityAge(&pemitb->emitv, &emitvx, emito, i, &pemitgen->apos[i], &normal, &pemitgen->av[i], &pemitgen->atCreated[i], &pemitgen->atDestroy[i]);

		// 3. Apply local emitter transform (if present)
		EMITOLXF* localXf = emito->pemitolxf;

		if (localXf != nullptr)
		{
			glm::vec3 localPos = pemitgen->apos[i];
			glm::vec3 localVel = pemitgen->av[i];

			const glm::mat3& R = localXf->matLocal;
			const glm::vec3& t = localXf->posLocal;
			const glm::vec3& v = localXf->vLocal;
			const glm::vec3& w = localXf->wLocal;

			glm::vec3 worldPos = R * localPos + t;
			glm::vec3 offset = worldPos - t;

			glm::vec3 worldVel = (R * localVel) + v + glm::cross(w, offset);

			pemitgen->apos[i] = worldPos;
			pemitgen->av[i] = worldVel;
		}

		// 4. Optional conversion step
		if (pemitgen->fConvertPosVec != 0)
			ConvertEmitoPosVec(emito, &pemitgen->apos[i], &pemitgen->av[i]);
	}
}

void OriginateSplineSinkParticles(int cParticle, EMITB* pemitb, EMITGEN* emitgen, EMITGEN* emitgenTarget)
{
	if (!pemitb || !emitgen || !emitgenTarget || cParticle <= 0)
		return;

	EXPLO* pexplo = pemitb->emitp.emitblip.pexploSplineTarget;

	if (!pexplo || !pexplo->pemitb)
		return;

	EMITO& emitoTarget = pexplo->pemitb->emito;
	ALO* paloReferenceHold = emitoTarget.paloReference;

	EMITOLXF emitolxf{};

	// The decompiler's &_heap_size comparison is almost certainly the
	// special "use explosion parent" reference value.
	if (pexplo->oidReference == OID_Nil)
		emitoTarget.paloReference = pexplo->paloParent;

	if (emitoTarget.paloReference == pexplo->paloParent)
	{
		CalculateExplTransform(pexplo, &emitolxf.posLocal, &emitolxf.matLocal);
		emitoTarget.pemitolxf = &emitolxf;
	}

	emitgenTarget->fConvertPosVec = false;

	OriginateParticles(cParticle, pexplo->pemitb.get(), emitgenTarget);

	emitoTarget.pemitolxf = nullptr;

	float dtPredictedPrevious = std::numeric_limits<float>::max();

	glm::vec3 posBasis(0.0f);
	glm::vec3 vBasis(0.0f);
	glm::vec3 wBasis(0.0f);
	glm::mat3 matBasis(1.0f);

	for (int iParticle = 0; iParticle < cParticle; ++iParticle)
	{
		if (emitoTarget.paloReference)
		{
			const float dtPredicted = emitgen->atDestroy[iParticle] - g_clock.t;

			if (!FFloatsNear(dtPredicted, dtPredictedPrevious, 0.0001f))
				PredictAloTransform(emitoTarget.paloReference, nullptr, dtPredicted, &posBasis, &matBasis, &vBasis, &wBasis);

			glm::vec3& posTarget = emitgenTarget->apos[iParticle];
			glm::vec3& vTarget = emitgenTarget->av[iParticle];

			const glm::vec3 posRotated = matBasis * posTarget;
			posTarget = posBasis + posRotated;

			if (emitoTarget.emitvk != EMITVK_World)
			{
				vTarget = matBasis * vTarget;

				if (emitoTarget.emitvk != EMITVK_NoRelative)
				{
					vTarget += vBasis;

					const glm::vec3& posRelative =
						emitoTarget.emitvk == EMITVK_RelativeSpin
						? posTarget
						: posRotated;

					vTarget += glm::cross(wBasis, posRelative);
				}
			}

			dtPredictedPrevious = dtPredicted;
		}

		// Sink particles travel opposite the target emitter's velocity.
		emitgenTarget->av[iParticle] *= -1.0f;
	}

	emitoTarget.paloReference = paloReferenceHold;
}

void CalculateEmitvx(int cParticlePerRing, LM* plmTilt, int cParticle, EMITVX* pemitvx)
{
	if (cParticlePerRing < 1) {
		pemitvx->cParticlePerRing = cParticle;
	}
	else {
		if (cParticlePerRing > cParticle) {
			cParticlePerRing = cParticle;
		}

		pemitvx->cParticlePerRing = cParticlePerRing;
	}

	const int particlesPerRing = pemitvx->cParticlePerRing;
	if (particlesPerRing == 0) {
		return;
	}

	const float tiltMin = plmTilt->gMin;
	const float tiltMax = plmTilt->gMax;
	const float tiltSpan = RadNormalize(tiltMax - tiltMin);

	const int ringCount = (cParticle - 1 + particlesPerRing) / particlesPerRing;
	const float tiltStep = tiltSpan / static_cast<float>(ringCount);

	pemitvx->dradPanSlice = glm::two_pi<float>() / static_cast<float>(particlesPerRing);
	pemitvx->dradTiltRing = tiltStep;
	pemitvx->radTiltMin = tiltMin + tiltStep * 0.5f;
	pemitvx->radPanMin = GRandInRange(0.0f, glm::two_pi<float>());
}

void ChooseEmitoPos(EMITO* pemito, int iParticle, int cParticle, glm::vec3* pposRet, glm::vec3* pnormalRet)
{
	if (!pemito || !pposRet || !pnormalRet || cParticle <= 0)
		return;

	*pposRet = glm::vec3(0.0f);
	*pnormalRet = g_normalX;

	bool fNormalReady = false;

	switch (pemito->emitok)
	{
		case EMITOK_Point:
		{
			*pposRet = pemito->posOrigin;
			break;
		}

		case EMITOK_Box:
		{
			const glm::vec3& posMin = pemito->posOrigin;
			const glm::vec3& posMax = pemito->boxOrigin.posMax;

			pposRet->x = GRandInRange(posMin.x, posMax.x);
			pposRet->y = GRandInRange(posMin.y, posMax.y);
			pposRet->z = GRandInRange(posMin.z, posMax.z);
			break;
		}

		case EMITOK_Curve:
		{
			CRV* pcrv = pemito->emitcrvOrigin.pcrv.get();

			if (!pcrv || !pcrv->pvtcrv)
				break;

			if (pcrv->ccv <= 0 || pcrv->mpicvs.empty())
				break;

			auto pfnEvaluateCrvFromS =
				pcrv->pvtcrv->pfnEvaluateCrvFromS;

			if (!pfnEvaluateCrvFromS)
				break;

			const float sMax = pcrv->mpicvs[pcrv->ccv - 1];
			const float dsParticle = sMax / static_cast<float>(cParticle);
			const float sMin = static_cast<float>(iParticle) * dsParticle;
			const float s = GRandInRange(sMin, sMin + dsParticle);

			switch (pemito->emitnk)
			{
				case EMITNK_CurveTangent:
				{
					pfnEvaluateCrvFromS(pcrv, s, pposRet, pnormalRet);
					fNormalReady = true;
					break;
				}

				case EMITNK_CurveNormal:
				{
					glm::vec3 tangent(0.0f);

					pfnEvaluateCrvFromS(pcrv, s, pposRet, &tangent);
					*pnormalRet = glm::cross(tangent, pemito->vec);
					fNormalReady = true;
					break;
				}

				default:
				{
					pfnEvaluateCrvFromS(pcrv, s, pposRet, nullptr);
					break;
				}
			}

			break;
		}

		case EMITOK_Skeleton:
		{
			SKEL& skeleton = pemito->skelOrigin;

			if (skeleton.cskelp <= 0 || skeleton.askelp.empty())
				break;

			const float weightPerParticle = skeleton.gTotalWeight / static_cast<float>(cParticle);

			float weight = GRandInRange(
				static_cast<float>(iParticle) * weightPerParticle,
				static_cast<float>(iParticle + 1) * weightPerParticle);

			SKELP* pskelp = &skeleton.askelp[0];

			for (int iskelp = 0; iskelp < skeleton.cskelp; ++iskelp)
			{
				pskelp = &skeleton.askelp[iskelp];

				if (weight < pskelp->gWeight)
					break;

				weight -= pskelp->gWeight;
			}

			if (!pskelp->apalo[0] || !pskelp->apalo[1])
				break;

			const glm::vec3 posStart = pskelp->apalo[0]->xf.posWorld;
			const glm::vec3 posEnd = pskelp->apalo[1]->xf.posWorld;

			glm::vec3 axis = posEnd - posStart;
			const float axisLength = glm::length(axis);

			if (axisLength < 0.0001f)
				axis = g_normalX;
			else
				axis /= axisLength;

			glm::vec3 basisX(0.0f);
			glm::vec3 basisY(0.0f);

			GetNormalVectors(axis, basisX, basisY, g_normalX, g_normalZ);

			float distance = 0.0f;

			if (pskelp->agDensity[0] == pskelp->agDensity[1])
			{
				if (std::abs(pskelp->agDensity[0]) > 0.0001f)
					distance = weight / pskelp->agDensity[0];
			}
			else
			{
				float solutions[2]{};
				CSolveQuadratic(((pskelp->agDensity[1] - pskelp->agDensity[0]) * 0.5f) / pskelp->s, pskelp->agDensity[0], -weight, solutions);

				distance = solutions[0];
			}

			const float u = std::abs(pskelp->s) > 0.0001f ? distance / pskelp->s : 0.0f;
			const float angle = GRandInRange(-glm::pi<float>(), glm::pi<float>());
			const float radius = glm::mix(pskelp->asRadius[0], pskelp->asRadius[1], u);

			glm::vec3 cylindrical;
			cylindrical.x = std::cos(angle) * radius;
			cylindrical.y = std::sin(angle) * radius;
			cylindrical.z = distance;

			const glm::mat3 basis(basisX, basisY, axis);
			glm::vec3 posWorld = posStart + basis * cylindrical;

			ConvertAloPos(nullptr, pemito->paloReference, &posWorld, pposRet);

			if (pemito->emitnk == EMITNK_CurveTangent)
			{
				ConvertAloVec(nullptr, pemito->paloReference, &axis, pnormalRet);
				fNormalReady = true;
			}
			else if (pemito->emitnk == EMITNK_CurveNormal)
			{
				glm::vec3 normal = glm::cross(axis, pemito->vec);
				const float normalLength = glm::length(normal);

				if (normalLength < 0.0001f)
					normal = g_normalX;
				else
					normal /= normalLength;

				ConvertAloVec(nullptr, pemito->paloReference, &normal, pnormalRet);
				fNormalReady = true;
			}

			break;
		}

		case EMITOK_Mesh:
		{
			EMITMESH& mesh = pemito->emitmeshOrigin;

			if (mesh.cemittri <= 0 || mesh.aemittri.empty() || mesh.apos.empty())
				break;

			float remainingArea = GRandInRange(0.0f, mesh.sTotalArea);
			EMITTRI* pemittri = &mesh.aemittri[0];

			for (int iemittri = 0; iemittri < mesh.cemittri; ++iemittri)
			{
				pemittri = &mesh.aemittri[iemittri];
				remainingArea -= pemittri->sArea;

				if (remainingArea <= 0.0f)
					break;
			}

			const glm::vec3& pos1 = mesh.apos[pemittri->aipos[0]];
			const glm::vec3& pos2 = mesh.apos[pemittri->aipos[1]];
			const glm::vec3& pos3 = mesh.apos[pemittri->aipos[2]];

			const float u = GRandInRange(0.0f, 1.0f);
			const float remaining = 1.0f - u;
			const float v = GRandInRange(0.0f, remaining);
			const float w = remaining - v;

			*pposRet = pos1 * u + pos2 * v + pos3 * w;

			if (pemito->emitnk == EMITNK_MeshNormal)
			{
				*pnormalRet = glm::cross(pos2 - pos1, pos3 - pos1);
				fNormalReady = true;
			}

			break;
		}

		default:
		break;
	}

	if (!fNormalReady)
	{
		switch (pemito->emitnk)
		{
			case EMITNK_Radial:
			*pnormalRet = *pposRet - pemito->vec;
			break;

			case EMITNK_Normal:
			*pnormalRet = pemito->vec;
			break;

			case EMITNK_Screen:
			{
				const glm::vec3 cameraNormal = glm::mat3(g_pcm->mat) * pemito->vec;

				if (pemito->paloReference)
					*pnormalRet = glm::transpose(glm::mat3(pemito->paloReference->xf.matWorld)) * cameraNormal;
				else
					*pnormalRet = cameraNormal;

				break;
			}

			default:
			*pnormalRet = g_normalX;
			break;
		}
	}

	const float normalLength = glm::length(*pnormalRet);

	if (normalLength < 0.0001f)
		*pnormalRet = g_normalX;
	else
		*pnormalRet /= normalLength;

	const float offset = GRandInRange(pemito->lmSOffset.gMin, pemito->lmSOffset.gMax);
	*pposRet += *pnormalRet * offset;
}

void ChooseEmitvVelocityAge(EMITV* pemitv, EMITVX* pemitvx, EMITO* pemito, int iParticle, glm::vec3* ppos, glm::vec3* pnormal, glm::vec3* pv, float* ptCreated, float* ptDestroy)
{
	// Choose velocity
	ChooseEmitVelocity(pemitvx, pemitv->uRandomRad, pemitv->rSvz, &pemitv->lmSv, pnormal, iParticle, pv);

	// Birth time offset
	float dtBirth = GRandInRange(pemitv->lmDtBirth.gMin, pemitv->lmDtBirth.gMax);

	float tCreated = g_clock.t - dtBirth;

	*ptCreated = tCreated;
	*ptDestroy = tCreated + pemitv->dtLifetime;

	// Skip time (advance particle immediately)
	float dtSkip = GRandInRange(pemitv->lmDtSkip.gMin, pemitv->lmDtSkip.gMax);

	if (dtSkip != 0.0f) {
		*ppos += (*pv) * dtSkip;
	}
}

void ChooseEmitVelocity(EMITVX* pemitvx, float uRandom, float rSvz, LM* plmSv, glm::vec3* pvecNormal, int iParticle, glm::vec3* pv)
{
	const int particlesPerRing = pemitvx->cParticlePerRing;

	if (particlesPerRing == 0) {
		return;
	}

	const int ringIndex = iParticle / particlesPerRing;
	const int sliceIndex = iParticle % particlesPerRing;

	float tilt = pemitvx->radTiltMin +
		static_cast<float>(ringIndex) * pemitvx->dradTiltRing;

	float pan = pemitvx->radPanMin +
		static_cast<float>(sliceIndex) * pemitvx->dradPanSlice;

	if (uRandom != 0.0f) {
		tilt += uRandom * GRandInRange(-0.5f, 0.5f) * pemitvx->dradTiltRing;
		pan += uRandom * GRandInRange(-0.5f, 0.5f) * pemitvx->dradPanSlice;
	}

	const float sphereTilt = RadNormalize(tilt + glm::half_pi<float>());
	const float spherePan = RadNormalize(pan);

	const float speed = GRandInRange(plmSv->gMin, plmSv->gMax);

	glm::vec3 localVelocity{};
	SetVectorSphere(&localVelocity, spherePan, sphereTilt, speed);

	// Original code scales the local Z contribution before basis transform.
	localVelocity.z *= rSvz;

	// Build an orthonormal basis around the supplied normal.
	glm::vec3 tangent{};
	glm::vec3 bitangent{};
	GetNormalVectors(*pvecNormal, tangent, bitangent, g_normalX, g_normalZ);

	// Transform from local emitter space to world space.
	// local.x -> tangent
	// local.y -> bitangent
	// local.z -> normal
	*pv =
		tangent * localVelocity.x +
		bitangent * localVelocity.y +
		(*pvecNormal) * localVelocity.z;
}

void ConvertEmitoPosVec(EMITO* pemito, glm::vec3* ppos, glm::vec3* pv)
{
	if (pemito->paloReference == nullptr) {
		return;
	}

	// Always convert the spawn position into world space if we have a reference ALO.
	ConvertAloPos(pemito->paloReference, nullptr, ppos, ppos);

	// If velocity is already world-space, nothing else to do.
	if (pemito->emitvk == EMITVK_World)
		return;

	// Convert the velocity vector by the reference transform.
	ConvertAloVec(pemito->paloReference, nullptr, pv, pv);

	// Some modes stop after the vector-space conversion.
	if (pemito->emitvk == EMITVK_NoRelative)
		return;

	glm::vec3 movement{};

	// Original code switches which position is used when calculating inherited motion.
	glm::vec3* pMovementPos = ppos;
	if (pemito->emitvk != EMITVK_RelativeSpin)
		pMovementPos = &pemito->paloReference->xf.posWorld;

	CalculateAloMovement(pemito->paloReference, nullptr, *pMovementPos, &movement, nullptr, nullptr, nullptr);

	// Inherit motion from the reference object.
	*pv += movement;
}

void UpdateEmitter(EMITTER* pemitter, float dt)
{
	EMITG emitg{};
	int particleCountToEmit = 0;

	if (pemitter->fValuesChanged != 0)
		OnEmitterValuesChanged(pemitter);

	UpdateAlo(pemitter, dt);

	// Group members do not emit directly. Ensure their synthetic master and
	// intrusive membership exist before taking the member early-return. This
	// also covers objects created after the world's post-load traversal.
	if (pemitter->oidGroup != OID_Nil)
		EnsureEmitterGroup(pemitter);

	if (pemitter->oidGroup != OID_Nil)
	{
		return;
	}

	if (!FIsDlEmpty(&pemitter->dlGroup))
	{
		EMITTER* groupEmitter = reinterpret_cast<EMITTER*>(pemitter->dlGroup.pblipFirst);

		if (groupEmitter == nullptr) {
			return;
		}

		while (groupEmitter != nullptr) {
			if ((groupEmitter->cpaloFindSwObjects & 2) == 0) {
				if (FIsLoInWorld(reinterpret_cast<LO*>(groupEmitter)) != 0) {
					if (groupEmitter->paloParent == nullptr)
						goto emitter_is_relevant;

					float maxRelevantDist = groupEmitter->sMRD * g_pcm->rMRDAdjust;

					glm::vec3 delta = groupEmitter->xf.posWorld - g_pcm->pos;
					float distSq = glm::dot(delta, delta);
					float maxDistSq = maxRelevantDist * maxRelevantDist;

					if (distSq < maxDistSq) {
						goto emitter_is_relevant;
					}
				}
			}

			groupEmitter = reinterpret_cast<EMITTER*>(groupEmitter->dleGroup.pblipNext);
		}

		return;
	}

	if (pemitter->paloParent != nullptr) 
	{
		float maxRelevantDist = pemitter->sMRD * g_pcm->rMRDAdjust;

		/*glm::vec3 cameraPos;
		if (g_cmlk == CMLK_Mrd) {
			cameraPos = g_posCmLock;
		}
		else {
			cameraPos = g_pcm->pos;
		}*/

		glm::vec3 delta = pemitter->xf.posWorld - g_pcm->pos;
		float distSq = glm::dot(delta, delta);
		float maxDistSq = maxRelevantDist * maxRelevantDist;

		if (distSq > maxDistSq) {
			return;
		}
	}

	emitter_is_relevant:

	if (FPausedEmitter(pemitter) != 0) {
		return;
	}

	EMITRK emitMode = pemitter->emitrk;
	particleCountToEmit = 0;

	if (emitMode == EMITRK_ConstantCount) 
	{
		float density = pemitter->rDensity;
		float constantCount = pemitter->cParticleConstant;
		int liveParticleCount = 0;

		if (pemitter->pemitb->emitp.emitpk == EMITPK_Blip) {
			if (pemitter->pblipg != nullptr) {
				liveParticleCount = pemitter->pblipg->cblipe;
			}
		}
		else {
			if (pemitter->pripg != nullptr) {
				liveParticleCount = CPvDl(&pemitter->pripg->dlRip);
			}
		}

		int wantedCount = static_cast<int>(constantCount * density) - liveParticleCount;
		if (wantedCount > 0) {
			particleCountToEmit = wantedCount;
		}
	}

	else if (emitMode == EMITRK_Continuous) 
	{
		float accumulated = pemitter->uParticle + pemitter->svcParticle * pemitter->rDensity * dt;
		particleCountToEmit = static_cast<int>(accumulated);
		pemitter->uParticle = accumulated - static_cast<float>(particleCountToEmit);

		if (particleCountToEmit != 0) {
			int remaining = pemitter->cParticle;
			if (remaining >= 0) {
				if (remaining <= particleCountToEmit) {
					particleCountToEmit = remaining;
				}
				pemitter->cParticle = remaining - particleCountToEmit;
			}
		}
	}
	else if (emitMode == EMITRK_Burst) {
		int remaining = pemitter->cParticle;
		particleCountToEmit = static_cast<int>(pemitter->svcParticle);

		if (remaining >= 0) {
			if (remaining <= particleCountToEmit) {
				particleCountToEmit = remaining;
			}
			pemitter->cParticle = remaining - particleCountToEmit;
		}

		if (pemitter->fAutoPause == 0) {
			float roll = GRandInRange(0.0f, 1.0f);

			if (roll <= pemitter->uPauseProb) {
				float pauseDt = GRandInRange(pemitter->lmDtPause.gMin, pemitter->lmDtPause.gMax);
				PauseEmitter(pemitter, pauseDt);
			}

			pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);
		}
		else {
			PauseEmitterIndefinite(pemitter);
			pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);
		}
	}
	else if (emitMode == EMITRK_BurstOld) {
		float accumulated = pemitter->uParticle + pemitter->svcParticle * pemitter->rDensity * dt;
		particleCountToEmit = static_cast<int>(accumulated);
		pemitter->uParticle = accumulated - static_cast<float>(particleCountToEmit);

		if (particleCountToEmit > 1) {
			int remaining = pemitter->cParticle;
			if (remaining >= 0) {
				if (remaining <= particleCountToEmit) {
					particleCountToEmit = remaining;
				}
				pemitter->cParticle = remaining - particleCountToEmit;
			}

			if (pemitter->fAutoPause == 0) {
				float roll = GRandInRange(0.0f, 1.0f);

				if (roll <= pemitter->uPauseProb) {
					float pauseDt = GRandInRange(pemitter->lmDtPause.gMin, pemitter->lmDtPause.gMax);
					PauseEmitter(pemitter, pauseDt);
					pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax) * 60.0f;
				}
			}
			else {
				PauseEmitterIndefinite(pemitter);
				pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax) * 60.0f;
			}
		}
	}

	if (particleCountToEmit != 0) 
	{
		emitg.ppripg = &pemitter->pripg;
		emitg.ppblipg = &pemitter->pblipg;

		ALO* oldReference = pemitter->pemitb->emito.paloReference;
		if (pemitter->oidReference == OID_Nil)
			pemitter->pemitb->emito.paloReference = pemitter;

		emitg.ploSubscribe = pemitter;
		EmitParticles(particleCountToEmit, pemitter->pemitb.get(), &emitg);
		pemitter->pemitb->emito.paloReference = oldReference;
	}

	if (pemitter->tRecalcSvc <= g_clock.t) 
	{
		pemitter->svcParticle = GRandInRange(pemitter->lmSvcParticle.gMin, pemitter->lmSvcParticle.gMax);
		pemitter->tRecalcSvc = g_clock.t + pemitter->dtRecalcSvc;
	}

	ModifyEmitterParticles(pemitter);
}

void InheritEmitterGrfzon(EMITTER* pemitter)
{
	if (pemitter->viss == 2)
		return;

	for (ALO* palo = pemitter->paloParent; palo != nullptr; palo = palo->paloParent)
	{
		if (palo->viss == 2)
		{
			pemitter->grfzon = palo->grfzon;
			return;
		}
	}
}

void DeleteEmitter(EMITTER *pemitter)
{
	delete pemitter;
}

void StockSplashBig(glm::vec3* ppos, float gScale, SO* psoTouch)
{
	if (ppos == nullptr || g_psw == nullptr || gScale <= 0.0f)
		return;

	// OID_stock_splash is an EXPLG whose children require different override
	// flags. Scaling the whole group also scales its BLIP sheets, producing the
	// enormous screen-filling splash.
	EXPLG* const pexplgSplash = reinterpret_cast<EXPLG*>(g_psw->aploStock[0]);

	if (pexplgSplash == nullptr || pexplgSplash->cpexpl <= 0)
		return;

	for (int ipexpl = 0; ipexpl < pexplgSplash->cpexpl; ++ipexpl)
	{
		EXPLS* const pexpls = reinterpret_cast<EXPLS*>(pexplgSplash->apexpl[ipexpl]);

		if (pexpls == nullptr || pexpls->pemitb == nullptr ||
			pexpls->pvtexpl == nullptr || pexpls->pvtexpl->pfnExplodeExplExplso == nullptr)
			continue;

		EXPLSO explso{};
		explso.grfexplso = 4; // Explicit world-space origin.
		explso.posOrigin = *ppos;

		if (pexpls->pemitb->emitp.emitpk == EMITPK_Rip)
		{
			const RIPT ript = pexpls->pemitb->emitp.emitrip.ript;

			if (ript == RIPT_Droplet)
			{
				explso.grfexplso = 4 | 32;
				explso.psoTouch = psoTouch;
			}
			else if (ript == RIPT_Ripple)
			{
				explso.grfexplso = 4 | 16;
				explso.rScale = gScale;
			}
		}

		pexpls->pvtexpl->pfnExplodeExplExplso(reinterpret_cast<EXPLO*>(pexpls), &explso);
	}
}
