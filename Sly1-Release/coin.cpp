#include "coin.h"
#include "target.h"
#include "chkpnt.h"
#include "acr.h"
#include "acp.h"
#include "sm.h"
#include "jt.h"
#include "emitter.h"
#include "expl.h"
#include "binoc.h"
#include "smack.h"
#include "credit.h"
#include "glint.h"

DPRIZE* NewDprize()
{
	return new DPRIZE{};
}

void InitSwDprizeDl(SW* psw)
{
	InitDl(&psw->dlDprize, offsetof(DPRIZE, dle));
}

void InitDprize(DPRIZE* pdprize)
{
	pdprize->dprizesInit = DPRIZES_Normal;
	pdprize->dprizes = DPRIZES_Nil;
	pdprize->svcAttract = 30.0;
	pdprize->oidInitialState = OID_Nil;
	InitAlo(pdprize);
	AppendDlEntry(&pdprize->psw->dlDprize, pdprize);
	pdprize->ichkCollected = IchkAllocChkmgr(&g_chkmgr);
	pdprize->fCollectible = 1;
	pdprize->ficg.grficBomb = 0x10;
	pdprize->ficg.grficSmash = 0x10;
	pdprize->ficg.grficRush = 0x10;
	pdprize->ficg.grficSweep = 0x10;
}

void SetDprizeOidInitialState(DPRIZE* pdprize, OID oidInitialState)
{
	pdprize->oidInitialState = oidInitialState;
}

void* GetDprizeOidInitialState(DPRIZE* pdprize)
{
	return &pdprize->oidInitialState;
}

void SetDprizeDtInitialSkip(DPRIZE* pdprize, float dtInitialSkip)
{
	pdprize->dtInitialSkip = dtInitialSkip;
}

void* GetDprizeDtInitialSkip(DPRIZE* pdprize)
{
	return &pdprize->dtInitialSkip;
}

void* GetDprizeDprizes(DPRIZE* pdprize)
{
	return &pdprize->dprizes;
}

void SetDprizeFNeverReuse(DPRIZE* pdprize, int fNeverReuse)
{
	pdprize->fNeverReuse = fNeverReuse;
}

void* GetDprizeFNeverReuse(DPRIZE* pdprize)
{
	return &pdprize->fNeverReuse;
}

void SetDprizeSvcAttract(DPRIZE* pdprize, float svcAttract)
{
	pdprize->svcAttract = svcAttract;
}

void* GetDprizeSvcAttract(DPRIZE* pdprize)
{
	return &pdprize->svcAttract;
}

void SetDprizeFCollectible(DPRIZE* pdprize, int fCollectible)
{
	pdprize->fCollectible = fCollectible;
}

void* GetDprizeFCollectible(DPRIZE* pdprize)
{
	return &pdprize->fCollectible;
}

void* AccessDprizeOidInitialState(void* pv)
{
	DPRIZE* pdprize = static_cast<DPRIZE*>(pv);
	return &pdprize->oidInitialState;
}

void* AccessDprizeDtInitialSkip(void* pv)
{
	DPRIZE* pdprize = static_cast<DPRIZE*>(pv);
	return &pdprize->dtInitialSkip;
}

void* AccessDprizeDprizes(void* pv)
{
	DPRIZE* pdprize = static_cast<DPRIZE*>(pv);
	return &pdprize->dprizes;
}

void* AccessDprizeFNeverReuse(void* pv)
{
	DPRIZE* pdprize = static_cast<DPRIZE*>(pv);
	return &pdprize->fNeverReuse;
}

void* AccessDprizeSvcAttract(void* pv)
{
	DPRIZE* pdprize = static_cast<DPRIZE*>(pv);
	return &pdprize->svcAttract;
}

void* AccessDprizeFCollectible(void* pv)
{
	DPRIZE* pdprize = static_cast<DPRIZE*>(pv);
	return &pdprize->fCollectible;
}

int GetDprizeSize()
{
	return sizeof(DPRIZE);
}

void LoadDprizeFromBrx(DPRIZE* pdprize, CBinaryInputStream* pbis)
{
	SetAloTargetHitTest(pdprize, 1);
	LoadAloFromBrx(pdprize, pbis);
	SnipAloObjects(pdprize, 5, s_asnipDprize);
}

void CloneDprize(DPRIZE* pdprize, DPRIZE* pdprizeBase)
{
	DLE savedDle = pdprize->dle;
	int savedIchkCollected = pdprize->ichkCollected;

	CloneAlo(pdprize, pdprizeBase);

	pdprize->dle = savedDle;
	pdprize->ichkCollected = savedIchkCollected;

	pdprize->dprizes = pdprizeBase->dprizes;
	pdprize->tDprizes = pdprizeBase->tDprizes;
	pdprize->dprizesInit = pdprizeBase->dprizesInit;
	pdprize->oidInitialState = pdprizeBase->oidInitialState;
	pdprize->dtInitialSkip = pdprizeBase->dtInitialSkip;

	pdprize->psm = pdprizeBase->psm;
	pdprize->psma = pdprizeBase->psma;
	pdprize->ptarget = pdprizeBase->ptarget;

	pdprize->posCenter = pdprizeBase->posCenter;
	pdprize->vCenter = pdprizeBase->vCenter;
	pdprize->dvCenter = pdprizeBase->dvCenter;

	pdprize->uGlintChance = pdprizeBase->uGlintChance;
	pdprize->ppntFrontGlint = pdprizeBase->ppntFrontGlint;
	pdprize->ppntBackGlint = pdprizeBase->ppntBackGlint;

	pdprize->fLeft = pdprizeBase->fLeft;
	pdprize->tGlint = pdprizeBase->tGlint;

	pdprize->fNeverReuse = pdprizeBase->fNeverReuse;
	pdprize->fReuseCandidate = pdprizeBase->fReuseCandidate;
	pdprize->fLastBounce = pdprizeBase->fLastBounce;

	pdprize->svLastBounceMax = pdprizeBase->svLastBounceMax;
	pdprize->svLastBounce = pdprizeBase->svLastBounce;
	pdprize->sRadiusBounce = pdprizeBase->sRadiusBounce;
	pdprize->sRadiusCollect = pdprizeBase->sRadiusCollect;
	pdprize->rzBounce = pdprizeBase->rzBounce;
	pdprize->rxyBounce = pdprizeBase->rxyBounce;
	pdprize->radSmooth = pdprizeBase->radSmooth;
	pdprize->normalSmooth = pdprizeBase->normalSmooth;

	pdprize->fSwirlDone = pdprizeBase->fSwirlDone;

	pdprize->pexplCollect = pdprizeBase->pexplCollect;
	pdprize->pexplAttract = pdprizeBase->pexplAttract;
	pdprize->svcAttract = pdprizeBase->svcAttract;
	pdprize->cAttract = pdprizeBase->cAttract;
}

void PostDprizeLoad(DPRIZE* pdprize)
{
	PostAloLoad(pdprize);

	pdprize->ptarget = reinterpret_cast<TARGET*>(PloFindSwObjectByClass(pdprize->psw, 1, CID_TARGET, pdprize));

	if (pdprize->ptarget != nullptr)
		pdprize->ptarget->pvtlo->pfnRemoveLo(pdprize->ptarget);

	const int levelId = (g_pgsCur->gameWorldCur << 8) | g_pgsCur->worldLevelCur;

	if (levelId != 776 && FGetChkmgrIchk(&g_chkmgr, pdprize->ichkCollected) != 0)
		pdprize->dprizesInit = DPRIZES_Removed;

	pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, pdprize->dprizesInit);
}

void ProjectDprizeTransform(DPRIZE* pdprize, float dt, int fParentDirty)
{
	switch (pdprize->dprizes)
	{
		case DPRIZES_Stick:
		{
			PZI* ppzi = PpziCur();

			const float dtRemaining = (pdprize->tDprizes + DT_DprizeStick) - g_clock.t;

			glm::vec3 posTarget;
			glm::mat3 matTarget;

			// Release uses charm index 0.
			PredictAloTransform(ppzi->mpccharmpaloCharm[0], nullptr, dtRemaining, &posTarget, &matTarget, nullptr, nullptr);

			glm::mat3 dmat;
			CalculateDmat(&pdprize->xf.mat, &matTarget, &dmat);

			float rad;
			glm::vec3 normal;
			DecomposeRotateMatrixRad(&dmat, &rad, &normal);

			// Advance only the fraction of the remaining stick interval covered by
			// this frame. Using min() made u equal to one for almost the entire
			// phase and snapped the prize directly to the overhead target.
			const float u = dt / std::max(dt, dtRemaining);

			LoadRotateMatrixRad(rad * u, &normal, &dmat);

			pdprize->xf.mat = dmat * pdprize->xf.mat;
			pdprize->xf.pos = glm::mix(pdprize->xf.pos, posTarget, u);
			break;
		}

		case DPRIZES_Swirl:
		{
			PZI* ppzi = PpziCur();

			glm::vec3 posTarget = s_posTarget;

			float radTarget = std::atan2(ppzi->paloCollect->xf.matWorld[0].y, ppzi->paloCollect->xf.matWorld[0].x);
			radTarget = RadNormalize(radTarget);
			radTarget += 15.707964f;

			const float dRad = radTarget - pdprize->posCenter.x;

			if (std::abs(dRad + 6.2831855f) < std::abs(dRad))
				radTarget += 6.2831855f;

			posTarget.x = radTarget;

			for (int i = 0; i < 3; ++i)
				pdprize->posCenter[i] = GSmoothA(pdprize->posCenter[i], pdprize->vCenter[i], posTarget[i], dt, &s_asmpaSwirl[i], &pdprize->vCenter[i]);

			// The release gives the swirl a fixed one-second presentation window.
			// GSmoothA approaches the target but is not guaranteed to land within
			// an exact 0.0001 epsilon on a variable PC frame step, which left the
			// charm permanently hovering above Sly. The elapsed-time condition is
			// the authoritative completion condition; the epsilon merely permits
			// an early finish if all three channels settle first.
			if (g_clock.t - pdprize->tDprizes >= DT_DprizeSwirl ||
				glm::all(glm::epsilonEqual(pdprize->posCenter, posTarget, 0.0001f)))
				pdprize->fSwirlDone = true;

			glm::vec3 posOffset;
			SetVectorCylind(&posOffset, pdprize->posCenter.x, pdprize->posCenter.y, pdprize->posCenter.z);

			pdprize->xf.pos = ppzi->paloBag->xf.posWorld + posOffset;
			pdprize->xf.v = glm::vec3(0.0f);

			glm::vec3 dpos = ppzi->paloBag->xf.posWorld - pdprize->xf.pos;
			dpos.z *= RZ_DprizeSwirlLookAt;

			glm::mat3 matLookAt;
			glm::mat3 matTilt;
			glm::mat3 matSmooth;

			LoadLookAtMatrix(&dpos, &matLookAt);
			LoadRotateMatrixRad(-2.5f, &g_normalX, &matTilt);

			matLookAt = matLookAt * matTilt;

			pdprize->radSmooth = RadSmooth(pdprize->radSmooth, 0.0f, dt, &s_smpRadSmooth, nullptr);
			LoadRotateMatrixRad(pdprize->radSmooth, &pdprize->normalSmooth, &matSmooth);

			pdprize->xf.mat = matSmooth * matLookAt;
			break;
		}

		case DPRIZES_Attract:
		{
			float dtPredict = g_pjt->tContact - g_clock.t;
			dtPredict = std::max(dtPredict, 0.0f);

			const float u = dt / (dtPredict + dt);

			glm::vec3 posTarget;
			PredictAloTransform(g_pjt->paloHook, nullptr, dtPredict, &posTarget, nullptr, nullptr, nullptr);

			pdprize->xf.pos = glm::mix(pdprize->xf.pos, posTarget, u);
			break;
		}

		default:
		{
			float damping = 1.0f;

			if (pdprize->dprizes == DPRIZES_Normal || pdprize->dprizes == DPRIZES_Lose)
				damping += R_DprizeDamping * dt;

			pdprize->vCenter = pdprize->vCenter * damping + pdprize->dvCenter * dt;
			pdprize->posCenter += pdprize->vCenter * dt;

			pdprize->xf.pos = pdprize->posCenter;
			pdprize->xf.v = pdprize->vCenter;
			break;
		}
	}

	ASEGA* pasega = pdprize->psma != nullptr ? pdprize->psma->pasegaCur : nullptr;

	if (pasega != nullptr && pasega->paseg->pchnStrip != nullptr)
	{
		CHN* pchnStrip = pasega->paseg->pchnStrip;

		glm::vec3 dposCamera = g_pcm->pos - pdprize->xf.pos;
		dposCamera.z = 0.0f;

		glm::mat3 matStrip;
		BuildOrthonormalMatrixZ(dposCamera, g_normalZ, matStrip);

		if (pchnStrip->pacp != nullptr)
		{
			glm::vec3 dpos;
			glm::vec3 dv;

			pchnStrip->pacp->pvtacp->pfnEvaluateAcp(pchnStrip->pacp.get(), pdprize, pasega->tLocal, pasega->svtLocal, 0, &dpos, &dv);

			pdprize->xf.pos += matStrip * dpos;
			pdprize->xf.v += matStrip * dv;
		}

		if (pchnStrip->pacr != nullptr)
		{
			glm::mat3 matLocal;
			glm::vec3 wLocal;

			pchnStrip->pacr->pvtacr->pfnEvaluateAcr(pchnStrip->pacr.get(), pdprize, pasega->tLocal, pasega->svtLocal, 0, &matLocal, &wLocal);

			pdprize->xf.mat = matStrip * matLocal;
			pdprize->xf.w = matStrip * wLocal;
		}
	}

	DLI dli;

	dli.m_pdl = &pdprize->dlChild;
	dli.m_ibDle = pdprize->dlChild.ibDle;
	dli.m_pdliNext = s_pdliFirst;

	LO* ploChild = pdprize->dlChild.ploFirst;

	if (ploChild != nullptr)
		dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(ploChild) + dli.m_ibDle);
	else
		dli.m_ppv = nullptr;

	s_pdliFirst = &dli;

	while (ploChild != nullptr)
	{
		if ((ploChild->pvtalo->grfcid & 1U) != 0 && ploChild->pvtalo->pfnProjectAloTransform != nullptr)
			ploChild->pvtalo->pfnProjectAloTransform(reinterpret_cast<ALO*>(ploChild), dt, true);

		ploChild = *reinterpret_cast<LO**>(dli.m_ppv);

		if (ploChild != nullptr)
			dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(ploChild) + dli.m_ibDle);
		else
			dli.m_ppv = nullptr;
	}

	if (!fParentDirty && pdprize->pvtlo->pfnUpdateLoXfWorld != nullptr)
		pdprize->pvtlo->pfnUpdateLoXfWorld(pdprize);

	s_pdliFirst = dli.m_pdliNext;
}

int FFilterDprizeObjects(DPRIZE* pdprize, SO* pso)
{
	if (pso->fNoXpsSelf)
		return false;

	ALO* paloRoot = pso->paloRoot;

	return
		FIsBasicDerivedFrom(paloRoot, CID_PO) == 0 &&
		FIsBasicDerivedFrom(paloRoot, CID_BRKP) == 0 &&
		FIsBasicDerivedFrom(paloRoot, CID_BOMB) == 0 &&
		FIsBasicDerivedFrom(paloRoot, CID_FLY) == 0 &&
		FIsBasicDerivedFrom(paloRoot, CID_CLUE) == 0;
}

void CheckDprizeBounce(DPRIZE* pdprize)
{
	float penetrationBest = 0.0f;
	glm::vec3 normalBest(0.0f);

	const glm::vec3 radius(pdprize->sRadiusBounce);
	const glm::vec3 posMin = pdprize->xf.posWorld - radius;
	const glm::vec3 posMax = pdprize->xf.posWorld + radius;

	std::vector<SO*> found;
	IntersectSwBoundingBox(pdprize->psw, nullptr, &posMin, &posMax, (PFNFILTER)FFilterDprizeObjects, pdprize, found);

	for (SO* pso : found)
	{
		// The original reads fSphere from a packed flag word adjacent to the
		// camera BSP. In the port BSPC owns a std::vector, so reinterpreting the
		// vector bytes as that flag word makes this depend on pointer bits and
		// randomly treats ordinary floor geometry as a sphere.
		const bool fUseSphere = pso->fSphere != 0;

		if (!fUseSphere)
		{
			if (pso->bspc.absp.size() == 0)
				continue;

			std::array <SBI, 8> asbi;

			const int csbi = CsbiIntersectSphereBsp(&pdprize->xf.posWorld, pdprize->sRadiusBounce, pso->bspc.cbsp, pso->bspc.absp.data(), nullptr, &pso->geomWorld, static_cast<int>(asbi.size()), asbi.data());

			for (int isbi = 0; isbi < csbi; ++isbi)
			{
				const SBI& sbi = asbi[isbi];

				if (sbi.gDist >= penetrationBest)
					continue;

				if (glm::dot(pdprize->vCenter, sbi.normal) >= 0.0f)
					continue;

				normalBest = sbi.normal;
				penetrationBest = sbi.gDist;
			}
		}
		else
		{
			const glm::vec3 dposWorld = pso->xf.posWorld - pdprize->xf.posWorld;
			const float penetration = glm::length(dposWorld) - pso->sRadiusSelf - pdprize->sRadiusBounce;

			if (penetration >= penetrationBest)
				continue;

			glm::vec3 normal = pdprize->posCenter - pso->xf.posWorld;
			const float normalLength = glm::length(normal);

			if (normalLength < 0.0001f)
				normal = g_normalX;
			else
				normal /= normalLength;

			if (glm::dot(pdprize->vCenter, normal) >= 0.0f)
				continue;

			normalBest = normal;
			penetrationBest = penetration;
		}
	}

	if (penetrationBest >= 0.0f)
		return;

	glm::vec3 vNormal;
	glm::vec3 vTangent;
	glm::vec3 vBounce;

	if (normalBest.z <= 0.7f || pdprize->dprizes != DPRIZES_Fall)
	{
		SProjectVector(&pdprize->vCenter, &normalBest, &vNormal, &vTangent);
		vBounce = vNormal * -pdprize->rzBounce + vTangent * pdprize->rxyBounce;
	}
	else
	{
		SProjectVector(&pdprize->vCenter, &g_normalZ, &vNormal, &vTangent);
		vBounce = vNormal * -pdprize->rzBounce + vTangent * pdprize->rxyBounce;

		if (!pdprize->fLastBounce && glm::dot(vBounce, normalBest) < pdprize->svLastBounce)
		{
			pdprize->fLastBounce = true;

			vTangent.z = std::max(vTangent.z, 0.0f);
			vBounce = vTangent + g_normalZ * pdprize->svLastBounce;
		}
	}

	pdprize->posCenter -= normalBest * penetrationBest;
	pdprize->vCenter = vBounce;
}

void CheckDprizeCollect(DPRIZE* pdprize)
{
	if (pdprize->fCollectible == 0)
		return;

	PZI* ppzi = PpziCur();

	if (!ppzi->fCollectEnabled)
		return;

	SO* paloCollect = (SO*)ppzi->paloCollect;

	if (paloCollect == nullptr)
		return;

	const glm::vec3 dpos = pdprize->xf.posWorld - paloCollect->xf.posWorld;
	const float distSq = glm::dot(dpos, dpos);

	if (distSq > S_DprizeCollectMax * S_DprizeCollectMax)
		return;

	if (distSq < ppzi->sAutoCollect * ppzi->sAutoCollect)
	{
		pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Collect);
		return;
	}

	const uint64_t grfvault = GetAvailableVaultFlags();
	
	if ((grfvault & 32) != 0 && FIsBasicDerivedFrom(pdprize, CID_COIN) != 0)
	{
		glm::vec3 vCollect;
		glm::vec3 vDprize;

		CalculateAloMovement(paloCollect, nullptr, paloCollect->xf.posWorld, &vCollect, nullptr, nullptr, nullptr);
		CalculateAloMovement(pdprize, nullptr, pdprize->xf.posWorld, &vDprize, nullptr, nullptr, nullptr);

		glm::vec3 normal(0.0f);
		const float distance = glm::length(dpos);

		if (distance >= 0.0001f)
			normal = dpos / distance;

		const glm::vec3 dv = vDprize - vCollect;
		const float svRelative = glm::dot(dv, normal);

		if (svRelative > -50.0f)
		{
			pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Collect);
			return;
		}
	}

	if ((paloCollect->pvtalo->grfcid & 2U) == 0)
		return;

	if (paloCollect->bspc.absp.size() == 0)
		return;

	SBI sbi;

	const int csbi = CsbiIntersectSphereBsp(&pdprize->xf.posWorld, pdprize->sRadiusCollect, paloCollect->bspc.cbsp, paloCollect->bspc.absp.data(), nullptr, &paloCollect->geomWorld, 1, &sbi);

	if (csbi != 0)
		pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Collect);
}

int FAbsorbDprizeWkr(DPRIZE* pdprize, WKR* pwkr)
{
	const int fAbsorbed = (pdprize->fCollectible != 0) && (pwkr->grfic & 16U) != 0;
	if (fAbsorbed)
		pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Collect);
	return fAbsorbed;
}

void UpdateDprize(DPRIZE* pdprize, float dt)
{
	UpdateAlo(pdprize, dt);

	glm::vec3 normal;
	ConvertAloVec(pdprize, nullptr, &s_normalGlint, &normal);

	const bool fLeft = glm::dot(normal, g_pcm->mat[1]) < 0.0f;

	if (fLeft != pdprize->fLeft)
	{
		const float uGlintRoll = GRandInRange(0.0f, 1.0f);
		if (uGlintRoll < pdprize->uGlintChance &&
			g_clock.t - pdprize->tGlint > DT_DprizeGlintIntervalMin)
		{
			PNT* ppntGlint;

			if (glm::dot(normal, g_pcm->mat[0]) < 0.0f)
				ppntGlint = pdprize->ppntFrontGlint;
			else
				ppntGlint = pdprize->ppntBackGlint;

			if (ppntGlint != nullptr)
			{
				RIP* prip = PripNewRipg(RIPT_Glint, nullptr);

				if (prip != nullptr)
				{
					pdprize->tGlint = g_clock.t;

					prip->pvtrip->pfnInitRip(prip, &ppntGlint->posLocal, 1.0f, nullptr);
					static_cast<GLINT*>(prip)->paloParent = pdprize;

					const float sw = std::max(std::abs(pdprize->xf.w.z), 1.0f);
					prip->dtLifetime = 1.5f / sw;
				}
			}
		}

		pdprize->fLeft = fLeft;
	}

	if (pdprize->dprizes == DPRIZES_Attract && pdprize->pexplAttract != nullptr)
	{
		pdprize->cAttract += dt * pdprize->svcAttract;

		if (pdprize->cAttract >= 1.0f)
		{
			EXPLSO explso{};
			explso.grfexplso = 129;
			explso.paloReference = pdprize;

			pdprize->pexplAttract->pvtexpl->pfnExplodeExplExplso((EXPLO*)pdprize->pexplAttract, &explso);

			pdprize->cAttract -= std::floor(pdprize->cAttract);
		}
	}

	switch (pdprize->dprizes)
	{
		case DPRIZES_Normal:
		if (glm::dot(pdprize->vCenter, pdprize->vCenter) > 10000.0f)
			CheckDprizeBounce(pdprize);

		[[fallthrough]];

		case DPRIZES_Lose:
		CheckDprizeCollect(pdprize);
		break;

		case DPRIZES_Fall:
		{
			const float dtFall = g_clock.t - pdprize->tDprizes;

			if (dtFall > DT_DprizeFallMax)
			{
				pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Removed);
				break;
			}

			if (pdprize->fLastBounce && pdprize->vCenter.z < 25.0f)
			{
				pdprize->fLastBounce = false;
				pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Normal);
				break;
			}

			CheckDprizeBounce(pdprize);

			if (dtFall > 1.0f)
			{
				if (pdprize->ptarget != nullptr)
					pdprize->ptarget->pvtlo->pfnAddLo(pdprize->ptarget);

				CheckDprizeCollect(pdprize);
			}

			break;
		}

		case DPRIZES_Swirl:
		{
			PZI* ppzi = PpziCur();

			if (!ppzi->fCollectEnabled)
			{
				pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Fall);
				break;
			}
			
			if (pdprize->fSwirlDone)
				pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Stick);

			break;
		}

		case DPRIZES_Stick:
		{
			PZI* ppzi = PpziCur();

			if (ppzi->fCollectEnabled && g_pgsCur->ccharm < CcharmMost())
			{
				if (g_clock.t - pdprize->tDprizes > DT_DprizeStick)
				{
					++g_pgsCur->ccharm;
					pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Removed);
					RemoveSwExtraneousCharms(g_psw);
				}

				break;
			}

			ALO* paloCollect = ppzi->paloCollect;
			glm::vec3 vReject = paloCollect->xf.mat * s_vReject + paloCollect->xf.v;

			pdprize->pvtalo->pfnSetAloVelocityVec(pdprize, &vReject);
			pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Fall);
			break;
		}

		case DPRIZES_Attract:
		if (g_pjt->tContact <= g_clock.t)
			pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Collect);

			break;

		default:
		break;
	}

	ResolveAlo(pdprize);
}

void RenderDprizeAll(DPRIZE* pdprize, CM* pcm, RO* pro)
{
	if (pdprize->fHidden != 0)
		return;

	RO roLocal;
	RO* proRender = pro;

	const DPRIZES state = pdprize->dprizes;

	if (state > DPRIZES_Fall && state < DPRIZES_Lose)
	{
		const bool isSwirl = state == DPRIZES_Swirl;

		const float duration = isSwirl ? DT_DprizeSwirl : DT_DprizeStick;
		const CLQ* pclq = isSwirl ? &s_clqUToRSwirl : &s_clqUToRStick;

		float t = (g_clock.t - pdprize->tDprizes) / duration;
		t = glm::clamp(t, 0.0f, 1.0f);

		const float scale = pclq->g0 + t * (pclq->g1 + t * pclq->g2);
		const glm::mat4 matScale = glm::scale(glm::mat4(1.0f), glm::vec3(scale));

		DupAloRo(pdprize, pro, &roLocal);

		roLocal.model = roLocal.model * matScale;
		proRender = &roLocal;
	}

	RenderAloAll(pdprize, pcm, proRender);
}

void SetDprizeDprizes(DPRIZE* pdprize, DPRIZES dprizes)
{
	if (pdprize->dprizes == dprizes)
		return;

	OID oidState = OID_Nil;
	PZI* ppzi = PpziCur();

	// The PS2 clone starts with a shallow copy of PZI, including paloBag.
	// Resolve it here as well because our field-by-field clone may reach prize
	// collection before the playable object's post-load binding is available.
	if (ppzi != nullptr && ppzi->paloBag == nullptr)
	{
		PO* ppo = PpoCur();

		if (ppo != nullptr)
		{
			ppzi->paloBag = PaloFindPoBag(ppo);

			if (ppzi->paloBag == nullptr)
				ppzi->paloBag = static_cast<ALO*>(
					PloFindSwObject(ppo->psw, 0x104, OID_step_bag, ppo));

			if (ppzi->paloBag == nullptr)
				ppzi->paloBag = ppzi->mpccharmpaloCharm[0];

			if (ppzi->paloBag == nullptr)
				ppzi->paloBag = ppzi->paloCollect;
		}
	}

	if (dprizes != DPRIZES_Normal && dprizes != DPRIZES_Lose)
		pdprize->pvtlo->pfnSetLoParent(pdprize, nullptr);

	const DPRIZES dprizesPrev = pdprize->dprizes;

	if (dprizesPrev == DPRIZES_Removed ||
		dprizesPrev == DPRIZES_Nil ||
		(dprizesPrev > DPRIZES_Fall && dprizesPrev < DPRIZES_Lose))
	{
		pdprize->posCenter = pdprize->xf.pos;
		pdprize->vCenter = pdprize->xf.v;
	}

	switch (dprizes)
	{
		case DPRIZES_Normal:
		oidState = (OID)609;

		if (pdprize->oidInitialState != OID_Nil)
			oidState = (OID)pdprize->oidInitialState;

		pdprize->dvCenter = glm::vec3(0.0f);
		break;

		case DPRIZES_Removed:
		if (!pdprize->fNeverReuse)
			pdprize->fReuseCandidate = true;

		pdprize->pvtlo->pfnRemoveLo(pdprize);
		break;

		case DPRIZES_Fall:
		oidState = (OID)608;
		pdprize->dvCenter = s_dvDprizeGravity;
		break;

		case DPRIZES_Swirl:
		{
			if (ppzi == nullptr || ppzi->paloBag == nullptr)
				return;

			DecomposeCylind(&pdprize->xf.posWorld, &ppzi->paloBag->xf.posWorld, &pdprize->posCenter.x, &pdprize->posCenter.y, &pdprize->posCenter.z);

			pdprize->vCenter = s_vCenter;
			pdprize->dvCenter = glm::vec3(0.0f);

			glm::vec3 dpos = ppzi->paloBag->xf.posWorld - pdprize->xf.posWorld;
			dpos.z *= RZ_DprizeSwirlLookAt;

			glm::mat3 matLookAt;
			glm::mat3 matTilt;
			glm::mat3 dmat;

			LoadLookAtMatrix(&dpos, &matLookAt);
			LoadRotateMatrixRad(-2.5f, &g_normalX, &matTilt);

			matLookAt = matLookAt * matTilt;

			CalculateDmat(&matLookAt, &pdprize->xf.matWorld, &dmat);
			DecomposeRotateMatrixRad(&dmat, &pdprize->radSmooth, &pdprize->normalSmooth);

			pdprize->fSwirlDone = false;
			break;
		}

			case DPRIZES_Stick:
			if (ppzi == nullptr || ppzi->paloBag == nullptr)
				return;

			pdprize->posCenter = pdprize->xf.posWorld - ppzi->paloBag->xf.posWorld;
			pdprize->vCenter = glm::vec3(0.0f);
			pdprize->dvCenter = glm::vec3(0.0f);
			break;

			case DPRIZES_Lose:
			oidState = (OID)611;
			pdprize->dvCenter = glm::vec3(0.0f);
			break;

			case DPRIZES_Attract:
			oidState = (OID)610;
			pdprize->dvCenter = glm::vec3(0.0f);
			break;

			default:
			break;
	}

	if (pdprize->ptarget != nullptr)
	{
		if (dprizes == DPRIZES_Normal || dprizes == DPRIZES_Lose)
			pdprize->ptarget->pvtlo->pfnAddLo(pdprize->ptarget);
		else
			pdprize->ptarget->pvtlo->pfnRemoveLo(pdprize->ptarget);
	}

	pdprize->dprizes = dprizes;
	pdprize->tDprizes = g_clock.t;

	if (oidState == OID_Nil)
	{
		if (pdprize->psma != nullptr)
		{
			RetractSma(pdprize->psma);
			pdprize->psma = nullptr;
		}
	}
	else if (pdprize->psma == nullptr)
	{
		pdprize->psma = PsmaApplySm(pdprize->psm, pdprize, oidState, 0);

		if (pdprize->dtInitialSkip > 0.0f)
			SkipSma(pdprize->psma, pdprize->dtInitialSkip);
	}
	else
		SetSmaGoal(pdprize->psma, oidState);
}

void DeleteDprize(DPRIZE* pdprize)
{
	delete pdprize;
}

int CpdprizeFindSwDprizes(SW* psw, CID cid, int cpdprizeMax, DPRIZE** apdprize)
{
	if (cpdprizeMax <= 0 || apdprize == nullptr)
		return 0;

	PO* ppo = PpoCur();
	const glm::vec3 posCenter = ppo != nullptr ? ppo->xf.posWorld : g_pcm->pos;

	struct SDPRIZE
	{
		float sDist;
		DPRIZE* pdprize;
	};

	std::vector<SDPRIZE> candidates;

	for (DPRIZE* pdprize = psw->dlDprize.pdprizeFirst; pdprize != nullptr; pdprize = pdprize->dle.pdprizeNext)
	{
		if (pdprize->fReuseCandidate == 0)
			continue;

		if (pdprize->pvtbasic->cid != cid)
			continue;

		float sDist = std::numeric_limits<float>::max();

		if (pdprize->dprizes != DPRIZES_Removed)
		{
			const glm::vec3 dpos = posCenter - pdprize->xf.posWorld;
			sDist = glm::dot(dpos, dpos);
		}

		candidates.push_back({ sDist, pdprize });
	}

	std::sort(candidates.begin(), candidates.end(), [](const SDPRIZE& lhs, const SDPRIZE& rhs)
		{
			return lhs.sDist < rhs.sDist;
		});

	const int cpdprize = std::min(cpdprizeMax, static_cast<int>(candidates.size()));

	for (int i = 0; i < cpdprize; ++i)
		apdprize[i] = candidates[i].pdprize;

	return cpdprize;
}

CHARM* NewCharm()
{
	return new CHARM{};
}

void InitCharm(CHARM* pcharm)
{
	InitDprize(pcharm);
	pcharm->sRadiusCollect = 60.0;
	pcharm->uGlintChance = 0.35;
	pcharm->svLastBounceMax = 300.0;
	pcharm->rxyBounce = 0.75;
	pcharm->rzBounce = 0.5;
	pcharm->sRadiusBounce = 60.0;
	pcharm->svLastBounce = 0.0;
}

int GetCharmSize()
{
	return sizeof(CHARM);
}

void CloneCharm(CHARM* pcharm, CHARM* pcharmBase)
{
	CloneDprize(pcharm, pcharmBase);
}

void SetCharmDprizes(CHARM* pcharm, DPRIZES dprizes)
{
	if (pcharm->dprizes == dprizes)
		return;

	if (dprizes == DPRIZES_Collect)
	{
		StartSound((SFXID)116, nullptr, pcharm, nullptr, 1500.0f, 0.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
		HandleLoSpliceEvent(pcharm, 2, 0, nullptr);

		dprizes = DPRIZES_Swirl;
	}

	SetDprizeDprizes(pcharm, dprizes);
}

void DeleteCharm(CHARM* pcharm)
{
	delete pcharm;
}

void StartupCoinCtr(COINCTR* pcoinctr)
{
	pcoinctr->pvtcoinctr = &g_vtcoinctr;
}

COIN* NewCoin()
{
	return new COIN{};
}

void InitCoin(COIN* pcoin)
{
	InitDprize(pcoin);
	pcoin->sRadiusBounce = 45.0;
	pcoin->lmDtMaxLifetime.gMax = 10.0;
	pcoin->svLastBounceMax = 250.0;
	pcoin->svLastBounce = 50.0;
	pcoin->rxyBounce = 0.6;
	pcoin->rzBounce = 0.75;
	pcoin->uGlintChance = 0.25;
	pcoin->lmDtMaxLifetime.gMin = 8.0;
	pcoin->sRadiusCollect = 50.0;
}

int GetCoinSize()
{
	return sizeof(COIN);
}

void CloneCoin(COIN* pcoin, COIN* pcoinBase)
{
	CloneDprize(pcoin, pcoinBase);

	pcoin->lmDtMaxLifetime = pcoinBase->lmDtMaxLifetime;
	pcoin->tLose = pcoinBase->tLose;
}

void PostCoinCallback(COIN* pcoin)
{
	pcoin->pvtdprize->pfnSetDprizeDprizes(pcoin, DPRIZES_Removed);
}

void HandleCoinMessage(COIN* pcoin, MSGID msgid, SMA* psma)
{
	HandleAloMessage(pcoin, msgid, psma);

	if (msgid != MSGID_sma_transition || psma != pcoin->psma)
		return;

	OID oidCur = OID_Nil;
	GetSmaCur(psma, &oidCur);

	if (oidCur != 611)
		return;

	EXPL* pexpl = reinterpret_cast<EXPL*>(pcoin->psw->aploStock[30]);

	if (pexpl != nullptr)
	{
		EXPLSO explso{};
		explso.grfexplso = 1;
		explso.paloReference = pcoin;

		pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, &explso);
	}

	PostSwCallback(pcoin->psw, reinterpret_cast<PFNMQ>(PostCoinCallback), pcoin, MSGID_callback, nullptr);
}

void UpdateCoin(COIN* pcoin, float dt)
{
	UpdateDprize(pcoin, dt);

	if (pcoin->dprizes != DPRIZES_Normal || pcoin->fReuseCandidate == 0)
		return;

	if (g_binoc.binocs != BINOCS_Dialog)
		pcoin->tLose -= g_clock.dt;

	if (pcoin->tLose <= 0.0f)
		pcoin->pvtdprize->pfnSetDprizeDprizes(pcoin, DPRIZES_Lose);
}

void CreateSwCharm(SW* psw)
{
	CHARM* pcharm = nullptr;

	if (CpdprizeFindSwDprizes(psw, CID_CHARM, 1, (DPRIZE**)&pcharm) == 0)
	{
		++g_pgsCur->ccharm;
		return;
	}

	pcharm->pvtlo->pfnAddLo(pcharm);
	pcharm->pvtlo->pfnSetLoParent(pcharm, nullptr);

	glm::vec3 pos;
	ConvertCmScreenToWorld(g_pcm, &s_posCharmScreen, &pos);

	pcharm->pvtalo->pfnTranslateAloToPos(pcharm, &pos);
	pcharm->pvtalo->pfnRotateAloToMat(pcharm, &g_matIdentity);
	pcharm->pvtalo->pfnSetAloVelocityVec(pcharm, &g_vecZero);
	pcharm->pvtalo->pfnSetAloAngularVelocityVec(pcharm, &g_vecZero);
	pcharm->pvtcharm->pfnSetCharmDprizes(pcharm, DPRIZES_Swirl);

	pcharm->vCenter = s_vCenter;

	ClipVismapSphereOneHop(psw->pvismap, &pcharm->xf.posWorld, pcharm->sRadiusRenderAll, &pcharm->grfzon);
}

void AddLife()
{
	if (g_pgsCur->clife < 99)
		++g_pgsCur->clife;
	else
		g_pgsCur->clife = 99;
}

void OnCoinSmack(COIN* pcoin)
{
	++g_pgsCur->ccoin;

	if (g_pgsCur->ccoin <= 99)
		return;

	if (g_pgsCur->ccharm < CcharmMost())
	{
		g_pgsCur->ccoin = 0;

		g_coinctr.dgDisplayMax = 250.0f;
		g_coinctr.pfnsmack =(PFNSMACK) CreateSwCharm;
		g_coinctr.pv = pcoin->psw;
	}
	else if (g_pgsCur->clife < 99)
	{
		g_pgsCur->ccoin = 0;

		g_coinctr.dgDisplayMax = 250.0f;
		g_coinctr.pfnsmack = (PFNSMACK)AddLife;
		g_coinctr.pv = nullptr;

		g_lifectr.pvtblot->pfnShowBlot(&g_lifectr);
	}
	else
		g_pgsCur->ccoin = 99;
}

void SetCoinDprizes(COIN* pcoin, DPRIZES dprizes)
{
	if (pcoin->dprizes == dprizes)
		return;

	if (dprizes == DPRIZES_Normal)
	{
		if (pcoin->fReuseCandidate)
			// tLose is a remaining-duration countdown. UpdateCoin subtracts dt
			// from it (except while binocular dialog is active), so including the
			// absolute world time made later drops remain for minutes.
			pcoin->tLose = GRandInRange(pcoin->lmDtMaxLifetime.gMin, pcoin->lmDtMaxLifetime.gMax);
	}
	else if (dprizes == DPRIZES_Collect)
	{
		SMACK* psmack = static_cast<SMACK*>(PripNewRipg(RIPT_Smack, nullptr));

		if (psmack == nullptr)
			OnCoinSmack(pcoin);
		else
		{
			psmack->pv = pcoin;
			psmack->psmacki = &s_smackiCoin;

			psmack->pvtrip->pfnInitRip(psmack, &pcoin->xf.posWorld, 1.0f, nullptr);

			psmack->paloRender = pcoin;
			psmack->mat = pcoin->xf.matWorld;
		}

		g_coinctr.pvtblot->pfnShowBlot(&g_coinctr);
		SetChkmgrIchk(&g_chkmgr, pcoin->ichkCollected);

		StartSound((SFXID)120, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

		if (pcoin->pexplCollect != nullptr)
		{
			EXPLSO explso{};
			explso.grfexplso = 1;
			explso.paloReference = pcoin;

			pcoin->pexplCollect->pvtexpl->pfnExplodeExplExplso((EXPLO*)pcoin->pexplCollect, &explso);
		}

		dprizes = DPRIZES_Removed;
	}

	SetDprizeDprizes(pcoin, dprizes);
}
void DeleteCoin(COIN* pcoin)
{
	delete pcoin;
}

void StartupLifeCtr(LIFECTR* plifectr)
{
	plifectr->pvtlifectr = &g_vtlifectr;
}

LIFETKN* NewLifetkn()
{
	return new LIFETKN{};
}

void InitLifeTkn(LIFETKN* plifetkn)
{
	InitDprize(plifetkn);

	plifetkn->sRadiusBounce = 45.0f;
	plifetkn->lmDtReuse.gMin = 4.0f;
	plifetkn->lmDtReuse.gMax = 6.0f;
	plifetkn->svLastBounceMax = 250.0f;
	plifetkn->svLastBounce = 50.0f;
	plifetkn->rxyBounce = 0.05f;
	plifetkn->rzBounce = 0.5f;
	plifetkn->sRadiusCollect = 50.0f;
	plifetkn->uGlintChance = 0.0f;
}

void SetLifetknLmDtReuse(LIFETKN* plifetkn, glm::vec2 dtReuse)
{
	plifetkn->lmDtReuse.gMin = dtReuse.x;
	plifetkn->lmDtReuse.gMax = dtReuse.y;
}

void* GetLifetknLmDtReuse(LIFETKN* plifetkn)
{
	return &plifetkn->lmDtReuse;
}

int GetLifetknSize()
{
	return sizeof(LIFETKN);
}

void CloneLifetkn(LIFETKN* plifetkn, LIFETKN* plifetknBase)
{
	CloneDprize(plifetkn, plifetknBase);
	
	plifetkn->lmDtReuse = plifetknBase->lmDtReuse;
	plifetkn->dtReuseRemaining = plifetknBase->dtReuseRemaining;
}

void PostLifetknCallback(LIFETKN* plifetkn)
{
	plifetkn->pvtdprize->pfnSetDprizeDprizes(plifetkn, DPRIZES_Removed);
}

void HandleLifetknMessage(LIFETKN* plifetkn, MSGID msgid, SMA* psma)
{
	HandleAloMessage(plifetkn, msgid, psma);

	if (msgid != MSGID_sma_transition || psma != plifetkn->psma)
		return;

	OID oidGoal = OID_Nil;
	GetSmaGoal(psma, &oidGoal);

	if (oidGoal != 611)
		return;

	EXPL* pexpl = reinterpret_cast<EXPL*>(plifetkn->psw->aploStock[30]);

	if (pexpl != nullptr)
	{
		EXPLSO explso{};
		explso.grfexplso = 1;
		explso.paloReference = plifetkn;

		pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, &explso);
	}
	
	PostSwCallback(plifetkn->psw, (PFNMQ)PostLifetknCallback, plifetkn, MSGID_callback, nullptr);
}

void UpdateLifetkn(LIFETKN* plifetkn, float dt)
{
	UpdateDprize(plifetkn, dt);

	if (plifetkn->dprizes != DPRIZES_Normal || plifetkn->fReuseCandidate == 0)
		return;

	if (g_binoc.binocs != BINOCS_Dialog)
		plifetkn->dtReuseRemaining -= g_clock.dt;

	if (plifetkn->dtReuseRemaining <= 0.0f)
		plifetkn->pvtdprize->pfnSetDprizeDprizes(plifetkn, DPRIZES_Removed);
}

void AwardLife()
{
	g_lifectr.pvtblot->pfnShowBlot(&g_lifectr);

	if (g_pgsCur->clife < 99)
	{
		++g_pgsCur->clife;

		g_lifectr.dgDisplayMax = 250.0f;
		g_lifectr.pv = nullptr;
		g_lifectr.pfnsmack = nullptr;
	}
	else
		g_pgsCur->clife = 99;
}

void SetLifetknDprizes(LIFETKN* plifetkn, DPRIZES dprizes)
{
	if (plifetkn->dprizes == dprizes)
		return;

	if (dprizes == DPRIZES_Normal)
	{
		if (plifetkn->fReuseCandidate)
			plifetkn->dtReuseRemaining = GRandInRange(plifetkn->lmDtReuse.gMin, plifetkn->lmDtReuse.gMax);
	}
	else if (dprizes == DPRIZES_Collect)
	{
		SMACK* psmack = static_cast<SMACK*>(PripNewRipg(RIPT_Smack, nullptr));

		if (psmack == nullptr)
			AwardLife();
		else
		{
			psmack->pv = plifetkn;
			psmack->psmacki = &g_smackiLifeToken;

			psmack->pvtrip->pfnInitRip(psmack, &plifetkn->xf.posWorld, 1.0f, nullptr);

			psmack->paloRender = plifetkn;
			psmack->mat = plifetkn->xf.matWorld;
		}

		g_lifectr.pvtblot->pfnShowBlot(&g_lifectr);
		SetChkmgrIchk(&g_chkmgr, plifetkn->ichkCollected);
		StartSound((SFXID)105, nullptr, nullptr, nullptr, 3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);

		if (plifetkn->pexplCollect != nullptr)
		{
			EXPLSO explso{};
			explso.grfexplso = 1;
			explso.paloReference = plifetkn;

			plifetkn->pexplCollect->pvtexpl->pfnExplodeExplExplso((EXPLO*)plifetkn->pexplCollect, &explso);
		}

		dprizes = DPRIZES_Removed;
	}

	SetDprizeDprizes(plifetkn, dprizes);
}

void DeleteLifetkn(LIFETKN* plifetkn)
{
	delete plifetkn;
}

void StartupKeyCtr(KEYCTR* pkeyctr)
{
	pkeyctr->pvtkeyctr = &g_vtkeyctr;
}

KEY* NewKey()
{
	return new KEY{};
}

void InitKey(KEY* pkey)
{
	InitDprize(pkey);

	pkey->sRadiusCollect = 35.0;
	pkey->svLastBounceMax = 500.0;
	pkey->svLastBounce = 250.0;
	pkey->rzBounce = 0.6;
	pkey->uGlintChance = 0.75;
	pkey->sRadiusBounce = 35.0;
	pkey->rxyBounce = 0.6;

	if ((g_plsCur->grfls & 2U) != 0)
		pkey->dprizesInit = DPRIZES_Removed;
}

int GetKeySize()
{
	return sizeof(KEY);
}

void CloneKey(KEY* pkey, KEY* pkeyBase)
{
	CloneDprize(pkey, pkeyBase);
}

void SetKeyDprizes(KEY* pkey, DPRIZES dprizes)
{
	if (pkey->dprizes == dprizes)
		return;

	if (dprizes == DPRIZES_Collect)
	{
		if ((g_plsCur->grfls & 2U) == 0)
		{
			g_plsCur->grfls |= 2U;
			++g_pwsCur->ckey;
		}

		g_keyctr.pvtblot->pfnShowBlot(&g_keyctr);

		PO* ppo = PpoCur();

		if (ppo != nullptr)
			ppo->pvtpo->pfnCollectPoPrize(ppo, PCK_Key, pkey);

		HandleLoSpliceEvent(pkey, 2, 0, nullptr);

		dprizes = DPRIZES_Removed;

		OnDifficultyCollectKey(&g_difficulty);
		AutosaveCurrentGame(&g_saveData);
	}

	SetDprizeDprizes(pkey, dprizes);
}

void DeleteKey(KEY* pkey)
{
	delete pkey;
}

void StartupGoldCtr(GOLDCTR* pgoldctr)
{
	pgoldctr->pvtgoldctr = &g_vtgoldctr;
}

GOLD* NewGold()
{
	return new GOLD{};
}

void InitGold(GOLD* pgold)
{
	InitDprize(pgold);

	pgold->sRadiusCollect = 60.0;
	pgold->svLastBounceMax = 500.0;
	pgold->svLastBounce = 250.0;
	pgold->rzBounce = 0.6;
	pgold->uGlintChance = 0.75;
	pgold->sRadiusBounce = 60.0;
	pgold->rxyBounce = 0.6;

	if (g_note.fGoldEnabled == 0) 
		pgold->dprizesInit = DPRIZES_Removed;
}

int GetGoldSize()
{
	return sizeof(GOLD);
}

void CloneGold(GOLD* pgold, GOLD* pgoldBase)
{
	CloneDprize(pgold, pgoldBase);

	pgoldBase = pgold;
}

void PostGoldLoad(GOLD* pgold)
{
	PostDprizeLoad(pgold);
	g_note.pgold = pgold;

	// tGoldEnable is the level-authored game-clock deadline for the starting
	// hourglass. UpdateGold tests tGoldLose, so copy the loaded deadline into
	// the runtime field after the world options have been read.
	pgold->psw->tGoldLose = pgold->psw->tGoldEnable;
}

void PostGoldCallback(GOLD* pgold)
{
	pgold->pvtdprize->pfnSetDprizeDprizes(pgold, DPRIZES_Removed);
}

void HandleGoldMessage(GOLD* pgold, MSGID msgid, SMA* psma)
{
	HandleAloMessage(pgold, msgid, psma);

	if (msgid != MSGID_sma_transition || psma != pgold->psma)
		return;

	OID oidGoal = OID_Nil;
	GetSmaGoal(psma, &oidGoal);

	if (oidGoal != 611)
		return;

	EXPL* pexpl = reinterpret_cast<EXPL*>(pgold->psw->aploStock[30]);

	if (pexpl != nullptr)
	{
		EXPLSO explso{};
		explso.grfexplso = 1;
		explso.paloReference = pgold;

		pexpl->pvtexpl->pfnExplodeExplExplso((EXPLO*)pexpl, &explso);
	}

	PostSwCallback(pgold->psw, reinterpret_cast<PFNMQ>(PostGoldCallback), pgold, MSGID_callback, nullptr);
}

void UpdateGold(GOLD* pgold, float dt)
{
	UpdateDprize(pgold, dt);

	if (pgold->dprizes == DPRIZES_Normal && pgold->psw->tGoldLose <= g_clock.t)
		pgold->pvtdprize->pfnSetDprizeDprizes(pgold, DPRIZES_Lose);
}

void SetGoldDprizes(GOLD* pgold, DPRIZES dprizes)
{
	if (pgold->dprizes == dprizes)
		return;

	if (dprizes == DPRIZES_Collect)
	{
		SetNoteMtsState(&g_note, MTSSTATE_Active);

		if (pgold->pexplCollect != nullptr)
		{
			EXPLSO explso{};
			explso.grfexplso = 1;
			explso.paloReference = pgold;

			pgold->pexplCollect->pvtexpl->pfnExplodeExplExplso((EXPLO*)pgold->pexplCollect, &explso);
		}

		dprizes = DPRIZES_Removed;
	}

	SetDprizeDprizes(pgold, dprizes);
}

void DeleteGold(GOLD* pgold)
{
	delete pgold;
}

int CpdprizeAttractSwDprizes(SW* psw, CID cid, glm::vec3* pposCenter, int cpdprizeMax, DPRIZE** apdprize)
{
	std::vector<DPRIZE*> adprizeTemp;

	if (apdprize == nullptr)
	{
		adprizeTemp.resize(cpdprizeMax);
		apdprize = adprizeTemp.data();
	}

	int cpdprize = CpdprizeFindSwDprizes(psw, cid, cpdprizeMax, apdprize);
	float rad = GRandInRange(0.0f, 6.2831855f);

	for (int ipdprize = 0; ipdprize < cpdprize; ++ipdprize)
	{
		DPRIZE* pdprize = apdprize[ipdprize];

		pdprize->pvtlo->pfnAddLo(pdprize);
		pdprize->pvtlo->pfnSetLoParent(pdprize, nullptr);
		pdprize->pvtalo->pfnTranslateAloToPos(pdprize, pposCenter);
		pdprize->pvtalo->pfnRotateAloToMat(pdprize, &g_matIdentity);
		pdprize->pvtalo->pfnSetAloVelocityVec(pdprize, &g_vecZero);
		pdprize->pvtalo->pfnSetAloAngularVelocityVec(pdprize, &g_vecZero);

		pdprize->fLastBounce = 0;
		pdprize->dtInitialSkip = static_cast<float>(ipdprize) * DT_DprizeSkipEach;

		pdprize->pvtdprize->pfnSetDprizeDprizes(pdprize, DPRIZES_Fall);

		float sXY = GRandInRange(s_lmXY.gMin, s_lmXY.gMax);
		float sZ  = GRandInRange(s_lmZ.gMin, s_lmZ.gMax);

		SetVectorCylind(&pdprize->vCenter, rad, sXY, sZ);

		rad += 6.2831855f / static_cast<float>(cpdprizeMax);

		ClipVismapSphereOneHop(psw->pvismap, &pdprize->xf.posWorld, pdprize->sRadiusRenderAll, &pdprize->grfzon);
	}

	return cpdprize;
}

void RemoveSwExtraneousCharms(SW* psw)
{
	const int levelId = (g_pgsCur->gameWorldCur << 8) | g_pgsCur->worldLevelCur;

	if (levelId == 776)
		return;

	if (g_pgsCur->ccharm < CcharmMost() && g_note.mtsState <= 1)
		return;

	DPRIZE* pdprize = psw->dlDprize.pdprizeFirst;

	while (pdprize != nullptr)
	{
		DPRIZE* pdprizeNext = pdprize->dle.pdprizeNext;

		if (FIsBasicDerivedFrom(pdprize, CID_CHARM) != 0)
		{
			CHARM* pcharm = reinterpret_cast<CHARM*>(pdprize);
			pcharm->pvtcharm->pfnSetCharmDprizes(pcharm, DPRIZES_Removed);
		}

		pdprize = pdprizeNext;
	}
}

LIFECTR g_lifectr;
COINCTR g_coinctr;
GOLDCTR g_goldctr;
KEYCTR  g_keyctr;
float DT_DprizeSwirl = 1.0;
float DT_DprizeStick = 0.25;
CLQ s_clqUToRStick = { 0.69999999, -0.4, 0.0, 0.0 };
CLQ s_clqUToRSwirl = { 1.0, -0.3, 0.0, 0.0 };
glm::vec3 s_posTarget = {0.0, 75.0, 85.0};
SMPA s_asmpaSwirl[3] = 
{
	20, 5.0, 0.15, 120,
	800, 50, 0.25, 4500,
	800, 50, 0.25, 4500
};
float RZ_DprizeSwirlLookAt = 0.25;
SMP s_smpRadSmooth = 
{
	10, 0.0, 0.25
};
float R_DprizeDamping = -5.0;
glm::vec3 s_dvDprizeGravity = {0, 0, -2250};
glm::vec3 s_vCenter = {0, 800, 1200};
glm::vec3 s_normalGlint = {0.0, 0.0, 1.0};
float DT_DprizeGlintIntervalMin = 1.0;
float DT_DprizeFallMax = 5.0;
glm::vec3 s_vReject = {-250, 0.0, 600};
float S_DprizeCollectMax = 300.0;
SNIP s_asnipDprize[5] = 
{
	{ 0x00, (OID)0x25F, offsetof(DPRIZE, psm) },
	{ 0x02, (OID)0x25B, offsetof(DPRIZE, ppntFrontGlint) },
	{ 0x02, (OID)0x25C, offsetof(DPRIZE, ppntBackGlint) },
	{ 0x02, (OID)0x25E, offsetof(DPRIZE, pexplCollect) },
	{ 0x02, (OID)0x25D, offsetof(DPRIZE, pexplAttract) },
};

glm::vec3 s_posCharmScreen = {-0.6, 0.85, 300};
float DT_DprizeSkipEach = 0.5;
LM s_lmXY = {75, 100};
LM s_lmZ = {1000, 1300};
int g_fGoldEnabled = 0;
