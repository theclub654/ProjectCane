#include "jlo.h"
#include "sm.h"
#include "po.h"
#include "rwm.h"
#include "jt.h"
#include "missile.h"
#include "xform.h"

JLOVOL* NewJlovol()
{
	return new JLOVOL{};
}

void InitJlovol(JLOVOL* pjlovol)
{
	InitVolbtn(pjlovol);

	pjlovol->btn.dtRepush = 0.0;
	pjlovol->oidJloc = (OID)-1;
	pjlovol->oidLand = (OID)-1;
}

void* GetJlovolOidLand(JLOVOL* pjlovol)
{
	return &pjlovol->oidLand;
}

void SetJlovolOidLand(JLOVOL* pjlovol, int oidLand)
{
	pjlovol->oidLand = (OID)oidLand;
}

void* GetJlovolPpntLand(JLOVOL* pjlovol)
{
	return &pjlovol->ppntLand;
}

void SetJlovolPpntLand(JLOVOL* pjlovol, PNT* ppntLand)
{
	pjlovol->ppntLand = ppntLand;
}

void* GetJlovolOidJloc(JLOVOL* pjlovol)
{
	return &pjlovol->oidJloc;
}

void SetJlovolOidJloc(JLOVOL* pjlovol, int oidJloc)
{
	pjlovol->oidJloc = (OID)oidJloc;
}

void* GetJlovolPjloc(JLOVOL* pjlovol)
{
	return &pjlovol->pjloc;
}

void SetJlovolPjloc(JLOVOL* pjlovol, JLOC* pjloc)
{
	pjlovol->pjloc = pjloc;
}

int GetJlovolSize()
{
	return sizeof(JLOVOL);
}

void CloneJlovol(JLOVOL* pjlovol, JLOVOL* pjlovolBase)
{
	CloneVolbtn(pjlovol, pjlovolBase);

	pjlovol->oidLand = pjlovolBase->oidLand;
	pjlovol->ppntLand = pjlovolBase->ppntLand;
	pjlovol->oidJloc = pjlovolBase->oidJloc;
	pjlovol->pjloc = pjlovolBase->pjloc;
	pjlovol->dleJlo = pjlovolBase->dleJlo;
}

void BindJlovol(JLOVOL* pjlovol)
{
	BindAlo(pjlovol);

	if (pjlovol->oidLand == OID_Nil)
		pjlovol->ppntLand = (PNT*)PloFindSwObjectByClass(pjlovol->psw, 2, CID_PNT, pjlovol);
	else
		pjlovol->ppntLand = (PNT*)PloFindSwObject(pjlovol->psw, 4, (OID)pjlovol->oidLand, pjlovol);

	pjlovol->pjloc = (JLOC*)PloFindSwObject(pjlovol->psw, 4, (OID)pjlovol->oidJloc, pjlovol);
}

void HandleJlovolMessage(JLOVOL* pjlovol, MSGID msgid, void* pv)
{
	HandleAloMessage(pjlovol, msgid, pv);

	if (g_pjloCur == nullptr)
		return;

	if (msgid == MSGID_button_trigger) 
		AppendDlEntry(&g_pjloCur->dlJlovol, pjlovol);
	else if (msgid == MSGID_removed) 
	{
		if (FFindDlEntry(&g_pjloCur->dlJlovol, pjlovol)) {
			RemoveDlEntry(&g_pjloCur->dlJlovol, pjlovol);
		}
	}
	else if (msgid == MSGID_button_untrigger) 
		RemoveDlEntry(&g_pjloCur->dlJlovol, pjlovol);
}

void DeleteJlovol(JLOVOL* pjlovol)
{
	delete pjlovol;
}

JLO* NewJlo()
{
	return new JLO{};
}

void InitJlo(JLO* pjlo)
{
	InitSo(pjlo);
	pjlo->jlos = JLOS_Nil;
	pjlo->sStart = 3000.0f;
	pjlo->sFull = 1000.0f;
	pjlo->oidJlovolStart = OID_Nil;
	InitDl(&pjlo->dlJlovol, offsetof(JLOVOL, dleJlo));
	pjlo->smpSpin.svFast = 25.0f;
	pjlo->smpSpin.svSlow = 0.0f;
	pjlo->smpSpin.dtFast = 0.25f;

	g_pjloCur = nullptr;
}

void SetJloOidJlovolStart(JLO* pjlo, OID oidJlovolStart)
{
	pjlo->oidJlovolStart = oidJlovolStart;
}

void* GetJloOidJlovolStart(JLO* pjlo)
{
	return &pjlo->oidJlovolStart;
}

void SetJloFAmbientEnabled(JLO* pjlo, int fAmbientEnabled)
{
	pjlo->fAmbientEnabled = fAmbientEnabled;
}

void* GetJloFAmbientEnabled(JLO* pjlo)
{
	return &pjlo->fAmbientEnabled;
}

void SetJloSStart(JLO* pjlo, float sStart)
{
	pjlo->sStart = sStart;
}

void* GetJloSStart(JLO* pjlo)
{
	return &pjlo->sStart;
}

void SetJloSFull(JLO* pjlo, float sFull)
{
	pjlo->sFull = sFull;
}

void* GetJloSFull(JLO* pjlo)
{
	return &pjlo->sFull;
}

void SetJloFDisableAutoSpin(JLO* pjlo, int fDisableAutoSpin)
{
	pjlo->fDisableAutoSpin = fDisableAutoSpin;
}

void* GetJloFDisableAutoSpin(JLO* pjlo)
{
	return &pjlo->fDisableAutoSpin;
}

int GetJloSize()
{
	return sizeof(JLO);
}

void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pjlo, pbis);
	SnipAloObjects(pjlo, 5, s_asnipLoadJlo);

	SM* psm = pjlo->psm;

	for (int ismt = 0; ismt < psm->csmt; ++ismt) 
	{
		SMT* psmt = &psm->asmt[ismt];
		OID oidFrom = (OID)OidFromSmIsms(psm, psmt->ismsFrom);
		OID oidTo = (OID)OidFromSmIsms(psm, psmt->ismsTo);

		if (oidFrom == (OID)751 && oidTo == (OID)752) {
			pjlo->dtLand = psmt->paseg->tMax;
			return;
		}
	}
}

void CloneJlo(JLO* pjlo, JLO* pjloBase)
{
	CloneSo(pjlo, pjloBase);

	pjlo->psm = pjloBase->psm;
	pjlo->psma = pjloBase->psma;
	pjlo->pjlovolCur = pjloBase->pjlovolCur;
	pjlo->dlJlovol = pjloBase->dlJlovol;
	pjlo->ppntFeet = pjloBase->ppntFeet;
	pjlo->dzFeet = pjloBase->dzFeet;
	pjlo->prwm = pjloBase->prwm;
	pjlo->tFireNext = pjloBase->tFireNext;
	pjlo->oidJlovolStart = pjloBase->oidJlovolStart;
	pjlo->paloReticle = pjloBase->paloReticle;
	pjlo->paloTracer = pjloBase->paloTracer;
	pjlo->ppntTracerStart = pjloBase->ppntTracerStart;
	pjlo->pasegReticlePop = pjloBase->pasegReticlePop;
	pjlo->pasegReticleConverge = pjloBase->pasegReticleConverge;
	pjlo->pxfmTarget = pjloBase->pxfmTarget;
	pjlo->pxfmStatic = pjloBase->pxfmStatic;
	pjlo->jlos = pjloBase->jlos;
	pjlo->tJlos = pjloBase->tJlos;
	pjlo->radTarget = pjloBase->radTarget;
	pjlo->dtLand = pjloBase->dtLand;
	pjlo->tLand = pjloBase->tLand;
	pjlo->tStick = pjloBase->tStick;
	pjlo->smpSpin = pjloBase->smpSpin;
	pjlo->pexc = pjloBase->pexc;
}

void PostJloLoad(JLO* pjlo)
{
	PostAloLoad(pjlo);
	SnipAloObjects(pjlo, 2, s_asnipPostJloLoad);

	pjlo->psma = PsmaApplySm(pjlo->psm, pjlo, (OID)-1, 1);
	pjlo->prwm = (RWM*)PloFindSwObjectByClass(pjlo->psw, 1, CID_RWM, pjlo);

	glm::vec3 posFeet;
	GetPntPos(pjlo->ppntFeet, &posFeet);
	pjlo->dzFeet = pjlo->xf.posWorld.z - posFeet.z;

	if (pjlo->oidJlovolStart != -1) 
		SetJloStartJlovol(pjlo, (OID)pjlo->oidJlovolStart);

	pjlo->fNoXpsAll = true;
	pjlo->pxfmStatic = (XFM*)PloNew(CID_XFM, g_psw, nullptr, (OID)770, -1);
}

void SetJloStartJlovol(JLO* pjlo, OID oidJlovol)
{
	JLOVOL* pjlovol;

	pjlo->oidJlovolStart = oidJlovol;
	pjlovol = (JLOVOL*)PloFindSwNearest(pjlo->psw, oidJlovol, pjlo);

	if (pjlovol == nullptr) 
		return;

	SetJloJlovol(pjlo, pjlovol);
	LandJlo(pjlo);

	if (pjlovol->pjloc == nullptr) 
		SetJloJlos(pjlo, (JLOS)4);
	else
		SetJloJlos(pjlo, (JLOS)2);
}

void PresetJloAccel(JLO* pjlo, float dt)
{
	PresetSoAccel(pjlo, dt);

	if (!pjlo->fDisableAutoSpin) 
	{
		glm::mat3 matTarget;
		LoadRotateMatrixRad(pjlo->radTarget, &g_normalZ, &matTarget);
		AccelSoTowardMatSmooth(pjlo, dt, &matTarget, &pjlo->smpSpin);
	}
}

void UpdateJlo(JLO* pjlo, float dt)
{
	UpdateSo(pjlo, dt);
	UpdateJloSound(pjlo);

	if (pjlo != g_pjloCur)
		return;

	JLOS jlos;

	while ((jlos = JlosNextJlo(pjlo)) != pjlo->jlos)
		SetJloJlos(pjlo, jlos);

	switch (jlos) 
	{
		case JLOS_Idle:
		case JLOS_Taunt:
		{
			PO* ppo = PpoCur();

			if (ppo == nullptr)
				return;

			glm::vec3 dpos = ppo->xf.posWorld - pjlo->xf.posWorld;
			pjlo->radTarget = atan2f(dpos.y, dpos.x);
			break;
		}

		default:
		break;
	}
}

JLOS JlosNextJlo(JLO* pjlo)
{
	JLOS jlos = pjlo->jlos;

	switch (jlos) 
	{
		case JLOS_Jump:
		if (g_clock.t >= pjlo->tLand)
			jlos = JLOS_Land;
		break;

		case JLOS_Land:
		if (g_clock.t >= pjlo->tStick)
			jlos = pjlo->pjlovolCur->pjloc != nullptr ? JLOS_Idle : JLOS_Taunt;
		break;

		case JLOS_Idle:
		case JLOS_Taunt:
		{
			JLOVOL* pjlovol = pjlo->dlJlovol.pjlovolLast;
			JLOVOL* pjlovolCur = pjlo->pjlovolCur;

			if (pjlovol != pjlovolCur) {
				jlos = JLOS_Taunt;

				if (pjlovol != nullptr) {
					if (pjlovolCur == nullptr || pjlovol->ppntLand != pjlovolCur->ppntLand) {
						glm::vec3 posLand;
						GetPntPos(pjlovol->ppntLand, &posLand);
						posLand.z += pjlo->dzFeet;

						float distance = glm::length(posLand - pjlo->xf.posWorld);

						if (distance >= 5.0f) {
							jlos = JLOS_Jump;
						}
						else if (pjlovol->pjloc != nullptr) {
							jlos = JLOS_Idle;
						}
					}
					else if (pjlovol->pjloc != nullptr) {
						jlos = JLOS_Idle;
					}
				}

				SetJloJlovol(pjlo, pjlovol);
			}

			JT* pjt = (JT*)PpoCur();

			if (pjt == nullptr) {
				break;
			}

			if (pjt == g_pjt) {
				if (!pjt->pvtpo->pfnJthsCurrentPo(pjt)) {
					if (jlos == JLOS_Taunt && pjlo->pjlovolCur != nullptr && pjlo->pjlovolCur->pjloc != nullptr) {
						jlos = JLOS_Idle;
					}
				}
				else if (jlos == JLOS_Idle) {
					jlos = JLOS_Taunt;
				}
			}

			if (jlos != JLOS_Idle)
				return jlos;

			if (g_clock.t <= pjlo->tFireNext)
				return JLOS_Idle;

			if (pjlo->prwm == nullptr)
				return JLOS_Idle;

			if (!FEnsureRwmLoaded(pjlo->prwm))
				return JLOS_Idle;

			XFM* pxfmTarget = (XFM*)PxfmChooseJloc(pjlo->pjlovolCur->pjloc);

			if (pxfmTarget == nullptr)
				return JLOS_Idle;

			pjlo->pxfmTarget = pxfmTarget;
			jlos = JLOS_Fire;
			break;
		}

		default:
		break;
	}

	return jlos;
}

void SetJloJlovol(JLO* pjlo, JLOVOL* pjlovol)
{
	if (pjlovol == pjlo->pjlovolCur)
		return;

	JLOC* pjloc = pjlovol != nullptr ? pjlovol->pjloc : nullptr;

	if (pjloc != pjlo->pjlocFire)
	{
		pjlo->pjlocFire = pjloc;
		pjlo->cShotsUntilTargetJt = 3;
	}

	pjlo->pjlovolCur = pjlovol;

	if (pjloc == nullptr) 
	{
		if (pjlo->pexc != nullptr) 
		{
			UnsetExcitementHyst(pjlo->pexc);
			pjlo->pexc = nullptr;
		}

		return;
	}

	pjlo->prwm->irwc = pjloc->jlomk;
	ReloadRwm(pjlo->prwm);

	if (pjlo->pexc == nullptr)
		pjlo->pexc = (EXC*)PexcSetExcitement(107);
}

void FireJlo(JLO* pjlo)
{
	glm::vec3 posTarget;
	glm::vec3 posFire;
	glm::mat3 matFire;

	GetXfmPos(pjlo->pxfmTarget, &posTarget);

	RWM* prwm = pjlo->prwm;
	ACCMISS* paccmiss = (ACCMISS*)prwm->psoAmmo;

	// Retail does not read the active JLOC until after confirming that the
	// RWM has produced an ammo object.  A fire animation label may arrive
	// while that object is unavailable; in that case the shot is simply
	// skipped rather than consulting pjlovolCur.
	if (paccmiss == nullptr)
		return;

	JLOC* pjloc = pjlo->pjlovolCur->pjloc;

	GetRwfiPosMat(&prwm->rwfi, &posFire, &matFire, nullptr);

	if (FIsBasicDerivedFrom((BASIC*)paccmiss, CID_ACCMISS)) {
		glm::vec3 dpos = posTarget - posFire;
		float distance = glm::length(dpos);

		if (distance < 0.0001f)
			dpos = g_normalX;
		else
			dpos /= distance;

		float svLaunch;
		float rMissileSpeed = pjloc->rMissileSpeed;
		float rAccelTime = pjloc->rAccelTime;

		if (rMissileSpeed == 1.0f) 
		{
			rAccelTime = pjloc->dtMissile;
			svLaunch = distance / rAccelTime;

			paccmiss->svMissileLaunch = svLaunch;
			paccmiss->svMissileMax = svLaunch;
			paccmiss->sdvMissileMax = 0.0f;
		}
		else 
		{
			svLaunch = (2.0f * distance) / (((rAccelTime - rMissileSpeed * rAccelTime) + 2.0f * rMissileSpeed) * pjloc->dtMissile);

			paccmiss->svMissileLaunch = svLaunch;
			paccmiss->svMissileMax = svLaunch * rMissileSpeed;
			paccmiss->sdvMissileMax = (paccmiss->svMissileMax - svLaunch) / (rAccelTime * pjloc->dtMissile);
		}

		prwm->rwfi.v = dpos * svLaunch;
	}
	else if (FIsBasicDerivedFrom((BASIC*)paccmiss, (CID)62)) 
	{
		prwm->rwfi.v = g_normalZ * pjloc->svLaunch;
		paccmiss->vTarget = g_normalZ * -pjloc->svTarget;
		paccmiss->svMissileMax = pjloc->dtMissile;
	}
	else 
	{
		glm::vec3 v;

		if (!FCalculateMuzzleVelocity(posFire, posTarget, pjloc->radTilt, v, nullptr))
			return;

		float distance = glm::length(posFire - posTarget);
		float speed = glm::length(v);
		float scale = (distance / speed) / pjloc->dtMissile;

		paccmiss->dvGravity = pjlo->psw->dvGravity * (scale * scale);
		prwm->rwfi.v = v * scale;
	}

	prwm->rwfi.tMax = pjloc->dtMissile;
	prwm->rwti.pos = posTarget;

	if (FFireRwm(prwm, 0))
		pjlo->tFireNext = g_clock.t + pjloc->dtFire;
}

void LandJlo(JLO* pjlo)
{
	SetSoConstraints((SO*)pjlo, CT_Locked, nullptr, CT_Locked, nullptr);
	pjlo->pvtalo->pfnSetAloVelocityVec(pjlo, &g_vecZero);
	pjlo->pvtalo->pfnSetAloAngularVelocityVec(pjlo, &g_vecZero);

	glm::vec3 pos;
	GetPntPos(pjlo->pjlovolCur->ppntLand, &pos);
	pos.z += pjlo->dzFeet;
	pjlo->pvtalo->pfnTranslateAloToPos(pjlo, &pos);
}

void JumpJlo(JLO* pjlo)
{
	glm::vec3 posLand;
	GetPntPos(pjlo->pjlovolCur->ppntLand, &posLand);
	posLand.z += pjlo->dzFeet;

	glm::vec3 vLaunch;
	FCalculateMuzzleVelocity(pjlo->xf.posWorld, posLand, 0.5f, vLaunch, pjlo);

	SetSoConstraints(pjlo, CT_Free, nullptr, CT_Locked, nullptr);
	pjlo->pvtalo->pfnSetAloVelocityVec(pjlo, &vLaunch);

	const glm::vec2 dposXy(posLand.x - pjlo->xf.posWorld.x, posLand.y - pjlo->xf.posWorld.y);
	const glm::vec2 velocityXy(vLaunch.x, vLaunch.y);
	float distance = glm::length(dposXy);
	float speed = glm::length(velocityXy);
	float dtFlight = distance / speed;
	pjlo->tStick = g_clock.t + dtFlight;
	pjlo->tLand = pjlo->tStick - pjlo->dtLand;
}

void StartJloAmbient(JLO* pjlo)
{
	if (!pjlo->fAmbientEnabled || FVagPlaying())
		return;

	if (NRandInRange(0, 100) >= 20)
		return;

	// These are the six JLO VAG streams extracted from the original descriptor
	// table at 0x002482D8.  The PC VAG loader resolves the filename recursively
	// beneath Worlds/Sounds, including the "JLO Ambient" directory.
	static const char* const s_jloAmbientFiles[] =
	{
		"JloAmbient_1.wav",
		"JloAmbient_2.wav",
		"JloAmbient_3.wav",
		"JloAmbient_4.wav",
		"JloAmbient_5.wav",
		"JloAmbient_6.wav",
	};

	const int ambientIndex = NRandInRange(0, 5);
	PreloadVag(s_jloAmbientFiles[ambientIndex]);
	pjlo->fStartAmbient = true;
}

void UpdateJloSound(JLO* pjlo)
{
	if (pjlo->fStartAmbient && !FPauseForVag()) 
	{
		pjlo->fStartAmbient = false;
		StartAloAmbient(pjlo->sStart, pjlo->sFull, pjlo, nullptr);
	}
}

void HandleJloMessage(JLO* pjlo, MSGID msgid, void* pv)
{
	HandleAloMessage(pjlo, msgid, pv);

	if (msgid != MSGID_label || pv == nullptr)
		return;

	ASEGLABELMSG* pmsg = static_cast<ASEGLABELMSG*>(pv);

	// JLO labels belong to the animation currently driven by her state machine.
	// Treating this payload as an LO reads the wrong field and prevents the
	// jump-launch label from ever reaching JumpJlo.
	if (pjlo->psma == nullptr || pmsg->pasega != pjlo->psma->pasegaCur)
		return;

	switch (pmsg->label)
	{
		case 765:
		FireJlo(pjlo);
		break;

		case 766:
		StartJloAmbient(pjlo);
		break;

		case 767:
		SetJloJlos(pjlo, JLOS_Idle);
		break;

		case 768:
		JumpJlo(pjlo);
		break;

		default:
		break;
	}
}

void SetJloJlos(JLO* pjlo, JLOS jlos)
{
	if (pjlo->jlos == jlos) 
		return;

	if (pjlo->jlos == JLOS_Land)
		LandJlo(pjlo);

	pjlo->jlos = jlos;
	pjlo->tJlos = g_clock.t;

	int oidState;

	switch (jlos) 
	{
		case JLOS_Jump:
		{
			oidState = 751;

			glm::vec3 posTarget;
			GetPntPos(pjlo->pjlovolCur->ppntLand, &posTarget);

			glm::vec3 dpos = posTarget - pjlo->xf.posWorld;
			pjlo->radTarget = atan2f(dpos.y, dpos.x);
			pjlo->tLand = FLT_MAX;
			pjlo->tStick = FLT_MAX;
			break;
		}

		case JLOS_Land:
		oidState = 752;
		pjlo->tFireNext = g_clock.t + 1.0f;
		break;

		case JLOS_Idle:
		{
			JLOMK jlomk = pjlo->pjlovolCur->pjloc->jlomk;

			if (jlomk == JLOMK_Straight) 
				oidState = GRandInRange(0.0f, 1.0f) < 0.5f ? 756 : 757;
			else if (jlomk == JLOMK_Lobbed)
				oidState = 758;
			else
				oidState = 756;
			break;
		}

		case JLOS_Fire:
		{
			JT* pjt = g_pjt;
			JLOC* pjloc = pjlo->pjlovolCur->pjloc;

			if (pjlo->cShotsUntilTargetJt < 1) {
				pjlo->pxfmTarget = pjlo->pxfmStatic;
				pjlo->pxfmStatic->posLocal = pjt->xf.posWorld;
				pjlo->pxfmStatic->matLocal = g_matIdentity;
				pjlo->cShotsUntilTargetJt = NRandInRange(1, 3);
			}
			else {
				pjlo->pxfmTarget = PxfmChooseJloc(pjloc);
				--pjlo->cShotsUntilTargetJt;
			}

			oidState = pjloc->jlomk == JLOMK_Lobbed ? 754 : 753;

			glm::vec3 posTarget;
			GetXfmPos(pjlo->pxfmTarget, &posTarget);

			glm::vec3 dpos = posTarget - pjlo->xf.posWorld;
			pjlo->radTarget = atan2f(dpos.y, dpos.x);
			break;
		}

		case JLOS_Taunt:
		oidState = 755;
		break;

		default:
		return;
	}

	SetSmaGoal(pjlo->psma, (OID)oidState);
}

void ActivateJlo(JLO* pjlo)
{
	g_pjloCur = pjlo;
}

void DeactivateJlo()
{
	g_pjloCur = nullptr;
}

void DeleteJlo(JLO *pjlo)
{
	delete pjlo;
}

JLOC* NewJloc()
{
	return new JLOC{};
}

void InitJloc(JLOC* pjloc)
{
	InitAlo(pjloc);
	pjloc->dtFire = 2.25;
	pjloc->svTarget = 2000.0;
	pjloc->dtMissile = 2.0;
	pjloc->rAccelTime = 0.8;
	pjloc->rMissileSpeed = 4.0;
	pjloc->radTilt = 0.5;
	pjloc->svLaunch = 1000.0;
}

void SetJlocJlock(JLOC* pjloc, JLOCK jlock)
{
	pjloc->jlock = jlock;
}

void* GetJlocJlock(JLOC* pjloc)
{
	return &pjloc->jlock;
}

void SetJlocDtFire(JLOC* pjloc, float dtFire)
{
	pjloc->dtFire = dtFire;
}

void* GetJlocDtFire(JLOC* pjloc)
{
	return &pjloc->dtFire;
}

void SetJlocDtMissile(JLOC* pjloc, float dtMissile)
{
	pjloc->dtMissile = dtMissile;
}

void* GetJlocDtMissile(JLOC* pjloc)
{
	return &pjloc->dtMissile;
}

void SetJlocJlomk(JLOC* pjloc, JLOMK jlomk)
{
	pjloc->jlomk = jlomk;
}

void* GetJlocJlomk(JLOC* pjloc)
{
	return &pjloc->jlomk;
}

void SetJlocRAccelTime(JLOC* pjloc, float rAccelTime)
{
	pjloc->rAccelTime = rAccelTime;
}

void* GetJlocRAccelTime(JLOC* pjloc)
{
	return &pjloc->rAccelTime;
}

void SetJlocRMissileSpeed(JLOC* pjloc, float rMissileSpeed)
{
	pjloc->rMissileSpeed = rMissileSpeed;
}

void* GetJlocRMissileSpeed(JLOC* pjloc)
{
	return &pjloc->rMissileSpeed;
}

void SetJlocRadTilt(JLOC* pjloc, float radTilt)
{
	pjloc->radTilt = radTilt;
}

void* GetJlocRadTilt(JLOC* pjloc)
{
	return &pjloc->radTilt;
}

void SetJlocSvLaunch(JLOC* pjloc, float svLaunch)
{
	pjloc->svLaunch = svLaunch;
}

void* GetJlocSvLaunch(JLOC* pjloc)
{
	return &pjloc->svLaunch;
}

void SetJlocSvTarget(JLOC* pjloc, float svTarget)
{
	pjloc->svTarget = svTarget;
}

void* GetJlocSvTarget(JLOC* pjloc)
{
	return &pjloc->svTarget;
}

int GetJlocSize()
{
	return sizeof(JLOC);
}

void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjloc, pbis);

	pjloc->cpxfm = CploFindSwObjectsByClass(pjloc->psw, 0x201, CID_XFM, pjloc, 16, reinterpret_cast<LO**>(pjloc->apxfm));
	pjloc->cpxfm = std::min(pjloc->cpxfm, 16);
}

void CloneJloc(JLOC* pjloc, JLOC* pjlocBase)
{
	CloneAlo(pjloc, pjlocBase);

	pjloc->cpxfm = pjlocBase->cpxfm;
	for (int i = 0; i < pjloc->cpxfm; ++i)
	{
		pjloc->apxfm[i] = pjlocBase->apxfm[i];
	}

	pjloc->jlock = pjlocBase->jlock;
	pjloc->jlomk = pjlocBase->jlomk;
	pjloc->dtFire = pjlocBase->dtFire;
	pjloc->dtMissile = pjlocBase->dtMissile;
	pjloc->rAccelTime = pjlocBase->rAccelTime;
	pjloc->rMissileSpeed = pjlocBase->rMissileSpeed;
	pjloc->radTilt = pjlocBase->radTilt;
	pjloc->svLaunch = pjlocBase->svLaunch;
	pjloc->svTarget = pjlocBase->svTarget;
}

void PostJlocLoad(JLOC* pjloc)
{
	PostAloLoad(pjloc);
	pjloc->pvtlo->pfnRemoveLo(pjloc);
}

XFM* PxfmChooseJloc(JLOC* pjloc)
{
	if (pjloc->cpxfm <= 0) {
		return nullptr;
	}

	switch (pjloc->jlock) 
	{
		case JLOCK_Closest:
		{
			PO* ppo = PpoCur();
			XFM* pxfmClosest = nullptr;
			float distanceSqClosest = FLT_MAX;

			for (int i = 0; i < pjloc->cpxfm; ++i) {
				XFM* pxfm = pjloc->apxfm[i];

				glm::vec3 pos;
				GetXfmPos(pxfm, &pos);

				float distanceSq = glm::length2(pos - ppo->xf.posWorld);

				if (distanceSq < distanceSqClosest) {
					distanceSqClosest = distanceSq;
					pxfmClosest = pxfm;
				}
			}

			return pxfmClosest;
		}

		case JLOCK_Random:
		{
			int i = NRandInRange(0, pjloc->cpxfm - 1);
			return pjloc->apxfm[i];
		}

		default:
		return nullptr;
	}
}

void DeleteJloc(JLOC* pjloc)
{
	delete pjloc;
}

JLO* g_pjloCur;
int s_avagdescJlo[4];
SNIP s_asnipLoadJlo[5] = 
{
	0, (OID)0x2EE, offsetof(JLO, psm),
	0, (OID)0x2F8, offsetof(JLO, paloReticle),
	0, (OID)0x2F9, offsetof(JLO, paloTracer),
	0, (OID)0x2FB, offsetof(JLO, pasegReticlePop),
	0, (OID)0x2FC, offsetof(JLO, pasegReticleConverge)
};
SNIP s_asnipPostJloLoad[2] = 
{
	0, (OID)0x2FA, offsetof(JLO, ppntTracerStart),
	0, (OID)0x2F7, offsetof(JLO, ppntFeet)
};
