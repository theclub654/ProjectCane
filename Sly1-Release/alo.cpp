#include "alo.h"
#include "hide.h"
#include "render.h"
#include "target.h"
#include "sm.h"
#include "actadj.h"
#include "proxy.h"
#include "aseg.h"
#include "actseg.h"
#include "actla.h"
#include "actbank.h"
#include "actref.h"
#include "spliceobj.h"
#include "ik.h"
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <cstdio>
#include "vis.h"

#include "jt.h"
#include "mb.h"

ALO* NewAlo()
{
	return new ALO{};
}

void InitAlo(ALO* palo)
{

	InitDl(&palo->dlChild,  offsetof(LO, dleChild));
	InitDl(&palo->dlFreeze, offsetof(ALO, dleFreeze));

	if (palo->paloParent == nullptr)
	{
		palo->zons = 2;
		palo->viss = 2;
	}
	else
	{
		palo->zons = 1;
		palo->viss = 0;
	}

	InitLo(palo);

	palo->ackPos = ACK_Nil;
	palo->ackRot = ACK_Nil;

	palo->sCelBorderMRD = FLT_MAX;
	palo->sMRD = FLT_MAX;

	// Default motion tuning
	palo->pclqPosSpring  = std::make_shared<CLQ>(g_clqPosSpringDefault);
	palo->pclqRotSpring  = std::make_shared<CLQ>(g_clqRotSpringDefault);
	palo->pclqPosDamping = std::make_shared<CLQ>(g_clqPosDampingDefault);
	palo->pclqRotDamping = std::make_shared<CLQ>(g_clqRotDampingDefault);

	palo->psmpaPos = std::make_shared<SMPA>(g_smpaPosDefault);
	palo->psmpaRot = std::make_shared<SMPA>(g_smpaRotDefault);

	// The release initializes only the 28 valid zone bits.
	palo->grfzon = 0x0fffffff;
	palo->mtlk = 0;
	palo->xf.mat = glm::identity <glm::mat3>();
	palo->xf.matWorld = glm::identity <glm::mat3>();
	palo->matOrig = glm::identity <glm::mat3>();

	InitDl(&palo->dlAct, offsetof(ACT, dleAlo));

	if (palo->pvtlo->cid != CID_LIGHT)
		allSWAloObjs.push_back(palo);
}

void RemoveAloHierarchy(ALO* palo)
{
	DLI dliChild{};

	dliChild.m_pdl = &palo->dlChild;
	dliChild.m_ibDle = palo->dlChild.ibDle;
	dliChild.m_pdliNext = s_pdliFirst;
	dliChild.m_ppv = reinterpret_cast<void**>(dliChild.m_pdl);

	s_pdliFirst = &dliChild;

	/*
	 * The original calls OnLoRemove directly for this object.
	 */
	palo->pvtlo->pfnOnLoRemove(palo);

	while (true)
	{
		LO* child = static_cast<LO*>(*dliChild.m_ppv);

		if (child == nullptr)
			break;

		dliChild.m_ppv = reinterpret_cast<void**>(
			reinterpret_cast<char*>(child) + dliChild.m_ibDle);

		/*
		 * Call RemoveLoHierarchy, not OnLoRemove. This recursively
		 * processes the child's descendants.
		 */
		child->pvtlo->pfnRemoveLoHierarchy(child);
	}

	palo->pvtlo->pfnSendLoMessage(palo, MSGID_removed, palo);

	s_pdliFirst = dliChild.m_pdliNext;
}
void OnAloAdd(ALO* palo)
{
	// Original does this first
	OnLoAdd(palo);

	ALO* parent = palo->paloParent;
	SW* sw = palo->psw;

	if (parent == nullptr)
	{
		// Root object
		palo->paloRoot = palo;

		if (palo->fRealClock == 0)
		{
			// Normal-clock roots go into MRD + Busy processing lists
			AppendDlEntry(&sw->dlMRD, palo);

			palo->fBusy = true;

			AppendDlEntry(&sw->dlBusy, palo);

			// If this LO is an SO-ish thing, also append to dlBusySo
			if ((palo->pvtlo->grfcid & 2u) != 0)
				AppendDlEntry(&sw->dlBusySo, palo);

			// Freeze root init + merge groups
			palo->paloFreezeRoot = palo;
			palo->dlFreeze.paloFirst = palo;
			palo->dlFreeze.paloLast = palo;

			for (int i = palo->cpmrg - 1; i >= 0; --i)
				MergeSwGroup(sw, palo->apmrg[i]);
		} 
		else
		{
			// Real-clock roots go into the real-clock MRD list
			AppendDlEntry(&sw->dlMRDRealClock, palo);
		}
	}
	else
	{
		// Child inherits root from parent
		palo->paloRoot = parent->paloRoot;

		// If parent's "freeze-propagate" bit is set, freeze child on add (if supported)
		if ((parent->fFrozen) != 0)
		{
			if (palo->pvtalo && palo->pvtlo->pfnFreezeLo)
				palo->pvtlo->pfnFreezeLo(palo, 1);
		}
	}

	// Apply ACT position goal on add
	if (palo->pactPos != nullptr) {
		glm::vec3 pos;
		glm::vec3 vel;

		palo->pactPos->pvtact->pfnGetActPositionGoal(palo->pactPos, 0.0f, &pos, &vel);

		palo->pvtalo->pfnTranslateAloToPos(palo, &pos);
		palo->pvtalo->pfnSetAloVelocityVec(palo, &vel);
	}

	if (palo->pactRot != nullptr) {
		glm::mat3 mat;
		glm::vec3 angVel;

		palo->pactRot->pvtact->pfnGetActRotationGoal(palo->pactRot, 0.0f, &mat, &angVel);

		palo->pvtalo->pfnRotateAloToMat(palo, &mat);
		palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &angVel);
	}


	// If flagged, add to camera fade list
	if (palo->fForceCameraFade != 0)
		AddCmFadeObject(g_pcm, palo);

		// Shadow registration
	if (palo->pshadow != nullptr && palo->psw != nullptr)
		AppendDlEntry(&palo->psw->dlShadow, palo->pshadow.get());

	// Update world transform if the LO has a handler
	if (palo->pvtalo && palo->pvtalo->pfnUpdateAloXfWorld)
		palo->pvtalo->pfnUpdateAloXfWorld(palo);

	HandleLoSpliceEvent(palo, 4, 0, nullptr);
	ResolveAlo(palo);
}

void OnAloRemove(ALO* palo)
{
	OnLoRemove(palo);
	
	SW* psw = palo->psw;

	if (palo->paloParent == nullptr)
	{
		if (palo->fRealClock)
			RemoveDlEntry(&psw->dlMRDRealClock, palo);
		else
		{
			RemoveDlEntry(&psw->dlMRD, palo);

			if (palo->fBusy)
			{
				palo->fBusy = false;

				RemoveDlEntry(&psw->dlBusy, palo);

				if ((palo->pvtlo->grfcid & 2U) != 0)
					RemoveDlEntry(&psw->dlBusySo, palo);
			}

			SplinterSwFreezeGroup(psw, palo->paloFreezeRoot);

			palo->paloFreezeRoot = nullptr;
			ClearDl(&palo->dlFreeze);
		}
	}

	if (palo->fForceCameraFade)
		RemoveCmFadeObject(g_pcm, palo);

	if (palo->pshadow != nullptr)
		RemoveDlEntry(&palo->psw->dlShadow, palo->pshadow.get());

	if (palo->fFrozen && palo->pvtlo->pfnFreezeLo != nullptr)
		palo->pvtlo->pfnFreezeLo(palo, 0);

	ResolveAlo(palo);

	palo->paloRoot = nullptr;

	HandleLoSpliceEvent(palo, 5, 0, nullptr);
}
void UpdateAloOrig(ALO* palo)
{
	palo->matOrig = palo->xf.mat;   // glm::mat3 (rotation)
	palo->posOrig = palo->xf.pos;   // glm::vec3

	if (palo->pvtalo && palo->pvtalo->pfnUnadjustAloRotation)
		palo->pvtalo->pfnUnadjustAloRotation(palo, &palo->matOrig);

		// Decompose to Euler (radians), PS2 logic
	DecomposeRotateMatrixEuler(palo->matOrig, palo->eulOrig);
}

void AdjustAloRtckMat(ALO* palo, CM* pcm, RTCK rtck, glm::vec3* pposCenter, glm::mat4& pmat)
{
	glm::vec3 dpos = -glm::vec3(pcm->mat[0]);

	glm::vec3 vecFrom = glm::vec3(pmat[0]);
	glm::mat3 matRotate(1.0f);

	BuildRotateVectorsMatrix(&vecFrom, &dpos, &matRotate);

	glm::mat4 mat = pmat;
	mat[3].x -= pposCenter->x;
	mat[3].y -= pposCenter->y;
	mat[3].z -= pposCenter->z;

	glm::mat4 dmat(1.0f);
	LoadMatrixFromPosRot(&g_vecZero, &matRotate, &dmat);

	glm::mat4 alignedMat = dmat * mat;

	glm::vec3 alignedZ = glm::vec3(alignedMat[2]);
	glm::vec3 vec = alignedZ - dpos * glm::dot(alignedZ, dpos);

	pmat = alignedMat;

	glm::vec3 cameraZ = glm::vec3(pcm->mat[2]);

	BuildRotateVectorsMatrix(&vec, &cameraZ, &matRotate);
	LoadMatrixFromPosRot(pposCenter, &matRotate, &dmat);

	pmat = dmat * pmat;
}

void CloneAloHierarchy(ALO* palo, ALO* paloBase)
{
	DLI it{};

	it.m_pdl = &paloBase->dlChild;
	it.m_ibDle = paloBase->dlChild.ibDle;
	it.m_pdliNext = s_pdliFirst;
	it.m_ppv = (void**)it.m_pdl;

	s_pdliFirst = &it;

	// Clone parent (keep your signature)
	palo->pvtlo->pfnCloneLo(palo, paloBase);

	// Walk base children and clone each
	LO* child = (LO*)*it.m_ppv;
	while (child != nullptr)
	{
		// Advance iterator to next-pointer-field for this child
		it.m_ppv = (void**)((uintptr_t)child + it.m_ibDle);
		
		PloCloneLo(child, palo->psw, palo);

		child = (LO*)*it.m_ppv;
	}

	// Proxy roots can refer to later siblings, so remap them after the whole
	// cloned hierarchy exists.
	RebuildClonedProxyRoots(palo, paloBase);

	if ((((palo->pvtlo->grfcid & 2U) != 0) && palo->paloParent == nullptr)) {
		RecalcSoLocked((SO*)palo);
	}

	s_pdliFirst = it.m_pdliNext;
}

void CloneAlo(ALO* palo, ALO* paloBase)
{
	DLE  savedDleMRD = palo->dleMRD;
	DLE  savedDleBusy = palo->dleBusy;
	DLE  savedDleFreeze = palo->dleFreeze;
	DL   savedDlFreeze = palo->dlFreeze;
	DL   savedDlAct = palo->dlAct;
	ALO* savedFreezeRoot = palo->paloFreezeRoot;
	ALO* savedRoot = palo->paloRoot;

	CloneLo(palo, paloBase);

	palo->dleMRD = savedDleMRD;
	palo->dleBusy = savedDleBusy;
	palo->dleFreeze = savedDleFreeze;
	palo->dlFreeze = savedDlFreeze;
	palo->dlAct = savedDlAct;
	palo->paloFreezeRoot = savedFreezeRoot;
	palo->paloRoot = savedRoot;

	ClearDl(&palo->dlChild);

	palo->cpmrg = paloBase->cpmrg;
	for (int i = 0; i < 4; ++i)
		palo->apmrg[i] = paloBase->apmrg[i];

	palo->sMRD = paloBase->sMRD;
	palo->sCelBorderMRD = paloBase->sCelBorderMRD;
	palo->grfzon = paloBase->grfzon;
	palo->dsMRDSnap = paloBase->dsMRDSnap;
	palo->frz = paloBase->frz;

	palo->xf = paloBase->xf;
	palo->posOrig = paloBase->posOrig;
	palo->matOrig = paloBase->matOrig;
	palo->eulOrig = paloBase->eulOrig;

	palo->pactPos = paloBase->pactPos;
	palo->pactRot = paloBase->pactRot;
	palo->pactScale = paloBase->pactScale;
	palo->apactPose = paloBase->apactPose;

	palo->pactRestore = nullptr;
	palo->pactla = nullptr;
	palo->pactbank = nullptr;

	palo->pikh = paloBase->pikh;

	palo->pclqPosSpring = paloBase->pclqPosSpring;
	palo->pclqPosDamping = paloBase->pclqPosDamping;
	palo->pclqRotSpring = paloBase->pclqRotSpring;
	palo->pclqRotDamping = paloBase->pclqRotDamping;

	palo->psmpaPos = paloBase->psmpaPos;
	palo->psmpaRot = paloBase->psmpaRot;

	if (paloBase->palox)
		palo->palox = std::make_shared<ALOX>(*paloBase->palox);
	else
		palo->palox.reset();

	palo->cframeStatic = paloBase->cframeStatic;

	CloneGlobset(&palo->globset, palo, &paloBase->globset);

	palo->sFastShadowRadius = paloBase->sFastShadowRadius;
	palo->sFastShadowDepth = paloBase->sFastShadowDepth;
	palo->fRealClock = paloBase->fRealClock;
	palo->pfader = paloBase->pfader;
	palo->dtUpdatePause = paloBase->dtUpdatePause;
	palo->pasegd = paloBase->pasegd;

	palo->sRadiusRenderSelf = paloBase->sRadiusRenderSelf;
	palo->sRadiusRenderAll = paloBase->sRadiusRenderAll;

	palo->ficg = paloBase->ficg;
	palo->cposec = paloBase->cposec;
	palo->aposec = paloBase->aposec;
	palo->pactrefCombo = paloBase->pactrefCombo;
	palo->pdlrFirst = paloBase->pdlrFirst;

	palo->zons = paloBase->zons;
	palo->viss = paloBase->viss;
	palo->mrds = paloBase->mrds;
	palo->dms = paloBase->dms;
	palo->fHidden = paloBase->fHidden;
	palo->fFixedPhys = paloBase->fFixedPhys;
	palo->fMtlkFromDls = paloBase->fMtlkFromDls;
	palo->fWater = paloBase->fWater;
	palo->fForceCameraFade = paloBase->fForceCameraFade;
	palo->fBusy = true;
	palo->fFrozen = paloBase->fFrozen;
	palo->fRemerge = paloBase->fRemerge;
	palo->freezeMode = paloBase->freezeMode;
	palo->cpaloFindSwObjects = paloBase->cpaloFindSwObjects;
	palo->fApplyAseg = paloBase->fApplyAseg;

	palo->ackPos = paloBase->ackPos;
	palo->ackRot = paloBase->ackRot;

	palo->pvtalo->pfnRecacheAloActList(palo);

	if (paloBase->pactRestore != nullptr)
	{
		palo->pactRestore = PactNewClone(paloBase->pactRestore.get(), palo->psw, palo);
		InsertAloAct(palo, palo->pactRestore.get());
	}

	if (paloBase->pactla != nullptr)
	{
		palo->pactla = PactlaNewClone(paloBase->pactla.get(), palo->psw, palo);
		InsertAloAct(palo, palo->pactla.get());
	}

	if (paloBase->pactbank != nullptr)
	{
		palo->pactbank = PactbankNewClone(paloBase->pactbank.get(), palo->psw, palo);;
		InsertAloAct(palo, palo->pactbank.get());
	}

	if (paloBase->pshadow)
	{
		palo->pshadow = std::make_shared<SHADOW>(*paloBase->pshadow);
		ClearDle(&palo->pshadow->dle);
		AppendDlEntry(&palo->psw->dlShadow, palo->pshadow.get());
	}

	if (paloBase->psfx)
		palo->psfx = std::make_shared <SFX>(*paloBase->psfx);

	if (paloBase->pthrob)
		palo->pthrob = std::make_shared <THROB>(*paloBase->pthrob);

	if (palo->fForceCameraFade && FIsLoInWorld(palo))
		AddCmFadeObject(g_pcm, palo);

	if (paloBase->pclqPosSpring && paloBase->pclqPosSpring.get() != &g_clqPosSpringDefault)
		palo->pclqPosSpring = std::make_shared<CLQ>(*paloBase->pclqPosSpring);

	if (paloBase->pclqPosDamping && paloBase->pclqPosDamping.get() != &g_clqPosDampingDefault)
		palo->pclqPosDamping = std::make_shared<CLQ>(*paloBase->pclqPosDamping);

	if (paloBase->pclqRotSpring && paloBase->pclqRotSpring.get() != &g_clqRotSpringDefault)
		palo->pclqRotSpring = std::make_shared<CLQ>(*paloBase->pclqRotSpring);

	if (paloBase->pclqRotDamping && paloBase->pclqRotDamping.get() != &g_clqRotDampingDefault)
		palo->pclqRotDamping = std::make_shared<CLQ>(*paloBase->pclqRotDamping);

}

void HandleAloMessage(ALO* palo, MSGID msgid, void* pv)
{
	if (msgid != MSGID_added)
		return;

	SFX *psfx = palo->psfx.get();

	if (psfx == nullptr)
		return;

	if (g_clock.t == 0.0f)
		return;

	if (psfx->pamb != nullptr)
		return;

	StartSound(psfx->sfxid, &psfx->pamb, palo, nullptr, psfx->sStart, psfx->sFull, psfx->uVol, psfx->uPitch, psfx->uDoppler, &psfx->lmRepeat, nullptr);
}

bool FIsZeroV(const glm::vec3& v)
{
	return glm::dot(v, v) < 4.0f;
}

bool FIsZeroW(const glm::vec3& w)
{
	return glm::dot(w, w) < 0.0004f;
}

bool FIsZeroDv(const glm::vec3& dv)
{
	return glm::dot(dv, dv) < 25.0f;
}

bool FIsZeroDw(const glm::vec3& dw)
{
	return glm::dot(dw, dw) < 0.0025000002f;
}

int FIsAloStatic(ALO* palo)
{
	if (!FIsZeroV(palo->xf.v))
		return false;

	if (!FIsZeroW(palo->xf.w))
		return false;

	for (ALO* child = palo->dlChild.paloFirst; child != nullptr; child = child->dleChild.paloNext)
	{
		if ((child->pvtlo->grfcid & 1U) == 0)
			continue;

		if (!FIsAloStatic(child))
			return false;
	}

	return true;
}

void ResolveAlo(ALO* palo)
{
	if (palo->paloRoot != nullptr)
		palo->paloRoot->cframeStatic = 0;
}

void InvalidateAloLighting(ALO* palo)
{
	if (palo == nullptr)
		return;

	const size_t count = std::min(palo->globset.aglob.size(), palo->globset.aglobi.size());

	for (size_t i = 0; i < count; ++i)
	{
		GLOB& glob = palo->globset.aglob[i];
		GLOBI& globi = palo->globset.aglobi[i];

		// Retail invalidates the saved predraw packet for every glob. In this
		// renderer that packet is the cached three-way material SSBO.
		globi.cframeStaticLights = -1;

		if (glob.fThreeWay == 1 && glob.fDynamic == 0 && glob.pwarpGlob == nullptr)
			glob.trlk = TRLK_Relight;
	}
}

void SetAloParent(ALO* palo, ALO* paloParent)
{
	if (palo->paloParent == paloParent)
		return;


	glm::vec3 posWorld = palo->xf.posWorld;
	glm::mat3 matWorld = palo->xf.matWorld;

	glm::vec3 vWorld{};
	glm::vec3 wWorld{};

	if (palo->paloParent == nullptr)
	{
		vWorld = palo->xf.v;
		wWorld = palo->xf.w;
	}
	else
		CalculateAloMovement(palo, nullptr, palo->xf.posWorld, &vWorld, &wWorld, nullptr, nullptr);

	const bool wasRoot = palo->paloParent == nullptr;
	const int mrdsOld = palo->mrds;

	palo->pvtlo->pfnRemoveLo(palo);

	ConvertAloPos(nullptr, paloParent, &posWorld, &palo->xf.pos);
	ConvertAloMat(nullptr, paloParent, &matWorld, &palo->xf.mat);

	if (paloParent != nullptr)
	{
		glm::vec3 vParent{};
		glm::vec3 wParent{};

		CalculateAloMovement(paloParent, nullptr, palo->xf.posWorld, &vParent, &wParent, nullptr, nullptr);

		vWorld -= vParent;
		wWorld -= wParent;
	}

	glm::vec3 v{};
	glm::vec3 w{};

	ConvertAloVec(nullptr, paloParent, &vWorld, &v);
	palo->pvtalo->pfnSetAloVelocityVec(palo, &v);

	ConvertAloVec(nullptr, paloParent, &wWorld, &w);
	palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &w);

	const bool isRoot = paloParent == nullptr;

	if (wasRoot != isRoot)
	{
		if (isRoot)
		{
			palo->viss = 2;
			palo->zons = 2;

			if (mrdsOld != 1)
				palo->mrds = 2;
		}
		else
		{
			palo->viss = 1;

			/* Retail reads the field after assigning viss = 1.  Consequently
			 * this condition is false and the existing zons mode is preserved. */
			if (palo->viss != 1)
				palo->zons = 1;

			if (mrdsOld != 1 &&
				!(mrdsOld == 2 && palo->sMRD != 1e+10f))
			{
				palo->mrds = 0;
			}
		}
	}

	UpdateAloOrig(palo);

	palo->paloParent = paloParent;
	palo->pvtlo->pfnAddLo(palo);

}
void ApplyAloProxy(ALO* palo, PROXY* proxy)
{
	glm::vec3 posWorld;
	glm::mat3 matWorld;

	// Convert proxy-relative position into world space
	ConvertAloPos(reinterpret_cast<ALO*>(proxy), nullptr, &palo->xf.pos, &posWorld);
	palo->pvtalo->pfnTranslateAloToPos(palo, &posWorld);
	// Convert proxy-relative orientation into world space
	ConvertAloMat(reinterpret_cast<ALO*>(proxy), nullptr, &palo->xf.mat, &matWorld);

	palo->pvtalo->pfnRotateAloToMat(palo, &matWorld);

	glm::vec3 vParent{};
	glm::vec3 wParent{};
	CalculateAloMovement(proxy->paloParent, nullptr, posWorld, &vParent, &wParent, nullptr, nullptr);

	glm::vec3 vPalo;
	glm::vec3 wPalo;
	ConvertAloVec(reinterpret_cast<ALO*>(proxy), nullptr, &palo->xf.v, &vPalo);
	ConvertAloVec(reinterpret_cast<ALO*>(proxy), nullptr, &palo->xf.w, &wPalo);

	glm::vec3 v = vParent + vPalo;
	glm::vec3 w = wParent + wPalo;

	palo->pvtalo->pfnSetAloVelocityVec(palo, &v);
	palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &w);

	palo->posOrig = palo->xf.pos;
	palo->matOrig = palo->xf.mat;
}

void BindAlo(ALO* palo)
{
	BindAloAlox(palo);
	UpdateAloOrig(palo);

	if (palo->pactla != nullptr)
	{
		// Retail loads pactla+0x20 with lw: before binding this field contains
		// a 32-bit OID, despite being used as an ALO pointer after resolution.
		const uint32_t oidTargetRaw = static_cast<uint32_t>(reinterpret_cast<std::uintptr_t>(palo->pactla->paloTarget));

		if (oidTargetRaw != UINT32_MAX)
		{
			ALO* paloTarget = reinterpret_cast<ALO*>(PloFindSwNearest(palo->psw, static_cast<OID>(oidTargetRaw), palo));

			if (paloTarget != nullptr)
				SetAloActlaTarget(palo, paloTarget);
		}
	}

	BindGlobset(&palo->globset, palo);

	LO* plo = palo->dlChild.ploFirst;

	while (plo != nullptr)
	{
		if (plo->pvtalo->pfnBindAlo != nullptr)
			plo->pvtalo->pfnBindAlo((ALO*)plo);

		plo = plo->dleChild.ploNext;
	}
}

void BindGlobset(GLOBSET* pglobset, ALO* palo)
{
	glm::mat4 matAlo;
	glm::mat4 matBndInv;

	for (int ibnd = 0; ibnd < pglobset->cbnd; ibnd++)
	{
		BND *pbnd = &pglobset->abnd[ibnd];
		pbnd->palo = (ALO*)PloFindSwNearest(palo->psw, pglobset->mpibndoid[ibnd], palo);

		if (pbnd->palo != nullptr)
		{
			LoadMatrixFromPosRot(&palo->xf.posWorld, &palo->xf.matWorld, &matAlo);
			LoadMatrixFromPosRotInverse(&pbnd->palo->xf.posWorld, &pbnd->palo->xf.matWorld, &matBndInv);

			pbnd->dmatOrig = matBndInv * matAlo;
		}

	}

}

void UpdateAloXfWorld(ALO* palo)
{
	palo->pvtalo->pfnUpdateAloXfWorldHierarchy(palo);
}

void UpdateAloXfWorldHierarchy(ALO* palo)
{
	ALO* paloPosParent = palo->paloParent;
	ALO* paloRotParent = palo->paloParent;

	if (palo->palox != nullptr && (palo->palox->grfalox & 0x0CU) != 0)
	{
		// Retail overlays the foster basis and SCJ basis pointer in ALOX.
		// They are separate members in this port, so prefer the foster basis
		// used by fired RWM ammo and fall back to the SCJ copy.
		ALO* paloTransformBasis = palo->palox->foster.paloParent != nullptr
			? palo->palox->foster.paloParent
			: palo->palox->scj.paloSchRot;

		if ((palo->palox->grfalox & 0x04U) != 0)
			paloPosParent = paloTransformBasis;

		if ((palo->palox->grfalox & 0x08U) != 0)
			paloRotParent = paloTransformBasis;
	}

	if (paloPosParent != nullptr)
		palo->xf.posWorld = paloPosParent->xf.posWorld + paloPosParent->xf.matWorld * palo->xf.pos;
	else
		palo->xf.posWorld = palo->xf.pos;

	if (paloRotParent != nullptr)
		palo->xf.matWorld = paloRotParent->xf.matWorld * palo->xf.mat;
	else
		palo->xf.matWorld = palo->xf.mat;

	if (palo->zons == 2)
	{
		if (palo->psw == nullptr || palo->psw->pvismap == nullptr)
			palo->grfzon = 0x0FFFFFFF;
		else
			ClipVismapSphereOneHop(palo->psw->pvismap, &palo->xf.posWorld, palo->sRadiusRenderAll, &palo->grfzon);
	}

	if (palo->pshadow != nullptr)
	{
		SetShadowCastPosition(palo->pshadow.get(), palo->xf.posWorld);

		if (palo->pshadow->pshd != nullptr && palo->pshadow->pshd->shdk == 3)
		{
			const glm::vec3 normalCast = -palo->xf.matWorld[2];

			SetShadowCastNormal(palo->pshadow.get(), normalCast);
			SetShadowFrustrumUp(palo->pshadow.get(), &palo->xf.matWorld[1]);
		}
	}

	for (LO* ploChild = palo->dlChild.ploFirst; ploChild != nullptr;)
	{
		LO* ploNext = ploChild->dleChild.ploNext;

		if (ploChild->pvtlo != nullptr && ploChild->pvtlo->pfnUpdateLoXfWorldHierarchy != nullptr)
			ploChild->pvtlo->pfnUpdateLoXfWorldHierarchy(ploChild);

		ploChild = ploNext;
	}

	if (palo->palox != nullptr && (palo->palox->grfalox & 0x8020U) == 0x8020U)
	{
		ALO* paloShoulder = palo->palox->ikh.paloShoulder;
		ALO* paloElbow = palo->palox->ikh.paloElbow;

		if (paloShoulder != nullptr && paloShoulder->palox != nullptr)
			paloShoulder->palox->ikj.fInvalid = 1;

		if (paloElbow != nullptr && paloElbow->palox != nullptr)
			paloElbow->palox->ikj.fInvalid = 1;
	}
}

void UpdateAloHierarchy(ALO* palo, float dt)
{
	if (palo->pvtalo->pfnUpdateAlo != nullptr)
		palo->pvtalo->pfnUpdateAlo(palo, dt);

	int isInSw = FIsLoInWorld(palo);

	if (isInSw == true)
	{
		DLI dlBusyWalker;

		dlBusyWalker.m_pdl = &palo->dlChild;        // Point to the actual DL list
		dlBusyWalker.m_ibDle = palo->dlChild.ibDle; // Offset to the 'next' pointer inside each object
		dlBusyWalker.m_pdliNext = s_pdliFirst;      // Link this walker into a global list of DLI walkers

		// Get the first object (LO) in the busy list
		LO* currentObject = palo->dlChild.ploFirst;

		// Set up the pointer to the "next" object in the list,
		// using offset-based pointer arithmetic from current object
		dlBusyWalker.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle);

		// Save the current DLI walker globally
		s_pdliFirst = &dlBusyWalker;

		// Loop over every object in the busy list
		while (currentObject != nullptr)
		{
			// Call the update function on the current object child
			// This updates the object and all of its attached ALO children
			if ((currentObject->pvtalo->grfcid & 1U) != 0)
				UpdateAloHierarchy(reinterpret_cast<ALO*>(currentObject), dt);
			
			// Move to the next object in the list using the stored offset
			currentObject = reinterpret_cast<LO*>(*dlBusyWalker.m_ppv);

			// If there is a next object, update the walker�s pointer to its next link
			dlBusyWalker.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle);
		}

		s_pdliFirst = dlBusyWalker.m_pdliNext;
	}
}

void TranslateAloToPos(ALO* palo, glm::vec3 *ppos)
{
	MSGTRANS msgtrans{};
	msgtrans.palo = palo;
	msgtrans.posWorldPrev = palo->xf.posWorld;
	palo->xf.pos = *ppos;
	if (VTALO* pvtalo = palo->pvtalo)
	{
		if (pvtalo->pfnUpdateAloXfWorld)
			pvtalo->pfnUpdateAloXfWorld(palo);

		if (palo->pactPos)
			AdaptAct(palo->pactPos);

		if (pvtalo->pfnSendLoMessage)
			pvtalo->pfnSendLoMessage(palo, MSGID_translated, &msgtrans);
	}
}

void ConvertAloPos(ALO* paloFrom, ALO* paloTo, glm::vec3* pposFrom, glm::vec3* pposTo)
{
	if (pposFrom == nullptr || pposTo == nullptr)
		return;

	if (paloFrom == paloTo)
	{
		*pposTo = *pposFrom;
		return;
	}

	glm::vec3 pos = *pposFrom;

	if (paloFrom != nullptr)
		pos = paloFrom->xf.matWorld * pos + paloFrom->xf.posWorld;

	if (paloTo != nullptr)
	{
		const glm::vec3 delta = pos - paloTo->xf.posWorld;
		pos = glm::transpose(paloTo->xf.matWorld) * delta;
	}

	*pposTo = pos;
}

void ConvertAloVec(ALO* paloFrom, ALO* paloTo, glm::vec3* pvecFrom, glm::vec3* pvecTo)
{
	if (pvecFrom == nullptr || pvecTo == nullptr)
		return;

	if (paloFrom == paloTo)
	{
		*pvecTo = *pvecFrom;
		return;
	}

	glm::vec3 vec = *pvecFrom;

	if (paloFrom != nullptr)
		vec = paloFrom->xf.matWorld * vec;

	if (paloTo != nullptr)
		vec = glm::transpose(paloTo->xf.matWorld) * vec;

	*pvecTo = vec;
}

void RotateAloToMat(ALO* palo, glm::mat3 *pmat)
{
	//Copy local rotation matrix into ALO transform
	palo->xf.mat = *pmat;

	VTALO* pvtalo = palo->pvtalo;

	// Update world transform if this ALO type provides an update callback
	if (pvtalo->pfnUpdateAloXfWorld) {
		pvtalo->pfnUpdateAloXfWorld(palo);
	}

	// Re-adapt rotation action if present
	if (palo->pactRot) {
		AdaptAct(palo->pactRot);
	}

	// Send transform/rotation changed message
	pvtalo->pfnSendLoMessage(palo, MSGID_rotated, palo);
}

void ConvertAloMat(ALO* paloFrom, ALO* paloTo, glm::mat3* pmatFrom, glm::mat3* pmatTo)
{
	if (pmatFrom == nullptr || pmatTo == nullptr)
		return;

	if (paloFrom == paloTo)
	{
		*pmatTo = *pmatFrom;
		return;
	}

	glm::mat3 mat = *pmatFrom;

	if (paloFrom != nullptr)
		mat = paloFrom->xf.matWorld * mat;

	if (paloTo != nullptr)
		mat = glm::transpose(paloTo->xf.matWorld) * mat;

	*pmatTo = mat;
}

void SetAloTransformBasis(ALO* palo, ALO* paloPosBasis, ALO* paloRotBasis)
{
	if (palo == nullptr)
		return;

	ALO* paloParent = palo->paloParent;
	if (paloPosBasis != paloParent && paloRotBasis != paloParent &&
		paloPosBasis != paloRotBasis)
	{
		return;
	}

	const bool fPosOverride = paloPosBasis != paloParent;
	const bool fRotOverride = paloRotBasis != paloParent;
	if ((fPosOverride || fRotOverride) && palo->palox == nullptr)
		palo->palox = std::make_shared<ALOX>();

	if (palo->palox != nullptr)
	{
		ALOX* palox = palo->palox.get();
		if (fPosOverride)
		{
			palox->foster.paloParent = paloPosBasis;
			palox->scj.paloSchRot = paloPosBasis;
			palox->grfalox = static_cast<GRFALOX>(palox->grfalox | 0x04U);
		}
		else
		{
			palox->grfalox = static_cast<GRFALOX>(palox->grfalox & ~0x04U);
		}

		if (fRotOverride)
		{
			palox->foster.paloParent = paloRotBasis;
			palox->scj.paloSchRot = paloRotBasis;
			palox->grfalox = static_cast<GRFALOX>(palox->grfalox | 0x08U);
		}
		else
		{
			palox->grfalox = static_cast<GRFALOX>(palox->grfalox & ~0x08U);
		}
	}

	ConvertAloPos(nullptr, paloPosBasis, &palo->xf.posWorld, &palo->xf.pos);
	ConvertAloMat(nullptr, paloRotBasis, &palo->xf.matWorld, &palo->xf.mat);
}

int FDrivenAlo(ALO* palo)
{
	if (palo->pactPos != nullptr && palo->pactPos->ackPos == ACK_Drive) {
		return 1;
	}

	if (palo->pactRot != nullptr && palo->pactRot->ackRot == ACK_Drive) {
		return 1;
	}

	return 0;
}

void ConvertAloMovement(ALO* paloFrom, ALO* paloTo, glm::vec3* ppos, glm::vec3* pvFrom, glm::vec3* pwFrom, glm::vec3* pdvFrom, glm::vec3* pdwFrom, glm::vec3* pvTo, glm::vec3* pwTo, glm::vec3* pdvTo, glm::vec3* pdwTo)
{
	ALO* paloCommon = PaloFindLoCommonParent(paloFrom, paloTo);

	glm::vec3 vCommon{};
	glm::vec3 wCommon{};
	glm::vec3 dvCommon{};
	glm::vec3 dwCommon{};

	if (pvFrom)  ConvertAloVec(paloFrom, paloCommon, pvFrom, &vCommon);
	if (pwFrom)  ConvertAloVec(paloFrom, paloCommon, pwFrom, &wCommon);
	if (pdvFrom) ConvertAloVec(paloFrom, paloCommon, pdvFrom, &dvCommon);
	if (pdwFrom) ConvertAloVec(paloFrom, paloCommon, pdwFrom, &dwCommon);

	if (paloFrom)
	{
		glm::vec3 vFromExtra{};
		glm::vec3 wFromExtra{};
		glm::vec3 dvFromExtra{};
		glm::vec3 dwFromExtra{};

		CalculateAloMovement(paloFrom, paloCommon, *ppos, pvFrom ? &vFromExtra : nullptr, pwFrom ? &wFromExtra : nullptr, pdvFrom ? &dvFromExtra : nullptr, pdwFrom ? &dwFromExtra : nullptr);

		if (pvFrom)  vCommon += vFromExtra;
		if (pwFrom)  wCommon += wFromExtra;
		if (pdvFrom) dvCommon += dvFromExtra;
		if (pdwFrom) dwCommon += dwFromExtra;
	}

	if (paloTo)
	{
		glm::vec3 vToExtra{};
		glm::vec3 wToExtra{};
		glm::vec3 dvToExtra{};
		glm::vec3 dwToExtra{};

		CalculateAloMovement(paloTo, paloCommon, *ppos, pvTo ? &vToExtra : nullptr, pwTo ? &wToExtra : nullptr, pdvTo ? &dvToExtra : nullptr, pdwTo ? &dwToExtra : nullptr);

		if (pvTo)  vCommon -= vToExtra;
		if (pwTo)  wCommon -= wToExtra;
		if (pdvTo) dvCommon -= dvToExtra;
		if (pdwTo) dwCommon -= dwToExtra;
	}

	if (pvTo)  ConvertAloVec(paloCommon, paloTo, &vCommon, pvTo);
	if (pwTo)  ConvertAloVec(paloCommon, paloTo, &wCommon, pwTo);
	if (pdvTo) ConvertAloVec(paloCommon, paloTo, &dvCommon, pdvTo);
	if (pdwTo) ConvertAloVec(paloCommon, paloTo, &dwCommon, pdwTo);
}

void SetAloVelocityVec(ALO* palo, glm::vec3* velocity)
{
	palo->xf.v = *velocity;

	if (palo->pactPos)
		AdaptAct(palo->pactPos);

	if (glm::length2(*velocity) != 0.0f)
		ResolveAlo(palo);
}

void SetAloVelocityXYZ(ALO* palo, float x, float y, float z)
{
	glm::vec3 velocity(x, y, z);
	palo->pvtalo->pfnSetAloVelocityVec(palo, &velocity);
}

void SetAloAngularVelocityVec(ALO* palo, glm::vec3* angularVelocity)
{
	palo->xf.w = *angularVelocity;

	if (palo->pactRot)
		AdaptAct(palo->pactRot);

	if (glm::length2(*angularVelocity) != 0.0f)
		ResolveAlo(palo);
}

void SetAloAngularVelocityXYZ(ALO* palo, float x, float y, float z)
{
	glm::vec3 angularVelocity(x, y, z);
	palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &angularVelocity);
}

void PresetAloAccel(ALO* palo, float dt)
{
}

void ProjectAloTransform(ALO* palo, float dt, int fParentDirty)
{
	int dirty = fParentDirty;
	const float dtClock = g_clock.dt;

	ACT* pactPos = palo->pactPos;
	ACT* pactRot = palo->pactRot;

	bool drivenPos = pactPos == nullptr || pactPos->ackPos == ACK_Drive;
	bool drivenRot = pactRot == nullptr || pactRot->ackRot == ACK_Drive;

	// If either transform channel is driven, reset accel first.
	if (drivenPos || drivenRot)
	{
		palo->xf.dv = glm::vec3{};
		palo->xf.dw = glm::vec3{};
		palo->pvtalo->pfnPresetAloAccel(palo, dtClock);
	}

	// ----------------------------
	// Position
	// ----------------------------
	if (drivenPos)
	{
		if (!FIsZeroV(palo->xf.v) || !FIsZeroDv(palo->xf.dv))
		{
			// pos += v * dt + dv * 0.5f * dt^2
			palo->xf.pos += palo->xf.v * dtClock;
			palo->xf.pos += palo->xf.dv * (0.5f * dtClock * dtClock);

			// v += dv * dt
			palo->xf.v += palo->xf.dv * dtClock;
			dirty = 1;
		}
	}
	else
	{
		ProjectActPosition(pactPos);
		dirty = 1;
	}

	// ----------------------------
	// Rotation
	// ----------------------------
	if (drivenRot)
	{
		if (!FIsZeroW(palo->xf.w) || !FIsZeroDw(palo->xf.dw))
		{
			glm::vec3 oldW = palo->xf.w;

			// w += dw * dt
			palo->xf.w += palo->xf.dw * dtClock;

			// average angular velocity over frame
			glm::vec3 rot = (oldW + palo->xf.w) * (0.5f * dtClock);

			glm::mat3 dmat;
			LoadRotateMatrix(&rot, &dmat);

			// xf.mat = dmat * xf.mat
			palo->xf.mat = dmat * palo->xf.mat;

			NormalizeRotateMatrix3(&palo->xf.mat);
			dirty = 1;
		}
	}
	else
	{
		ProjectActRotation(pactRot);
		dirty = 1;
	}

	// ----------------------------
	// Children
	// ----------------------------
	DLI dliChild;
	dliChild.m_pdl = &palo->dlChild;
	dliChild.m_ibDle = palo->dlChild.ibDle;
	dliChild.m_pdliNext = s_pdliFirst;

	LO* ploChild = static_cast<LO*>(palo->dlChild.ploFirst);

	while (ploChild != nullptr)
	{
		s_pdliFirst = &dliChild;

		if (ploChild->pvtlo->grfcid & 1)
		{
			ALO* paloChild = static_cast<ALO*>(ploChild);
			paloChild->pvtalo->pfnProjectAloTransform(paloChild, dt, dirty);
		}

		ploChild = *reinterpret_cast<LO**>(reinterpret_cast<char*>(ploChild) + dliChild.m_ibDle);
	}

	s_pdliFirst = dliChild.m_pdliNext;

	// Only update this node's world transform if this node became dirty
	// and the parent was not already dirty.
	if (dirty && !fParentDirty)
	{
		if (palo->pvtalo->pfnUpdateAloXfWorld != nullptr)
			palo->pvtalo->pfnUpdateAloXfWorld(palo);
	}

}

void PredictAloTransform(ALO* paloLeaf, ALO* paloBasis, float dtOffset, glm::vec3* ppos, glm::mat3* pmat, glm::vec3* pv, glm::vec3* pw)
{
	ALO* apalo[16];
	int cpalo = 0;

	for (ALO* palo = paloLeaf; palo != paloBasis; palo = palo->paloParent)
		apalo[cpalo++] = palo;

	glm::vec3 posSum(0.0f);
	glm::vec3 vSum(0.0f);
	glm::vec3 wSum(0.0f);
	glm::mat3 matSum(1.0f);

	for (int i = cpalo - 1; i >= 0; --i)
	{
		ALO* palo = apalo[i];

		if (ppos != nullptr || pv != nullptr)
		{
			glm::vec3 pos;
			glm::vec3 v;

			palo->pvtalo->pfnPredictAloPosition(palo, dtOffset, &pos, &v);

			glm::vec3 posRotated;
			posRotated.x = matSum[0].x * pos.x + matSum[1].x * pos.y + matSum[2].x * pos.z;
			posRotated.y = matSum[0].y * pos.x + matSum[1].y * pos.y + matSum[2].y * pos.z;
			posRotated.z = matSum[0].z * pos.x + matSum[1].z * pos.y + matSum[2].z * pos.z;

			posSum += posRotated;

			if (pv != nullptr)
			{
				glm::vec3 vRotated;
				vRotated.x = matSum[0].x * v.x + matSum[1].x * v.y + matSum[2].x * v.z;
				vRotated.y = matSum[0].y * v.x + matSum[1].y * v.y + matSum[2].y * v.z;
				vRotated.z = matSum[0].z * v.x + matSum[1].z * v.y + matSum[2].z * v.z;

				vSum += vRotated + glm::cross(wSum, posRotated);
			}
		}

		glm::mat3 mat;
		glm::vec3 w;

		palo->pvtalo->pfnPredictAloRotation(palo, dtOffset, &mat, &w);

		if (pw != nullptr || pv != nullptr)
		{
			glm::vec3 wRotated;
			wRotated.x = matSum[0].x * w.x + matSum[1].x * w.y + matSum[2].x * w.z;
			wRotated.y = matSum[0].y * w.x + matSum[1].y * w.y + matSum[2].y * w.z;
			wRotated.z = matSum[0].z * w.x + matSum[1].z * w.y + matSum[2].z * w.z;

			wSum += wRotated;
		}

		glm::mat3 matNext;

		matNext[0].x = matSum[0].x * mat[0].x + matSum[1].x * mat[0].y + matSum[2].x * mat[0].z;
		matNext[0].y = matSum[0].y * mat[0].x + matSum[1].y * mat[0].y + matSum[2].y * mat[0].z;
		matNext[0].z = matSum[0].z * mat[0].x + matSum[1].z * mat[0].y + matSum[2].z * mat[0].z;

		matNext[1].x = matSum[0].x * mat[1].x + matSum[1].x * mat[1].y + matSum[2].x * mat[1].z;
		matNext[1].y = matSum[0].y * mat[1].x + matSum[1].y * mat[1].y + matSum[2].y * mat[1].z;
		matNext[1].z = matSum[0].z * mat[1].x + matSum[1].z * mat[1].y + matSum[2].z * mat[1].z;

		matNext[2].x = matSum[0].x * mat[2].x + matSum[1].x * mat[2].y + matSum[2].x * mat[2].z;
		matNext[2].y = matSum[0].y * mat[2].x + matSum[1].y * mat[2].y + matSum[2].y * mat[2].z;
		matNext[2].z = matSum[0].z * mat[2].x + matSum[1].z * mat[2].y + matSum[2].z * mat[2].z;

		matSum = matNext;
	}

	if (ppos != nullptr)
		*ppos = posSum;

	if (pmat != nullptr)
		*pmat = matSum;

	if (pv != nullptr)
		*pv = vSum;

	if (pw != nullptr)
		*pw = wSum;
}

void PredictAloTransformAdjust(ALO* paloLeaf, ALO* paloBasis, float dtOffset, glm::vec3* ppos, glm::mat3* pmat, glm::vec3* pv, glm::vec3* pw)
{
	glm::vec3 posBasis(0.0f);
	glm::vec3 vBasis(0.0f);
	glm::vec3 wBasis(0.0f);
	glm::mat3 matBasis(1.0f);

	glm::vec3* pposBasis = nullptr;
	glm::vec3* pvBasis = nullptr;
	glm::vec3* pwBasis = nullptr;

	if (ppos != nullptr || pv != nullptr)
		pposBasis = &posBasis;

	if (pv != nullptr)
		pvBasis = &vBasis;

	if (pw != nullptr || pv != nullptr)
		pwBasis = &wBasis;

	PredictAloTransform(paloLeaf, paloBasis, dtOffset, pposBasis, &matBasis, pvBasis, pwBasis);

	glm::vec3 posRotated(0.0f);

	if (ppos != nullptr)
	{
		posRotated.x = matBasis[0].x * ppos->x + matBasis[1].x * ppos->y + matBasis[2].x * ppos->z;
		posRotated.y = matBasis[0].y * ppos->x + matBasis[1].y * ppos->y + matBasis[2].y * ppos->z;
		posRotated.z = matBasis[0].z * ppos->x + matBasis[1].z * ppos->y + matBasis[2].z * ppos->z;

		*ppos = posBasis + posRotated;
	}

	if (pv != nullptr)
	{
		glm::vec3 vRotated;

		vRotated.x = matBasis[0].x * pv->x + matBasis[1].x * pv->y + matBasis[2].x * pv->z;
		vRotated.y = matBasis[0].y * pv->x + matBasis[1].y * pv->y + matBasis[2].y * pv->z;
		vRotated.z = matBasis[0].z * pv->x + matBasis[1].z * pv->y + matBasis[2].z * pv->z;

		*pv = vBasis + vRotated + glm::cross(wBasis, posRotated);
	}

	if (pw != nullptr)
	{
		glm::vec3 wRotated;

		wRotated.x = matBasis[0].x * pw->x + matBasis[1].x * pw->y + matBasis[2].x * pw->z;
		wRotated.y = matBasis[0].y * pw->x + matBasis[1].y * pw->y + matBasis[2].y * pw->z;
		wRotated.z = matBasis[0].z * pw->x + matBasis[1].z * pw->y + matBasis[2].z * pw->z;

		*pw = wBasis + wRotated;
	}

	if (pmat != nullptr)
	{
		const glm::mat3 matLocal = *pmat;
		glm::mat3 matResult;

		matResult[0].x = matBasis[0].x * matLocal[0].x + matBasis[1].x * matLocal[0].y + matBasis[2].x * matLocal[0].z;
		matResult[0].y = matBasis[0].y * matLocal[0].x + matBasis[1].y * matLocal[0].y + matBasis[2].y * matLocal[0].z;
		matResult[0].z = matBasis[0].z * matLocal[0].x + matBasis[1].z * matLocal[0].y + matBasis[2].z * matLocal[0].z;

		matResult[1].x = matBasis[0].x * matLocal[1].x + matBasis[1].x * matLocal[1].y + matBasis[2].x * matLocal[1].z;
		matResult[1].y = matBasis[0].y * matLocal[1].x + matBasis[1].y * matLocal[1].y + matBasis[2].y * matLocal[1].z;
		matResult[1].z = matBasis[0].z * matLocal[1].x + matBasis[1].z * matLocal[1].y + matBasis[2].z * matLocal[1].z;

		matResult[2].x = matBasis[0].x * matLocal[2].x + matBasis[1].x * matLocal[2].y + matBasis[2].x * matLocal[2].z;
		matResult[2].y = matBasis[0].y * matLocal[2].x + matBasis[1].y * matLocal[2].y + matBasis[2].y * matLocal[2].z;
		matResult[2].z = matBasis[0].z * matLocal[2].x + matBasis[1].z * matLocal[2].y + matBasis[2].z * matLocal[2].z;

		*pmat = matResult;
	}
}

void UpdateAloInfluences(ALO* palo, RO* pro)
{
	GLOBSET& globset = palo->globset;

	if (globset.cbnd <= 0)
		return;

	glm::mat4 matAloInv;
	LoadMatrixFromPosRotInverse(&palo->xf.posWorld, &palo->xf.matWorld, &matAloInv);

	glm::mat4 matRoInv;

	if (pro == nullptr)
		matRoInv = matAloInv;
	else
	{
		glm::vec3 posRo = glm::vec3(pro->model[3]);
		glm::mat3 matRo = glm::mat3(pro->model);

		LoadMatrixFromPosRotInverse(&posRo, &matRo, &matRoInv);
	}

	for (int ibnd = 0; ibnd < globset.cbnd; ++ibnd)
	{
		BND& bnd = globset.abnd[ibnd];
		ALO* paloBone = bnd.palo;

		if (paloBone == nullptr)
			continue;

		if (paloBone->pvtalo != nullptr && paloBone->pvtalo->pfnUpdateAloConstraints != nullptr)
			paloBone->pvtalo->pfnUpdateAloConstraints(paloBone);

		glm::mat4 matBnd;
		const glm::mat4* pmatLeft = &matAloInv;

		ALOX* palox = paloBone->palox.get();

		const bool useJointInfluence =
			palox != nullptr &&
			(palox->grfalox & 0x400U) != 0 &&
			palox->joint.fMatInfluence != 0;

		if (useJointInfluence)
		{
			pmatLeft = &matRoInv;
			matBnd = palox->joint.matInfluence;
		}
		else
			LoadMatrixFromPosRot(&paloBone->xf.posWorld, &paloBone->xf.matWorld, &matBnd);

		const glm::mat4 matInfluence = (*pmatLeft) * matBnd;
		bnd.dmat = matInfluence * bnd.dmatOrig;
	}

	globset.boneMatrices.resize(globset.cbnd);

	for (int ibnd = 0; ibnd < globset.cbnd; ++ibnd)
		globset.boneMatrices[ibnd] = globset.abnd[ibnd].dmat;

	if (globset.boneMatrixSSBO != 0 && !globset.boneMatrices.empty())
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, globset.boneMatrixSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(globset.boneMatrices.size() * sizeof(glm::mat4)), globset.boneMatrices.data());
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
}

void AdjustAloRotation(ALO* palo, glm::mat3* pmat, glm::vec3* pw)
{
	ALOX* palox = palo->palox.get();

	if (palox == nullptr)
		return;
	
	GRFALOX flags = palox->grfalox;

	if ((flags & 1) != 0 && pmat != nullptr)
		*pmat = *pmat * palox->matPreRotation;

	if ((flags & 2) != 0)
	{
		if (pmat != nullptr)
			*pmat = palox->matPostRotation * *pmat;

		if (pw != nullptr)
			*pw = palox->matPostRotation * *pw;
	}
}
void UnadjustAloRotation(ALO* palo, glm::mat3* pmat)
{
	ALOX* palox = palo->palox.get();

	if (palox == nullptr)
		return;

	uint32_t flags = palox->grfalox;

	if ((flags & 1) != 0)
		*pmat = *pmat * glm::transpose(palox->matPreRotation);

	if ((flags & 2) != 0)
		*pmat = glm::transpose(palox->matPostRotation) * *pmat;
}

void RecacheAloActList(ALO* palo)
{
	ACT* pactPos = nullptr;
	ACT* pactRot = nullptr;
	ACT* pactScale = nullptr;

	std::fill(palo->apactPose.begin(), palo->apactPose.end(), nullptr);

	for (ACT* pact = palo->dlAct.pactFirst; pact != nullptr; pact = pact->dleAlo.pactNext)
	{
		if (pactPos == nullptr && pact->ackPos != ACK_Nil)
			pactPos = pact;

		if (pactRot == nullptr && pact->ackRot != ACK_Nil)
			pactRot = pact;

		if (pactScale == nullptr && pact->ackScale != ACK_Nil)
			pactScale = pact;

		if (pact->ackPose == ACK_Nil)
			continue;

		if (pact->pvtact == reinterpret_cast<VTACT*>(&g_vtactseg))
		{
			ACTSEG* pactseg = static_cast<ACTSEG*>(pact);
			ASEG* paseg = pactseg->pasega->paseg;

			CHN* pchn = pactseg->ichn < paseg->cchn ? &paseg->achn[pactseg->ichn] : paseg->pchnStrip;

			if (pchn == nullptr)
				continue;

			const int cpose = glm::min(pchn->cpacgPose, static_cast<int>(palo->apactPose.size()));

			for (int ipose = 0; ipose < cpose; ++ipose)
			{
				if (pchn->apacgPose[ipose] != nullptr && palo->apactPose[ipose] == nullptr)
					palo->apactPose[ipose] = pact;
			}
		}
		else
		{
			const int cpose = glm::min(palo->globset.cpose, static_cast<int>(palo->apactPose.size()));

			for (int ipose = 0; ipose < cpose; ++ipose)
			{
				if (palo->apactPose[ipose] == nullptr)
					palo->apactPose[ipose] = pact;
			}
		}
	}

	if (pactPos != nullptr && pactPos->ackPos == ACK_None)
		pactPos = nullptr;

	if (pactRot != nullptr && pactRot->ackRot == ACK_None)
		pactRot = nullptr;

	if (pactScale != nullptr && pactScale->ackScale == ACK_None)
		pactScale = nullptr;

	for (ACT*& pactPose : palo->apactPose)
	{
		if (pactPose != nullptr && pactPose->ackPose == ACK_None)
			pactPose = nullptr;
	}

	palo->pactScale = pactScale;

	if (pactPos != palo->pactPos || pactRot != palo->pactRot)
	{
		ResolveAlo(palo);

		if (pactPos != palo->pactPos)
		{
			palo->pactPos = pactPos;

			if (pactPos != nullptr)
				AdaptAct(pactPos);
		}

		if (pactRot != palo->pactRot)
		{
			palo->pactRot = pactRot;

			if (pactRot != nullptr)
				AdaptAct(pactRot);
		}
	}
}

void ResortAloActList(ALO* palo)
{
	ACT* apact[16];
	int count = 0;

	// Remove every ACT from the ALO action list,
	// saving them in a temporary array.
	for (ACT* pact = palo->dlAct.pactFirst; pact != nullptr; )
	{
		ACT* next = pact->dleAlo.pactNext;
		apact[count++] = pact;

		RemoveDlEntry(&palo->dlAct, pact);
		pact = next;
	}

	// Reinsert sorted by priority.
	// Higher nPriority stays closer to the front.
	for (int i = count - 1; i >= 0; --i)
	{
		ACT* act = apact[i];
		ACT* insertBefore = palo->dlAct.pactFirst;

		while (insertBefore != nullptr && act->nPriority < insertBefore->nPriority)
			insertBefore = insertBefore->dleAlo.pactNext;

		InsertDlEntryBefore(&palo->dlAct, insertBefore, act);
	}

	palo->pvtalo->pfnRecacheAloActList(palo);
}

ASEGA* PasegaFindAlo(ALO* palo, OID oidAseg)
{
	for (ASEGA* pasega = palo->psw->dlAsega.pasegaFirst; pasega != nullptr; pasega = pasega->dleSw.pasegaNext)
	{
		if (pasega->paloRoot != palo)
			continue;

		if (oidAseg == OID_Nil)
			return pasega;

		if (FMatchesLoName((LO*)pasega->paseg, oidAseg))
			return pasega;
	}

	return nullptr;
}

ASEGA* PasegaFindAloNearest(ALO* paloLeaf)
{
	for (ALO* palo = paloLeaf; palo != nullptr; palo = palo->paloParent) {

		ACT* pact = palo->pactPos;

		if (pact != nullptr && pact->pvtact == (VTACT*)&g_vtactseg) {
			return static_cast<ACTSEG*>(pact)->pasega;
		}

		pact = palo->pactRot;

		if (pact != nullptr && pact->pvtact == (VTACT*)&g_vtactseg) {
			return static_cast<ACTSEG*>(pact)->pasega;
		}
	}

	return nullptr;
}

void RetractAloDrive(ALO* palo)
{
	while (palo->pactPos != nullptr && palo->pactPos->ackPos == ACK_Drive)
	{
		ACT* pact = palo->pactPos;
		pact->pvtact->pfnRetractAct(pact, false);
	}

	while (palo->pactRot != nullptr && palo->pactRot->ackRot == ACK_Drive)
	{
		ACT* pact = palo->pactRot;
		pact->pvtact->pfnRetractAct(pact, false);
	}
}

SMA* PsmaFindAlo(ALO* palo, OID oidSm)
{
	for (SMA* psma = palo->psw->dlSma.psmaFirst; psma != nullptr; psma = psma->dleSw.psmaNext)
	{
		if (psma->paloRoot != palo)
			continue;

		if (oidSm == OID_Nil)
			return psma;

		if (FMatchesLoName((LO*)psma->psm, oidSm))
			return psma;
	}

	return nullptr;
}

void CreateAloActadj(ALO* palo, int nPriority, ACTADJ** ppactadj)
{
	if (palo != nullptr) 
	{
		ACTADJ *pact = PactadjNew(palo->psw, palo);
		pact->nPriority = nPriority;
		InsertAloAct(palo, pact);
		*ppactadj = pact;
	}
}

void UpdateAloConstraints(ALO* palo)
{
	ALOX* palox = palo->palox.get();

	if (palox == nullptr)
		return;

	if ((palox->grfalox & 0x10) != 0 && palox->ikj.fInvalid != 0)
		SolveAloIK(palox->ikj.paloIkh);
}

int FAbsorbAloWkr(ALO* palo, WKR* pwkr)
{
	return (int)(pwkr->grfic != 0);
}

void SetAloInitialVelocity(ALO* palo, glm::vec3* pv)
{
	palo->xf.v = palo->xf.mat * *pv;
}

void SetAloInitialAngularVelocity(ALO* palo, const glm::vec3* pw)
{
	palo->xf.w = palo->xf.mat * *pw;
}

void MatchAloOtherObject(ALO* palo, ALO* paloOther)
{
	glm::vec3 pos;
	glm::vec3 v;
	glm::vec3 w;
	glm::mat3 mat;

	glm::vec3* pposFrom = &paloOther->xf.posWorld;

	ConvertAloPos(nullptr, palo->paloParent, pposFrom, &pos);
	ConvertAloMat(nullptr, palo->paloParent, &paloOther->xf.matWorld, &mat);
	ConvertAloMovement(paloOther->paloParent,palo->paloParent, pposFrom, &paloOther->xf.v, &paloOther->xf.w, nullptr, nullptr, &v, &w, nullptr, nullptr);

	palo->pvtalo->pfnTranslateAloToPos(palo, &pos);
	palo->pvtalo->pfnRotateAloToMat(palo, &mat);
	palo->pvtalo->pfnSetAloVelocityVec(palo, &v);
	palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &w);

	if (palo->palox && paloOther->palox && (palo->palox->grfalox & 0x20) && (paloOther->palox->grfalox & 0x20))
	{
		palo->palox->ikh.radTwist  = paloOther->palox->ikh.radTwist;
		palo->palox->ikh.dradTwist = paloOther->palox->ikh.dradTwist;
	}

	for (ALO* pChild = palo->dlChild.paloFirst; pChild; pChild = pChild->dleChild.paloNext)
	{
		if ((pChild->pvtlo->grfcid & 1) == 0)
			continue;

		LO* pOtherChild = PloFindSwObject(palo->psw, 2, (OID)pChild->oid, paloOther);

		if (!pOtherChild)
			continue;

		if ((pOtherChild->pvtlo->grfcid & 1) == 0)
			continue;

		if (pChild->palox && (pChild->palox->grfalox & 0x0C))
			continue;

		pChild->pvtalo->pfnMatchAloOtherObject(pChild, reinterpret_cast<ALO*>(pOtherChild));
	}
}

ASEGD* PasegdEnsureAlo(ALO* palo)
{
	if (palo->pasegd == nullptr)
	{
		palo->pasegd = std::make_shared <ASEGD>();
		palo->pasegd->oidAseg = OID_Nil;
		palo->pasegd->iak = IAK_Time;
		palo->pasegd->tLocal = 0.0f;
		palo->pasegd->svtLocal = 1.0f;
	}

	return palo->pasegd.get();
}

OID* PasegdEnsureAloOid(ALO* palo)
{
	return &PasegdEnsureAlo(palo)->oidAseg;
}

float* PasegdEnsureAlotLocal(ALO* palo)
{
	return &PasegdEnsureAlo(palo)->tLocal;
}

float* PasegdEnsureAlosvtLocal(ALO* palo)
{
	return &PasegdEnsureAlo(palo)->svtLocal;
}

IAK* PasegdEnsureAloiak(ALO* palo)
{
	return &PasegdEnsureAlo(palo)->iak;
}

SHADOW* PshadowAloEnsure(ALO* palo)
{
	if (palo->pshadow == nullptr)
	{
		palo->pshadow = std::make_shared <SHADOW>();
		InitShadow(palo->pshadow.get());
		AppendDlEntry(&palo->psw->dlShadow, palo->pshadow.get());
	}

	return palo->pshadow.get();
}

SHADOW* PshadowInferAlo(ALO* palo)
{
	return nullptr;
}

void EnsureAloActRestore(ALO* palo)
{
	if (palo->pactRestore == nullptr)
	{
		palo->pactRestore = PactNew(palo->psw, palo, &g_vtact);
		InsertAloAct(palo, palo->pactRestore.get());
	}
}

void EnsureAloActla(ALO* palo)
{
	if (palo->pactla == nullptr)
	{
		palo->pactla = PactlaNew(palo->psw, palo, &g_vtactla);
		InsertAloAct(palo, reinterpret_cast<ACT*>(palo->pactla.get()));
	}
}

void InsertAloAct(ALO* palo, ACT* pact)
{
	ACT *pactBefore = palo->dlAct.pactFirst;

	while (pactBefore != nullptr && pact->nPriority < pactBefore->nPriority)
		pactBefore = pactBefore->dleAlo.pactNext;

	InsertDlEntryBefore(&palo->dlAct, pactBefore, pact);

	palo->pvtalo->pfnRecacheAloActList(palo);
}

void SetAloAsegdOid(ALO* palo, short oid)
{
	palo->pasegd->oidAseg = (OID)oid;
}

void SetAloAsegdtLocal(ALO* palo, float tLocal)
{
	palo->pasegd->tLocal = tLocal;
}

void SetAloAsegdSvtLocal(ALO* palo, float svtLocal)
{
	palo->pasegd->svtLocal = svtLocal;
}

void SetAloAsegdiak(ALO* palo, int iak)
{
	palo->pasegd->iak = (IAK)iak;
}

void SetAloFrozen(ALO* palo, bool fFrozen)
{
	palo->fFrozen = fFrozen;
}

void SetAloEuler(ALO* palo, glm::vec3* peul)
{
	glm::vec3 eul = *peul * 0.017453294f; // degrees to radians

	glm::mat3 mat;
	LoadRotateMatrixEuler(&eul, &mat);

	palo->pvtalo->pfnAdjustAloRotation(palo, &mat, 0);
	palo->pvtalo->pfnRotateAloToMat(palo, &mat);
}

void SetAloVelocityLocal(ALO* palo, glm::vec3* pvec)
{
	// Convert local-space velocity to world-space velocity
	glm::vec3 localVelocity = *pvec;
	glm::vec3 worldVelocity = palo->xf.matWorld * localVelocity;

	palo->pvtalo->pfnSetAloVelocityVec(palo, &worldVelocity);
}

void SetAloFastShadowRadius(ALO* palo, float sRadius)
{
	palo->sFastShadowRadius = sRadius;
}

void SetAloFastShadowDepth(ALO* palo, float sDepth)
{
	palo->sFastShadowDepth = sDepth;
}

void SetAloCastShadow(ALO* palo, byte fCastShadow)
{
	if (fCastShadow == 0) 
	{
		if (palo->pshadow != nullptr) 
		{
			RemoveDlEntry(&palo->psw->dlShadow, palo->pshadow.get());
			AppendDlEntry(&g_dlShadowPending, palo->pshadow.get());
			palo->pshadow = nullptr;
		}
	}
	else
		PshadowAloEnsure(palo);
}

void SetAloShadowShader(ALO* palo, OID oidShdShadow)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowShader(pshadow, oidShdShadow);
}

void GetAloShadowShader(ALO* palo, OID* poidShdShadow)
{
	if (palo && palo->pshadow && palo->pshadow->pshd) 
		*poidShdShadow = static_cast<OID>(palo->pshadow->pshd->oid);
	else 
		*poidShdShadow = OID_Nil;
}

void GetAloShadowNearRadius(ALO* palo, float* psNearRadius)
{
	SHADOW* pshadow = PshadowInferAlo(palo);
	*psNearRadius = pshadow->sNearRadius;
}

void SetAloShadowNearRadius(ALO* palo, float sNearRadius)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowNearRadius(pshadow, sNearRadius);
}

void SetAloShadowFarRadius(ALO* palo, float sFarRadius)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowFarRadius(pshadow, sFarRadius);
}

void GetAloShadowFarRadius(ALO* palo, float* psFarRadius)
{
	SHADOW* pshadow = PshadowInferAlo(palo);
	*psFarRadius = pshadow->sFarRadius;
}

void SetAloShadowNearCast(ALO* palo, float sNearCast)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowNearCast(pshadow, sNearCast);
}

void GetAloShadowNearCast(ALO* palo, float* psNearCast)
{
	SHADOW* pshadow = PshadowInferAlo(palo);
	*psNearCast = pshadow->sNearCast;
}

void SetAloShadowFarCast(ALO* palo, float sFarCast)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowFarCast(pshadow, sFarCast);
}

void GetAloShadowFarCast(ALO* palo, float* psFarCast)
{
	SHADOW* pshadow = PshadowInferAlo(palo);
	*psFarCast = pshadow->sFarCast;
}

void SetAloShadowConeAngle(ALO* palo, float degConeAngle)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowConeAngle(pshadow, degConeAngle);
}

void GetAloShadowConeAngle(ALO* palo, float* pdegConeAngle)
{
	SHADOW* pshadow = PshadowInferAlo(palo);

	float angleRadians = std::atan2(pshadow->sNearRadius / pshadow->sNearCast, 1.0f);
	*pdegConeAngle = 2.0f * angleRadians * 57.29578f;
}

void SetAloShadowFrustrumUp(ALO* palo, glm::vec3* pvecUp)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowFrustrumUp(pshadow, pvecUp);
}

void GetAloShadowFrustrumUp(ALO* palo, glm::vec3* pvecUp)
{
	SHADOW* pshadow = PshadowInferAlo(palo);

	*pvecUp = pshadow->vecUp;
}

void SetAloDynamicShadowObject(ALO* palo, OID oidDysh)
{
	SHADOW *pshadow = PshadowAloEnsure(palo);
	pshadow->oidDysh = oidDysh;
}

void SetAloNoFreeze(ALO* palo, int fNoFreeze)
{
	palo->freezeMode = fNoFreeze != 0 ? FREEZEMODE_AlwaysBusy : FREEZEMODE_Normal;
}

void SetAloFreezeMode(ALO* palo, FREEZEMODE freezeMode)
{
	palo->freezeMode = freezeMode;
}

void GetAloFreezeMode(ALO* palo, FREEZEMODE* pfreezeMode)
{
	*pfreezeMode = static_cast<FREEZEMODE>(palo->freezeMode);
}

void SetAloRestorePosition(ALO* palo, int fRestore)
{
	ACK ack = ACK_Spring;

	if (fRestore == 0)
		ack = ACK_Nil;
	
	SetAloRestorePositionAck(palo, ack);
}

void SetAloRestorePositionAck(ALO* palo, ACK ack)
{
	EnsureAloActRestore(palo);

	palo->pactRestore->ackPos = ack;
	palo->pvtalo->pfnRecacheAloActList(palo);
}

void SetAloPositionSpring(ALO* palo, float r)
{
	CLQ clq{
	   g_clqPosSpringDefault.g0 * r,
	   g_clqPosSpringDefault.g1 * r,
	   g_clqPosSpringDefault.g2 * r,
	   g_clqPosSpringDefault.gUnused * r
	};

	SetAloPositionSpringDetail(palo, &clq);
}

void SetAloPositionSpringDetail(ALO* palo, CLQ* pclq)
{
	if (palo->pclqPosSpring.get() == &g_clqPosSpringDefault)
		palo->pclqPosSpring = std::make_shared<CLQ>();

	*palo->pclqPosSpring = *pclq;
}

void SetAloPositionDamping(ALO* palo, float r)
{
	CLQ clq{
		g_clqPosDampingDefault.g0 * r,
		g_clqPosDampingDefault.g1 * r,
		g_clqPosDampingDefault.g2 * r,
		g_clqPosDampingDefault.gUnused * r
	};

	SetAloPositionDampingDetail(palo, &clq);
}

void SetAloPositionDampingDetail(ALO* palo, CLQ* pclq)
{
	if (palo->pclqPosDamping.get() == &g_clqPosDampingDefault)
		palo->pclqPosDamping = std::make_shared <CLQ>();

	*palo->pclqPosDamping = *pclq;
}

void SetAloRestoreRotation(ALO* palo, int fRestore)
{
	ACK ack = ACK_Spring;

	if (fRestore == 0)
		ack = ACK_Nil;

	SetAloRestoreRotationAck(palo, ack);
}

void SetAloRestoreRotationAck(ALO* palo, ACK ack)
{
	EnsureAloActRestore(palo);

	palo->pactRestore->ackRot = ack;
	palo->pvtalo->pfnRecacheAloActList(palo);
}

void SetAloActPriority(ALO* palo, int nPriority)
{
	EnsureAloActRestore(palo);
	palo->pactRestore->nPriority = nPriority;
	ResortAloActList(palo);
}

void SetAloRotationMatchesVelocity(ALO* palo, float uBank, float dtPredict, ACK ackRot)
{
	if (palo->pactbank == nullptr) 
	{
		palo->pactbank = PactbankNew(palo->psw, palo, &g_vtactbank);
		InsertAloAct(palo, palo->pactbank.get());
	}

	palo->pactbank->ackRot = ackRot;
	palo->pactbank->uBank = uBank;
	palo->pactbank->dtPredict = dtPredict;

	palo->pvtalo->pfnRecacheAloActList(palo);
}

void SetAloRotationSpring(ALO* palo, float r)
{
	CLQ clq{
		g_clqRotSpringDefault.g0 * r,
		g_clqRotSpringDefault.g1 * r,
		g_clqRotSpringDefault.g2 * r,
		g_clqRotSpringDefault.gUnused * r
	};

	SetAloRotationSpringDetail(palo, &clq);
}

void SetAloRotationSpringDetail(ALO* palo, CLQ* pclq)
{
	if (palo->pclqRotSpring.get() == &g_clqRotSpringDefault)
		palo->pclqRotSpring = std::make_shared<CLQ>();

	*palo->pclqRotSpring = *pclq;
}

void SetAloRotationDamping(ALO* palo, float r)
{
	CLQ clq{
		g_clqRotDampingDefault.g0 * r,
		g_clqRotDampingDefault.g1 * r,
		g_clqRotDampingDefault.g2 * r,
		g_clqRotDampingDefault.gUnused * r
	};

	SetAloRotationDampingDetail(palo, &clq);
}

void SetAloRotationDampingDetail(ALO* palo, CLQ* pclq)
{
	if (palo->pclqRotDamping.get() == &g_clqRotDampingDefault)
		palo->pclqRotDamping = std::make_shared<CLQ>();

	*palo->pclqRotDamping = *pclq;
}

void SetAloPositionSmooth(ALO* palo, float r)
{
	SMPA smpa{};
	smpa.svFast = g_smpaPosDefault.svFast * r;
	smpa.svSlow = g_smpaPosDefault.svSlow * r;
	smpa.dtFast = g_smpaPosDefault.dtFast;
	smpa.sdvMax = g_smpaPosDefault.sdvMax * r;

	SetAloPositionSmoothDetail(palo, &smpa);
}

void SetAloPositionSmoothDetail(ALO* palo, SMPA* psmpa)
{
	if (palo->psmpaPos.get() == &g_smpaPosDefault)
		palo->psmpaPos = std::make_shared<SMPA>();

	*palo->psmpaPos = *psmpa;
}

void SetAloRotationSmooth(ALO* palo, float r)
{
	SMPA smpa{};
	smpa.svFast = g_smpaRotDefault.svFast * r;
	smpa.svSlow = g_smpaRotDefault.svSlow * r;
	smpa.dtFast = g_smpaRotDefault.dtFast;
	smpa.sdvMax = g_smpaRotDefault.sdvMax * r;

	SetAloRotationSmoothDetail(palo, &smpa);
}

void SetAloRotationSmoothDetail(ALO* palo, SMPA* psmpa)
{
	if (palo->psmpaRot.get() == &g_smpaRotDefault)
		palo->psmpaRot = std::make_shared<SMPA>();

	*palo->psmpaRot = *psmpa;
}

void SetAloPositionSmoothMaxAccel(ALO* palo, float r)
{
	SMPA smpa = *palo->psmpaPos;

	smpa.sdvMax = (r * (smpa.svFast - smpa.svSlow)) / smpa.dtFast;

	SetAloPositionSmoothDetail(palo, &smpa);
}

void SetAloRotationSmoothMaxAccel(ALO* palo, float r)
{
	SMPA smpa = *palo->psmpaRot;

	smpa.sdvMax = (r * (smpa.svFast - smpa.svSlow)) / smpa.dtFast;

	SetAloRotationSmoothDetail(palo, &smpa);
}

void SetAloDefaultAckPos(ALO* palo, ACK ack)
{
	palo->ackPos = ack;
}

void SetAloDefaultAckRot(ALO* palo, ACK ack)
{
	palo->ackRot = ack;
}

void SetAloLookAt(ALO* palo, ACK ack)
{
	EnsureAloActla(palo);

	if (ack == ACK_Smooth) 
		ack = ACK_SmoothNoLock;

	palo->pactla->ackRot = ack;
	palo->pvtalo->pfnRecacheAloActList(palo);
}

void SetAloLookAtIgnore(ALO* palo, float sIgnore)
{
	EnsureAloActla(palo);
	palo->pactla->sIgnore = sIgnore;
}

void SetAloLookAtPanFunction(ALO* palo, CLQ* pclq)
{
	EnsureAloActla(palo);

	palo->pactla->clqPanToPan = *pclq;
}

void SetAloLookAtPanLimits(ALO* palo, LM* plm)
{
	EnsureAloActla(palo);

    palo->pactla->lmPan = *plm;
}

void SetAloLookAtTiltFunction(ALO* palo, CLQ* pclq)
{
	EnsureAloActla(palo);

	palo->pactla->clqTiltToTilt = *pclq;
}

void SetAloLookAtTiltLimits(ALO* palo, LM* plm)
{
	EnsureAloActla(palo);

    palo->pactla->lmTilt = *plm;
}

void SetAloLookAtEnabledPriority(ALO* palo, int nPriority)
{
	EnsureAloActla(palo);
	palo->pactla->nPriorityEnabled = nPriority;
}

void SetAloLookAtDisabledPriority(ALO* palo, int nPriority)
{
	EnsureAloActla(palo);
	palo->pactla->nPriorityDisabled = nPriority;
}

TARGET* PtargetEnsureAlo(ALO* palo)
{
	TARGET *ptarget = (TARGET*)PloFindSwObject(palo->psw, 258, (OID)554, palo);

	if (ptarget == nullptr) 
		ptarget = (TARGET*)PloNew(CID_TARGET, palo->psw, palo, (OID)554, -1);

	return ptarget;
}

void SetAloTargetAttacks(ALO* palo, int grftak)
{
	TARGET *ptarget = PtargetEnsureAlo(palo);

	if (grftak != -1) 
		ptarget->grftak = grftak;
}

void SetAloTargetRadius(ALO* palo, float sRadiusTarget)
{
	TARGET *ptarget = PtargetEnsureAlo(palo);
	ptarget->sRadiusTarget = sRadiusTarget;
}

void SetAloTargetHitTest(ALO* palo, int fHitTest)
{
	TARGET *ptarget = PtargetEnsureAlo(palo);
	ptarget->fHitTest = fHitTest;
}

void EnsureAloThrob(ALO *palo)
{
	if (palo->pthrob == nullptr)
	{
		palo->pthrob  = std::make_shared <THROB>();
		*palo->pthrob = s_mpthrobkthrob[4];
		palo->pthrob->throbk = THROBK_Nil;
	}
}

void SetAloThrobKind(ALO* palo, THROBK throbk)
{
	EnsureAloThrob(palo);
	palo->pthrob->throbk = throbk;

	if (throbk != THROBK_Nil)
		*palo->pthrob = s_mpthrobkthrob[throbk];
}

void SetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor)
{
	EnsureAloThrob(palo);
	palo->pthrob->hsvIn = *phsvInColor;
}

void SetAloThrobOutColor(ALO *palo, glm::vec3 *phsvOutColor)
{
	EnsureAloThrob(palo);
	palo->pthrob->hsvOut = *phsvOutColor;
}

void SetAloThrobDtInOut(ALO* palo, float dtInOut)
{
	EnsureAloThrob(palo);
	palo->pthrob->dtInOut = dtInOut;
}

void NewSfx(std::shared_ptr <SFX> &psfx)
{
	psfx = std::make_shared<SFX>();

	psfx->sfxid = SFXID_Nil;
	psfx->sStart = 3000.0f;
	psfx->sFull = 300.0f;
	psfx->uVol = 1.0f;
	psfx->uPitch = 0.0f;
	psfx->pamb = nullptr;
	psfx->lmRepeat.gMin = -1.0f;
	psfx->uDoppler = 0.0f;
}

void EnsureAloSfx(ALO* palo)
{
	if (palo->psfx == nullptr) 
		NewSfx(palo->psfx);
}

void SetAloSfxid(ALO* palo, SFXID sfxid)
{
	EnsureAloSfx(palo);
	palo->psfx->sfxid = sfxid;
}

void SetAloSStart(ALO* palo, float sStart)
{
	EnsureAloSfx(palo);
	palo->psfx->sStart = sStart;
}

void SetAloSFull(ALO* palo, float sFull)
{
	EnsureAloSfx(palo);
	palo->psfx->sFull = sFull;
}

void SetAloUVolumeSpl(ALO* palo, float uVol)
{
	if (palo->psfx == nullptr)
		return;

	AMB* pamb = palo->psfx->pamb;

	if (pamb == nullptr)
		return;

	SetPambVol(pamb, uVol);
}

void SetAloUVolume(ALO* palo, float uVol)
{
	EnsureAloSfx(palo);
	palo->psfx->uVol = uVol;
}

void SetAloUPitchSpl(ALO* palo, float uPitch)
{
	if (palo->psfx == nullptr)
		return;

	AMB* pamb = palo->psfx->pamb;

	if (pamb == nullptr)
		return;

	SetPambFrq(pamb, uPitch);
}

void SetAloUPitch(ALO* palo, float uPitch)
{
	EnsureAloSfx(palo);
	palo->psfx->uPitch = uPitch;
}

void SetAloSndRepeat(ALO* palo, LM plm)
{
	EnsureAloSfx(palo);

	palo->psfx->lmRepeat = plm;
}

void SetAloUDoppler(ALO* palo, float uDoppler)
{
	EnsureAloSfx(palo);
	palo->psfx->uDoppler = uDoppler;
}

void SetAloInteractCane(ALO* palo, int grfic)
{
	uint8_t value = (uint8_t)grfic;

	palo->ficg.grficSweep = value;
	palo->ficg.grficSmash = value;
	palo->ficg.grficRush  = value;
}

void SetAloInteractCaneSweep(ALO* palo, int grfic)
{
	palo->ficg.grficSweep = (u8)grfic;
}

void SetAloInteractCaneRush(ALO* palo, int grfic)
{
	palo->ficg.grficRush = (u8)grfic;
}

void SetAloInteractCaneSmash(ALO* palo, int grfic)
{
	(palo->ficg).grficSmash = (u8)grfic;
}

void SetAloInteractBomb(ALO* palo, int grfic)
{
	(palo->ficg).grficBomb = (u8)grfic;
}

void SetAloInteractShock(ALO* palo, int grfic)
{
	(palo->ficg).grficShock = (u8)grfic;
}

void SetAloPoseCombo(ALO* palo, OID oidCombo)
{
	bool recache = false;

	if (oidCombo == OID_Nil)
	{
		if (palo->pactrefCombo != nullptr)
		{
			ACK oldAck = palo->pactrefCombo->ackPose;
			palo->pactrefCombo->ackPose = ACK_Nil;

			recache = oldAck != ACK_Nil;
		}
	}
	else
	{
		POSEC* pposec = nullptr;

		for (int i = 0; i < palo->cposec; i++)
		{
			if (palo->aposec[i].oid == oidCombo)
			{
				pposec = &palo->aposec[i];
				break;
			}
		}

		if (pposec != nullptr)
		{
			if (palo->pactrefCombo == nullptr)
			{
				std::shared_ptr <ACTREF> pactref = PactrefNew(palo->psw, palo);

				palo->pactrefCombo = pactref.get();
				InsertAloAct(palo, static_cast<ACT*>(pactref.get()));
			}

			ACK oldAck = palo->pactrefCombo->ackPose;
			palo->pactrefCombo->ackPose = ACK_Smooth;

			recache = oldAck != ACK_Smooth;

			palo->pactrefCombo->pcgPoses = &palo->globset.cpose;
			palo->pactrefCombo->agPoses = pposec->agPoses.data();
		}
	}

	if (recache)
		palo->pvtalo->pfnRecacheAloActList(palo);
}

void SetAloForceCameraFade(ALO* palo, int fFade)
{
	int fNewFade = (fFade != 0);
	int fOldFade = palo->fForceCameraFade;

	if (fNewFade == fOldFade)
		return;

	palo->fForceCameraFade = fNewFade;

	if (FIsLoInWorld(palo)) {
		if (fNewFade) 
			AddCmFadeObject(g_pcm, palo);
		else
			RemoveCmFadeObject(g_pcm, palo);
	}
}

void SetAloCelRgba(ALO* palo, RGBA prgba)
{
	palo->globset.rgbaCel.r = prgba.bRed   / 255.0;
	palo->globset.rgbaCel.g = prgba.bGreen / 255.0;
	palo->globset.rgbaCel.b = prgba.bBlue  / 255.0;
	palo->globset.rgbaCel.a = prgba.bAlpha / 255.0;

	palo->globset.grfglobset = palo->globset.grfglobset | 2;
}

void SetAloOverrideCel(ALO *palo, glm::vec4 *rgba)
{
	palo->globset.grfglobset |= 0x2;
	palo->globset.rgbaCel = *rgba;

	ALO *child = palo->dlChild.paloFirst;

	while (child != nullptr)
	{
		if ((child->pvtlo->grfcid & 0x1) != 0)
			SetAloOverrideCel(child, rgba);

		child = child->dleChild.paloNext;
	}
}

void UpdateAloThrob(ALO* palo, float dt)
{
	THROB* throb = palo->pthrob.get();

	if (throb->dtInOut <= 0.0f)
		return;

	float t = std::fmod(g_clock.t, throb->dtInOut);
	float wave = std::sin((t * glm::two_pi<float>()) / throb->dtInOut);
	float blend = wave * 0.5f + 0.5f; // 0..1

	glm::vec3 hsv = throb->hsvIn * blend + throb->hsvOut * (1.0f - blend);

	glm::vec3 rgb{};
	ConvertUserHsvToUserRgb(hsv, rgb);

	glm::vec4 overrideCel(rgb, 0.5f); // 0x80 / 255 ~= 0.502

	SetAloOverrideCel(palo, &overrideCel);
}

void SetAloBlotContext(ALO* palo, BLOT* pblot)
{
	for (int i = 0; i < palo->globset.cglob; i++)
	{
		GLOB* glob = &palo->globset.aglob[i];

		glob->pblot = pblot;
		glob->rp = RP_BlotContext;
	}

	for (ALO* child = palo->dlChild.paloFirst; child != nullptr; child = child->dleChild.paloNext)
	{
		if (child->pvtlo->grfcid & 1)
			SetAloBlotContext(child, pblot);
	}
}

void GetAloFrozen(ALO* palo, int* pfFrozen)
{
	*pfFrozen = palo->fFrozen;
}

void SetAloHidden(ALO* palo, int fHidden)
{
	palo->fHidden = fHidden != 0;
}

void GetAloHidden(ALO* palo, int* pfHidden)
{
	*pfHidden = palo->fHidden ? 1 : 0;
}

void* GetAloXfPos(ALO* palo)
{
	return &palo->xf.pos;
}

void SetAloXfPos(ALO* palo, glm::vec3 value)
{
	palo->xf.pos = value;
}

void* GetAloXfPosOrig(ALO* palo)
{
	return &palo->posOrig;
}

void SetAloXfPosOrig(ALO* palo, glm::vec3 value)
{
	palo->posOrig = value;
}

void* GetAloXfPosWorld(ALO* palo)
{
	return &palo->xf.posWorld;
}

void SetAloXfPosWorld(ALO* palo, glm::vec3 value)
{
	palo->xf.posWorld = value;
}

void* GetAloXfMat(ALO* palo)
{
	return &palo->xf.mat;
}

void SetAloXfMat(ALO* palo, glm::mat3 value)
{
	palo->xf.mat = value;
}

void* GetAloMatOrig(ALO* palo)
{
	return &palo->matOrig;
}

void SetAloMatOrig(ALO* palo, glm::mat3 value)
{
	palo->matOrig = value;
}

void* GetAloXfMatWorld(ALO* palo)
{
	return &palo->xf.matWorld;
}

void SetAloXfMatWorld(ALO* palo, glm::mat3 value)
{
	palo->xf.matWorld = value;
}

void GetAloEuler(ALO* palo, glm::vec3* peul)
{
	glm::mat3 mat = palo->xf.mat;
	palo->pvtalo->pfnUnadjustAloRotation(palo, &mat);

	glm::vec3 eulRadians{};
	DecomposeRotateMatrixEuler(glm::mat4(mat), eulRadians);
	*peul = eulRadians * 57.295776f;
}

void GetAloVelocityLocal(ALO* palo, glm::vec3* pvec)
{
	// Transform world velocity into local space by applying the inverse of the rotation matrix.
	// If the matrix is orthonormal, the inverse is just the transpose.
	glm::mat3 rotation = palo->xf.mat;
	glm::vec3 worldVelocity = palo->xf.v;

	// Convert world velocity to local space
	*pvec = glm::transpose(rotation) * worldVelocity;
}

void* GetAloXfw(ALO* palo)
{
	return &palo->xf.w;
}

void SetAloXfw(ALO* palo, glm::vec3 value)
{
	palo->xf.w = value;
}

void* GetAloXfdv(ALO* palo)
{
	return &palo->xf.dv;
}

void SetAloXfdv(ALO* palo, glm::vec3 value)
{
	palo->xf.dv = value;
}

void* GetAloXfdw(ALO* palo)
{
	return &palo->xf.dw;
}

void* GetAloRoot(ALO* palo)
{
	return palo->paloRoot;
}

void GetAloFastShadowRadius(ALO* palo, float* psRadius)
{
	*psRadius = palo->sFastShadowRadius * 100.0;
}

void GetAloFastShadowDepth(ALO* palo, float* psDepth)
{
	*psDepth = palo->sFastShadowDepth * 100.0;
}

void GetAloCastShadow(ALO* palo, int* pfCastShadow)
{
	*pfCastShadow = palo->pshadow != nullptr;
}

void GetAloLookAtIgnore(ALO* palo, float* psIgnore)
{
	float sIgnore = 0.0;
	if (palo->pactla != nullptr)
		sIgnore = palo->pactla->sIgnore;
	
	*psIgnore = sIgnore;
}

void GetAloLookAtPanFunction(ALO* palo, CLQ* pclq)
{
	const CLQ* pclqSource;

	if (palo->pactla != nullptr)
		pclqSource = &palo->pactla->clqPanToPan;
	else
		pclqSource = &g_clqZero;

	*pclq = *pclqSource;
}

void GetAloLookAtPanLimits(ALO* palo, LM* plm)
{
	const LM *plmSource{};

	if (palo->pactla != nullptr)
		plmSource = &palo->pactla->lmPan;
	else
		plmSource = &g_lmZeroOne;

	*plm = *plmSource;
}

void GetAloLookAtTiltFunction(ALO* palo, CLQ* pclq)
{
	const CLQ* pclqSource;

	if (palo->pactla != nullptr)
		pclqSource = &palo->pactla->clqTiltToTilt;
	else
		pclqSource = &g_clqZero;

	*pclq = *pclqSource;
}

void GetAloLookAtTiltLimits(ALO* palo, LM* plm)
{
	const LM* plmSource;

	if (palo->pactla != nullptr)
		plmSource = &palo->pactla->lmTilt;
	else
		plmSource = &g_lmZeroOne;

	*plm = *plmSource;
}

void GetAloLookAtEnabledPriority(ALO* palo, int* pnPriority)
{
	if (palo->pactla != nullptr)
		*pnPriority = palo->pactla->nPriorityEnabled;
	else
		*pnPriority = 0;
}

void GetAloLookAtDisabledPriority(ALO* palo, int* pnPriority)
{
	int nPriorityDisabled = 0;

	if (palo->pactla != nullptr) 
		nPriorityDisabled = palo->pactla->nPriorityDisabled;

	*pnPriority = nPriorityDisabled;
}

int FGetAloChildrenList(ALO* palo, void* pvstate)
{
	DLI dliChild;

	dliChild.m_pdl = &palo->dlChild;
	dliChild.m_ibDle = palo->dlChild.ibDle;
	dliChild.m_pdliNext = s_pdliFirst;

	LO* plo = static_cast<LO*>(palo->dlChild.ploFirst);

	while (plo != nullptr)
	{
		s_pdliFirst = &dliChild;

		if (!FAppendSpliceListElement(pvstate, &plo))
		{
			s_pdliFirst = dliChild.m_pdliNext;
			return 0;
		}

		plo = *reinterpret_cast<LO**>(reinterpret_cast<char*>(plo) + dliChild.m_ibDle);
	}

	s_pdliFirst = dliChild.m_pdliNext;
	return 1;
}

void GetAloThrobKind(ALO* palo, THROBK* pthrobk)
{
	THROBK throbk = THROBK_Nil;

	if (palo->pthrob != nullptr) 
		throbk = palo->pthrob->throbk;
	
	*pthrobk = throbk;
}

void GetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor)
{
	if (palo->pthrob)
		*phsvInColor = palo->pthrob->hsvIn;
	else
		*phsvInColor = glm::vec3(0.0f);
}

void GetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor)
{
	if (palo->pthrob)
		*phsvOutColor = palo->pthrob->hsvIn;
	else
		*phsvOutColor = glm::vec3(0.0f);
}

void GetAloThrobDtInOut(ALO* palo, float* pdtInOut)
{
	*pdtInOut = palo->pthrob ? palo->pthrob->dtInOut : 0.0f;
}

void SetAloSfxidSpl(ALO* palo, SFXID sfxid)
{
	EnsureAloSfx(palo);

	auto& psfx = palo->psfx;

	StopSound(psfx->pamb, 0);
	psfx->sfxid = sfxid;
	StartSound(psfx->sfxid, &psfx->pamb, palo, nullptr, psfx->sStart, psfx->sFull, psfx->uVol, psfx->uPitch, psfx->uDoppler, &psfx->lmRepeat, nullptr);
}

void GetAloSfxid(ALO* palo, SFXID* psfxid)
{
	*psfxid = SFXID_Nil;

	if (palo->psfx)
		*psfxid = palo->psfx->sfxid;
}

void GetAloSStart(ALO* palo, float* psStart)
{
	if (palo->psfx)
		*psStart = palo->psfx->sStart;
	else
		*psStart = 3000.0f;
}

void GetAloSFull(ALO* palo, float* psFull)
{
	if (palo->psfx)
		*psFull = palo->psfx->sFull;
	else
		*psFull = 300.0f;
}

void GetAloUVolume(ALO* palo, float* puVol)
{
	if (palo->psfx)
		*puVol = palo->psfx->uVol;
	else
		*puVol = 1.0f;
}

void GetAloUPitch(ALO* palo, float* puPitch)
{
	*puPitch = 0.0f;

	if (palo && palo->psfx)
		*puPitch = palo->psfx->uPitch;
}

void GetAloSndRepeat(ALO* palo, LM* plmRepeat)
{
	if (!plmRepeat)
		return;

	if (palo && palo->psfx)
	{
		*plmRepeat = palo->psfx->lmRepeat;
		return;
	}

	plmRepeat->gMax = -1.0f;
	plmRepeat->gMin = -1.0f;
}

void GetAloUDoppler(ALO* palo, float* puDoppler)
{
	if (palo->psfx)
		*puDoppler = palo->psfx->uDoppler;
	else
		*puDoppler = 0.0f;
}

void GetAloInteractCane(ALO* palo, int* pgrfic)
{
	*pgrfic = (uint32_t)(palo->ficg).grficSweep;
}

void GetAloInteractCaneSweep(ALO* palo, int* pgrfic)
{
	*pgrfic = (uint32_t)(palo->ficg).grficSweep;
}

void GetAloInteractCaneRush(ALO* palo, int* pgrfic)
{
	*pgrfic = (uint32_t)(palo->ficg).grficRush;
}

void GetAloInteractCaneSmash(ALO* palo, int* pgrfic)
{
	*pgrfic = (uint32_t)(palo->ficg).grficSmash;
}

void GetAloInteractBomb(ALO* palo, int* pgrfic)
{
	*pgrfic = (uint32_t)(palo->ficg).grficBomb;
}

void GetAloInteractShock(ALO* palo, int* pgrfic)
{
	*pgrfic = (uint32_t)(palo->ficg).grficShock;
}

void* GetAlofRealClock(ALO* palo)
{
	return &palo->fRealClock;
}

void SetAloRealClock(ALO* palo, int fRealClock)
{
	/* Only update if the value is actually changing */
	if ((fRealClock != 0) == (palo->fRealClock == 0)) 
	{
		int fInWorld = FIsLoInWorld(palo);

		if (!fInWorld)
			palo->fRealClock = fRealClock;
		else if (palo->paloParent == nullptr) 
		{
			palo->pvtlo->pfnRemoveLo(palo);
			palo->fRealClock = fRealClock;
			palo->pvtlo->pfnAddLo(palo);
		}
		else
			palo->fRealClock = fRealClock;
	}
}

void SetAloScrollingMasterSpeeds(ALO* palo, float svu, float svv)
{
	for (int isaa = 0; isaa < palo->globset.cpsaa; isaa++) {
		SAA* psaa = palo->globset.apsaa[isaa];

		if (psaa->saak == SAAK_Scroller) {
			SetScrollerMasterSpeeds((SCROLLER*)psaa, svu, svv);
		}
	}
}

void SetAloEyesClosed(ALO* palo, float uClosed)
{
	for (int isaa = 0; isaa < palo->globset.cpsaa; isaa++) {
		SAA* psaa = palo->globset.apsaa[isaa];

		if (psaa->saak == SAAK_Eyes) {
			SetEyesClosed(reinterpret_cast<EYES*>(psaa), uClosed);
		}
	}

	for (ALO* paloChild = palo->dlChild.paloFirst; paloChild != nullptr; paloChild = paloChild->dleChild.paloNext) {
		if ((paloChild->pvtlo->grfcid & 1) != 0) {
			SetAloEyesClosed(paloChild, uClosed);
		}
	}
}

void GetAloActlaTarget(ALO* palo, ALO** ppaloTarget)
{
	*ppaloTarget = PaloGetActlaTarget(palo->pactla.get());
}

void SetAloActlaTarget(ALO* palo, ALO* paloTarget)
{
	SetActlaTarget(palo->pactla.get(), paloTarget, &g_vecZero);
}

void ResumeAloActla(ALO* palo)
{
	palo->pactla->fPaused = 0;
}

void PauseAloActla(ALO* palo)
{
	palo->pactla->fPaused = 1;
}

void StartAloSound(ALO* palo, SFXID sfxid, float sStart, float sFull, float uVol, LM* plmRepeat)
{
	EnsureAloSfx(palo);

	SFX* psfx = palo->psfx.get();

	psfx->sfxid = sfxid;
	psfx->sStart = sStart;
	psfx->sFull = sFull;
	psfx->uVol = uVol;
	psfx->lmRepeat = *plmRepeat;

	StopSound(psfx->pamb, 0);

	StartSound(psfx->sfxid, &psfx->pamb, palo, nullptr, psfx->sStart, psfx->sFull, psfx->uVol, psfx->uPitch, psfx->uDoppler, &psfx->lmRepeat, nullptr);
}

void StopAloSound(ALO* palo)
{
	if (palo->psfx != nullptr) {
		StopSound(palo->psfx->pamb, 0);
	}
}

void EnsureAloFader(ALO* palo)
{
	if (palo->pfader == nullptr) {
		palo->pfader = PfaderNew(palo);
		palo->pfader->uAlpha = FIsLoInWorld(palo) ? 1.0f : 0.0f;
	}
}

void FadeAloIn(ALO* palo, float dtFade)
{
	if (!FIsLoInWorld(palo) || palo->pfader != nullptr) 
	{
		EnsureAloFader(palo);
		palo->pvtlo->pfnAddLo(palo);

		if (dtFade == 0.0f) {
			palo->pfader->duAlpha = glm::max(1.0f - palo->pfader->uAlpha + 0.0001f, 0.0f);
		}
		else {
			palo->pfader->duAlpha = 1.0f / dtFade;
		}
	}
}

void FadeAloOut(ALO* palo, float dtFade)
{
	if (FIsLoInWorld(palo)) {
		EnsureAloFader(palo);

		if (dtFade == 0.0f) {
			palo->pfader->duAlpha = glm::min(-palo->pfader->uAlpha - 0.0001f, 0.0f);
		}
		else {
			palo->pfader->duAlpha = -1.0f / dtFade;
		}
	}
}

void CalculateAloDrive(ALO* palo, CLQ* pclqHoming, LM* plmHoming, float dt, float radPanCur, float* pradPanTarget, float* pradTiltTarget, float* psvTarget)
{
	if (pclqHoming == nullptr) {
		pclqHoming = &s_clqHomingSpeed;
	}

	if (plmHoming == nullptr) {
		plmHoming = &s_lmHomingSpeed;
	}

	glm::vec3 posGoal;
	glm::vec3 vGoal;
	glm::mat3 matGoal;
	glm::vec3 wGoal;

	ACT* pactPos = palo->pactPos;
	ACT* pactRot = palo->pactRot;

	if (pactPos == nullptr) {
		posGoal = palo->xf.pos;
		vGoal = glm::vec3(0.0f);
	}
	else {
		pactPos->pvtact->pfnGetActPositionGoal(pactPos, 0.0f, &posGoal, &vGoal);
	}

	if (pactRot == nullptr) {
		matGoal = palo->xf.mat;
		wGoal = glm::vec3(0.0f);
	}
	else {
		pactRot->pvtact->pfnGetActRotationGoal(pactRot, 0.0f, &matGoal, &wGoal);
	}

	glm::vec3 dpos = posGoal - palo->xf.pos;

	if (pradTiltTarget == nullptr) {
		dpos.z = 0.0f;
	}

	float sToGoal = glm::length(dpos);
	float svGoal = glm::length(vGoal);

	if (palo->dms == 0) {
		if (sToGoal < 25.0f && svGoal < 10.0f) {
			palo->dms = 1;
		}
	}
	else if (palo->dms == 1) {
		if (sToGoal > 50.0f || svGoal > 10.0f) {
			palo->dms = 0;
		}
	}

	if (palo->dms == 0) {
		glm::vec3 vHoming(0.0f);

		if (sToGoal > 0.0001f) {
			float svHoming = pclqHoming->g0 + sToGoal * (pclqHoming->g1 + sToGoal * pclqHoming->g2);
			svHoming = glm::clamp(svHoming, plmHoming->gMin, plmHoming->gMax);
			vHoming = dpos * (svHoming / sToGoal);
		}

		glm::vec3 v = vHoming + vGoal;
		DecomposeSphere(&v, nullptr, pradPanTarget, pradTiltTarget, psvTarget);
	}
	else if (palo->dms == 1) {
		if (pactRot == nullptr) {
			*pradPanTarget = radPanCur;

			if (pradTiltTarget != nullptr) {
				*pradTiltTarget = 0.0f;
			}

			*psvTarget = 0.0f;
		}
		else {
			glm::vec3 vRot = matGoal[0];
			DecomposeSphere(&vRot, nullptr, pradPanTarget, pradTiltTarget, nullptr);
			*psvTarget = 0.0f;
		}
	}
}

void CalculateAloMovement(ALO* paloLeaf, ALO* paloBasis, glm::vec3& pos, glm::vec3* pv, glm::vec3* pw, glm::vec3* pdv, glm::vec3* pdw)
{
	// A null leaf represents a stationary world-space basis.  Several loading
	// and proxy paths intentionally call this routine that way.
	if (paloLeaf == nullptr)
	{
		if (pv != nullptr)
			*pv = glm::vec3(0.0f);
		if (pw != nullptr)
			*pw = glm::vec3(0.0f);
		if (pdv != nullptr)
			*pdv = glm::vec3(0.0f);
		if (pdw != nullptr)
			*pdw = glm::vec3(0.0f);
		return;
	}

	glm::vec3 angularVelocitySum(0.0f);
	glm::vec3 angularAccelSum(0.0f);

	if (pv) {
		*pv = glm::vec3(0.0f);
	}

	if (pdv) {
		*pdv = glm::vec3(0.0f);
	}

	// Match the original fixed scratch array.  Nodes carrying the special 0xC
	// ALOX flags are omitted from the accumulated movement, but traversal still
	// continues through the ordinary parent link.
	ALO* chain[16]{};
	int chainCount = 0;

	for (ALO* node = paloLeaf; node != paloBasis && node != nullptr; node = node->paloParent)
	{
		const std::shared_ptr<ALOX>& palox = node->palox;
		if ((palox == nullptr || (palox->grfalox & 0xCu) == 0) && chainCount < 16)
			chain[chainCount++] = node;
	}

	for (int i = chainCount - 1; i >= 0; --i) {
		ALO* node = chain[i];
		ALO* paloFrom = node->paloParent;

		if (pv || pw || pdv) {
			glm::vec3 w(0.0f);
			ConvertAloVec(paloFrom, paloBasis, &node->xf.w, &w);
			angularVelocitySum += w;
		}

		if (pdw || pdv) {
			glm::vec3 dw(0.0f);
			ConvertAloVec(paloFrom, paloBasis, &node->xf.dw, &dw);
			angularAccelSum += dw;
		}

		if (pv || pdv) {
			glm::vec3 referencePoint = pos;

			if (i > 0) {
				referencePoint = chain[i - 1]->xf.posWorld;
			}

			glm::vec3 offset = referencePoint - node->xf.posWorld;
			ConvertAloVec(nullptr, paloBasis, &offset, &offset);

			if (pv) {
				*pv += glm::cross(angularVelocitySum, offset);

				glm::vec3 v(0.0f);
				ConvertAloVec(paloFrom, paloBasis, &node->xf.v, &v);
				*pv += v;
			}

			if (pdv) {
				*pdv += glm::cross(angularVelocitySum, glm::cross(angularVelocitySum, offset));
				*pdv += glm::cross(angularAccelSum, offset);

				glm::vec3 dv(0.0f);
				ConvertAloVec(paloFrom, paloBasis, &node->xf.dv, &dv);
				*pdv += dv;
			}
		}

	}

	if (pw) {
		*pw = angularVelocitySum;
	}

	if (pdw) {
		*pdw = angularAccelSum;
	}
}

void CalculateAloTransformAdjust(ALO* paloLeaf, ALO* paloBasis, glm::vec3* ppos, glm::mat3* pmat, glm::vec3* pv, glm::vec3* pw)
{
	glm::vec3 posBasis = glm::vec3(0.0f);
	glm::mat3 matBasis = glm::mat3(1.0f);
	glm::vec3 vBasis = glm::vec3(0.0f);
	glm::vec3 wBasis = glm::vec3(0.0f);

	glm::vec3* pposBasis = (ppos != nullptr || pv != nullptr) ? &posBasis : nullptr;
	glm::vec3* pvBasis = (pv != nullptr) ? &vBasis : nullptr;
	glm::vec3* pwBasis = (pw != nullptr || pv != nullptr) ? &wBasis : nullptr;

	CalculateAloTransform(paloLeaf, paloBasis, 1, pposBasis, &matBasis, pvBasis, pwBasis);

	glm::vec3 r = glm::vec3(0.0f);

	if (ppos != nullptr) {
		r = matBasis * *ppos;
		*ppos = posBasis + r;
	}

	if (pv != nullptr) {
		glm::vec3 v = matBasis * *pv;
		*pv = vBasis + v + glm::cross(wBasis, r);
	}

	if (pw != nullptr) {
		glm::vec3 w = matBasis * *pw;
		*pw = wBasis + w;
	}

	if (pmat != nullptr) {
		*pmat = matBasis * *pmat;
	}
}

void CalculateAloTransform(ALO* paloLeaf, ALO* paloBasis, int cpaloPredict, glm::vec3* apos, glm::mat3* amat, glm::vec3* av, glm::vec3* aw)
{
	ALO* apalo[16];
	int cpalo = 0;

	for (ALO* palo = paloLeaf; palo != paloBasis; palo = palo->paloParent) {
		apalo[cpalo++] = palo;
	}

	glm::vec3 posSum(0.0f);
	glm::mat3 matSum(1.0f);
	glm::vec3 vSum(0.0f);
	glm::vec3 wSum(0.0f);

	for (int i = cpalo; i < cpaloPredict; ++i) {
		if (apos != nullptr) {
			apos[i] = glm::vec3(0.0f);
		}

		if (amat != nullptr) {
			amat[i] = glm::mat3(1.0f);
		}

		if (av != nullptr) {
			av[i] = glm::vec3(0.0f);
		}

		if (aw != nullptr) {
			aw[i] = glm::vec3(0.0f);
		}
	}

	for (int i = cpalo - 1; i >= 0; --i) {
		ALO* palo = apalo[i];

		glm::vec3 r(0.0f);

		if (apos != nullptr || av != nullptr) {
			r = matSum * palo->xf.pos;
			posSum += r;

			if (av != nullptr) {
				vSum += matSum * palo->xf.v;
				vSum += glm::cross(wSum, r);
			}
		}

		if (aw != nullptr || av != nullptr) {
			wSum += matSum * palo->xf.w;
		}

		matSum = matSum * palo->xf.mat;

		if (i < cpaloPredict) {
			if (apos != nullptr) {
				apos[i] = posSum;
			}

			if (amat != nullptr) {
				amat[i] = matSum;
			}

			if (av != nullptr) {
				av[i] = vSum;
			}

			if (aw != nullptr) {
				aw[i] = wSum;
			}
		}
	}
}

ACTSEG *PactsegNewAlo(ALO* palo)
{
	return PactsegNew(palo->psw, palo);
}

void AddAloHierarchy(ALO* palo)
{
	DLI dliChild{};

	dliChild.m_pdl = &palo->dlChild;
	dliChild.m_ibDle = palo->dlChild.ibDle;
	dliChild.m_pdliNext = s_pdliFirst;
	dliChild.m_ppv = reinterpret_cast<void**>(dliChild.m_pdl);

	s_pdliFirst = &dliChild;

	/*
	 * The original calls OnLoAdd directly only for this object.
	 */
	palo->pvtlo->pfnOnLoAdd(palo);

	while (true)
	{
		LO* child = static_cast<LO*>(*dliChild.m_ppv);

		if (child == nullptr)
			break;

		dliChild.m_ppv = reinterpret_cast<void**>(
			reinterpret_cast<char*>(child) + dliChild.m_ibDle);

		/*
		 * Call AddLoHierarchy, not OnLoAdd. This recursively processes
		 * the child's descendants.
		 */
		child->pvtlo->pfnAddLoHierarchy(child);
	}

	palo->pvtlo->pfnSendLoMessage(palo, MSGID_added, palo);

	s_pdliFirst = dliChild.m_pdliNext;
}

void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis)
{
	palo->xf.mat = pbis->ReadMatrix();
	palo->xf.pos = pbis->ReadVector();

	palo->zons = pbis->U8Read();
	palo->viss = pbis->U8Read();
	palo->mrds = pbis->U8Read();

	palo->grfzon = pbis->U32Read();
	palo->sMRD = pbis->F32Read();
	palo->sCelBorderMRD = pbis->F32Read();
	palo->sRadiusRenderSelf = pbis->F32Read();
	palo->sRadiusRenderAll = pbis->F32Read();

	if (palo->sMRD == FLT_MAX)
		palo->sMRD = 1.0e+10f;

	if (palo->sCelBorderMRD == FLT_MAX)
		palo->sCelBorderMRD = (palo->sMRD > 2000.0f) ? 2000.0f : palo->sMRD;
	
	LoadOptionsFromBrx(palo, pbis);
	LoadGlobsetFromBrx(&palo->globset, palo, pbis);
	LoadAloAloxFromBrx(palo, pbis);

	if (palo->pvtalo && palo->pvtalo->pfnUpdateAloXfWorld)
		palo->pvtalo->pfnUpdateAloXfWorld(palo);

	palo->cposec = pbis->U8Read();
	palo->aposec.resize(palo->cposec);

	for (int i = 0; i < palo->cposec; i++)
	{
		palo->aposec[i].oid = (OID)pbis->S16Read();
		palo->aposec[i].agPoses.resize(palo->globset.cpose);

		for (int a = 0; a < palo->globset.cpose; a++)
			palo->aposec[i].agPoses[a] = pbis->F32Read();
	}

	// Loads ALO children objects
	LoadSwObjectsFromBrx(palo->psw, palo, pbis);
}

void LoadAloAloxFromBrx(ALO* palo, CBinaryInputStream* pbis)
{
	const GRFALOX grfalox = static_cast<GRFALOX>(pbis->U32Read());

	if (grfalox == 0) {
		return;
	}

	palo->palox = std::make_shared<ALOX>();
	ALOX* alox = palo->palox.get();

	alox->matPreRotation = glm::mat3(1.0f);
	alox->matPostRotation = glm::mat3(1.0f);
	alox->grfalox = grfalox;

	if ((grfalox & 0x01) != 0) {
		alox->matPreRotation = pbis->ReadMatrix();
	}

	if ((grfalox & 0x02) != 0) {
		alox->matPostRotation = pbis->ReadMatrix();
	}

	if ((grfalox & 0x0c) != 0) {
		OID oidSchRot = static_cast<OID>(pbis->S16Read());

		if (oidSchRot != OID_Nil) {
			alox->scj.paloSchRot = static_cast<ALO*>(PloFindSwObject(palo->psw, 3, oidSchRot, palo));
		}
	}

	if ((grfalox & 0x10) != 0) {
		alox->ikj.oidIkh = static_cast<OID>(pbis->S16Read());
	}

	if ((grfalox & 0x20) != 0) {
		alox->ikh.oidElbow = static_cast<OID>(pbis->S16Read());
		alox->ikh.posIkh = pbis->ReadVector();
		alox->ikh.posWrist = pbis->ReadVector();

		float radTwist = RadNormalize(pbis->F32Read());
		alox->ikh.grfik = 0;
		alox->ikh.radTwist = radTwist;
		alox->ikh.radTwistOrig = radTwist;
	}

	if ((grfalox & 0x40) != 0) {
		alox->joint.oidParent = static_cast<OID>(pbis->S16Read());
		alox->joint.oidChild  = static_cast<OID>(pbis->S16Read());
	}

	if ((grfalox & 0x80) != 0) {
		alox->joint.fSsc = pbis->U8Read();
	}
}

void SetAloIkhTarget(ALO* palo, ALO* ploTarget)
{
	//SetActlaTarget(palo->pikh, ploTarget, &g_vecZero);
}

void BindAloAlox(ALO* palo)
{
	if (palo == nullptr || palo->palox == nullptr)
		return;
	
	ALOX* palox = palo->palox.get();
	const GRFALOX grfalox = palox->grfalox;

	palox->grfalox = static_cast<GRFALOX>(grfalox | 0x8000);

	if ((grfalox & 0x10) != 0)
	{
		const OID oidIkh = palox->ikj.oidIkh;

		palox->ikj.paloIkh = static_cast<ALO*>(PloFindSwNearest(palo->psw, oidIkh, palo));
		palox->ikj.fInvalid = 1;
	}

	if ((grfalox & 0x20) != 0)
	{
		const OID oidElbow = palox->ikh.oidElbow;
		palox->ikh.paloElbow = static_cast<ALO*>(PloFindSwNearest(palo->psw, oidElbow, palo));

		if (palox->ikh.paloElbow != nullptr)
		{
			palox->ikh.paloShoulder = palox->ikh.paloElbow->paloParent;
			palox->ikh.paloCommon = PaloFindLoCommonParent(palo, palox->ikh.paloShoulder);
		}

	}

	if ((grfalox & 0x40) != 0)
	{
		const OID oidParent = palox->joint.oidParent;
		const OID oidChild = palox->joint.oidChild;

		palox->joint.paloParent = static_cast<ALO*>(PloFindSwNearest(palo->psw, oidParent, palo));
		palox->joint.paloChild = static_cast<ALO*>(PloFindSwNearest(palo->psw, oidChild, palo));
	}

}

void SnipAloObjects(ALO* palo, int csnip, SNIP* asnip)
{
	SW* psw = palo->psw;

	for (int i = 0; i < csnip; ++i)
	{
		const SNIP& snip = asnip[i];
		uint32_t grffso = (snip.grfsnip & 0x1) ? 0x105 : 0x101;

		if ((snip.grfsnip & 0x20) == 0)
			grffso &= ~0x100; // Remove 0x100 if bit 0x20 is not set

		LO* plo = PloFindSwObject(psw, grffso, snip.oid, palo);

		if (plo != nullptr)
		{
			if ((snip.grfsnip & 0x08) == 0)
			{
				// Store the pointer to the found object at a specific offset
				*(LO**)((char*)palo + snip.ib) = plo;
			}

			if ((snip.grfsnip & 0x04) == 0)
				SnipLo(plo);

			if ((snip.grfsnip & 0x10) != 0)
				SubscribeLoObject(plo, palo);
		}
	}
}

void PostAloLoad(ALO* palo)
{
	DLI dli;
	dli.m_pdl = &palo->dlChild;
	dli.m_ibDle = palo->dlChild.ibDle;
	dli.m_pdliNext = s_pdliFirst;
	s_pdliFirst = &dli;
	dli.m_ppv = reinterpret_cast<void**>(dli.m_pdl);

	PostLoLoad(palo);

	// ASEG post-load callback
	if (palo->pasegd != nullptr && (palo->pasegd->paseg != nullptr || palo->pasegd->oidAseg != OID_Nil))
		PostSwCallback(palo->psw, (PFNMQ)PostAloLoadCallback, palo, MSGID_callback, nullptr);

	PostGlobsetLoad(&palo->globset, palo);

	// Shadow setup
	if (palo->pshadow != nullptr)
	{
		SHADOW* pshadow = palo->pshadow.get();

		if (!FShadowRadiusSet(pshadow))
		{
			SetShadowNearRadius(pshadow, palo->sRadiusRenderAll);
			SetShadowFarRadius(pshadow, palo->sRadiusRenderAll * 0.5f);
		}

		if (pshadow->oidDysh != OID_Nil)
		{
			DYSH *pdysh = (DYSH*)PloFindSwNearest(palo->psw, pshadow->oidDysh, palo);
			
			pshadow->pdysh = pdysh;

			if (pdysh != nullptr)
				SetDyshShadow(pdysh, pshadow);
		}

		PostShadowLoad(pshadow);
	}

	// Sound effect setup
	if (palo->psfx != nullptr)
	{
		SFX* psfx = palo->psfx.get();

		if (psfx->sfxid == SFXID_Nil)
			palo->psfx = nullptr;
		else
			StartSound(psfx->sfxid, &psfx->pamb, palo, nullptr, psfx->sStart, psfx->sFull, psfx->uVol, psfx->uPitch, psfx->uDoppler, &psfx->lmRepeat, nullptr);
	}

	// Allocate pose ACT pointer array
	if (palo->globset.cpose != 0)
		palo->apactPose.resize(palo->globset.cpose, nullptr);

	// Process deferred child removals
	for (DLR* pdlr = palo->pdlrFirst.get(); pdlr != nullptr; pdlr = pdlr->pdlrNext.get())
	{
		LO* ploChild = nullptr;

		if (FMatchesLoName((LO*)palo, pdlr->oidChild))
			ploChild = (LO*)palo;
		else
			ploChild = PloFindSwObject(palo->psw, 1, pdlr->oidChild, (LO*)palo);

		if (ploChild == nullptr)
			continue;

		if (FIsLoInWorld(ploChild) && ploChild != (LO*)palo)
			ploChild->pvtlo->pfnPostLoLoad(ploChild);

		ploChild->pvtlo->pfnRemoveLo(ploChild);
	}

	palo->pdlrFirst.reset();

	// Walk child list and post-load each child
	void** ppv = dli.m_ppv;
	void* pv = *ppv;
	dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(pv) + dli.m_ibDle);

	while (pv != nullptr)
	{
		LO* ploChild = reinterpret_cast<LO*>(pv);
		ploChild->pvtlo->pfnPostLoLoad(ploChild);

		ppv = dli.m_ppv;
		pv = *ppv;
		dli.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(pv) + dli.m_ibDle);
	}

	s_pdliFirst = dli.m_pdliNext;
}

void PostAloLoadCallback(ALO* palo, MSGID msgid, void* pvData)
{
	ASEGD* pasegd = palo->pasegd.get();
	ASEG* paseg = pasegd->paseg;
	float tLocal;
	OVR ovr;

	if (paseg == nullptr) {
		paseg = (ASEG*)PloFindSwObject(palo->psw, 260, pasegd->oidAseg, palo);
	}
	if (paseg == nullptr) {
		return;
	}

	switch (pasegd->iak) {
		case IAK_Proportion:
		tLocal = pasegd->tLocal;

		if (tLocal < 0.0f) {
			tLocal = 0.0f;
		}
		else if (tLocal > 1.0f) {
			tLocal = 1.0f;
		}

		tLocal *= paseg->tMax;
		break;

		case IAK_Time:
		tLocal = pasegd->tLocal;
		break;

		case IAK_Nearest:
		FindAsegClosestPoint(paseg, palo, &palo->xf.pos, 0.0f, &tLocal, nullptr, nullptr);
		break;

		default:
		return;
	}

	ovr.oid = (OID)paseg->oidRoot;
	ovr.palo = palo;

	ApplyAsegOvr(paseg, palo, 1,  &ovr, tLocal, pasegd->svtLocal, 0, nullptr);
}

void UpdateAlo(ALO* palo, float dt)
{
	UpdateGlobset(&palo->globset, palo, dt);

	if (palo->pshadow != nullptr)
		UpdateShadow(palo->pshadow.get(), dt);

	if (palo->pthrob != nullptr)
		UpdateAloThrob(palo, dt);

	DLI dli{};

	dli.m_pdl = &palo->dlAct;
	dli.m_ibDle = palo->dlAct.ibDle;
	dli.m_pdliNext = s_pdliFirst;

	ACT* pact = palo->dlAct.pactFirst;

	dli.m_ppv = pact != nullptr ? reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pact) + dli.m_ibDle) : nullptr;

	s_pdliFirst = &dli;

	while (pact != nullptr)
	{
		if (pact->pvtact != nullptr && pact->pvtact->pfnUpdateAct != nullptr)
			pact->pvtact->pfnUpdateAct(pact, dt);

		pact = dli.m_ppv != nullptr ? static_cast<ACT*>(*dli.m_ppv) : nullptr;

		dli.m_ppv = pact != nullptr ? reinterpret_cast<void**>(reinterpret_cast<std::byte*>(pact) + dli.m_ibDle) : nullptr;
	}

	s_pdliFirst = dli.m_pdliNext;

	void* pvArgs = &dt;
	HandleLoSpliceEvent(palo, 1, 1, &pvArgs);
}

void RenderFastShadow(ALO* palo, CM* pcm, RO* pro)
{
	RO ro{};
	DupAloRo(palo, pro, &ro);

	glm::vec3 shadowScale(palo->sFastShadowRadius * 0.01f, palo->sFastShadowRadius * 0.01f, palo->sFastShadowDepth * 0.01f);
	glm::vec3 pos = glm::vec3(ro.model[3]);
	ro.model = glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(glm::quat_cast(glm::mat3(1.0f))) * glm::scale(glm::mat4(1.0f), shadowScale);

	ALO* pShadowLo = (ALO*)palo->psw->aploStock[17];

	if (pShadowLo != nullptr && pShadowLo->pvtalo != nullptr && pShadowLo->pvtalo->pfnRenderAloSelf != nullptr)
		pShadowLo->pvtalo->pfnRenderAloSelf(pShadowLo, pcm, &ro);
}

void RenderAloAll(ALO* palo, CM* pcm, RO* pro)
{
	if (palo->fHidden != 0)
		return;

	// Kept behind g_fBsp for debugging.
	if (g_fBsp != 0 && palo->grfzon != 0 && palo->viss == 2 &&
		(palo->grfzon & pcm->grfzon) != pcm->grfzon)
		return;

	RO ro{};
	RO roChild{};

	RO* proOrig = pro;

	/*
	 * Whole-ALO distance and frustum culling.
	 */
	if (palo->mrds == 2)
	{
		// Retail uses the render override's translation when one is supplied.
		const glm::vec3 posWorld =
			pro != nullptr
			? glm::vec3(pro->model[3])
			: palo->xf.posWorld;

		const glm::vec3 dpos = posWorld - pcm->pos;
		const bool inFrustum = SphereInFrustum(pcm->frustum, posWorld, palo->sRadiusRenderAll);
		if (!inFrustum)
			return;

		float uAlpha = 1.0f;

		if (!FInsideCmMrd(pcm, dpos, palo->sRadiusRenderAll, palo->sMRD, uAlpha))
			return;

		if (uAlpha != 1.0f)
		{
			DupAloRo(palo, pro, &ro);
			ro.uAlpha *= uAlpha;
			proOrig = &ro;
		}

		/*
		 * Cel borders can have a shorter render distance.
		 */
		if (palo->sCelBorderMRD < palo->sMRD)
		{
			float uAlphaCelBorder = 1.0f;

			if (!FInsideCmMrd(pcm, dpos, palo->sRadiusRenderAll, palo->sCelBorderMRD, uAlphaCelBorder))
				uAlphaCelBorder = 0.0f;

			if (uAlphaCelBorder != 1.0f)
			{
				DupAloRo(palo, proOrig, &ro);
				ro.uAlphaCelBorder *= uAlphaCelBorder;
				proOrig = &ro;
			}
		}
	}

	/*
	 * Apply object fading.
	 */
	if (palo->pfader != nullptr)
	{
		DupAloRo(palo, proOrig, &ro);
		ro.uAlpha *= palo->pfader->uAlpha;
		proOrig = &ro;
	}

	/*
	 * Segment scale compensation.
	 *
	 * The release build uses GRFALOX bit 0x80.
	 */
	if (proOrig != nullptr &&
		palo->palox != nullptr &&
		(palo->palox->grfalox & 0x80U) != 0 &&
		palo->palox->joint.fSsc != 0)
	{
		ALO* parent = palo->paloParent;

		if (parent != nullptr &&
			parent->pactScale != nullptr &&
			parent->palox != nullptr &&
			(parent->palox->grfalox & 0x80U) != 0)
		{
			glm::mat3 inverseParentScale(1.0f);
			parent->pactScale->pvtact->pfnGetActScale(parent->pactScale, &inverseParentScale);

			/*
			 * The release function reciprocates only the diagonal
			 * components of the parent scale matrix.
			 */
			if (inverseParentScale[0][0] != 0.0f)
				inverseParentScale[0][0] = 1.0f / inverseParentScale[0][0];

			if (inverseParentScale[1][1] != 0.0f)
				inverseParentScale[1][1] = 1.0f / inverseParentScale[1][1];

			if (inverseParentScale[2][2] != 0.0f)
				inverseParentScale[2][2] = 1.0f / inverseParentScale[2][2];

			DupAloRo(palo, proOrig, &ro);

			glm::vec3 position = glm::vec3(ro.model[3]);
			glm::mat3 currentRotation = glm::mat3(ro.model);

			// The SSC matrix is applied on the left in the release code.
			glm::mat3 correctedRotation = inverseParentScale * currentRotation;

			LoadMatrixFromPosRot(&position, &correctedRotation, &ro.model);
			proOrig = &ro;
		}
	}

	/*
	 * Apply this ALO's animated scale.
	 */
	ACT* pactScale = palo->pactScale;

	if (pactScale != nullptr)
	{
		glm::mat3 matScale(1.0f);
		pactScale->pvtact->pfnGetActScale(pactScale, &matScale);

		glm::mat4 scaleMatrix;
		LoadMatrixFromPosRot(&g_vecZero, &matScale, &scaleMatrix);

		DupAloRo(palo, proOrig, &ro);

		// Normal object scale is applied on the right.
		ro.model = ro.model * scaleMatrix;
		proOrig = &ro;
	}

	/*
	 * Render this object.
	 */
	if (palo->pvtalo != nullptr && palo->pvtalo->pfnRenderAloSelf != nullptr)
	{
		palo->pvtalo->pfnRenderAloSelf(palo, pcm, proOrig);
	}

	/*
	 * Recursively render child ALOs.
	 */
	DLI dliChild{};
	dliChild.m_pdl = &palo->dlChild;
	dliChild.m_ibDle = palo->dlChild.ibDle;
	dliChild.m_pdliNext = s_pdliFirst;
	dliChild.m_ppv = reinterpret_cast<void**>(dliChild.m_pdl);
	s_pdliFirst = &dliChild;

	while (true)
	{
		ALO* child = static_cast<ALO*>(*dliChild.m_ppv);

		if (child == nullptr)
			break;

		// Point at this child's next-link before dispatch. RemoveDlEntry can
		// then repair the iterator if rendering reparents or removes the child.
		dliChild.m_ppv = reinterpret_cast<void**>(
			reinterpret_cast<char*>(child) + dliChild.m_ibDle);

		if (child->pvtlo == nullptr || (child->pvtlo->grfcid & 1U) == 0)
			continue;

		if (child->pvtalo == nullptr || child->pvtalo->pfnRenderAloAll == nullptr)
			continue;

		/*
		 * With no render override, children resolve their own world
		 * transforms when RenderAloAll is called.
		 */
		if (proOrig == nullptr)
		{
			child->pvtalo->pfnRenderAloAll(child, pcm, nullptr);
			continue;
		}

		glm::mat4 childMatrix;
		glm::mat4 parentMatrix;

		const bool useWorldTransform =
			child->palox != nullptr &&
			(child->palox->grfalox & 0x0CU) != 0;

		if (!useWorldTransform)
		{
			/*
			 * Normal child: parent render matrix multiplied by the
			 * child's local transform.
			 */
			LoadMatrixFromPosRot(&child->xf.pos, &child->xf.mat, &childMatrix);
			parentMatrix = proOrig->model;
		}
		else
		{
			/*
			 * Special child: convert its resolved world transform
			 * into the current overridden parent render space.
			 */
			glm::mat4 parentWorldInverse;

			LoadMatrixFromPosRot(&child->xf.posWorld, &child->xf.matWorld, &childMatrix);
			LoadMatrixFromPosRotInverse(&palo->xf.posWorld, &palo->xf.matWorld, &parentWorldInverse);

			parentMatrix = proOrig->model * parentWorldInverse;
		}

		roChild = {};
		roChild.model = parentMatrix * childMatrix;
		roChild.uAlpha = proOrig->uAlpha;
		roChild.uAlphaCelBorder = proOrig->uAlphaCelBorder;
		roChild.fDynamicLight = proOrig->fDynamicLight;

		child->pvtalo->pfnRenderAloAll(child, pcm, &roChild);
	}

	s_pdliFirst = dliChild.m_pdliNext;

	/*
	 * The release build submits fast shadows whenever the radius is
	 * positive. Use your singular function name if that is the function
	 * currently present in your port.
	 */
	if (palo->sFastShadowRadius > 0.0f)
		RenderFastShadow(palo, pcm, proOrig);
}

void RenderAloSelf(ALO* palo, CM* pcm, RO* pro)
{
	palo->pvtalo->pfnUpdateAloConstraints(palo);

	if (palo->palox != nullptr && (palo->palox->grfalox & 0x80) != 0) {
		if (pro == nullptr) {
			palo->palox->joint.fMatInfluence = 0;
		}
		else {
			palo->palox->joint.fMatInfluence = 1;
			palo->palox->joint.matInfluence = pro->model;
		}
	}

	palo->pvtalo->pfnRenderAloGlobset(palo, pcm, pro);
}

void FreezeAlo(ALO* palo, int fFreeze)
{
	if (!fFreeze)
	{
		// Unfreeze.
		palo->fFrozen = false;

		palo->pvtalo->pfnSetAloVelocityVec(palo, &palo->frz.v);

		palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &palo->frz.w);

		if (palo->psfx)
			StartSound(palo->psfx->sfxid, &palo->psfx->pamb, palo, nullptr, palo->psfx->sStart, palo->psfx->sFull, palo->psfx->uVol, palo->psfx->uPitch, palo->psfx->uDoppler, &palo->psfx->lmRepeat, nullptr);

		return;
	}

	// Freeze: save current velocities.
	palo->frz.v = palo->xf.v;
	palo->frz.w = palo->xf.w;

	palo->dtUpdatePause = 0.0f;

	if (palo->psfx)
		StopSound(palo->psfx->pamb, 0);

		
	palo->pvtalo->pfnSetAloVelocityVec(palo, &g_vecZero);
	palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &g_vecZero);

	palo->fFrozen = true;
}

void DupAloRo(ALO* palo, RO* proOrig, RO* proDup)
{
	if (proOrig == nullptr)
	{
		LoadMatrixFromPosRotScale(&palo->xf.posWorld, &palo->xf.matWorld, &g_vecOne, &proDup->model);
		proDup->uAlpha = 1.0;
		proDup->uAlphaCelBorder = 1.0;
	}
	else
	{
		if (proOrig != proDup)
		{
			proDup->model = proOrig->model;
			proDup->uAlpha = proOrig->uAlpha;
			proDup->uAlphaCelBorder = proOrig->uAlphaCelBorder;
			proDup->fDynamicLight = proOrig->fDynamicLight;
		}
	}
}

void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro)
{
	RPL    rpl{};
	RPLCEL rplCel{};
	
	DupAloRo(palo, pro, &rpl.ro);

	const glm::mat4 baseModel = rpl.ro.model;
	const float     baseAlpha = rpl.ro.uAlpha;
	const float     baseAlphaCel = rpl.ro.uAlphaCelBorder;

	bool fGlobsetVisible = false;
	const bool doPerGlobMrd = (palo->mrds == 1);
	
	for (int i = 0; i < palo->globset.cglob; ++i)
	{
		auto& glob = palo->globset.aglob[i];
		auto* pglobi = (palo->globset.aglobi.empty() ? nullptr : &palo->globset.aglobi[i]);

		if (g_fBsp != 0 && pglobi != nullptr && pglobi->grfzon != 0 && palo->viss == 1)
		{
			if ((pglobi->grfzon & pcm->grfzon) != pcm->grfzon)
				continue;
		}

		glm::vec3 posCenter;

		if (!palo->fFixedPhys)
			posCenter = glm::vec3(baseModel * glm::vec4(glob.posCenter, 1.0f));
		else
			posCenter = glob.posCenter;

		const glm::vec3 dpos3 = posCenter - pcm->pos;
		const glm::vec4 dpos  = glm::vec4(dpos3, 0.0f);

		float uAlphaFromMrd = 1.0f;

		if (doPerGlobMrd)
		{
			if (!SphereInFrustum(pcm->frustum, posCenter, glob.sRadius))
				continue;

			if (!FInsideCmMrd(pcm, dpos, glob.sRadius, glob.sMRD, uAlphaFromMrd))
				continue;
		}
		else
		{
			if (pro == nullptr)
			{
				if (!SphereInFrustum(pcm->frustum, posCenter, glob.sRadius))
					continue;
			}

			uAlphaFromMrd = 1.0f;
		}

		rpl.ro.model = baseModel;
		rpl.ro.uAlpha = baseAlpha * uAlphaFromMrd;
		rpl.ro.uAlphaCelBorder = baseAlphaCel;

		if (glob.pwarpGlob)
		{
			rpl.ro.warpType = glob.pwrbg->warpType;
			rpl.ro.warpCmat = glob.pwarpGlob->pwr->cmat;
			rpl.ro.warpCvtx = glob.pwarpGlob->vertexCount;

			const size_t count = static_cast<size_t>(glob.pwrbg->pwr->cmat);

			switch (rpl.ro.warpType)
			{
				case WARP_POS:
				std::memcpy(rpl.ro.amatDpos, glob.pwrbg->pwr->amatDpos, count * sizeof(*rpl.ro.amatDpos));
				break;

				case WARP_UV:
				std::memcpy(rpl.ro.amatDuv, glob.pwrbg->pwr->amatDuv, count * sizeof(*rpl.ro.amatDuv));
				break;

				case WARP_BOTH:
				std::memcpy(rpl.ro.amatDpos, glob.pwrbg->pwr->amatDpos, count * sizeof(*rpl.ro.amatDpos));
				std::memcpy(rpl.ro.amatDuv,  glob.pwrbg->pwr->amatDuv,  count * sizeof(*rpl.ro.amatDuv));
				break;

				default:
				rpl.ro.warpType = WARP_NONE;
				break;
			}
		}
		else
			rpl.ro.warpType = WARP_NONE;

		if (glob.gleam != nullptr)
		{
			glm::vec3 v = glm::mat3(rpl.ro.model) * glob.gleam->normal;

			const float length = glm::length(v);
			const glm::vec3 dir = length < 0.0001f ? glm::vec3(0.0f) : v / length;

			const glm::vec3 cameraX = glm::vec3(g_pcm->mat[0]);
			const float intensity = std::abs(glm::dot(dir, cameraX));

			const CLQC& clqc = glob.gleam->clqc;
			float gain = clqc.g0 + intensity * (clqc.g1 + intensity * (clqc.g2 + intensity * clqc.g3));

			gain = GLimitLm(&g_lmZeroOne, gain);
			rpl.ro.uAlpha *= gain;
		}

		if (pglobi != nullptr)
		{
			float target = (g_clock.tReal < pglobi->tUnfade) ? 0.5f : 1.0f;

			if (pglobi->uAlpha != target)
				pglobi->uAlpha = GSmooth(pglobi->uAlpha, target, g_clock.dt, &g_smpAlphaFade, nullptr);

			rpl.ro.uAlpha *= pglobi->uAlpha;
		}

		rpl.ro.uAlpha *= g_uAlpha;

		if (rpl.ro.uAlpha <= 0.0f)
			continue;

		fGlobsetVisible = true;

		rpl.palo = palo;
		rpl.pglob = &glob;
		rpl.rp = glob.rp;

		rpl.ro.uFog = glob.uFog;
		rpl.ro.darken = ((glob.grfglob & 4U) == 0) ? g_psw->rDarken : 1.0f;
		rpl.ro.fDynamic = glob.fDynamic;
		rpl.ro.sRadius = glob.sRadius;
		rpl.ro.posCenter = glm::vec4(posCenter, 1.0f);
		rpl.ro.grfglob = glob.grfglob;

		if (glob.gZOrder != FLT_MAX)
			rpl.z = glob.gZOrder;
		else
			rpl.z = glm::dot(dpos3, dpos3);

		rpl.ro.uAlphaCelBorder = 0.0f;

		if (rpl.ro.uAlpha != 1.0f)
		{
			switch (rpl.rp)
			{
				case RP_Opaque:
				case RP_Cutout:
				case RP_OpaqueAfterProjVolume:
				case RP_CutoutAfterProjVolume:
				rpl.rp = RP_Translucent;
				break;

				case RP_CelBorder:
				case RP_CelBorderAfterProjVolume:
				rpl.rp = RP_TranslucentCelBorder;
				break;

				default:
				break;
			}
		}

		int sortT = 0;

		if (rpl.ro.uAlpha < 1.0f)
		{
			if (rpl.rp == RP_Translucent || rpl.rp == RP_TranslucentCelBorder)
				sortT = 1;
		}

		if (!sortT)
		{
			if (rpl.rp == RP_Background || rpl.rp == RP_Cutout || rpl.rp == RP_CutoutAfterProjVolume || rpl.rp == RP_Translucent)
				sortT = glob.fTransluscentSort;
		}

		rpl.fTransluscentSort = sortT;

		if (!allSwDynamicLights.empty() && glob.fThreeWay == 1)
			rpl.ro.fDynamicLight = FindSwDynamicLights(&posCenter, glob.sRadius);
		else
			rpl.ro.fDynamicLight = 0;

		if (pglobi != nullptr && glob.fThreeWay == 1 && glob.fDynamic == 0 &&
			glob.pwarpGlob == nullptr &&
			pglobi->cframeStaticLights < g_cframeStaticLightsInvalid)
		{
			glob.trlk = TRLK_Relight;
		}

		if (glob.trlk == TRLK_Relight)
		{
			rpl.ro.trlk = TRLK_Relight;
			glob.trlk = TRLK_BakePending;

			if (pglobi != nullptr)
				pglobi->cframeStaticLights = g_cframe;
		}
		else if (glob.trlk == TRLK_BakePending)
		{
			// The cache-writing packet is still in a deferred render list.
			// Fully light this additional packet instead of reading that cache.
			rpl.ro.trlk = TRLK_Dynamic;
		}
		else
		{
			rpl.ro.trlk = glob.trlk;
		}

		if (glob.fDynamic == 1 || glob.pwarpGlob != nullptr)
			rpl.ro.trlk = TRLK_Dynamic;

		if (glob.psaa != nullptr)
		{
			if (glob.psaa && glob.psaa->pvtlooker && glob.psaa->pvtlooker->pfnNotifyLookerRender)
				glob.psaa->pvtlooker->pfnNotifyLookerRender((LOOKER*)glob.psaa, palo, &rpl);
		}

		glm::mat4 submitModel = baseModel;

		if (glob.pdmat != nullptr)
			submitModel = baseModel * (*glob.pdmat);

		if (glob.rtck != RTCK_None)
			AdjustAloRtckMat(palo, pcm, glob.rtck, &posCenter, submitModel);

		rpl.ro.model = submitModel;

		SubmitRpl(&rpl);

		if (g_fRenderCelBorders > 0 && glob.csubcel > 0)
		{
			float celAlpha = baseAlphaCel;

			if (glob.sCelBorderMRD < glob.sMRD)
			{
				float dummyCB = 1.0f;

				if (!FInsideCmMrd(pcm, dpos, glob.sRadius, glob.sCelBorderMRD, dummyCB))
					celAlpha = 0.0f;
				else
					celAlpha = baseAlphaCel * uAlphaFromMrd;
			}

			const float cb = celAlpha * rpl.ro.uAlpha;

			if (cb > 0.0f)
			{
				//rplCel.pglob = &glob;
				rplCel.rocel.model = submitModel;
				rplCel.rocel.celRgba = ((palo->globset.grfglobset & 2) == 0) ? g_rgbaCel : palo->globset.rgbaCel;

				// The cel geometry is a second submission.  It does not inherit the
				// surface's render phase verbatim: the PS2 renderer partitions it into
				// the corresponding cel-border phase.
				switch (rpl.rp)
				{
					case RP_OpaqueAfterProjVolume:
					case RP_CutoutAfterProjVolume:
					rplCel.rp = RP_CelBorderAfterProjVolume;
					break;

					case RP_Translucent:
					case RP_TranslucentCelBorder:
					rplCel.rp = RP_TranslucentCelBorder;
					break;

					default:
					rplCel.rp = RP_CelBorder;
					break;
				}
				rplCel.rocel.uAlphaCelBorder = cb;
				rplCel.palo = palo;
				rplCel.pglob = &glob;

				SubmitRplCel(&rplCel);
			}
		}
	}

	if (fGlobsetVisible)
	{
		if (palo->pvtalo != nullptr && palo->pvtalo->pfnUpdateAloInfluences != nullptr)
			palo->pvtalo->pfnUpdateAloInfluences(palo, pro);

		if (!palo->apactPose.empty() && palo->globset.cpose > 0)
		{
			for (int ipose = 0; ipose < palo->globset.cpose; ++ipose)
			{
				ACT* pact = palo->apactPose[ipose];

				if (pact == nullptr)
					palo->globset.agPoses[ipose] = palo->globset.agPosesOrig[ipose];
				else
					ProjectActPose(pact, ipose);
			}
		}
	}
}

void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha)
{
	glm::vec3 p0 = *ppos0;
	glm::vec3 p1 = *ppos1;

	// Original:
	// dpos   = p1 - p0
	// dposCm = p0 - cameraPos
	const glm::vec3 dir = p1 - p0;
	const glm::vec3 toCam = p0 - pcm->pos;

	//
	// axis1 = normalize(cross(toCam, dir))
	//
	glm::vec3 axis1 = glm::cross(toCam, dir);
	const float axis1Len = glm::length(axis1);

	// Original eventually only renders when this length > 0.01
	if (axis1Len <= 0.01f)
		return;

	axis1 /= axis1Len;

	//
	// axis0 = normalize(cross(axis1, dir))
	// THIS was the mismatch.
	//
	glm::vec3 axis0 = glm::cross(axis1, dir);

	const float axis0Len = glm::length(axis0);

	if (axis0Len < 0.0001f)
		return;

	axis0 /= axis0Len;

	//
	// Original scaling:
	//
	axis1 *= rWidth;
	const glm::vec3 axis2 = dir * 0.01f;

	glm::mat3 rot(1.0f);

	// GLM columns
	rot[0] = axis0;
	rot[1] = axis1;
	rot[2] = axis2;

	glm::mat4 model(1.0f);
	LoadMatrixFromPosRot(&p0, &rot, &model);

	RO ro{};
	ro.model = model;
	ro.uAlpha = uAlpha;
	ro.uAlphaCelBorder = uAlpha;

	palo->pvtalo->pfnRenderAloGlobset(palo, pcm, &ro);
}

void DeleteModel(ALO* palo)
{
	std::unordered_set<GLuint> deletedBuffers;
	std::unordered_set<GLuint> deletedVertexArrays;

	auto DeleteBuffer = [&](GLuint& buffer)
	{
		if (buffer != 0 && deletedBuffers.insert(buffer).second)
			glDeleteBuffers(1, &buffer);

		buffer = 0;
	};

	auto DeleteVertexArray = [&](GLuint& vertexArray)
	{
		if (vertexArray != 0 && deletedVertexArrays.insert(vertexArray).second)
			glDeleteVertexArrays(1, &vertexArray);

		vertexArray = 0;
	};

	DeleteBuffer(palo->globset.boneMatrixSSBO);

	for (GLOB& glob : palo->globset.aglob)
	{
		DeleteVertexArray(glob.VAO);
		DeleteBuffer(glob.VBO);
		DeleteBuffer(glob.EBO);
		DeleteBuffer(glob.ssboCachedMaterial);
		DeleteBuffer(glob.poseDposSSBO);
		DeleteBuffer(glob.poseDnormalSSBO);
		DeleteBuffer(glob.edgeSSBO);
		DeleteBuffer(glob.edgeBoneIndicesSSBO);
		DeleteBuffer(glob.edgeBoneWeightsSSBO);
		DeleteBuffer(glob.edgePoseDposSSBO);

		if (glob.pwarpGlob != nullptr)
			DeleteBuffer(glob.pwarpGlob->ssboState);

		glob.poseDpos.clear();
		glob.poseDnormal.clear();
		glob.edgeBoneIndices.clear();
		glob.edgeBoneWeights.clear();
		glob.edgePoseDpos.clear();

		glob.poseCount = 0;
		glob.poseVertexCount = 0;
		glob.edgeCount = 0;
	}
}

int GetAloSize()
{
	return sizeof(ALO);
}

void DeleteAlo(ALO* palo)
{
	delete palo;
}

std::vector <ALO*> allSWAloObjs;
THROB s_mpthrobkthrob[6] =
{
	THROBK_Pipe,  glm::vec3(120.0, 190.0, 115.0), glm::vec3(120.0, 190.0, 60.0), 2.0,
	THROBK_Ninja, glm::vec3(0.0f, 0.0, 255.0f),   glm::vec3(0.0, 0.0, 0.0),      2.0,
	THROBK_Rail,  glm::vec3(120.0, 255.0, 255.0), glm::vec3(120.0, 0.0, 0.0),    2.0,
	THROBK_Hide,  glm::vec3(240.0, 255.0, 255.0), glm::vec3(240.0, 0.0, 0.0),    2.0,
	THROBK_Foo,   glm::vec3(0.0, 255.0, 255.0),   glm::vec3(0.0, 0.0, 0.0),      2.0,
	THROBK_Bar,   glm::vec3(300.0, 255.0, 255.0), glm::vec3(300.0, 0.0, 0.0),    2.0
};

CLQ g_clqPosSpringDefault  = {0.0, 15.0, 0.015, 0.0};
CLQ g_clqPosDampingDefault = {0.0, 2.5, 0.0024999999, 0.0};
CLQ g_clqRotSpringDefault  = {0.0, 15.0, 1.5, 0.0};
CLQ g_clqRotDampingDefault = {0.0, 2.5, 0.25, 0.0};
SMPA g_smpaPosDefault = {1500.0, 0.0, 0.5, 4500.0};
SMPA g_smpaRotDefault ={15.0, 0.0, 0.5, 45.0};
CLQ s_clqHomingSpeed = { -50.0, 4.0, 0.0 };
LM s_lmHomingSpeed = { 0.0, 400.0 };
