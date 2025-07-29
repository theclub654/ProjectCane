#include "jlo.h"

JLO* NewJlo()
{
	return new JLO{};
}

void InitJlo(JLO* pjlo)
{
	InitSo(pjlo);
	pjlo->jlos = JLOS_Nil;
	pjlo->oidJlovolStart = OID_Nil;
	InitDl(&pjlo->dlJlovol, offsetof(JLOVOL, dleJlo));
	pjlo->smpSpin.svFast = 25.0;
	pjlo->smpSpin.dtFast = 0.25;
	pjlo->smpSpin.svSlow = 0.0;
	g_pjloCur = nullptr;
}

int GetJloSize()
{
	return sizeof(JLO);
}

void LoadJloFromBrx(JLO* pjlo, CBinaryInputStream* pbis)
{
	LoadSoFromBrx(pjlo, pbis);
	SnipAloObjects(pjlo, 5, s_asnipLoadJlo);

	SM* sm = pjlo->psm;
	if (sm == nullptr || sm->csmt == 0) return;

	// Look for a SMT transition from "jump" to "land"
	for (int i = 0; i < sm->csmt; ++i) {
		SMT* smt = &sm->asmt[i];
		OID from = OidFromSmIsms(sm, smt->ismsFrom);
		OID to = OidFromSmIsms(sm, smt->ismsTo);

		if (from == OID_state_jlo_jump && to == OID_state_jlo_land) {
			if (smt->paseg) {
				pjlo->dtLand = smt->paseg->tMax;
			}
			break;
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
	// Perform ALO post-load and prune unused objects
	PostAloLoad((ALO*)pjlo);
	SnipAloObjects(pjlo, 2, s_asnipPostJloLoad);

	// Apply SMA (state machine action) to this JLO
	//pjlo->psma = PsmaApplySm(pjlo->psm, (ALO*)pjlo, OID_Nil, 1);

	// Find RWM (rigid world motion) object for this JLO
	pjlo->prwm = (RWM*)PloFindSwObjectByClass(pjlo->psw, 1, CID_RWM, (LO*)pjlo);

	// Compute foot offset
	glm::vec3 posFeet;
	GetPntPos(pjlo->ppntFeet, &posFeet);
	pjlo->dzFeet = pjlo->xf.posWorld.z - posFeet.z;

	// Find nearby JLOVOL (volume this JLO belongs to)
	JLOVOL* pJlovol = (JLOVOL*)PloFindSwNearest(pjlo->psw, pjlo->oidJlovolStart, (LO*)pjlo);
	/*if (pJlovol != nullptr) {
		SetJloJlovol(pjlo, pJlovol);
		LandJlo(pjlo);

		if (pJlovol->pjloc) {
			SetJloJlos(pjlo, JLOS_Idle);
		}
		else {
			SetJloJlos(pjlo, JLOS_Taunt);
		}
	}*/

	// Enable camera BSPC override flag (0x40000000000)
	//*(uint64_t*)&pjlo->bspcCamera.absp |= 0x40000000000ULL;

	// Allocate static transform object for JLO
	pjlo->pxfmStatic = (XFM*)PloNew(CID_XFM, g_psw, nullptr, OID_xfm_jlo_static, -1);
}

void UpdateJlo(JLO* pjlo, float dt)
{
	
}

void DeleteJlo(JLO* pjlo)
{
	delete pjlo;
}

JLOVOL* NewJlovol()
{
	return new JLOVOL{};
}

void InitJlovol(JLOVOL* pjlovol)
{
	InitVolbtn(pjlovol);
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
	{
		/*PNT *ppnt0 = (PNT*)PloFindSwObjectByClass(pjlovol->psw, 2, CID_PNT, (LO*)pjlovol);
		pjlovol->ppntLand = ppnt0;*/
	}
	else 
	{
		PNT *ppnt1 = (PNT*)PloFindSwObject(pjlovol->psw, 4, pjlovol->oidLand, (LO*)pjlovol);
		pjlovol->ppntLand = ppnt1;
	}

	JLOC *pjloc = (JLOC*)PloFindSwObject(pjlovol->psw, 4, pjlovol->oidJloc, (LO*)pjlovol);
	pjlovol->pjloc = pjloc;
}

void DeleteJlovol(JLOVOL* pjlovol)
{
	delete pjlovol;
}

JLOC* NewJloc()
{
	return new JLOC{};
}

void InitJloc(JLOC* pjloc)
{
	InitAlo(pjloc);
}

int GetJlocSize()
{
	return sizeof(JLOC);
}

void LoadJlocFromBrx(JLOC* pjloc, CBinaryInputStream* pbis)
{
	LoadAloFromBrx(pjloc, pbis);

	int count = CploFindSwObjectsByClass(pjloc->psw, 0x201, CID_XFM, pjloc, 0x10, (LO**)pjloc->apxfm);

	pjloc->cpxfm = count;
	if (0x10 < count) {
		pjloc->cpxfm = 0x10;
	}
}

void CloneJloc(JLOC* pjloc, JLOC* pjlocBase)
{
	CloneAlo(pjloc, pjlocBase);

	// Clone the array of XFM pointers
	pjloc->cpxfm = pjlocBase->cpxfm;
	for (int i = 0; i < pjloc->cpxfm; ++i)
	{
		pjloc->apxfm[i] = pjlocBase->apxfm[i]; // Assuming shallow copy is enough for XFM pointers
	}

	// Clone other members
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

void DeleteJloc(JLOC* pjloc)
{
	delete pjloc;
}

SNIP s_asnipLoadJlo[5] =
{
	0, OID_sm_jlo, offsetof(JLO, psm),
	0, OID_jlo_reticle, offsetof(JLO, paloReticle),
	0, OID_jlo_tracer, offsetof(JLO, paloTracer),
	0, OID_aseg_reticle_pop, offsetof(JLO, pasegReticlePop),
	0, OID_aseg_reticle_converge, offsetof(JLO, pasegReticleConverge)
};

SNIP s_asnipPostJloLoad[2] =
{
	0, OID_pnt_jlo_tracer, offsetof(JLO, ppntTracerStart),
	0, OID_jlo_feet_point, offsetof(JLO, ppntFeet)
};

JLO* g_pjloCur = nullptr;