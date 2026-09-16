#include "expls.h"
#include "blip.h"

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

void* GetExplsLmcParticle(EXPLS* pexpls)
{
	return &pexpls->lmcParticle;
}

void SetExplsLmcParticle(EXPLS* pexpls, LM lmcParticle)
{
	pexpls->lmcParticle = lmcParticle;
}

void* GetExplsOidRender(EXPLS* pexpls)
{
	return &pexpls->oidRender;
}

void SetExplsOidRender(EXPLS* pexpls, int oidRender)
{
	pexpls->oidRender = (OID)oidRender;
}

void* GetExplsOidNextRender(EXPLS* pexpls)
{
	return &pexpls->oidNextRender;
}

void SetExplsOidNextRender(EXPLS* pexpls, int oidNextRender)
{
	pexpls->oidNextRender = (OID)oidNextRender;
}

void* GetExplsOidTouch(EXPLS* pexpls)
{
	return &pexpls->oidTouch;
}

void SetExplsOidTouch(EXPLS* pexpls, int oidTouch)
{
	pexpls->oidTouch = (OID)oidTouch;
}

void* GetExplsDtDelay(EXPLS* pexpls)
{
	return &pexpls->dtDelay;
}

void SetExplsDtDelay(EXPLS* pexpls, float dtDelay)
{
	pexpls->dtDelay = dtDelay;
}

void* GetExplsFGrouped(EXPLS* pexpls)
{
	return &pexpls->fGrouped;
}

void SetExplsFGrouped(EXPLS* pexpls, int fGrouped)
{
	pexpls->fGrouped = fGrouped;
}

// EMITB/EMITP ensure function bodies

glm::vec3* PemitbEnsureExplsDv(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitv.dv;
}

float* PemitbEnsureExplsRvDamping(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitv.rvDamping;
}

float* PemitbEnsureExplsSwCurl(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitv.swCurl;
}

glm::vec3* PemitbEnsureExplsNormalCurl(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitv.normalCurl;
}

void* PemitbEnsureExplsEmitcnk(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitv.emitcnk;
}

float* PemitbEnsureExplsDtLifetime(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitv.dtLifetime;
}

LM* PemitbEnsureExplsLmDtBirth(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitv.lmDtBirth;
}

CLQ* PemitbEnsureExplsClqAlpha(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitv.clqAlpha;
}

void* PemitbEnsureExplsEmitpk(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitpk;
}

void* PemitbEnsureExplsRiptTrail(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitrip.riptTrail;
}

LM* PemitbEnsureExplsLmGScale(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitrip.lmGScale;
}

CLQ* PemitbEnsureExplsRipClqScale(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitrip.clqScale;
}

void* PemitbEnsureExplsRipgt(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitrip.ripgt;
}

float* PemitbEnsureExplsSExpand(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitrip.sExpand;
}

OID* PemitbEnsureExplsOidShader(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.oidShader;
}

CLQ* PemitbEnsureExplsBlipClqScale(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.clqScale;
}

CLQ* PemitbEnsureExplsClqTexture(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.clqTexture;
}

CLQ* PemitbEnsureExplsClqColor(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.clqColor;
}

int* PemitbEnsureExplsFShaderSpan(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.fShaderSpan;
}

float* PemitbEnsureExplsDtShaderLoop(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.dtShaderLoop;
}

int* PemitbEnsureExplsFRandomFrame(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.fRandomFrame;
}

void* PemitbEnsureExplsBlipmk(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.blipmk;
}

void* PemitbEnsureExplsBlipok(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.blipok;
}

float* PemitbEnsureExplsRSFlying(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.rSFlying;
}

void* PemitbEnsureExplsBliprk(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.bliprk;
}

LM* PemitbEnsureExplsLmSw(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.lmSw;
}

int* PemitbEnsureExplsFRandomRoll(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitp.emitblip.fRandomRoll;
}

// EMITX union ensure function bodies

int* PemitbEnsureExplsBulletFDamage(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitx.bullet.fDamage;
}

int* PemitbEnsureExplsBulletFRichochet(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitx.bullet.fRichochet;
}

OID* PemitbEnsureExplsRemitOidExpls(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitx.remit.oidExpls;
}

float* PemitbEnsureExplsRemitSvcParticle(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitx.remit.svcParticle;
}

float* PemitbEnsureExplsShrapnelSRadius(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitx.shrapnel.sRadius;
}

float* PemitbEnsureExplsShrapnelElas(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitx.shrapnel.elas;
}

float* PemitbEnsureExplsShrapnelMu(EXPLS* pexpls, ENSK ensk)
{
	return &PemitbEnsureExplo((EXPLO*)pexpls, ensk)->emitx.shrapnel.mu;
}

SFXID* PsfxEnsureExplsSfxid(EXPLS* pexpls, ENSK ensk)
{
	return &PsfxEnsureExpls(pexpls, ensk)->sfxid;
}

float* PsfxEnsureExplsSStart(EXPLS* pexpls, ENSK ensk)
{
	return &PsfxEnsureExpls(pexpls, ensk)->sStart;
}

float* PsfxEnsureExplsSFull(EXPLS* pexpls, ENSK ensk)
{
	return &PsfxEnsureExpls(pexpls, ensk)->sFull;
}

float* PsfxEnsureExplsUVol(EXPLS* pexpls, ENSK ensk)
{
	return &PsfxEnsureExpls(pexpls, ensk)->uVol;
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

	if (pexpls->oidRender == OID_Nil && pexpls->oidNextRender == OID_Nil && pexpls->oidTouch == OID_Nil)
		return;

	EMITB* pemitb = PemitbEnsureExplo(pexpls, ENSK_Set);
	EMITRIP& emitrip = pemitb->emitp.emitrip;

	if (pexpls->oidRender != OID_Nil)
		emitrip.paloRender = static_cast<ALO*>(PloFindSwObject(pexpls->psw, 260, (OID)pexpls->oidRender, pexpls));

	if (pexpls->oidNextRender != OID_Nil)
		emitrip.paloNextRender = static_cast<ALO*>(PloFindSwObject(pexpls->psw, 260, (OID)pexpls->oidNextRender, pexpls));

	if (pexpls->oidTouch != OID_Nil)
	{
		LO* ploTouch = PloFindSwObject(pexpls->psw, 260, (OID)pexpls->oidTouch, pexpls);

		if (ploTouch != nullptr && FIsBasicDerivedFrom(ploTouch, CID_SO))
			emitrip.psoTouch = static_cast<SO*>(ploTouch);
	}
}

void HandleExplsMessage(EXPLS* pexpls, MSGID msgid, void* pv)
{
	if (msgid != MSGID_removed)
		return;

	if (pv == pexpls->pripg)
	{
		pexpls->pripg->pvtlo->pfnUnsubscribeLoObject(pexpls->pripg, pexpls);
		pexpls->pripg = nullptr;
	}
	else if (pv == pexpls->pblipg)
	{
		pexpls->pblipg->pvtlo->pfnUnsubscribeLoObject(pexpls->pblipg, pexpls);
		pexpls->pblipg = nullptr;
	}
}

void ExplodeExplsExplso(EXPLS* pexpls, EXPLSO* pexplso)
{
	if (pexpls == nullptr || pexplso == nullptr)
		return;

	if (pexpls->dtDelay <= 0.0f)
	{
		FireExplsExplso(pexpls, pexplso);
		return;
	}

	SW* psw = pexpls->psw;
	if (psw == nullptr)
		return;

	auto pexplsteOwner = std::make_shared<EXPLSTE>();
	EXPLSTE* pexplste = pexplsteOwner.get();

	pexplste->pexpls = pexpls;
	pexplste->explso = *pexplso;
	pexplste->tEmit = g_clock.t + pexpls->dtDelay;
	ClearDle(&pexplste->dle);

	AppendDlEntry(&psw->dlExplste, pexplste);
	psw->explsteOwners.push_back(std::move(pexplsteOwner));
}

SFX* PsfxEnsureExpls(EXPLS* pexpls, ENSK ensk)
{
	if (pexpls->psfx == nullptr)
		NewSfx(pexpls->psfx);

	return pexpls->psfx.get();
}

void DeleteExpls(EXPLS* pexpls)
{
	delete pexpls;
}
