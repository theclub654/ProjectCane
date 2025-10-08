#include "alo.h"

ALO* NewAlo()
{
	return new ALO{};
}

void InitAlo(ALO* palo)
{
	InitDl(&palo->dlChild,  offsetof(LO, dleChild));
	InitDl(&palo->dlFreeze, offsetof(ALO, dleFreeze));

	if (palo->paloParent == nullptr)
		*(unsigned long*)&palo->bitfield = *(unsigned long*)&palo->bitfield & 0xfffffffff0ffffff | 0xa000000;
	else
		*(unsigned long*)&palo->bitfield = *(unsigned long*)&palo->bitfield & 0xfffffffff0ffffff | 0x1000000;

	InitLo(palo);

	byte value = 0xFF;
	byte value1 = 0x0;
	memcpy((char*)&palo->bitfield + 1, &value,  sizeof(byte));
	memcpy((char*)&palo->bitfield + 2, &value,  sizeof(byte));
	memcpy((char*)&palo->bitfield + 0, &value1, sizeof(byte));
	
	palo->sCelBorderMRD = FLT_MAX;
	palo->sMRD = FLT_MAX;
	palo->grfzon = -1;
	palo->xf.mat = glm::identity<glm::mat3>();
	palo->xf.matWorld = glm::identity<glm::mat3>();
	palo->matOrig = glm::identity<glm::mat3>();
	
	InitDl(&palo->dlAct, offsetof(ACT, dleAlo));

	if (palo->pvtlo->cid != CID_LIGHT)
		allSWAloObjs.push_back(palo);
}

void RemoveAloHierarchy(ALO *palo)
{
	// Setup a child DLI for traversal
	DLI dliChild{};
	dliChild.m_pdl = &palo->dlChild;
	dliChild.m_ibDle = palo->dlChild.ibDle;

	// Push onto global list stack
	dliChild.m_pdliNext = s_pdliFirst;
	s_pdliFirst = &dliChild;

	// Point to first child object
	dliChild.m_ppv = reinterpret_cast<void**>(dliChild.m_pdl);

	while (true)
	{
		// Call removal function on current parent
		palo->pvtlo->pfnOnLoRemove(palo);

		// Load next child object from the current pointer
		void* nextParentObject = *dliChild.m_ppv;
		if (!nextParentObject) break;

		// Advance to the next pointer in the list
		dliChild.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(nextParentObject) + dliChild.m_ibDle);
	}

	// Pop from global list stack
	s_pdliFirst = dliChild.m_pdliNext;
}

void OnAloAdd(ALO* palo)
{
	if (!palo) return;

	ALO* parent = palo->paloParent;
	SW* psw = palo->psw;

	if (!parent) {
		palo->paloRoot = palo;

		if (palo->fRealClock == 0) {
			AppendDlEntry(&psw->dlMRD, palo);
			palo->bitfield.fBusy = true;
			AppendDlEntry(&psw->dlBusy, palo);

			if (palo->pvtlo->grfcid & 0x2U) {
				AppendDlEntry(&psw->dlBusySo, palo);
			}

			palo->paloFreezeRoot = palo;
			palo->dlFreeze.paloFirst = palo;
			palo->dlFreeze.paloLast = palo;

			/*for (int i = palo->cpmrg - 1; i >= 0; --i) {
				if (MRG* pmrg = palo->apmrg[i]) {
					MergeSwGroup(psw, pmrg);
				}
			}*/
		}
		else {
			AppendDlEntry(&psw->dlMRDRealClock, palo);
		}
	}
	else {
		palo->paloRoot = parent->paloRoot;

		/*if (parent->bitfield.fFrozen)
			parent->pvtalo->pfnFreezeAlo();*/
		
	}

	// Position actuator
	/*if (palo->pactPos != nullptr) {
		glm::vec3 w{}, v{};
		actPos->pvtact->pfnGetActPositionGoal(0, actPos, &w, &v);
		palo->pvtalo->pfnTranslateAloToPos(palo, &w);
		palo->pvtalo->pfnSetAloVelocityVec(palo, &v);
	}*/

	// Rotation actuator
	/*if (palo->pactRot != nullptr) {
		glm::mat3 mat{};
		glm::vec3 w{};
		palo->pactRot->pvtact->pfnGetActRotationGoal(0, actRot, &mat, &w);
		palo->pvtalo->pfnRotateAloToMat(palo, &mat);
		palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &w);
	}*/

	/*if (palo->bitfield.fForceCameraFade == true) 
		AddCmFadeObject(g_pcm, palo);*/

	if (palo->pshadow != nullptr) 
		AppendDlEntry(&psw->dlShadow, &palo->pshadow);

	if (palo->pvtlo->pfnUpdateLoXfWorld != nullptr)
		palo->pvtalo->pfnUpdateAloXfWorld(palo);

	//HandleLoSpliceEvent(palo, 4, 0, nullptr);
	ResolveAlo(palo);
}

void OnAloRemove(ALO* palo)
{
	if (!palo) return;

	SW* psw = palo->psw;

	OnLoRemove(static_cast<LO*>(palo));

	if (!palo->paloParent) {
		if (palo->fRealClock != 0) {
			RemoveDlEntry(&psw->dlMRDRealClock, palo);
		}
		else {
			RemoveDlEntry(&psw->dlMRD, palo);

			if (palo->bitfield.fBusy) {
				palo->bitfield.fBusy = false;
				RemoveDlEntry(&psw->dlBusy, palo);

				if ((palo->pvtlo->grfcid & 0x2U) != 0) {
					RemoveDlEntry(&psw->dlBusySo, palo);
				}
			}
		}

		//SplinterSwFreezeGroup(psw, palo->paloFreezeRoot);
		palo->paloFreezeRoot = nullptr;
		ClearDl(&palo->dlFreeze);
	}

	// Check for camera fade flag
	/*if (palo->bitfield.fForceCameraFade) {
		RemoveCmFadeObject(g_pcm, palo);
	}*/

	if (palo->pshadow != nullptr) {
		RemoveDlEntry(&psw->dlShadow, &palo->pshadow);
	}

	// If object is frozen, call the freeze callback if it exists
	/*if (palo->bitfield.fFrozen) 
		palo->pvtalo->pfnFreezeLo()*/
	
	ResolveAlo(palo);
	palo->paloRoot = nullptr;

	//HandleLoSpliceEvent(palo, 5, 0, nullptr);
}

void UpdateAloOrig(ALO* palo)
{
	// Copy current transform matrix and position
	palo->matOrig = palo->xf.mat;
	palo->posOrig = palo->xf.pos;

	// Undo rotation adjustment using virtual function
	palo->pvtalo->pfnUnadjustAloRotation(palo, &palo->matOrig);

	// Convert matrix to Euler angles
	DecomposeRotateMatrixEuler(palo->matOrig, palo->eulOrig);
}

void UpdateAloInfluences(ALO* palo, RO* pro)
{
	
}

void AdjustAloRotation(ALO* palo, glm::mat3* pmat, glm::vec3* pw)
{
	if (!palo || !palo->palox) return;

	ALOX* palox = palo->palox.get();

	// Apply pre-rotation (bit 0)
	if ((palox->grfalox & 1) && pmat) {
		*pmat = (*pmat) * palox->matPreRotation;
	}

	// Apply post-rotation (bit 1)
	if ((palox->grfalox & 2)) {
		if (pmat) {
			*pmat = palox->matPostRotation * (*pmat);
		}
		if (pw) {
			*pw = palox->matPostRotation * (*pw);
		}
	}
}

void UnadjustAloRotation(ALO* palo, glm::mat3* pmat)
{
	ALOX* palox = palo->palox.get(); // unique_ptr

	if (!palox) return;

	if (palox->grfalox & 0x01) {
		// Pre-rotation is applied: mat = matPre * mat
		*pmat = palox->matPreRotation * (*pmat);
	}

	if (palox->grfalox & 0x02) {
		// Post-rotation is applied: mat = mat * transpose(matPost)
		*pmat = (*pmat) * glm::transpose(palox->matPostRotation);
	}
}

void MatchAloOtherObject(ALO* palo, ALO* paloOther)
{
	
}

void AdjustAloRtckMat(ALO* palo, CM* pcm, RTCK rtck, glm::vec3* pposCenter, glm::mat4 &pmat)
{
	
}

void CloneAloHierarchy(ALO* palo, ALO* paloBase)
{
	DLI parentPalo{};

	// Storing the parent ptr to child ALO
	parentPalo.m_pdl = &paloBase->dlChild;

	// Storing the base offset to child ALO
	parentPalo.m_ibDle = paloBase->dlChild.ibDle;

	// Storing the parent ALO
	parentPalo.m_pdliNext = s_pdliFirst;

	// Keeping track of the ALO parent
	s_pdliFirst = &parentPalo;

	// Storing ptr to next ALO to clone
	parentPalo.m_ppv = (void**)(uintptr_t)parentPalo.m_pdl;

	// Clones paloBase to palo
	palo->pvtlo->pfnCloneLo(palo, paloBase);

	// Loading next ALO to clone
	LO* plo = (LO*)*parentPalo.m_ppv;
	// Loading up next ALO to clone after than one above
	parentPalo.m_ppv = (void**) ((uintptr_t)plo + parentPalo.m_ibDle);

	while (plo != nullptr)
	{
		// Clones ALO object
		PloCloneLo(plo, palo->psw, palo);
		// Loads next ALO object to clone
		plo = (LO*)*parentPalo.m_ppv;
		// Loads next ALO object to clone after this one
		parentPalo.m_ppv = (void**) ((uintptr_t)plo + parentPalo.m_ibDle);
	}

	s_pdliFirst = parentPalo.m_pdliNext;
}

void CloneAlo(ALO* palo, ALO* paloBase)
{
	
	palo->dlChild = paloBase->dlChild;
	//palo->dleBusy = paloBase->dleBusy;
	//palo->dleMRD = paloBase->dleMRD;
	//palo->paloRoot = paloBase->paloRoot;
	//palo->paloFreezeRoot = paloBase->paloFreezeRoot;
	//palo->dleFreeze = paloBase->dleFreeze;
	//palo->dlFreeze = paloBase->dlFreeze;
	palo->cpmrg = paloBase->cpmrg;
	for (int i = 0; i < 4; ++i)
		palo->apmrg[i] = paloBase->apmrg[i];
	palo->sMRD = paloBase->sMRD;
	palo->sCelBorderMRD = paloBase->sCelBorderMRD;
	palo->grfzon = paloBase->grfzon;
	palo->dsMRDSnap = paloBase->dsMRDSnap;
	std::memcpy(palo->frz, paloBase->frz, sizeof(palo->frz));
	palo->xf = paloBase->xf;
	palo->posOrig = paloBase->posOrig;
	palo->matOrig = paloBase->matOrig;
	palo->eulOrig = paloBase->eulOrig;
	//palo->dlAct = paloBase->dlAct;
	palo->pactPos = paloBase->pactPos;
	palo->pactRot = paloBase->pactRot;
	palo->pactScale = paloBase->pactScale;
	palo->apactPose = paloBase->apactPose;
	palo->pactRestore = paloBase->pactRestore;
	palo->pactla = paloBase->pactla;
	palo->pactbank = paloBase->pactbank;
	palo->pikh = paloBase->pikh;
	palo->pclqPosSpring = paloBase->pclqPosSpring;
	palo->pclqPosDamping = paloBase->pclqPosDamping;
	palo->pclqRotSpring = paloBase->pclqRotSpring;
	palo->pclqRotDamping = paloBase->pclqRotDamping;
	palo->psmpaPos = paloBase->psmpaPos;
	palo->psmpaRot = paloBase->psmpaRot;
	if (paloBase->palox)
		palo->palox = std::make_unique<ALOX>(*paloBase->palox);
	else
		palo->palox.reset();
	palo->cframeStatic = paloBase->cframeStatic;
	palo->globset = paloBase->globset;
	for (int i = 0; i < palo->globset.aglob.size(); i++)
	{
		numRo += palo->globset.aglob[i].csubglob;
		numRo += palo->globset.aglob[i].csubcel;
	}
	palo->pshadow = paloBase->pshadow;
	palo->pthrob = paloBase->pthrob;
	palo->sFastShadowRadius = paloBase->sFastShadowRadius;
	palo->sFastShadowDepth = paloBase->sFastShadowDepth;
	palo->fRealClock = paloBase->fRealClock;
	palo->pfader = paloBase->pfader;
	palo->dtUpdatePause = paloBase->dtUpdatePause;
	palo->pasegd = paloBase->pasegd;
	palo->sRadiusRenderSelf = paloBase->sRadiusRenderSelf;
	palo->sRadiusRenderAll = paloBase->sRadiusRenderAll;
	palo->psfx = paloBase->psfx;
	palo->ficg = paloBase->ficg;
	palo->cposec = paloBase->cposec;
	palo->aposec = paloBase->aposec;
	palo->pactrefCombo = paloBase->pactrefCombo;
	palo->pdlrFirst = paloBase->pdlrFirst;
	//palo->bitfield = paloBase->bitfield;
	palo->ackRot = paloBase->ackRot;

	CloneLo(palo, paloBase);

	ClearDl(&palo->dlChild);
}

void ResolveAlo(ALO* palo)
{
	if (palo->paloRoot != nullptr) 
		palo->paloRoot->cframeStatic = 0;
}

void SetAloParent(ALO* palo, ALO* paloParent)
{
	if (palo->paloParent == paloParent)
		return;

	// Backup world-space transform
	glm::vec3 posWorld = palo->xf.posWorld;
	glm::mat3 matWorld = palo->xf.matWorld;

	// Backup velocities in world space
	glm::vec3 vWorld, wWorld;
	if (palo->paloParent == nullptr) {
		vWorld = palo->xf.v;
		wWorld = palo->xf.w;
	}
	else {
		CalculateAloMovement(palo, nullptr, &palo->xf.posWorld, &vWorld, &wWorld, nullptr, nullptr);
	}

	// Detach from old parent
	palo->pvtlo->pfnRemoveLo(palo);

	// Convert world-space transform to new parent's local space
	ConvertAloPos(nullptr, paloParent, posWorld, palo->xf.pos);
	ConvertAloMat(nullptr, paloParent, matWorld, palo->xf.mat);

	// Adjust relative velocities if we now have a parent
	if (paloParent != nullptr) {
		glm::vec3 vParent, wParent;
		CalculateAloMovement(paloParent, nullptr, &palo->xf.posWorld, &vParent, &wParent, nullptr, nullptr);
		vWorld -= vParent;
		wWorld -= wParent;
	}

	// Convert world-space velocities to new local-space
	glm::vec3 vLocal, wLocal;
	ConvertAloVec(nullptr, paloParent, &vWorld, &vLocal);
	palo->pvtalo->pfnSetAloVelocityVec(palo, &vLocal);

	ConvertAloVec(nullptr, paloParent, &wWorld, &wLocal);
	palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &wLocal);

	// Update flags if the parent changed (from null to non-null)
	if ((paloParent == nullptr) != (palo->paloParent == nullptr)) {
		uint64_t flags = *(uint64_t*)&palo->bitfield;

		if (paloParent == nullptr) {
			flags = (flags & 0xFFFFFFFFFCFFFFFFULL) | 0x02000000ULL; // set bit 25
			if ((flags & 0x0C000000ULL) != 0x04000000ULL)
				flags = (flags & 0xFFFFFFFFF0FFFFFFULL) | 0x0A000000ULL;

			if ((flags & 0x30000000ULL) != 0x10000000ULL)
				flags = (flags & 0xFFFFFFFFCFFFFFFFULL) | 0x20000000ULL;
		}
		else {
			flags = (flags & 0xFFFFFFFFFCFFFFFFULL) | 0x01000000ULL; // clear bit 25, set 24
			if ((flags & 0x0C000000ULL) != 0x04000000ULL)
				flags = (flags & 0xFFFFFFFFF0FFFFFFULL) | 0x01000000ULL;

			uint64_t mode = flags & 0x30000000ULL;
			bool keepFlags = (mode == 0x10000000ULL) ||
				(mode == 0x20000000ULL && palo->sMRD != 1e10f);

			if (!keepFlags)
				flags = flags & 0xFFFFFFFFCFFFFFFFULL;
		}

		*(uint64_t*)&palo->bitfield = flags;
	}

	// Finish reparenting
	UpdateAloOrig(palo);
	palo->paloParent = paloParent;
	palo->pvtlo->pfnAddLo(palo);
}

int FIsZeroV(const glm::vec3* pv)
{
	if (!pv) return 1;

	float lenSquared = glm::dot(*pv, *pv);
	return (lenSquared < 4.0f) ? 1 : 0;
}

int FIsZeroW(const glm::vec3* pw)
{
	if (!pw) return 1;

	float lenSquared = glm::dot(*pw, *pw);
	return (lenSquared < 0.0004f) ? 1 : 0;
}

void SetAloVelocityVec(ALO* palo, glm::vec3* pv)
{
	if (!palo || !pv) return;

	// Set velocity vector
	palo->xf.v = *pv;

	// If there's an ACT attached, adapt it
	if (palo->pactPos) {
		AdaptAct(palo->pactPos);
	}

	// If the velocity is not zero, resolve ALO
	if (!FIsZeroV(pv)) {
		ResolveAlo(palo);
	}
}

void SetAloAngularVelocityVec(ALO* palo, glm::vec3* pw)
{
	// Set angular velocity vector
	palo->xf.w = *pw;

	// Update associated rotation actuator
	if (palo->pactRot) {
		AdaptAct(palo->pactRot);
	}

	// Trigger resolve if angular velocity is non-zero
	if (!FIsZeroW(pw)) {
		ResolveAlo(palo);
	}
}

void BindAlo(ALO *palo)
{
	BindAloAlox(palo);
	UpdateAloOrig(palo);
	BindGlobset(&palo->globset, palo);

	ALO *plo = palo->dlChild.paloFirst;

	while (plo != nullptr)
	{
		if (plo->pvtalo->pfnBindAlo != nullptr)
			plo->pvtalo->pfnBindAlo(plo);
		
		plo = plo->dleChild.paloNext;
	}
}

void BindGlobset(GLOBSET *pglobset, ALO *palo)
{
	if (!pglobset || !palo || pglobset->cbnd <= 0) return;

	for (int i = 0; i < pglobset->cbnd; ++i) {
		// Pointer to the BND entry we’re updating
		BND *pbnd = &pglobset->abnd[i];

		// Find closest LO that matches the OID
		ALO *plo = (ALO*)PloFindSwNearest(palo->psw, pglobset->mpibndoid[i], (LO*)palo);
		pbnd->palo = plo;

		if (plo != nullptr) {
			// Compute the transform from the ALO world space to the BND local space
			glm::mat4 matAlo, matBndInv, matResult;

			LoadMatrixFromPosRot(palo->xf.posWorld, palo->xf.matWorld, matAlo);
			LoadMatrixFromPosRotInverse(plo->xf.posWorld, plo->xf.matWorld, matBndInv);

			matResult = matBndInv * matAlo;

			pbnd->dmatOrig = matResult;
		}
	}
}

void CalculateAloMovement(ALO* paloLeaf, ALO* paloBasis, const glm::vec3* ppos, glm::vec3* pv, glm::vec3* pw, glm::vec3* pdv, glm::vec3* pdw)
{
	if (!paloLeaf || paloLeaf == paloBasis)
		return;

	constexpr int kMaxHierarchy = 16;
	ALO* apalo[kMaxHierarchy];
	int count = 0;

	// Build ALO hierarchy path from leaf to basis
	ALO* current = paloLeaf;
	while (current && current != paloBasis && count < kMaxHierarchy) {
		if (!current->palox || (current->palox->grfalox & 0xC) == 0) {
			apalo[count++] = current;
		}
		current = current->paloParent;
	}

	glm::vec3 wSum(0.0f), dwSum(0.0f);
	if (pv) *pv = glm::vec3(0.0f);
	if (pdv) *pdv = glm::vec3(0.0f);

	const glm::vec3* prevPos = ppos;

	for (int i = count - 1; i >= 0; --i) {
		ALO* alo = apalo[i];
		ALO* parent = alo->paloParent;

		// Accumulate angular velocities
		if (pw || pdw || pv || pdv) {
			glm::vec3 w;
			ConvertAloVec(parent, paloBasis, &alo->xf.w, &w);
			wSum += w;
		}

		if (pdw) {
			glm::vec3 dw;
			ConvertAloVec(parent, paloBasis, &alo->xf.dw, &dw);
			dwSum += dw;
		}

		if (pv || pdv) {
			const glm::vec3* basePos = (i > 0) ? &apalo[i - 1]->xf.posWorld : prevPos;
			glm::vec3 offset = *basePos - alo->xf.posWorld;

			ConvertAloVec(nullptr, paloBasis, &offset, &offset);

			// Apply vector cross-multiplication: v = w × offset
			glm::vec3 rotVel = glm::cross(wSum, offset);

			if (pv) {
				*pv += rotVel;

				glm::vec3 v;
				ConvertAloVec(parent, paloBasis, &alo->xf.v, &v);
				*pv += v;
			}

			if (pdv) {
				glm::vec3 dv;
				ConvertAloVec(parent, paloBasis, &alo->xf.dv, &dv);

				glm::vec3 dwCross = glm::cross(dwSum, offset);
				glm::vec3 wCrossV = glm::cross(wSum, glm::cross(wSum, offset));

				*pdv += dwCross + wCrossV + dv;
			}
		}
	}

	if (pw) *pw = wSum;
	if (pdw) *pdw = dwSum;
}

void UpdateAloXfWorld(ALO* palo)
{
	palo->pvtalo->pfnUpdateAloXfWorldHierarchy(palo);
}

void UpdateAloXfWorldHierarchy(ALO* palo)
{
	ALOX* palox = palo->palox.get(); // if unique_ptr
	ALO* parent = nullptr;

	// --- Determine parent based on ALOX flags ---
	if (!palox || (palox->grfalox & 0xC) == 0) {
		parent = palo->paloParent;
	}
	else {
		parent = (palox->grfalox & 0x4) ? palox->scj.paloSchRot : palo->paloParent;
	}

	// --- Compute world position and matrix ---
	if (!parent) {
		palo->xf.posWorld = palo->xf.pos;
		palo->xf.matWorld = palo->xf.mat;
	}
	else {
		palo->xf.posWorld = parent->xf.matWorld * palo->xf.pos + parent->xf.posWorld;
		palo->xf.matWorld = parent->xf.matWorld * palo->xf.mat;
	}

	// --- Optionally override matrix parent ---
	if (palox) {
		parent = (palox->grfalox & 0x8) ? palox->scj.paloSchRot : palo->paloParent;

		if (parent) {
			palo->xf.matWorld = parent->xf.matWorld * palo->xf.mat;
		}
		else if ((palox->grfalox & 0x8) != 0) {
			palo->xf.matWorld = palo->xf.mat;
		}
	}

	// --- Vismap clipping ---
	/*if (((palo->field_0x2d8 << 8) >> 32 & 3U) == 2) {
		if (palo->psw && palo->psw->pvismap) {
			ClipVismapSphereOneHop(palo->psw->pvismap, &palo->xf.posWorld, palo->sRadiusRenderAll, &palo->grfzon);
		}
		else {
			palo->grfzon = 0xFFFFFFF;
		}
	}*/

	//// --- Shadow update ---
	if (palo->pshadow != nullptr) {
		SetShadowCastPosition(palo->pshadow.get(), palo->xf.posWorld);

		SHD* shd = palo->pshadow->pshd;
		if (shd && shd->shdk == 0x03) {
			glm::vec3 normal = -palo->xf.matWorld[2];
			SetShadowCastNormal(palo->pshadow.get(), normal);

			glm::vec3 up = palo->xf.matWorld[1];
			SetShadowFrustrumUp(palo->pshadow.get(), &up);
		}
	}

	// --- Recurse through children ---
	for (ALO* child = palo->dlChild.paloFirst; child; child = child->dleChild.paloNext) {
		if (child->pvtalo->pfnUpdateAloXfWorldHierarchy) {
			child->pvtalo->pfnUpdateAloXfWorldHierarchy(child);
		}
	}

	palox = palo->palox.get();

	if (palox) {
		// If ALOX has both IKH and SCJ flags set
		if ((palox->grfalox & 0x8020U) == 0x8020) {
			palox->ikh.paloShoulder->palox->ikj.fInvalid = true;
			palox->ikh.paloElbow->palox->ikj.fInvalid = true;
		}

		// If this is a SCJ controller
		if ((palox->grfalox & 0x8100U) == 0x8100 && palox->ikh.grfik > 0) {
			for (int i = 0; i < palox->ikh.grfik; ++i) {
				ALO* target = palox->sch.apalo[i];
				ALOX* targetAlox = target->palox.get();

				if (palo == targetAlox->scj.paloSchRot) {
					targetAlox->scj.fInvalidRot = true;
				}
				if (palo == targetAlox->scj.paloSchPos) {
					targetAlox->scj.fInvalidPos = true;
				}
			}
		}
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
		dlBusyWalker.m_pdl = &palo->dlChild;                // Point to the actual DL list
		dlBusyWalker.m_ibDle = palo->dlChild.ibDle;         // Offset to the 'next' pointer inside each object
		dlBusyWalker.m_pdliNext = s_pdliFirst;            // Link this walker into a global list of DLI walkers

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

			// If there is a next object, update the walker’s pointer to its next link
			if (currentObject != nullptr)
				dlBusyWalker.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle);
		}
	}
}

void TranslateAloToPos(ALO* palo, glm::vec3 &ppos)
{
	palo->xf.pos = ppos;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);

	// Update ACT if present
	if (palo->pactPos) {
		AdaptAct(palo->pactPos);
	}
}

void ConvertAloPos(ALO* paloFrom, ALO* paloTo, glm::vec3 &pposFrom, glm::vec3 &pposTo)
{
	glm::vec3 worldPos;

	// Transform local to world space if `paloFrom` is specified and different from `paloTo`
	if (paloFrom != paloTo) 
	{
		if (paloFrom) 
		{
			worldPos = paloFrom->xf.matWorld * (pposFrom) + paloFrom->xf.posWorld;
			pposFrom = worldPos;
		}

		// Convert world position to local position relative to `paloTo`
		if (paloTo) 
		{
			glm::vec3 localPos = pposFrom - paloTo->xf.posWorld;
			glm::mat3 invRot = glm::transpose(paloTo->xf.matWorld); // Inverse of rotation
			pposTo = invRot * localPos;
			return;
		}
	}

	// No transformation needed
	pposTo = pposFrom;
}

void ConvertAloVec(ALO* paloFrom, ALO* paloTo, glm::vec3 *pvecFrom, glm::vec3 *pvecTo)
{
	glm::vec3 vecWorld = *pvecFrom;

	// Transform from local to world space if paloFrom is valid and different from paloTo
	if (paloFrom && paloFrom != paloTo)
		vecWorld = paloFrom->xf.matWorld * (*pvecFrom);

	// Transform from world to local space of paloTo if it's valid
	if (paloTo) 
	{
		glm::mat3 invMat = glm::inverse(paloTo->xf.matWorld);
		*pvecTo = invMat * vecWorld;
	}
	else
		*pvecTo = vecWorld;
}

void RotateAloToMat(ALO* palo, glm::mat3 &pmat)
{
	palo->xf.mat = pmat;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);
}

void ConvertAloMat(ALO* paloFrom, ALO* paloTo, glm::mat3 &pmatFrom, glm::mat3 &pmatTo)
{
	if (paloFrom != paloTo)
	{
		if (paloFrom)
		{
			glm::mat3 fromRot = glm::mat3(paloFrom->xf.matWorld);  // rotation part only
			pmatFrom = fromRot * pmatFrom;
		}

		if (paloTo)
		{
			glm::mat3 toRot = glm::mat3(paloTo->xf.matWorld);      // rotation part only
			glm::mat3 toRotT = glm::transpose(toRot);              // transpose = inverse if orthonormal

			glm::mat3 temp = toRotT * pmatFrom;                    // rotate into paloTo local space
			pmatTo = toRot * temp;                                 // then re-apply paloTo's world rotation
			return;
		}
	}

	pmatTo = pmatFrom;
}

void SetAloInitialVelocity(ALO* palo, glm::vec3* pv)
{
	const glm::mat4& m = palo->xf.mat;

	glm::vec3 transformedVelocity =
		glm::vec3(m[0]) * pv->x +
		glm::vec3(m[1]) * pv->y +
		glm::vec3(m[2]) * pv->z;

	palo->xf.v = transformedVelocity;
}

void SetAloInitialAngularVelocity(ALO* palo, const glm::vec3* pw)
{
	const glm::mat4& m = palo->xf.mat;

	glm::vec3 transformedAngularVelocity =
		glm::vec3(m[0]) * pw->x +
		glm::vec3(m[1]) * pw->y +
		glm::vec3(m[2]) * pw->z;

	palo->xf.w = transformedAngularVelocity;
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

SHADOW* PshadowAloEnsure(ALO* palo)
{
	if (!palo->pshadow)
	{
		palo->pshadow = std::make_shared<SHADOW>();
		InitShadow(palo->pshadow.get());
		AppendDlEntry(&palo->psw->dlShadow, &palo->pshadow);
	}

	return palo->pshadow.get();
}

SHADOW* PshadowInferAlo(ALO* palo)
{
	return nullptr;
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
	palo->bitfield.fFrozen = fFrozen;
}

void SetAloEuler(ALO* palo, glm::vec3* peul)
{
	// Convert degrees to radians
	glm::vec3 eulerRadians = glm::radians(*peul);

	// Load rotation matrix from Euler angles
	glm::mat3 mat;
	LoadRotateMatrixEuler(eulerRadians, &mat);

	// Apply the rotation
	if (palo && palo->pvtalo) {
		if (auto fnAdjust = palo->pvtalo->pfnAdjustAloRotation) {
			fnAdjust(palo, &mat, nullptr);
		}
		if (auto pfnRotate = palo->pvtalo->pfnRotateAloToMat) {
			pfnRotate(palo, mat);
		}
	}
}

void SetAloVelocityLocal(ALO* palo, glm::vec3* pvec)
{
	// Convert local-space velocity to world-space velocity
	glm::vec3 localVelocity = *pvec;
	glm::vec3 worldVelocity = palo->xf.mat * localVelocity;

	//palo->pvtalo->pfnSetAloVelocityVec(palo, &worldVelocity);
}

void SetAloFastShadowRadius(ALO* palo, float sRadius)
{
	palo->sFastShadowRadius = sRadius;
}

void SetAloFastShadowDepth(ALO* palo, float sDepth)
{
	palo->sFastShadowDepth = sDepth;
}

void SetAloCastShadow(ALO* palo, int fCastShadow)
{
	if (fCastShadow == 0) {
		if (palo->pshadow != nullptr) {
			RemoveDlEntry(&palo->psw->dlShadow, &palo->pshadow);
			AppendDlEntry(&g_dlShadowPending, &palo->pshadow);
			palo->pshadow = nullptr;
		}
	}
	else {
		PshadowAloEnsure(palo);
	}
}

void SetAloShadowShader(ALO* palo, OID oidShdShadow)
{
	SHADOW* pshadow = PshadowAloEnsure(palo);
	SetShadowShader(pshadow, oidShdShadow);
}

void GetAloShadowShader(ALO* palo, OID* poidShdShadow)
{
	if (palo && palo->pshadow && palo->pshadow->pshd) {
		*poidShdShadow = static_cast<OID>(palo->pshadow->pshd->oid);
	}
	else {
		*poidShdShadow = OID_Nil;
	}
}

void GetAloShadowNearRadius(ALO* palo, float* psNearRadius)
{
	SHADOW* pshadow = PshadowInferAlo(palo);
	*psNearRadius = pshadow->sNearRadius;
}

void SetAloShadowNearRadius(ALO* palo, float sNearRadius)
{
	SHADOW *pshadow = PshadowAloEnsure(palo);
	SetShadowNearRadius(pshadow, sNearRadius);
}

void SetAloShadowFarRadius(ALO* palo, float sFarRadius)
{
	SHADOW *pshadow = PshadowAloEnsure(palo);
	SetShadowFarRadius(pshadow, sFarRadius);
}

void GetAloShadowFarRadius(ALO* palo, float* psFarRadius)
{
	SHADOW *pshadow = PshadowInferAlo(palo);
	*psFarRadius = pshadow->sFarRadius;
}

void SetAloShadowNearCast(ALO* palo, float sNearCast)
{
	SHADOW *pshadow = PshadowAloEnsure(palo);
	SetShadowNearCast(pshadow, sNearCast);
}

void GetAloShadowNearCast(ALO* palo, float* psNearCast)
{
	SHADOW *pshadow = PshadowInferAlo(palo);
	*psNearCast = pshadow->sNearCast;
}

void SetAloShadowFarCast(ALO* palo, float sFarCast)
{
	SHADOW *pshadow = PshadowAloEnsure(palo);
	SetShadowFarCast(pshadow, sFarCast);
}

void GetAloShadowFarCast(ALO* palo, float* psFarCast)
{
	SHADOW *pshadow = PshadowInferAlo(palo);
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
	SHADOW *pshadow = PshadowAloEnsure(palo);
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
	palo->bitfield.fFrozen = fNoFreeze;
}

void SetAloRestorePosition(ALO* palo, int fRestore)
{

}

void SetAloRestorePositionAck(ALO* palo, ACK ack)
{

}

void SetAloPositionSpring(ALO* palo, float r)
{

}

void SetAloPositionSpringDetail(ALO* palo, CLQ* pclq)
{

}

void SetAloPositionDamping(ALO* palo, float r)
{

}

void SetAloPositionDampingDetail(ALO* palo, CLQ* pclq)
{

}

void SetAloRestoreRotation(ALO* palo, int fRestore)
{

}

void SetAloRestoreRotationAck(ALO* palo, ACK ack)
{

}

void SetAloRotationSpring(ALO* palo, float r)
{

}

void SetAloRotationSpringDetail(ALO* palo, CLQ* pclq)
{

}

void SetAloRotationDamping(ALO* palo, float r)
{

}

void SetAloRotationDampingDetail(ALO* palo, CLQ* pclq)
{

}

void SetAloPositionSmooth(ALO* palo, float r)
{

}

void SetAloPositionSmoothDetail(ALO* palo, SMPA* psmpa)
{

}

void SetAloRotationSmooth(ALO* palo, float r)
{

}

void SetAloRotationSmoothDetail(ALO* palo, SMPA* psmpa)
{

}

void SetAloPositionSmoothMaxAccel(ALO* palo, float r)
{

}

void SetAloRotationSmoothMaxAccel(ALO* palo, float r)
{

}

void SetAloDefaultAckPos(ALO* palo, ACK ack)
{

}

void SetAloDefaultAckRot(ALO* palo, ACK ack)
{

}

void SetAloLookAt(ALO* palo, ACK ack)
{

}

void SetAloLookAtIgnore(ALO* palo, float sIgnore)
{

}

void SetAloLookAtPanFunction(ALO* palo, CLQ* pclq)
{

}

void SetAloLookAtPanLimits(ALO* palo, LM* plm)
{

}

void SetAloLookAtTiltFunction(ALO* palo, CLQ* pclq)
{

}

void SetAloLookAtTiltLimits(ALO* palo, LM* plm)
{

}

void SetAloLookAtEnabledPriority(ALO* palo, int nPriority)
{

}

void SetAloLookAtDisabledPriority(ALO* palo, int nPriority)
{

}

void SetAloTargetAttacks(ALO* palo, int grftak)
{

}

void SetAloTargetRadius(ALO* palo, float sRadiusTarget)
{

}

void SetAloThrobKind(ALO* palo, THROBK throbk)
{

}

void SetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor)
{

}

void SetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor)
{

}

void SetAloThrobDtInOut(ALO* palo, float dtInOut)
{

}

void SetAloSfxid(ALO* palo, SFXID sfxid)
{

}

void SetAloSStart(ALO* palo, float sStart)
{

}

void SetAloSFull(ALO* palo, float sFull)
{

}

void SetAloUVolumeSpl(ALO* palo, float uVol)
{

}

void SetAloUVolume(ALO* palo, float uVol)
{

}

void SetAloUPitchSpl(ALO* palo, float uPitch)
{

}

void SetAloUPitch(ALO* palo, float uPitch)
{

}

void SetAloSndRepeat(ALO* palo, LM* plm)
{

}

void SetAloUDoppler(ALO* palo, float uDoppler)
{

}

void SetAloInteractCane(ALO* palo, int grfic)
{

}

void SetAloInteractCaneSweep(ALO* palo, int grfic)
{

}

void SetAloInteractCaneRush(ALO* palo, int grfic)
{

}

void SetAloInteractCaneSmash(ALO* palo, int grfic)
{

}

void SetAloInteractBomb(ALO* palo, int grfic)
{

}

void SetAloInteractShock(ALO* palo, int grfic)
{

}

void SetAloPoseCombo(ALO* palo, OID oidCombo)
{

}

void SetAloForceCameraFade(ALO* palo, int fFade)
{

}

void* GetAloFrozen(ALO* palo)
{
	return nullptr;
}

void* GetAloXfPos(ALO* palo)
{
	return &palo->xf.pos;
}

void* GetAloXfPosOrig(ALO* palo)
{
	return &palo->posOrig;
}

void* GetAloXfPosWorld(ALO* palo)
{
	return &palo->xf.posWorld;
}

void* GetAloXfMat(ALO* palo)
{
	return &palo->xf.mat;
}

void* GetAloMatOrig(ALO* palo)
{
	return &palo->matOrig;
}

void* GetAloXfMatWorld(ALO* palo)
{
	return &palo->xf.matWorld;
}

void* GetAloEuler(ALO* palo)
{
	return nullptr;
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

void* GetAloXfdv(ALO* palo)
{
	return &palo->xf.dv;
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
	*psRadius = palo->sFastShadowRadius;
}

void GetAloFastShadowDepth(ALO* palo, float* psDepth)
{
	*psDepth = palo->sFastShadowDepth;
}

void GetAloCastShadow(ALO* palo, int* pfCastShadow)
{

}

void GetAloLookAtIgnore(ALO* palo, float* psIgnore)
{

}

void GetAloLookAtPanFunction(ALO* palo, CLQ* pclq)
{

}

void GetAloLookAtPanLimits(ALO* palo, LM* plm)
{

}

void GetAloLookAtTiltFunction(ALO* palo, CLQ* pclq)
{

}

void GetAloLookAtTiltLimits(ALO* palo, LM* plm)
{

}

void GetAloLookAtEnabledPriority(ALO* palo, int* pnPriority)
{

}

void GetAloLookAtDisabledPriority(ALO* palo, int* pnPriority)
{

}

int FGetAloChildrenList(ALO* palo, void* pvstate)
{
	return 0;
}

void GetAloThrobKind(ALO* palo, THROBK* pthrobk)
{

}

void GetAloThrobInColor(ALO* palo, glm::vec3* phsvInColor)
{

}

void GetAloThrobOutColor(ALO* palo, glm::vec3* phsvOutColor)
{

}

void GetAloThrobDtInOut(ALO* palo, float* pdtInOut)
{

}

void GetAloSfxid(ALO* palo, SFXID* psfxid)
{

}

void GetAloSStart(ALO* palo, float* psStart)
{

}

void GetAloSFull(ALO* palo, float* psFull)
{

}

void GetAloUVolume(ALO* palo, float* puVol)
{

}

void GetAloUPitch(ALO* palo, float* puPitch)
{

}

void GetAloSndRepeat(ALO* palo, LM* plmRepeat)
{

}

void GetAloUDoppler(ALO* palo, float* puDoppler)
{

}

void GetAloInteractCane(ALO* palo, int* pgrfic)
{

}

void GetAloInteractCaneSweep(ALO* palo, int* pgrfic)
{

}

void GetAloInteractCaneRush(ALO* palo, int* pgrfic)
{

}

void GetAloInteractCaneSmash(ALO* palo, int* pgrfic)
{

}

void GetAloInteractBomb(ALO* palo, int* pgrfic)
{

}

void GetAloInteractShock(ALO* palo, int* pgrfic)
{

}

void* GetAlofRealClock(ALO* palo)
{
	return &palo->fRealClock;;
}

void AddAloHierarchy(ALO* palo)
{
	DLI plo;
	// Loads parent object child entry
	plo.m_pdl = &palo->dlChild;
	// Loads the object base offset to child entry list
	plo.m_ibDle = palo->dlChild.ibDle;
	// Loading the first parent of the object
	plo.m_pdliNext = s_pdliFirst;
	
	s_pdliFirst = &plo;

	plo.m_ppv = (void**)(uintptr_t)plo.m_pdl;

	// Looping through all the child objects 
	while (true)
	{
		// Adding object to list
		palo->pvtlo->pfnOnLoAdd(palo);

		// Loading next object
		void *nextParentObject = *plo.m_ppv;
		
		plo.m_ppv = (void**) ((uintptr_t)nextParentObject + plo.m_ibDle);

		// If parent doesnt have a child break
		if (nextParentObject == nullptr) break;
	}

	s_pdliFirst = plo.m_pdliNext;
}

void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis)
{
	palo->xf.mat = pbis->ReadMatrix();
	palo->xf.pos = pbis->ReadVector();

	palo->bitfield.zons = pbis->U8Read() & 0x03;
	palo->bitfield.viss = pbis->U8Read() & 0x03;
	palo->bitfield.mrds = pbis->U8Read() & 0x03;
	
	palo->grfzon = pbis->U32Read();
	palo->sMRD = pbis->F32Read();
	palo->sCelBorderMRD = pbis->F32Read();
	palo->sRadiusRenderSelf = pbis->F32Read();
	palo->sRadiusRenderAll = pbis->F32Read();

	if (palo->sMRD == 3.402823e+38)
		palo->sMRD = 1e+10;
	
	LoadOptionsFromBrx(palo, pbis);
	LoadGlobsetFromBrx(&palo->globset, palo, pbis);
	LoadAloAloxFromBrx(palo, pbis);

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
	const uint32_t grfalox = pbis->U32Read();

	if (grfalox == 0)
		return;

	palo->palox = std::make_unique<ALOX>();

	palo->palox->grfalox = grfalox;
	palo->palox->matPreRotation  = glm::mat3(1.0f);
	palo->palox->matPostRotation = glm::mat3(1.0f);

	if (grfalox & 0x01)
		palo->palox->matPreRotation = pbis->ReadMatrix();

	if (grfalox & 0x02)
		palo->palox->matPostRotation = pbis->ReadMatrix();

	if ((grfalox & 0x0C) != 0)
	{
		int16_t schRotId = pbis->S16Read();

		if (schRotId != -1)
			palo->palox->scj.paloSchRot = static_cast<ALO*>(PloFindSwObject(palo->psw, 3, (OID)schRotId, palo));
	}

	if (grfalox & 0x10)
		palo->palox->scj.ipaloRot = static_cast<int16_t>(pbis->S16Read());

	if (grfalox & 0x20)
	{
		palo->palox->scj.ipaloPos = static_cast<int16_t>(pbis->S16Read());
		palo->palox->ikh.posIkh.x = pbis->F32Read();
		palo->palox->ikh.posIkh.y = pbis->F32Read();
		palo->palox->ikh.posIkh.z = pbis->F32Read();

		palo->palox->ikh.posWrist.x = pbis->F32Read();
		palo->palox->ikh.posWrist.y = pbis->F32Read();
		palo->palox->ikh.posWrist.z = pbis->F32Read();
		float twist = pbis->F32Read();
		float normTwist = RadNormalize(twist);
		palo->palox->ikh.radTwistOrig = normTwist;
		palo->palox->ikh.grfik = 0;
		palo->palox->joint.matInfluence[3][0] = normTwist;
	}

	if (grfalox & 0x80)
	{
		palo->palox->sch.posSch = pbis->ReadVector();
		palo->palox->sch.gStrength = pbis->F32Read();

		if (grfalox & 0x100)
		{
			palo->palox->sch.oidScjEnd = (OID)pbis->S16Read();
			palo->palox->sch.oidScjStart = (OID)pbis->S16Read();
			palo->palox->sch.oidSchPrev = (OID)pbis->S16Read();
			palo->palox->sch.paloScjStart = reinterpret_cast<ALO*>(pbis->S16Read());
			
			if (palo->palox->sch.scsk == SCSK_Fixed)
			{
				palo->palox->ikh.posWrist.x = pbis->F32Read();
				palo->palox->ikh.posWrist.y = pbis->F32Read();
				palo->palox->ikh.posWrist.z = pbis->F32Read();
			}
		}
	}

	if (grfalox & 0x200)
	{
		palo->palox->ikj.oidIkh = (OID)pbis->S16Read();
		palo->palox->ikj.fInvalid = static_cast<int16_t>(pbis->S16Read());
	}

	if (grfalox & 0x400)
		palo->palox->joint.fSsc = static_cast<int8_t>(pbis->U8Read());
}

void BindAloAlox(ALO* palo)
{
	if (!palo || !palo->palox)
		return;

	auto& alox = *palo->palox;
	alox.grfalox |= 0x8000; // Mark as bound

	if (alox.grfalox & 0x10)
	{
		alox.scj.paloSchRot = static_cast<ALO*>(PloFindSwNearest(palo->psw, (OID)alox.scj.ipaloRot, palo));
		alox.ikj.fInvalid = 1;
	}

	if (alox.grfalox & 0x20)
	{
		alox.ikh.paloElbow = static_cast<ALO*>(PloFindSwNearest(palo->psw, alox.ikh.oidElbow, palo));
		alox.ikh.paloShoulder = alox.ikh.paloElbow ? alox.ikh.paloElbow->paloParent : nullptr;
		alox.ikh.paloCommon = PaloFindLoCommonParent(palo, alox.ikh.paloShoulder);
	}

	/*if ((alox.grfalox & 0x80) && (alox.grfalox & 0x100))
	{
		ALO* elbow = static_cast<ALO*>(PloFindSwNearest(palo->psw, alox.ikh.oidElbow, palo));
		alox.ikh.paloElbow = elbow;

		ALO* common = static_cast<ALO*>(PloFindSwNearest(palo->psw, reinteralox.ikh.paloCommon), palo));
		alox.ikh.paloCommon = common;

		alox.ikh.radTwistOrig = reinterpret_cast<float>(PloFindSwNearest(palo->psw, static_cast<OID>(alox.ikh.radTwistOrig), palo));

		alox.sch.paloCommon = PaloFindLoCommonParent(palo, elbow);
		alox.sch.paloCommon = PaloFindLoCommonParent(alox.sch.paloCommon, reinterpret_cast<ALO*>(static_cast<intptr_t>(alox.ikh.radTwistOrig)));

		int boneCount = 0;
		ALO* current = common;

		SCSK scskType = alox.sch.scsk;

		while (current && current != elbow->paloParent)
		{
			if (scskType == SCSK_Fixed)
			{
				current->palox->scj.paloSchRot = palo;
				current->palox->scj.fInvalidRot = 1;
			}
			else if (scskType == SCSK_Stretch)
			{
				current->palox->scj.paloSchRot = palo;
				current->palox->scj.fInvalidRot = 1;
				if (current != elbow)
				{
					current->palox->scj.paloSchPos = palo;
					current->palox->scj.fInvalidPos = 1;
				}
			}
			current = current->paloParent;
			++boneCount;
		}

		alox.ikh.grfik = boneCount;
		alox.sch.apalo.resize(boneCount);

		current = common;
		for (int i = boneCount - 1; i >= 0 && current; --i)
		{
			alox.sch.apalo[i] = current;
			if (scskType == SCSK_Fixed || (scskType == SCSK_Stretch && current != elbow))
			{
				current->palox->ikj.fInvalid = i;
			}
			if (scskType == SCSK_Stretch && current != elbow)
			{
				current->palox->scj.ipaloPos = i;
			}
			current = current->paloParent;
		}
	}*/

	if (alox.grfalox & 0x200)
	{
		alox.scj.paloSchRot = static_cast<ALO*>(PloFindSwNearest(palo->psw, (OID)alox.scj.ipaloRot, palo));
		auto* p = PloFindSwNearest(palo->psw, static_cast<OID>(alox.ikj.fInvalid), palo);
		alox.ikj.fInvalid = reinterpret_cast<int>(p);
	}
}

void PostAloLoad(ALO* palo)
{
	PostLoLoad(palo);
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

void UpdateAlo(ALO* palo, float dt)
{

}

void RenderAloAll(ALO* palo, CM* pcm, RO* pro)
{
	if (g_fBsp != 0)
	{
		if ((palo->grfzon & pcm->grfzon) != pcm->grfzon)
			return;
	}

	RO ro{};
	RO* proOriginal = &ro;
	glm::vec3 posWorld{};

	if (pro != nullptr)
		posWorld = pro->model[3];
	else
		posWorld = palo->xf.posWorld;

	if (!SphereInFrustum(pcm->frustum, posWorld, palo->sRadiusRenderAll))
		return;

	float alpha{};
	if (!FInsideCmMrd(pcm, pcm->pos - palo->xf.posWorld, palo->sRadiusRenderAll, palo->sMRD, alpha))
		return;

	if (alpha == 1.0)
		proOriginal = pro;
	else
	{
		DupAloRo(palo, pro, &ro);
		ro.uAlpha = ro.uAlpha * alpha;
		proOriginal = &ro;
	}

	// ---- Cel-border MRD handling (matches original structure) ----
	// If cel-border MRD is tighter than main MRD, test and scale uAlphaCelBorder
	if (palo->sCelBorderMRD < palo->sMRD)
	{
		// In the original decomp, dpos_00 came from ro.mat.* (current RO).
		// Here we use the current render origin (proOriginal if set, otherwise ALO pos)
		glm::vec3 testPos = (proOriginal != nullptr) ? glm::vec3(proOriginal->model[3]) : palo->xf.posWorld;

		float alphaCB = 1.0f;
		if (!FInsideCmMrd(pcm, pcm->pos - testPos, palo->sRadiusRenderAll, palo->sCelBorderMRD, alphaCB))
			alphaCB = 0.0f; // original sets uAlpha to 0 when outside cel-border MRD

		if (alphaCB != 1.0f)
		{
			// In the original, when uAlpha != 1, it duped RO and scaled uAlphaCelBorder
			DupAloRo(palo, proOriginal, &ro);
			ro.uAlphaCelBorder = ro.uAlphaCelBorder * alphaCB;
			proOriginal = &ro;
		}
		// If alphaCB == 1.0f, keep current proOriginal as-is (same as original goto path)
	}

	// ---- Optional fader (post adjustments), multiplies base uAlpha ----
	if (palo->pfader != nullptr)
	{
		DupAloRo(palo, proOriginal, &ro);
		//ro.uAlpha = ro.uAlpha * palo->pfader->uAlpha;
		proOriginal = &ro;
	}

	palo->pvtalo->pfnRenderAloSelf(palo, pcm, proOriginal);

	ALO* child = palo->dlChild.paloFirst;
	float fastShadowRadius = palo->sFastShadowRadius;

	if (proOriginal == nullptr)
	{
		// No proxy: simple render all children
		for (; child != nullptr; child = child->dleChild.paloNext) {
			if (child->pvtlo->grfcid & 1U)
			{
				child->pvtalo->pfnRenderAloAll(child, pcm, nullptr);

			}
		}
	}
	else {
		// Proxy exists: need to adjust transforms
		RO roChild;

		for (; child != nullptr; child = child->dleChild.paloNext) {
			if (!(child->pvtlo->grfcid & 1U)) {
				continue;
			}

			glm::mat4 childLocalMatrix;
			if (child->ppxr == nullptr) {
				LoadMatrixFromPosRot(child->xf.pos, child->xf.mat, childLocalMatrix);
			}
			else
			{
				glm::mat4 invParentWorld = glm::inverse(palo->xf.matWorld);
				LoadMatrixFromPosRot(child->xf.posWorld, child->xf.matWorld, childLocalMatrix);
				glm::mat4 proxyAdjust = invParentWorld * proOriginal->model;
				childLocalMatrix = proxyAdjust * childLocalMatrix;
			}

			// Compute final child world matrix
			roChild.model = proOriginal->model * childLocalMatrix;
			roChild.uAlpha = proOriginal->uAlpha;
			roChild.uAlphaCelBorder = proOriginal->uAlphaCelBorder;

			// Render child with adjusted transform
			child->pvtalo->pfnRenderAloAll(child, pcm, &roChild);
		}
	}
}

void RenderAloSelf(ALO* palo, CM* pcm, RO* pro)
{
	//palo->pvtalo->pfnUpdateAloConstraints();

	/*if (palo->palox && (palo->palox->grfalox & 0x400)) {
		if (!pro) {
			palo->palox->joint.fMatInfluence = 0;
		}
		else {
			palo->palox->joint.fMatInfluence = 1;
			const glm::mat4& mat = pro->model;

			palo->palox->foster.paloParent = reinterpret_cast<ALO*>(glm::value_ptr(mat)[0]);
			palo->palox->ikj.fInvalid = *reinterpret_cast<const int*>(&glm::value_ptr(mat)[1]);
			palo->palox->ikh.posIkh.z = glm::value_ptr(mat)[2];
			palo->palox->ikh.posIkh.w = glm::value_ptr(mat)[3];

			palo->palox->scj.fInvalidRot = *reinterpret_cast<const int*>(&glm::value_ptr(mat)[4]);
			palo->palox->scj.fInvalidPos = *reinterpret_cast<const int*>(&glm::value_ptr(mat)[5]);
			palo->palox->ikh.posWrist.z = glm::value_ptr(mat)[6];
			palo->palox->ikh.posWrist.w = glm::value_ptr(mat)[7];

			palo->palox->ikh.paloShoulder = reinterpret_cast<ALO*>(glm::value_ptr(mat)[8]);
			palo->palox->ikh.oidElbow = static_cast<OID>(*reinterpret_cast<const uint32_t*>(&glm::value_ptr(mat)[9]));
			palo->palox->ikh.paloCommon = reinterpret_cast<ALO*>(glm::value_ptr(mat)[10]);
			palo->palox->ikh.radTwistOrig = glm::value_ptr(mat)[11];

			palo->palox->ikh.radTwist = glm::value_ptr(mat)[12];
			palo->palox->sch.scsk = reinterpret_cast<SCSK>(*reinterpret_cast<const uint32_t*>(&glm::value_ptr(mat)[13]));
			palo->palox->ikh.grfik = static_cast<GRFIK>(glm::value_ptr(mat)[14]);
			palo->palox->sch.apalo = reinterpret_cast<ALO**>(static_cast<uintptr_t>(glm::value_ptr(mat)[15]));
		}
	}*/

	palo->pvtalo->pfnRenderAloGlobset(palo, pcm, pro);
}

void DupAloRo(ALO* palo, RO* proOrig, RO* proDup)
{
	if (proOrig == nullptr)
	{
		glm::vec3 vecScale = glm::vec3(1.0);
		LoadMatrixFromPosRotScale(palo->xf.posWorld, palo->xf.matWorld, vecScale, proDup->model);
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
		}
	}
}

void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro)
{
	RPL rpl{};
	RO* proDup;

	proDup = &rpl.ro;
	// Duplicate rendering object from original
	DupAloRo(palo, pro, proDup);

	glm::mat4 baseModelMatrix = rpl.ro.model;

	float baseAlpha = rpl.ro.uAlpha;
	float baseAlphaCel = rpl.ro.uAlphaCelBorder;

	for (int i = 0; i < palo->globset.aglob.size(); ++i)
	{
		if (g_fBsp != 0)
		{
			if ((palo->globset.aglobi[i].grfzon & pcm->grfzon) != pcm->grfzon)
				continue;
		}

		auto& glob = palo->globset.aglob[i];
		auto& globi = palo->globset.aglobi[i];

		glm::vec3 posCenterWorld = glm::vec3(baseModelMatrix * glm::vec4(glob.posCenter, 1.0f));

		if (!SphereInFrustum(pcm->frustum, posCenterWorld, glob.sRadius))
			continue;

		if (!FInsideCmMrd(pcm, pcm->pos - posCenterWorld, glob.sRadius, glob.sMRD, rpl.ro.uAlpha))
			continue;

		for (auto& subglob : glob.asubglob)
		{
			rpl.ro.uAlpha *= baseAlpha;

			rpl.ro.VAO = &subglob.VAO;

			if (glob.gleam.size() != 0)
			{
				glm::vec3 normal = glob.gleam[0].normal;

				const glm::vec4& colY = rpl.ro.model[1];
				const glm::vec4& colZ = rpl.ro.model[2];

				// Blend vectors based on normal components
				glm::vec4 blendedY = colY + colY * normal.y;
				glm::vec4 blendedZ = colZ + colZ * normal.z;

				// Compute direction vector length
				glm::vec3 dirVec = glm::vec3(blendedZ);
				float lengthSquared = glm::dot(dirVec, dirVec);

				glm::vec4 resultDir = (lengthSquared < 0.0001f)
					? glm::vec4(0.0f)
					: glm::normalize(blendedZ);

				// Transform direction using camera matrix
				glm::vec4 transformed = g_pcm->lookAt * resultDir;
				float intensity = std::abs(transformed.x);

				// Polynomial light modulation
				const auto& clqc = glob.gleam[0].clqc;
				LM lm{ 0.0f, 1.0f };
				float gain = GLimitLm(&lm, clqc.g0 + intensity * (clqc.g1 + intensity * (clqc.g2 + intensity * clqc.g3)));

				rpl.ro.uAlpha *= gain;
			}

			if (rpl.ro.uAlpha == 0.0)
				continue;

			rpl.ro.fDynamic = glob.fDynamic;
			rpl.ro.uFog = glob.uFog;
			rpl.ro.posCenter = posCenterWorld;
			rpl.sRadius = glob.sRadius;

			if ((glob.grfglob & 4U) == 0)
				rpl.ro.darken = g_psw->rDarken;
			else
				rpl.ro.darken = 1.0;

			//rpl.ro.grfglob = glob.grfglob;
			rpl.ro.pshd = subglob.pshd;
			rpl.grfshd = subglob.pshd->grfshd;
			rpl.ro.unSelfIllum = subglob.unSelfIllum;
			rpl.ro.cvtx = subglob.cvtx;
			rpl.rp = glob.rp;
			rpl.ro.uAlpha = rpl.ro.uAlpha * g_uAlpha;

			if (rpl.ro.uAlpha != 1.0)
			{
				switch (rpl.rp)
				{
					case RP_Opaque:
					case RP_Cutout:
					case RP_OpaqueAfterProjVolume:
					case RP_CutoutAfterProjVolume:
					rpl.rp = RP_Translucent;
					break;
				}
			}

			if (glob.pdmat != nullptr)
				rpl.ro.model = baseModelMatrix * *glob.pdmat;
			else
				rpl.ro.model = baseModelMatrix;

			switch (rpl.rp)
			{
				case RP_Background:
				rpl.z = glob.gZOrder;
				break;
				case RP_Cutout:
				case RP_CutoutAfterProjVolume:
				case RP_Translucent:
				rpl.z = glm::length2(pcm->pos - glm::vec3(rpl.ro.model * glm::vec4(subglob.posCenter, 1.0f)));
				break;
			}

			if (glob.rtck != RTCK_None)
				AdjustAloRtckMat(palo, pcm, glob.rtck, &glob.posCenter, rpl.ro.model);

			SubmitRpl(&rpl);

			rpl.ro.model = baseModelMatrix;
		}

		if (glob.csubcel > 0)
		{
			if (g_fRenderCelBorders > 0)
			{
				// compute these once, OUTSIDE the subglob loop, like the original:
				const float alphaMRD = rpl.ro.uAlpha;            // written by first FInsideCmMrd
				const float mainBase = baseAlpha * alphaMRD;     // fVar33 * uAlpha
				float       mainAlpha = mainBase;

				//mainAlpha *= gleamGain;              // if any
				mainAlpha *= g_uAlpha;       // like the original

				// cel-base from MRD only (no gleam)
				float celBase = 0.0f;

				if (glob.sCelBorderMRD < glob.sMRD) {
					float dummy = 1.0f;
					const bool insideCB = FInsideCmMrd(pcm, pcm->pos - posCenterWorld, glob.sRadius, glob.sCelBorderMRD, dummy);
					celBase = insideCB ? (baseAlphaCel * alphaMRD) : 0.0f; // fVar21 * uAlpha
				}
				else {
					celBase = baseAlphaCel * alphaMRD;
				}

				// late multiply (PS2 did: uAlphaCelBorder *= final rpl.ro.uAlpha)
				const float celAlphaFinal = celBase * mainAlpha;

				// --- when submitting CELs ---
				for (int a = 0; a < glob.asubcel.size(); ++a) {
					rpl.rp = glob.rp;

					if (mainAlpha != 1.0f)
					{
						if (rpl.rp == RP_CelBorder || rpl.rp == RP_CelBorderAfterProjVolume)
							rpl.rp = RP_TranslucentCelBorder;
					}

					rpl.ro.edgeBuf   = glob.asubcel[a].edgeBuf;
					rpl.ro.edgeTex   = glob.asubcel[a].edgeTex;
					rpl.ro.edgeCount = glob.asubcel[a].edgeCount;
					rpl.ro.model     = (glob.pdmat ? baseModelMatrix * *glob.pdmat : baseModelMatrix);

					rpl.ro.uAlphaCelBorder = celAlphaFinal;   //  matches original intent

					SubmitRpl(&rpl);
				}
			}
		}
	}
}

void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha)
{
	glm::vec3 dir = *ppos1 - *ppos0;
	float length = glm::length(dir);

	if (length < 0.0001f)
		return;

	glm::vec3 forward = glm::normalize(dir);
	glm::vec3 toCamera = *ppos0 - pcm->pos;
	glm::vec3 right = glm::normalize(glm::cross(forward, toCamera));

	if (glm::length(right) < 0.0001f)
		return;

	glm::vec3 up = glm::normalize(glm::cross(right, forward));

	// Scale right and up
	right *= rWidth;
	up *= 0.01f;  // equivalent of hardcoded `0x3c23d70a` = 0.01f in float

	// Normalize forward after scaling others
	forward = glm::normalize(forward);

	glm::mat3 rot;
	rot[0] = right;
	rot[1] = forward;
	rot[2] = up;

	glm::mat4 model;
	LoadMatrixFromPosRot(*ppos0, rot, model);

	RO ro = {};
	ro.model = model;
	ro.uAlpha = uAlpha;

	//palo->pvtalo->pfnRenderAloGlobset(palo, pcm, &ro);
}

void DeleteModel(ALO *palo)
{
	for (int i = 0; i < palo->globset.aglob.size(); i++)
	{
		for (int a = 0; a < palo->globset.aglob[i].asubglob.size(); a++)
		{
			glDeleteVertexArrays(1, &palo->globset.aglob[i].asubglob[a].VAO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].VBO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].EBO);
		}

		for (int b = 0; b < palo->globset.aglob[i].asubcel.size(); b++)
		{
			glDeleteBuffers(1, &palo->globset.aglob[i].asubcel[b].edgeBuf);
			glDeleteTextures(1, &palo->globset.aglob[i].asubcel[b].edgeTex);
		}
	}
}

int GetAloSize()
{
	return sizeof(ALO);
}

void DeleteAlo(ALO *palo)
{
	delete palo;
}

std::vector <ALO*> allSWAloObjs;