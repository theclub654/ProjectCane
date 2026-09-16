#include "lgn.h"
#include "actadj.h"
#include "sm.h"
#include "jt.h"
#include "shape.h"
#include "turret.h"
#include "target.h"

LGN* NewLgn()
{
	return new LGN{};
}

void InitLgn(LGN* plgn)
{
	InitSuv(plgn);
	plgn->tFlash = -10.0;
}

int GetLgnSize()
{
	return sizeof(LGN);
}

void CloneLgn(LGN* plgn, LGN* plgnBase)
{
	CloneSuv(plgn, plgnBase);

	// Clone the LGNS structure
	plgn->lgns = plgnBase->lgns;

	// Clone the float tLgns
	plgn->tLgns = plgnBase->tLgns;

	// Clone the SM pointer
	plgn->psm = plgnBase->psm;

	// Clone the SMA pointer
	plgn->psma = plgnBase->psma;

	// Clone the ALO pointers (paloTurret, paloPlatform, paloJt, paloGut)
	plgn->paloTurret = plgnBase->paloTurret;
	plgn->paloPlatform = plgnBase->paloPlatform;
	plgn->paloJt = plgnBase->paloJt;
	plgn->paloGut = plgnBase->paloGut;

	// Clone the ACTADJ pointer
	plgn->pactadjGut = plgnBase->pactadjGut;

	// Clone the glm::vec3 posGutTurret
	plgn->posGutTurret = plgnBase->posGutTurret;

	// Clone the int fFlash
	plgn->fFlash = plgnBase->fFlash;

	// Clone the float tFlash, tCharm, tCharmPending
	plgn->tFlash = plgnBase->tFlash;
	plgn->tCharm = plgnBase->tCharm;
	plgn->tCharmPending = plgnBase->tCharmPending;

	// Clone the int fCharmEnabled
	plgn->fCharmEnabled = plgnBase->fCharmEnabled;

	// Clone the XP pointer pxpZap
	plgn->pxpZap = plgnBase->pxpZap;
}

void PostLgnLoad(LGN* plgn)
{
	PostSuvLoad(plgn);

	SnipAloObjects(plgn, 10, s_asnipLgn);

	plgn->psma = PsmaApplySm(plgn->psm, plgn, OID_Nil, 0);

	CreateAloActadj(plgn->paloGut, 5, &plgn->pactadjGut);

	plgn->pactadjGut->ackPos = ACK_SmoothNoLock;
	plgn->pactadjGut->ackRot = ACK_SmoothNoLock;

	if (plgn->paloGut->pvtalo->pfnRecacheAloActList != nullptr)
		plgn->paloGut->pvtalo->pfnRecacheAloActList(plgn->paloGut);

	ConvertAloPos(nullptr, plgn->paloTurret, &plgn->paloGut->xf.posWorld, &plgn->posGutTurret);

	SetLgnLgns(plgn, LGNS_Active);

	plgn->tBalance = 3.4028235E+38;

	g_lgnr.uAlpha = 0;
	g_lgnr.pshd = PshdFindShader((OID)874);
}

void UpdateLgnActive(LGN* plgn, JOY* pjoy, float dt)
{
	constexpr OID OID_LgnAimPrimary = static_cast<OID>(863);
	constexpr OID OID_LgnAimSecondary = static_cast<OID>(864);

	OID oidGoal = OID_Nil;
	OID oidCur = OID_Nil;

	UpdateSuvActive(plgn, pjoy);

	GetSmaGoal(plgn->psma, &oidGoal);
	GetSmaCur(plgn->psma, &oidCur);

	if (plgn->lgns == LGNS_Active)
	{
		if (!pjoy->IsHeld(BTN_SQUARE))
			SetSmaGoal(plgn->psma, OID_LgnAimPrimary);
		else
			SetSmaGoal(plgn->psma, OID_LgnAimSecondary);
	}
	else if (plgn->lgns == LGNS_Zap)
	{
		if (oidCur == OID_LgnAimPrimary)
		{
			plgn->pvtlgn->pfnUseLgnCharm(plgn);
		}
		else
		{
			if (pjoy->IsPressed(BTN_CROSS))
			{
				pjoy->SetHandled(BTN_CROSS);
				plgn->tCharmPending = g_clock.t;
			}

			if (plgn->fCharmEnabled != 0 && g_clock.t - plgn->tCharmPending < DT_LgnCharmBeforeEnabled)
			{
				plgn->pvtlgn->pfnUseLgnCharm(plgn);
				SeekSma(plgn->psma, OID_LgnAimPrimary);
				GetSmaGoal(plgn->psma, &oidGoal);
				GetSmaCur(plgn->psma, &oidCur);
			}
		}
	}

	const bool fValidState = oidCur == OID_LgnAimPrimary || oidCur == OID_LgnAimSecondary;
	const bool fValidGoal = oidGoal == OID_Nil || oidGoal == OID_LgnAimPrimary || oidGoal == OID_LgnAimSecondary;

	if (!fValidGoal || !fValidState)
		return;

	JOY* pjoyAim = plgn->lgns == LGNS_Zap ? &g_joyZero : pjoy;
	UpdateLgnrAim(&g_lgnr, pjoyAim);

	glm::mat3 matPlatform;
	matPlatform[2] = glm::normalize(glm::vec3(plgn->xf.matWorld[2]));

	glm::vec3 normalPlatform = glm::cross(matPlatform[2], glm::vec3(g_pcm->mat[0]));

	if (glm::length(normalPlatform) < 0.0001f)
		matPlatform[1] = glm::normalize(glm::vec3(plgn->xf.matWorld[1]));
	else
		matPlatform[1] = glm::normalize(normalPlatform);

	matPlatform[0] = glm::cross(matPlatform[1], matPlatform[2]);

	ConvertAloMat(nullptr, plgn->paloPlatform->paloParent, &matPlatform, &matPlatform);
	SmoothMatrix(&plgn->paloPlatform->xf.mat, &matPlatform, &s_smpLgnPlatformAim, dt, &matPlatform, nullptr);

	if (plgn->paloPlatform->pvtalo->pfnRotateAloToMat != nullptr)
		plgn->paloPlatform->pvtalo->pfnRotateAloToMat(plgn->paloPlatform, &matPlatform);

	glm::vec3 posAimWorld;
	ConvertCmScreenToWorld(g_pcm, &g_lgnr.posScreen, &posAimWorld);

	glm::vec3 directionTurret = posAimWorld - glm::vec3(plgn->paloTurret->xf.posWorld);

	BuildOrthonormalMatrixZ(directionTurret, g_normalZ, matPlatform);
	ConvertAloMat(nullptr, plgn->paloTurret->paloParent, &matPlatform, &matPlatform);

	if (plgn->paloTurret->pvtalo->pfnRotateAloToMat != nullptr)
		plgn->paloTurret->pvtalo->pfnRotateAloToMat(plgn->paloTurret, &matPlatform);

	glm::vec3 posGut = matPlatform * glm::vec3(plgn->posGutTurret);
	glm::vec3 dposGut = (posGut - glm::vec3(plgn->posGutTurret)) * R_GutAdjust;

	ConvertAloVec(plgn->paloTurret->paloParent, plgn->paloGut->paloParent, &dposGut, &plgn->pactadjGut->dposGoal);

	glm::mat3 dmatGut;
	CalculateDmat(&plgn->paloTurret->matOrig, &matPlatform, &dmatGut);
	ConvertAloMat(plgn->paloTurret->paloParent, plgn->paloGut->paloParent, &dmatGut, &dmatGut);

	float radGut;
	glm::vec3 axisGut;

	DecomposeRotateMatrixRad(&dmatGut, &radGut, &axisGut);
	LoadRotateMatrixRad(R_GutAdjust * radGut, &axisGut, &plgn->pactadjGut->dmatGoal);

	ResolveAlo(plgn);
}

void UpdateLgn(LGN* plgn, float dt)
{
	UpdateSuv(plgn, dt);

	XP* pxp = plgn->pxpZap;

	while (pxp != nullptr) 
	{
		LGN* plgnRoot = (LGN*)pxp->axpd[0].psoRoot;
		SO* psoSource = pxp->axpd[plgnRoot == plgn].psoLeaf;

		while (psoSource != nullptr && psoSource->zok == ZOK_Inherit)
			psoSource = (SO*)psoSource->paloParent;

		if (psoSource != nullptr && (psoSource->zok == ZOK_Zap || psoSource->zok == ZOK_ZapAndIgnore)) {
			ZPR zpr;

			InitZpr(&zpr, ZPK_Blunt, psoSource);

			while (psoSource != nullptr) {
				bool fInflicted = psoSource->pvtso->pfnFInflictSoZap(psoSource, pxp, &zpr);

				if (fInflicted)
					break;

				psoSource = (SO*)psoSource->paloParent;
			}

			plgn->pvtlgn->pfnFTakeLgnDamage(plgn, &zpr);
		}

		pxp = pxp->pxpNext;
	}

	if (plgn->pxpZap != nullptr) {
		FreeSwXpList(plgn->psw, plgn->pxpZap);
		plgn->pxpZap = nullptr;
	}

	float dtDamage = g_clock.t - plgn->tFlash;
	float uSpring = s_clqDtDamageToUSpring.g0 + dtDamage * (s_clqDtDamageToUSpring.g1 + dtDamage * s_clqDtDamageToUSpring.g2);

	uSpring = glm::clamp(uSpring, g_lmZeroOne.gMin, g_lmZeroOne.gMax);

	SetAloPositionSpring(plgn->paloGut, uSpring * R_LgnGutPosSpring);
	SetAloRotationSpring(plgn->paloGut, uSpring * R_LgnGutRotSpring);

	if (plgn->fFlash != 0.0f && plgn->lgns != LGNS_Zap && g_clock.t - plgn->tCharm > 2.0f)
		plgn->fFlash = 0.0f;
}

void UpdateLgnXfWorld(LGN* plgn)
{
	UpdateSuvXfWorld(plgn);

	if (plgn->pshapeTrack == nullptr || plgn->palo == nullptr)
		return;

	SHAPE* pshapeTrack = reinterpret_cast<SHAPE*>(plgn->pshapeTrack);
	CRV* pcrvTrack = pshapeTrack->pcrv.get();

	glm::vec3 posTrack;
	glm::vec3 normalTrack;

	if (pcrvTrack->pvtcrv->pfnEvaluateCrvFromU != nullptr)
		pcrvTrack->pvtcrv->pfnEvaluateCrvFromU(pcrvTrack, plgn->uTrack, &posTrack, &normalTrack);

	glm::mat3 dmatOrig;
	glm::mat3 matTrack;

	CalculateDmat(&plgn->matOrig, &plgn->palo->matOrig, &dmatOrig);
	BuildOrthonormalMatrixZ(normalTrack, g_normalZ, matTrack);

	matTrack = dmatOrig * matTrack;

	if (plgn->palo->pvtalo->pfnTranslateAloToPos != nullptr)
		plgn->palo->pvtalo->pfnTranslateAloToPos(plgn->palo, &posTrack);

	if (plgn->palo->pvtalo->pfnRotateAloToMat != nullptr)
		plgn->palo->pvtalo->pfnRotateAloToMat(plgn->palo, &matTrack);
}

void OnLgnActive(LGN* plgn, int fActive)
{
	OnSuvActive(plgn, fActive);

	if (fActive == 0) 
	{
		g_lgnr.pvtblot->pfnSetBlotBlots(&g_lgnr, BLOTS_Hidden);
		plgn->fFlash = 0.0f;
	}
	else 
	{
		g_lgnr.pvtblot->pfnSetBlotBlots(&g_lgnr, BLOTS_Visible);
		g_lgnr.posScreen = glm::vec4(2000.0f, 0.0f, 0.0f, 0.0f);
	}
}

void RenderLgnAll(LGN* plgn, CM* pcm, RO* pro)
{
	glm::vec4 rgbaCel = g_rgbaCel;

	if (plgn->fFlash != 0.0f) 
	{
		float uZap = plgn->lgns == LGNS_Zap ? 0.0f : (g_clock.t - plgn->tCharm) * 0.5f;
		float uFlash = std::cos((g_clock.t - plgn->tFlash) * 10.0f);

		float uZapCel = s_clqUZapToUCel.g0 + uZap * (s_clqUZapToUCel.g1 + uZap * s_clqUZapToUCel.g2);
		float uFlashCel = s_clqUFlashToUCel.g0 + uFlash * (s_clqUFlashToUCel.g1 + uFlash * s_clqUFlashToUCel.g2);

		uZapCel = glm::clamp(uZapCel, 0.0f, 1.0f);
		uFlashCel = glm::clamp(uFlashCel, 0.0f, 1.0f);

		float uCel = uZapCel * uFlashCel;

		glm::vec4 rgbaFlash(160.0f / 255.0f, 0.0f, 0.0f, g_rgbaCel.a);
		rgbaCel = glm::mix(g_rgbaCel, rgbaFlash, uCel);
	}

	SetAloOverrideCel(plgn, &rgbaCel);
	RenderAloAll(plgn, pcm, pro);
}

void GetLgnCpdefi(LGN* plgn, float dt, CPDEFI* pcpdefi)
{
	GetSuvCpdefi(plgn, dt, pcpdefi);
	pcpdefi->posBase = plgn->paloTurret->xf.posWorld;
}

int FInvulnerableLgn(LGN* plgn)
{
	if (plgn->lgns != LGNS_Active)
		return 1;

	if (plgn->fFlash != 0.0f)
		return 1;

	return FSwHandsOff(plgn->psw) != 0;
}

bool JthsCurrentLgn(LGN* plgn)
{
	return plgn->fFlash != 0.0f;
}

void UseLgnCharm(LGN* plgn)
{
	UsePoCharm(plgn);
	SetLgnLgns(plgn, LGNS_Active);
}

void ApplyLgnThrow(LGN* plgn, STEP* pstep)
{
	glm::vec3 posThrow;
	glm::vec3 velocity;

	ConvertAloPos(plgn, nullptr, &s_posThrowJt, &posThrow);
	CalculateSoTrajectoryApex(pstep, &posThrow, DZ_LgnThrowJtApex, &velocity);

	if (pstep->pvtalo->pfnSetAloVelocityVec != nullptr)
		pstep->pvtalo->pfnSetAloVelocityVec(pstep, &velocity);

	pstep->radTarget = RadNormalize(std::atan2(velocity.y, velocity.x) + glm::pi<float>());

	FixStepAngularVelocity(pstep);
	SetJtJts(static_cast<JT*>(pstep), JTS_Zap, JTBS_Zap_Dead);
}

int FTakeLgnDamage(LGN* plgn, ZPR* pzpr)
{
	if (plgn->pvtlgn->pfnFInvulnerableLgn(plgn))
		return 0;

	if (FCharmAvailable())
		plgn->fFlash = 1;

	plgn->tFlash = g_clock.t;

	if (!FCharmAvailable() && g_pjt != nullptr) {
		if (g_pjt->pvtlo->pfnAddLo != nullptr)
			g_pjt->pvtlo->pfnAddLo(g_pjt);

		if (g_pjt->pvtalo->pfnMatchAloOtherObject != nullptr)
			g_pjt->pvtalo->pfnMatchAloOtherObject(g_pjt, plgn->paloJt);

		if (plgn->paloJt->pvtlo->pfnRemoveLo != nullptr)
			plgn->paloJt->pvtlo->pfnRemoveLo(plgn->paloJt);

		SetPoPlayable(g_pjt, 1);
		SetJtJts(g_pjt, JTS_Jump, JTBS_Jump_Fall);
		SwitchToPo(g_pjt);
		SetLgnLgns(plgn, LGNS_Dead);

		ZPR zprThrow;
		InitZpr(&zprThrow, ZPK_Blunt, plgn);
		zprThrow.pfnzap = (PFNZAP)ApplyLgnThrow;

		g_pjt->pvtpo->pfnFTakePoDamage(g_pjt, &zprThrow);
	}
	else {
		SetLgnLgns(plgn, LGNS_Zap);
	}

	struct 
	{
		LGN* plgn;
		LO* ploSource;
	} damageInfo = {
		plgn,
		pzpr->ploSource
	};

	plgn->pvtlo->pfnSendLoMessage(plgn, MSGID_damaged, &damageInfo);

	return 1;
}

void HandleLgnMessage(LGN* plgn, MSGID msgid, void* pv)
{
	if (msgid == MSGID_label && plgn->psma != nullptr && pv != nullptr) 
	{
		struct LabelMessage {
			ASEGA* pasega;
			int field_0x4;
			int label;
		};

		LabelMessage* pmsg = reinterpret_cast<LabelMessage*>(pv);

		if (pmsg->pasega == plgn->psma->pasegaCur) {
			if (pmsg->label == 350)
				plgn->fCharmEnabled = 1;
			else if (pmsg->label == 351)
				plgn->fCharmEnabled = 0;
		}
	}

	HandleSuvMessage(plgn, msgid, pv);
}

void AdjustLgnXps(LGN* plgn)
{
	AdjustSoZapXps(plgn, &plgn->pxpZap);
}

void* GetLgnLgns(LGN* plgn)
{
	return &plgn->lgns;
}

void SetLgnLgns(LGN* plgn, LGNS lgns)
{
	constexpr OID OID_StateLgnPassive = static_cast<OID>(862);
	constexpr OID OID_StateLgnActive = static_cast<OID>(863);
	constexpr OID OID_StateLgnZap = static_cast<OID>(865);
	constexpr OID OID_StateLgnDead = static_cast<OID>(866);

	LGNS lgnsPrev = plgn->lgns;

	if (lgns == lgnsPrev)
		return;

	if (lgnsPrev > LGNS_Active && lgnsPrev < LGNS_Max)
		plgn->tCharm = g_clock.t;

	switch (lgns) 
	{
		case LGNS_Passive:
		SetSmaGoal(plgn->psma, OID_StateLgnPassive);
		break;

		case LGNS_Active:
		SetSmaGoal(plgn->psma, OID_StateLgnActive);
		break;

		case LGNS_Zap:
		SeekSma(plgn->psma, OID_StateLgnZap);
		plgn->tCharm = T_LgnCharmInitial;
		break;

		case LGNS_Dead:
		SeekSma(plgn->psma, OID_StateLgnDead);
		plgn->tCharm = T_LgnCharmInitial;
		break;

		default:
		break;
	}

	plgn->pzi.fCollectEnabled = lgns != LGNS_Passive && (lgns < LGNS_Zap || lgns > LGNS_Dead);

	plgn->lgns = lgns;
	plgn->tLgns = g_clock.t;
}

void DeleteLgn(LGN *plgn)
{
	delete plgn;
}

void StartupLgnr(LGNR* plgnr)
{
	plgnr->pvtlgnr = &g_vtlgnr;
}

void DrawLgnr(LGNR* plgnr)
{
	if (plgnr == nullptr || plgnr->pshd == nullptr || g_pcm == nullptr)
		return;

	const float alphaTarget = FCanShowGameplayBlot() ? 1.0f : 0.0f;
	static SMP smpReticleAlpha = { 5.0f, 0.0f, 0.1f };

	if (g_pcm->fCut == 0)
		plgnr->uAlpha = GSmooth(plgnr->uAlpha, alphaTarget, g_clock.dtReal,
			&smpReticleAlpha, nullptr);
	else
		plgnr->uAlpha = alphaTarget;

	if (plgnr->uAlpha <= 0.0f || plgnr->pshd->atex.empty())
		return;

	TEX& tex = plgnr->pshd->atex[0];
	if (tex.abmp.empty() || tex.abmp[0] == nullptr)
		return;

	BMP* const pbmp = tex.abmp[0];
	const GLuint texture = !tex.glDiffuseMap.empty() && tex.glDiffuseMap[0] != 0
		? tex.glDiffuseMap[0]
		: pbmp->glDiffuseMap;
	if (texture == 0)
		return;

	// The PS2 data defines a 48x48 quad (first corner is 24,-24) and
	// rotates it at three radians per second around the normalized aim point.
	const float centerX = (plgnr->posScreen.x * 0.5f + 0.5f) * g_gl.width;
	const float centerY = (0.5f - plgnr->posScreen.y * 0.5f) * g_gl.height;
	const float pixelScale = (std::min)(g_gl.width / 640.0f, g_gl.height / 480.0f);
	const float size = 48.0f * pixelScale;
	const float angle = g_clock.t * 3.0f;

	const glm::mat4 model =
		glm::translate(glm::mat4(1.0f), glm::vec3(centerX, centerY, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(size, -size, 1.0f)) *
		glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, 0.0f));

	glBlotShader.Use();
	glUniformMatrix4fv(u_projectionLoc, 1, GL_FALSE, glm::value_ptr(g_gl.blotProjection));
	glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniform4f(uvRectLoc, 0.0f, 0.0f, 1.0f, 1.0f);
	glUniform4f(blotColorLoc, 1.0f, 1.0f, 1.0f, glm::clamp(plgnr->uAlpha, 0.0f, 1.0f) * (128.0f / 255.0f));
	glUniform1i(u_useVertexColorLoc, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(u_fontTexLoc, 0);
	glBindVertexArray(g_gl.gao);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glBindVertexArray(0);
}

void UpdateLgnrAim(LGNR* plgnr, JOY* pjoy)
{
	glm::vec3 posBest(0.0f, 0.0f, 2000.0f);
	TARGET* ptargetBest = nullptr;
	float distanceBest = FLT_MAX;

	for (TARGET* ptarget = g_dlTarget.ptargetFirst; ptarget != nullptr; ptarget = ptarget->dleTarget.ptargetNext)
	{
		glm::vec3 posScreen{};

		GetXfmPos(ptarget, &posScreen);
		ConvertCmWorldToScreen(g_pcm, &posScreen, &posScreen);

		if (posScreen.z < 100.0f)
			continue;

		const glm::vec2 offset = glm::vec2(posScreen) - glm::vec2(plgnr->posScreen);
		const float radiusScreen = ptarget->sRadiusTarget / (posScreen.z * g_pcm->xScreenRange);

		float distance = glm::length(offset) - radiusScreen;
		distance = std::max(distance, 0.0001f);

		if (ptarget == plgnr->ptargetCur)
			distance *= 0.5f;

		if (distance < distanceBest || (distance == distanceBest && posScreen.z < posBest.z))
		{
			distanceBest = distance;
			posBest = posScreen;
			ptargetBest = ptarget;
		}
	}

	plgnr->vScreen.x = GSmooth(plgnr->vScreen.x, pjoy->x, g_clock.dt, &s_smp, nullptr);

	float joyY = pjoy->y;

	if ((g_pgsCur->grfgs & 1024U) != 0)
		joyY = -joyY;

	plgnr->vScreen.y = GSmooth(plgnr->vScreen.y, joyY, g_clock.dt, &s_smp, nullptr);

	plgnr->posScreen.x += g_clock.dt * plgnr->vScreen.x * SVU_Joy;
	plgnr->posScreen.x = std::clamp(plgnr->posScreen.x, -1.0f, 1.0f);

	plgnr->posScreen.y -= g_clock.dt * plgnr->vScreen.y * SVU_Joy;
	plgnr->posScreen.y = std::clamp(plgnr->posScreen.y, -1.0f, 1.0f);

	const float targetDepth = std::max(posBest.z, 1000.0f);

	plgnr->posScreen.z = GSmooth(plgnr->posScreen.z, targetDepth, g_clock.dt, &s_smpZ, nullptr);
	plgnr->ptargetCur = ptargetBest;
}

LGNB* NewLgnb()
{
	return new LGNB{};
}

void InitLgnb(LGNB* plgnb)
{
	InitSo(plgnb);
}

int GetLgnbSize()
{
	return sizeof(LGNB);
}

void CloneLgnb(LGNB* plgnb, LGNB* plgnbBase)
{
	LO lo = *plgnb;
	*plgnb = *plgnbBase;
	memcpy(plgnb, &lo, sizeof(LO));

	CloneLo(plgnb, plgnbBase);

	ClearDl(&plgnb->dlChild);

	plgnb->pxa = nullptr;
	plgnb->grfpvaXpValid = 0;
	plgnb->pstso = nullptr;
}

void PostLgnbLoad(LGNB* plgnb)
{
	PostAloLoad(plgnb);
	plgnb->pvtlo->pfnRemoveLo(plgnb);
}

void UpdateLgnb(LGNB* plgnb, float dt)
{
	UpdateSo(plgnb, dt);
}

void DeleteLgnb(LGNB *plgnb)
{
	delete plgnb;
}

SWP* NewSwp()
{
	return new SWP{};
}

void InitSwp(SWP* pswp)
{
	InitBreak(pswp);
	pswp->tMax = 10.0;
	pswp->smpImpact.dtFast = 0.5;
	pswp->svtLocal = 1.0;
	pswp->sdvMax = 3000.0;
	pswp->smpImpact.svFast = 1000.0;
	pswp->smpImpact.svSlow = 0.0;
}

void SetSwpSmpImpactSvFast(SWP* pswp, float svFast)
{
	pswp->smpImpact.svFast = svFast;
}

void* GetSwpSmpImpactSvFast(SWP* pswp)
{
	return &pswp->smpImpact.svFast;
}

void SetSwpSmpImpactSvSlow(SWP* pswp, float svSlow)
{
	pswp->smpImpact.svSlow = svSlow;
}

void* GetSwpSmpImpactSvSlow(SWP* pswp)
{
	return &pswp->smpImpact.svSlow;
}

void SetSwpSmpImpactDtFast(SWP* pswp, float dtFast)
{
	pswp->smpImpact.dtFast = dtFast;
}

void* GetSwpSmpImpactDtFast(SWP* pswp)
{
	return &pswp->smpImpact.dtFast;
}

void SetSwpSdvMax(SWP* pswp, float sdvMax)
{
	pswp->sdvMax = sdvMax;
}

void* GetSwpSdvMax(SWP* pswp)
{
	return &pswp->sdvMax;
}

int GetSwpSize()
{
	return sizeof(SWP);
}

void CloneSwp(SWP* pswp, SWP* pswpBase)
{
	CloneBreak(pswp, pswpBase);
	pswp->pshape = pswpBase->pshape;
	pswp->psm = pswpBase->psm;
	pswp->psma = pswpBase->psma;
	pswp->smpImpact = pswpBase->smpImpact;
	pswp->sdvMax = pswpBase->sdvMax;
	pswp->tMax = pswpBase->tMax;
	pswp->svtLocal = pswpBase->svtLocal;
	pswp->tLocal = pswpBase->tLocal;
	pswp->pacgb = pswpBase->pacgb;
	pswp->pexplCrash = pswpBase->pexplCrash;
}

void PostSwpLoad(SWP* pswp)
{
	PostBrkLoad(pswp);
	SnipAloObjects(pswp, 2, s_asnipSwp);

	pswp->psma = PsmaApplySm(pswp->psm, pswp, (OID)878, 0);

	pswp->pacgb = std::static_pointer_cast<ACGB>(PacgNew(ACGK_Bezier));
	pswp->pacgb->ckgb = 0;
	pswp->pacgb->akgb.clear();
	pswp->pacgb->akgb.reserve(128);
}

void UpdateSwp(SWP* pswp, float dt)
{
	UpdateBrk(pswp, dt);

	if (pswp->pacgb == nullptr)
	{
		ResolveAlo(pswp);
		return;
	}

	PO* ppoTarget = PpoCur();

	// Release-specific handling: if the current playable object is this
	// derived type, damage the associated JT/TURRET object instead.
	if (ppoTarget != nullptr && FIsBasicDerivedFrom(ppoTarget, CID_TURRET))
	{
		TURRET* pturret = reinterpret_cast<TURRET*>(ppoTarget);
		ppoTarget = reinterpret_cast<PO*>(pturret->pjt);
	}

	pswp->tLocal = glm::clamp(pswp->tLocal + dt * pswp->svtLocal, 0.0f, pswp->tMax);

	OID state;
	GetSmaGoal(pswp->psma, &state);

	if (state == OID_Nil)
		GetSmaCur(pswp->psma, &state);

	bool detonate = pswp->tLocal >= pswp->tMax;
	if (state == (OID)879 && ppoTarget != nullptr)
	{
		if (glm::length(pswp->xf.posWorld - ppoTarget->xf.posWorld) < S_SwpDetonate)
			detonate = true;
	}

	if (detonate)
	{
		ZPK zpk = ZPK_Blunt;

		// Release-specific behavior for world-level 0x0505.
		const int worldLevel =
			(g_pgsCur->gameWorldCur << 8) |
			g_pgsCur->worldLevelCur;

		if (worldLevel == 0x0505)
			zpk = static_cast<ZPK>(2);

		ZPR zpr;
		InitZpr(&zpr, zpk, pswp);

		if (ppoTarget != nullptr)
			ppoTarget->pvtpo->pfnFTakePoDamage(ppoTarget, &zpr);

		ExplodeExpl(pswp->pexplCrash);
		pswp->pvtlo->pfnRemoveLo(pswp);
		HandleLoSpliceEvent(pswp, 2, 0, nullptr);
		return;
	}

	if (pswp->psma != nullptr &&
		pswp->tMax - pswp->tLocal < DT_SwpSwoop)
	{
		SetSmaGoal(pswp->psma, (OID)879);
	}

	ResolveAlo(pswp);
}

void ProjectSwpTransform(SWP* pswp, float dt, int fParentDirty)
{
	if (pswp->pacgb != nullptr) {
		float s;
		float ds;

		pswp->pacgb->pvtacg->pfnEvaluateAcg(pswp->pacgb.get(), pswp, pswp->tLocal, pswp->svtLocal, 0, &s, &ds);

		CRV* pcrv = pswp->pshape->pcrv.get();

		glm::vec3 pos{};
		glm::vec3 tangent{};

		if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr)
			pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, s, &pos, &tangent);

		glm::vec3 v = tangent * ds;

		CalculateAloTransformAdjust(pswp->pshape->paloParent, nullptr, &pos, nullptr, &v, nullptr);

		glm::vec3 dv = (v - pswp->xf.v) / dt - pswp->dvGravity;
		glm::mat3 mat;

		BuildOrthonormalMatrixZ(v, dv, mat);

		pswp->xf.v = v;
		pswp->xf.pos = pos;

		SmoothMatrix(&pswp->xf.mat, &mat, &s_smpMat, dt, &pswp->xf.mat, &pswp->xf.w);
	}

	DLI dli{};
	dli.m_pdl = &pswp->dlChild;
	dli.m_ibDle = pswp->dlChild.ibDle;
	dli.m_pdliNext = s_pdliFirst;

	s_pdliFirst = &dli;

	LO* ploChild = pswp->dlChild.ploFirst;

	while (ploChild != nullptr) {
		dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(ploChild) + dli.m_ibDle);

		if ((ploChild->pvtlo->grfcid & 1U) != 0) {
			ALO* paloChild = reinterpret_cast<ALO*>(ploChild);
			paloChild->pvtalo->pfnProjectAloTransform(paloChild, dt, 1);
		}

		ploChild = static_cast<LO*>(*dli.m_ppv);
	}

	s_pdliFirst = dli.m_pdliNext;

	if (!fParentDirty && pswp->pvtlo->pfnUpdateLoXfWorld != nullptr)
		pswp->pvtlo->pfnUpdateLoXfWorld(pswp);
}

void SetSwpShape(SWP* pswp, SHAPE* pshape, float tMax)
{
	pswp->pshape = pshape;
	pswp->tMax = tMax;

	CRV* pcrv = pshape->pcrv.get();
	float sMax = pcrv->pvtcrv->pfnSMaxCrv(pcrv);
	int ckgb = pcrv->ccv;

	pswp->pacgb->ckgb = ckgb;
	pswp->pacgb->akgb.clear();
	pswp->pacgb->akgb.resize(ckgb);

	std::vector<float> asvMax(ckgb);

	for (int i = 0; i < ckgb; ++i) {
		glm::vec3 normalBefore{};
		glm::vec3 normalAfter{};

		float s = pcrv->mpicvs[i];

		if (pcrv->pvtcrv->pfnEvaluateCrvFromS != nullptr) {
			pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, s - 0.5f, nullptr, &normalBefore);
			pcrv->pvtcrv->pfnEvaluateCrvFromS(pcrv, s + 0.5f, nullptr, &normalAfter);
		}

		float dnormal = glm::length(normalBefore - normalAfter);
		float svCurvature = std::sqrt(pswp->sdvMax / dnormal);
		float svImpact;

		GSmooth(s, sMax + 100.0f, 0.0f, &pswp->smpImpact, &svImpact);

		asvMax[i] = std::min(svCurvature, svImpact);
	}

	for (int i = 0; i < ckgb; ++i) {
		KGB& kgb = pswp->pacgb->akgb[i];

		kgb.g = pcrv->mpicvs[i];

		if (i == 0) {
			kgb.t = 0.0f;
		}
		else {
			const KGB& kgbPrev = pswp->pacgb->akgb[i - 1];
			float ds = kgb.g - kgbPrev.g;
			kgb.t = kgbPrev.t + (2.0f * ds) / (asvMax[i] + asvMax[i - 1]);
		}

		kgb.kgbtIn.kgbtk = KGBTK_Smooth;
		kgb.kgbtIn.gSlope = asvMax[i];
	}

	if (ckgb > 0) {
		float tGenerated = pswp->pacgb->akgb.back().t;
		float tScale = tMax / tGenerated;

		for (KGB& kgb : pswp->pacgb->akgb) {
			kgb.t *= tScale;
			kgb.kgbtIn.gSlope /= tScale;
			kgb.kgbtOut.kgbtk = KGBTK_Smooth;
			kgb.kgbtOut.gSlope = kgb.kgbtIn.gSlope;
		}
	}

	pswp->tLocal = 0.0f;
	SeekSma(pswp->psma, (OID)878);
}

void DeleteSwp(SWP *pswp)
{
	delete pswp;
}

LGNR g_lgnr;
SNIP s_asnipLgn[10] = 
{
	{ 0x00, (OID)0x35D, offsetof(LGN, psm) },
	{ 0x04, (OID)0x363, offsetof(LGN, paloTurret) },
	{ 0x04, (OID)0x364, offsetof(LGN, paloPlatform) },
	{ 0x04, (OID)0x060, offsetof(LGN, paloGut) },
	{ 0x04, (OID)0x365, offsetof(LGN, paloJt) },

	{ 0x02, (OID)0x161, offsetof(LGN, pzi.mpccharmpaloCharm[1]) },
	{ 0x02, (OID)0x162, offsetof(LGN, pzi.mpccharmpaloCharm[2]) },
	{ 0x02, (OID)0x163, offsetof(LGN, pzi.mpccharmpaloCharm[3]) },
	{ 0x04, (OID)0x160, offsetof(LGN, pzi.mpccharmpaloCharm[0]) },
	{ 0x07, (OID)0x366, offsetof(LGN, palo)}
};

float DT_LgnCharmBeforeEnabled = 0.15;
float R_GutAdjust = 0.5;
SMP s_smpLgnPlatformAim = {30.0, 0.0, 0.5};
float R_LgnGutPosSpring = 6.0;
float R_LgnGutRotSpring = 3.0;
float DZ_LgnThrowJtApex = 150.0;
float T_LgnCharmInitial = 3.4028235E+38;
SMP s_smpMat = {20.0, 0.0, 0.5};
SNIP s_asnipSwp[2] =
{
	0x0,  (OID)0x36B, offsetof(SWP, psm),
	0x20, (OID)0x36C, offsetof(SWP, pexplCrash)
};

float S_SwpDetonate = 300;
float DT_SwpSwoop = 2.5;
float SVU_Joy = 1.5;;
SMP s_smp = {20, 0.0, 0.1};
