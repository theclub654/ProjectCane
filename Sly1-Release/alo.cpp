#include "alo.h"

ALO* NewAlo()
{
	return new ALO{};
}

void InitAlo(ALO* palo)
{
	InitDl(&palo->dlChild, offsetof(LO, dleChild));
	InitDl(&palo->dlFreeze, offsetof(ALO, dleFreeze));

	if (palo->paloParent == nullptr)
		*(unsigned long*)&palo->bitfield = *(unsigned long*)&palo->bitfield & 0xfffffffff0ffffff | 0xa000000;
	else
		*(unsigned long*)&palo->bitfield = *(unsigned long*)&palo->bitfield & 0xfffffffff0ffffff | 0x1000000;
	
	InitLo(palo);

	byte value = 0xFF;
	byte value1 = 0x0;
	memcpy((char*)&palo->bitfield + 1, &value, sizeof(byte));
	memcpy((char*)&palo->bitfield + 2, &value, sizeof(byte));
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

void RemoveAloHierarchy(ALO* palo)
{
	DLI it{};

	it.m_pdl = &palo->dlChild;
	it.m_ibDle = palo->dlChild.ibDle;
	it.m_pdliNext = s_pdliFirst;

	s_pdliFirst = &it;

	it.m_ppv = (void**)it.m_pdl;

	// Start with the parent
	LO* current = (LO*)palo;

	while (true)
	{
		// Call OnLoAdd for the CURRENT object (parent first, then each child)
		current->pvtlo->pfnOnLoRemove(current);

		// Load next child pointer from the current "next field"
		void* next = *it.m_ppv;
		if (next == nullptr)
			break;

		// Advance iterator to the "next pointer field" inside that next object
		it.m_ppv = (void**)((uintptr_t)next + it.m_ibDle);

		// Move to the next object so we don't keep calling the parent
		current = (LO*)next;
	}

	// palo->pvtlo->pfnSendLoMessage(palo, 1, palo);

	s_pdliFirst = it.m_pdliNext;
}

void OnAloAdd(ALO* palo)
{
	if (!palo) return;

	ALO* parent = palo->paloParent;
	SW* psw = palo->psw;

	if (parent == nullptr) {
		palo->paloRoot = palo;

		if (palo->fRealClock == 0) {
			AppendDlEntry(&psw->dlMRD, palo);
			palo->bitfield.fBusy = true;
			AppendDlEntry(&psw->dlBusy, palo);

			if ((palo->pvtlo->grfcid & 0x2U) != 0) {
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
		AppendDlEntry(&psw->dlShadow, palo->pshadow);

	if (palo->pvtlo->pfnUpdateLoXfWorld != nullptr)
		palo->pvtalo->pfnUpdateAloXfWorld(palo);

	//HandleLoSpliceEvent(palo, 4, 0, nullptr);
	ResolveAlo(palo);
}

void OnAloRemove(ALO* palo)
{
	if (!palo) return;

	SW* psw = palo->psw;

	OnLoRemove(palo);

	if (palo->paloParent == nullptr) {
		if (palo->fRealClock != 0) {
			RemoveDlEntry(&psw->dlMRDRealClock, palo);
		}
		else {
			RemoveDlEntry(&psw->dlMRD, palo);

			if (palo->bitfield.fBusy == true) {
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
		RemoveDlEntry(&psw->dlShadow, palo->pshadow);
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
	palo->matOrig = palo->xf.mat;   // glm::mat3 (rotation)
	palo->posOrig = palo->xf.pos;   // glm::vec3

	/*if (palo->pvtalo && palo->pvtalo->pfnUnadjustAloRotation)
		palo->pvtalo->pfnUnadjustAloRotation(palo, &palo->matOrig);*/

		// Decompose to Euler (radians), PS2 logic
	palo->eulOrig = DecomposeRotateMatrixEuler(palo->matOrig);
}

void AdjustAloRtckMat(ALO* palo, CM* pcm, RTCK rtck, glm::vec3* pposCenter, glm::mat4& pmat)
{
	// 1) dpos = -camera X
	glm::vec3 camX  = glm::vec3(pcm->mat[1]);
	glm::vec3 dpos  = camX;
	glm::vec3 dposN = glm::normalize(dpos);

	// 2) Rotate object Z to dpos (Z-normal billboard)
	glm::mat3 R1;
	glm::vec3 z0 = glm::vec3(pmat[2]);
	BuildRotateVectorsMatrix(&z0, &dposN, &R1);

	glm::mat4 D1;
	LoadMatrixFromPosRot(g_vecZero, R1, D1);

	// Move to center frame, apply rotation
	glm::mat4 M = pmat;
	M[3] = glm::vec4(glm::vec3(M[3]) - *pposCenter, M[3].w);
	glm::mat4 alignedMat = D1 * M;

	// 3) Reflect current X about dpos (swapped axis vs original)
	glm::vec3 vX = glm::vec3(alignedMat[0]);
	float s = 2.0f * glm::dot(vX, dposN);
	glm::vec3 vXr = vX - s * dposN;

	// 4) Write back local_e0 to pmat, keeping X column from local_e0
	pmat = alignedMat;
	pmat[0] = glm::vec4(vX, alignedMat[0].w);

	// 5) Rotate reflected X to camera Z, then compose back around +center
	glm::vec3 camZ = glm::vec3(pcm->mat[2]);
	glm::mat3 R2;
	glm::vec3 vXrN = glm::normalize(vXr);
	glm::vec3 camZN = glm::normalize(camZ);
	BuildRotateVectorsMatrix(&vXrN, &camZN, &R2);

	glm::mat4 D2;
	LoadMatrixFromPosRot(*pposCenter, R2, D2);

	// 6) Final result
	pmat = D2 * pmat;
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

	s_pdliFirst = it.m_pdliNext;
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
		for (int a = 0; a < palo->globset.aglob[i].asubglob.size(); a++)
			SetRpCount(palo->globset.aglob[i].rp, palo->globset.aglob[i].asubglob[a].pshd->grfshd);

		numRoCel += palo->globset.aglob[i].csubcel;
		for (int b = 0; b < palo->globset.aglob[i].asubcel.size(); b++)
			SetRpCount(palo->globset.aglob[i].rp, 0);
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

	glm::vec3 posWorld = palo->xf.posWorld;
	glm::mat3 matWorld = palo->xf.matWorld;

	palo->pvtalo->pfnRemoveLo(palo);

	ConvertAloPos(nullptr, paloParent, posWorld, palo->xf.pos);
	ConvertAloMat(nullptr, paloParent, matWorld, palo->xf.mat);

	UpdateAloOrig(palo);
	palo->paloParent = paloParent;

	palo->pvtlo->pfnAddLo(palo);
}

void ApplyAloProxy(ALO* palo, PROXY* pproxyApply)
{
	glm::vec3 posWorld{};
	ConvertAloPos((ALO*)pproxyApply, nullptr, palo->xf.pos, posWorld);
	palo->pvtalo->pfnTranslateAloToPos(palo, posWorld);

	glm::mat3 matWorld{};
	ConvertAloMat((ALO*)pproxyApply, nullptr, palo->xf.mat, matWorld);
	palo->pvtalo->pfnRotateAloToMat(palo, matWorld);

	palo->posOrig = palo->xf.pos;
	palo->matOrig = palo->xf.mat;
}

void BindAlo(ALO* palo)
{
	BindAloAlox(palo);
	UpdateAloOrig(palo);
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

}

void UpdateAloXfWorld(ALO* palo)
{
	palo->pvtalo->pfnUpdateAloXfWorldHierarchy(palo);
}

void UpdateAloXfWorldHierarchy(ALO* palo)
{
	if (!palo) return;

	ALOX* palox = palo->palox.get();

	// ------------------------------------------------------------
	// 1) Compute posWorld / matWorld
	// ------------------------------------------------------------

	// Choose parent for POSITION
	ALO* parentPos = palo->paloParent;

	if (palox)
	{
		const uint32_t f = palox->grfalox;

		if ((f & 0xCU) != 0)
		{
			if ((f & 4U) != 0)
				parentPos = palox->scj.paloSchRot;
		}
	}

	// posWorld
	if (!parentPos)
		palo->xf.posWorld = palo->xf.pos;
	else
		palo->xf.posWorld = parentPos->xf.matWorld * palo->xf.pos + parentPos->xf.posWorld;

	// Choose parent for ROTATION
	ALO* parentRot = nullptr;

	if (!palox)
	{
		parentRot = palo->paloParent;
	}
	else
	{
		const uint32_t f = palox->grfalox;
		parentRot = ((f & 8U) == 0) ? palo->paloParent : palox->scj.paloSchRot;
	}

	// matWorld
	if (!parentRot)
		palo->xf.matWorld = palo->xf.mat;
	else
		palo->xf.matWorld = parentRot->xf.matWorld * palo->xf.mat;

	// ------------------------------------------------------------
	// 2) VISMAP / grfzon update (replace with your real flag check)
	// ------------------------------------------------------------

	const bool needsVismapClip = (((*(long*)&palo->bitfield << 8) >> 0x20 & 3U) == 2);
	
	if (needsVismapClip)
	{
		VISMAP* pvismap = palo->psw ? palo->psw->pvismap : nullptr;

		if (!pvismap)
			palo->grfzon = 0x0FFFFFFF;
		else
			ClipVismapSphereOneHop(pvismap, &palo->xf.posWorld, palo->sRadiusRenderAll, &palo->grfzon);
	}

	// ------------------------------------------------------------
	// 3) Shadow update
	// ------------------------------------------------------------
	if (palo->pshadow)
	{
		/*SetShadowCastPosition(palo->pshadow, &palo->xf.posWorld);

		SHD* pshd = palo->pshadow->pshd;
		if (pshd && pshd->shdk == 3)
		{
			glm::vec3 normalCast = -palo->xf.matWorld[2];
			SetShadowCastNormal(palo->pshadow, &normalCast);

			glm::vec3 up = palo->xf.matWorld[1];
			SetShadowFrustrumUp(palo->pshadow, &up);
		}*/
	}

	// ------------------------------------------------------------
	// 4) Recurse children (LO vtable function pointer)
	// ------------------------------------------------------------
	for (ALO* child = palo->dlChild.paloFirst; child; child = child->dleChild.paloNext)
	{
		auto* vt = child->pvtalo;
		if (vt && vt->pfnUpdateAloXfWorldHierarchy)
			vt->pfnUpdateAloXfWorldHierarchy(child);
	}

	// ------------------------------------------------------------
	// 5) IK / scheduler invalidation
	// ------------------------------------------------------------
	/*palox = palo->palox.get();
	if (palox)
	{
		const uint32_t f = palox->grfalox;

		if ((f & 0x8020U) == 0x8020U)
		{
			ALO* shoulder = palox->ikh.paloShoulder;
			ALO* elbow = palox->ikh.paloElbow;

			if (shoulder && shoulder->palox)
				shoulder->palox->ikj.fInvalid = 1;

			if (elbow && elbow->palox)
				elbow->palox->ikj.fInvalid = 1;
		}

		if ((f & 0x8100U) == 0x8100U && palox->ikh.grfik > 0)
		{
			const int count = palox->ikh.grfik;
			ALO** apalo = palox->sch.apalo;

			for (int i = 0; i < count; ++i)
			{
				ALO* a = apalo ? apalo[i] : nullptr;
				if (!a) continue;

				ALOX* ax = a->palox.get();
				if (!ax) continue;

				if (palo == ax->scj.paloSchRot)
					ax->scj.fInvalidRot = 1;

				if (palo == ax->scj.paloSchPos)
					ax->scj.fInvalidPos = 1;
			}
		}
	}*/
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

			// If there is a next object, update the walker’s pointer to its next link
			dlBusyWalker.m_ppv = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(currentObject) + dlBusyWalker.m_ibDle);
		}

		s_pdliFirst = dlBusyWalker.m_pdliNext;
	}
}

void TranslateAloToPos(ALO* palo, glm::vec3& ppos)
{
	palo->xf.pos = ppos;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);
}

void ConvertAloPos(ALO* paloFrom, ALO* paloTo, glm::vec3& pposFrom, glm::vec3& pposTo)
{
	if (paloFrom == paloTo) {
		pposTo = pposFrom;
		return;
	}

	// local -> world (or already world if paloFrom == nullptr)
	glm::vec3 world = pposFrom;
	if (paloFrom) 
		world = paloFrom->xf.matWorld * pposFrom + paloFrom->xf.posWorld;

	// world -> paloTo local (or keep world if paloTo == nullptr)
	if (paloTo) {
		glm::vec3 delta = world - paloTo->xf.posWorld;
		glm::mat3 invRot = glm::transpose(paloTo->xf.matWorld);
		pposTo = invRot * delta;
	}
	else 
		pposTo = world;
}

void ConvertAloVec(ALO* paloFrom, ALO* paloTo, glm::vec3* pvecFrom, glm::vec3* pvecTo)
{
	glm::vec3 vecWorld = *pvecFrom;

	// Transform from local to world space if paloFrom is valid and different from paloTo
	if (paloFrom && paloFrom != paloTo)
		vecWorld = paloFrom->xf.matWorld * (*pvecFrom);

	// Transform from world to local space of paloTo if it's valid
	if (paloTo)
	{
		glm::mat3 invMat = glm::transpose(paloTo->xf.matWorld);
		*pvecTo = invMat * vecWorld;
	}
	else
		*pvecTo = vecWorld;
}

void RotateAloToMat(ALO* palo, glm::mat3& pmat)
{
	palo->xf.mat = pmat;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);
}

void ConvertAloMat(ALO* paloFrom, ALO* paloTo, glm::mat3& pmatFrom, glm::mat3& pmatTo)
{
	if (paloFrom == paloTo) {
		pmatTo = pmatFrom;
		return;
	}

	glm::mat3 world = pmatFrom;

	if (paloFrom) 
		world = paloFrom->xf.matWorld * pmatFrom;

	if (paloTo) 
		pmatTo = glm::transpose(paloTo->xf.matWorld) * world;
	else 
		pmatTo = world;
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

void SetAloCelRgba(ALO* palo, RGBA prgba)
{
	palo->globset.rgbaCel.r = prgba.bRed   / 255.0;
	palo->globset.rgbaCel.g = prgba.bGreen / 255.0;
	palo->globset.rgbaCel.b = prgba.bBlue  / 255.0;
	palo->globset.rgbaCel.a = prgba.bAlpha / 255.0;

	palo->globset.grfglobset = palo->globset.grfglobset | 2;
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

void AddAloHierarchy(ALO *palo)
{
	DLI it{};

	it.m_pdl = &palo->dlChild;
	it.m_ibDle = palo->dlChild.ibDle;
	it.m_pdliNext = s_pdliFirst;

	s_pdliFirst = &it;

	it.m_ppv = (void**)it.m_pdl;

	// Start with the parent
	LO* current = (LO*)palo;

	while (true)
	{
		// Call OnLoAdd for the CURRENT object (parent first, then each child)
		current->pvtlo->pfnOnLoAdd(current);

		// Load next child pointer from the current "next field"
		void* next = *it.m_ppv;
		if (next == nullptr)
			break;

		// Advance iterator to the "next pointer field" inside that next object
		it.m_ppv = (void**)((uintptr_t)next + it.m_ibDle);

		// Move to the next object so we don't keep calling the parent
		current = (LO*)next;
	}

	// palo->pvtlo->pfnSendLoMessage(palo, 1, palo);

	s_pdliFirst = it.m_pdliNext;
}

void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis)
{
	// Model matrix
	palo->xf.mat = pbis->ReadMatrix();
	palo->xf.pos = pbis->ReadVector();
	//

	palo->bitfield.zons = pbis->U8Read() & 0x03;
	palo->bitfield.viss = pbis->U8Read() & 0x03;
	palo->bitfield.mrds = pbis->U8Read() & 0x03;

	palo->grfzon = pbis->U32Read();
	palo->sMRD = pbis->F32Read();
	palo->sCelBorderMRD = pbis->F32Read();
	palo->sRadiusRenderSelf = pbis->F32Read();
	palo->sRadiusRenderAll = pbis->F32Read();

	if (palo->sMRD == 3.402823e+38f) {
		palo->sMRD = 1.0e+10f;
	}

	if (palo->sCelBorderMRD == 3.402823e+38f) {
		palo->sCelBorderMRD = (palo->sMRD > 2000.0f) ? 2000.0f : palo->sMRD;
	}
	
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
	const uint32_t grfalox = pbis->U32Read();

	if (grfalox == 0)
		return;

	ALOX alox;
	palo->palox = std::make_shared <ALOX>(alox);

	palo->palox->grfalox = grfalox;

	int unk_1;

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

	if ((grfalox & 0x20) != 0)
	{
		unk_1 = pbis->S16Read();
		pbis->ReadVector(); // Read Vector
		pbis->ReadVector(); // Read Vector
		pbis->F32Read();
	}

	if ((grfalox & 0x40) != 0)
	{
		unk_1 = pbis->S16Read();
		unk_1 = pbis->S16Read();
	}

	if ((grfalox & 0x80) != 0)
	{
		pbis->U8Read();
	}
}

void BindAloAlox(ALO* palo)
{

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
	PostLoLoad(palo);
	PostGlobsetLoad(&palo->globset, palo);

	// --- Iterate children DL and call each entry's fn at vtbl+0x50 (was inlined dl.h) ---
	DLI dlBusyWalker{};

	dlBusyWalker.m_ibDle = palo->dlChild.ibDle;
	dlBusyWalker.m_pdliNext = s_pdliFirst;
	dlBusyWalker.m_pdl = &palo->dlChild;

	ALO* currentObject = palo->dlChild.paloFirst;

	// Only valid if we have a first element
	dlBusyWalker.m_ppv = currentObject ? (void**)((uintptr_t)currentObject + dlBusyWalker.m_ibDle) : nullptr;

	s_pdliFirst = &dlBusyWalker;

	while (currentObject != nullptr)
	{
		if (currentObject->pvtalo->pfnPostAloLoad)
			currentObject->pvtalo->pfnPostAloLoad(currentObject);

		currentObject = (ALO*)*dlBusyWalker.m_ppv;

		// Guard before computing next pointer-field address
		dlBusyWalker.m_ppv = currentObject ? (void**)((uintptr_t)currentObject + dlBusyWalker.m_ibDle) : nullptr;
	}

	s_pdliFirst = dlBusyWalker.m_pdliNext;
}

void UpdateAlo(ALO* palo, float dt)
{
	UpdateGlobset(&palo->globset, palo, dt);
}

void RenderAloAll(ALO* palo, CM* pcm, RO* pro)
{
	if (g_fBsp != 0)
	{
		if ((palo->grfzon & pcm->grfzon) != pcm->grfzon)
			return;
	}

	glm::vec3 posWorld{};

	if (pro != nullptr)
		posWorld = pro->model[3];
	else
		posWorld = palo->xf.posWorld;

	if (!SphereInFrustum(pcm->frustum, posWorld, palo->sRadiusRenderAll))
		return;

	float uAlpha = 1.0f;
	if (!FInsideCmMrd(pcm, posWorld - pcm->pos, palo->sRadiusRenderAll, palo->sMRD, uAlpha))
		return;

	RO roLocal{};
	RO* proOriginal = pro;

	auto ensureLocal = [&]() {
		if (proOriginal == &roLocal) return;
		DupAloRo(palo, proOriginal, &roLocal);
		proOriginal = &roLocal;
	};

	if (uAlpha != 1.0f) {
		ensureLocal();
		proOriginal->uAlpha *= uAlpha;
	}

	float uCel = uAlpha;
	if (palo->sCelBorderMRD < palo->sMRD) {
		float dummy = 1.0f;
		if (!FInsideCmMrd(pcm, posWorld - pcm->pos, palo->sRadiusRenderAll, palo->sCelBorderMRD, dummy))
			uCel = 0.0f;
	}
	if (uCel != 1.0f) {
		ensureLocal();
		proOriginal->uAlphaCelBorder *= uCel;
	}

	if (palo->pfader) {
		ensureLocal();
		//proOriginal->uAlpha *= palo->pfader->uAlpha;
	}

	palo->pvtalo->pfnRenderAloSelf(palo, pcm, proOriginal);

	for (ALO* child = palo->dlChild.paloFirst; child; child = child->dleChild.paloNext)
	{
		if (!(child->pvtlo->grfcid & 1U))
			continue;

		if (!proOriginal)
		{
			child->pvtalo->pfnRenderAloAll(child, pcm, nullptr);
			continue;
		}

		// match original gate (palox + flags), not ppxr
		const bool useWorldProxy = child->palox && ((child->palox->grfalox & 0xC) != 0);

		// 1) Build child's matrix (different source depending on gate)
		glm::mat4 childMat;
		if (!useWorldProxy)
			LoadMatrixFromPosRot(child->xf.pos, child->xf.mat, childMat);
		else
			LoadMatrixFromPosRot(child->xf.posWorld, child->xf.matWorld, childMat);

		// 2) Choose parent matrix for child
		glm::mat4 parentForChild = proOriginal->model;
		if (useWorldProxy)
		{
			// Must be inverse of parent's true world transform (TR)
			glm::mat4 invParentWorld = glm::inverse(palo->xf.matWorld); // OK only if matWorld is full mat4 TR
			parentForChild = proOriginal->model * invParentWorld;
		}

		// 3) Final child transform
		RO roChild{};
		roChild.model = parentForChild * childMat;

		// 4) Inherit alpha
		roChild.uAlpha = proOriginal->uAlpha;
		roChild.uAlphaCelBorder = proOriginal->uAlphaCelBorder;

		child->pvtalo->pfnRenderAloAll(child, pcm, &roChild);
	}
}

void RenderAloSelf(ALO* palo, CM* pcm, RO* pro)
{
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
	RPLCEL rplCel{};

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

		auto& glob  = palo->globset.aglob[i];
		auto& globi = palo->globset.aglobi[i];

		glm::vec4 posCenterWorld = baseModelMatrix * glm::vec4(glob.posCenter, 1.0f);
		glm::vec3 dpos = glm::vec3(posCenterWorld) - pcm->pos;

		if (!SphereInFrustum(pcm->frustum, posCenterWorld, glob.sRadius))
			continue;

		float mrdAlphaDummy = 1.0f;
		if (!FInsideCmMrd(pcm, glm::vec4(dpos, 0.0f), glob.sRadius, glob.sMRD, mrdAlphaDummy))
			continue;
		
		float alpha = baseAlpha;
		
		float uAlpha = 1.0f;
		alpha *= uAlpha;

		// Gleam (if any) affects alpha
		if (glob.gleam != nullptr)
		{
			glm::vec3 n = glob.gleam->normal;
			glm::vec3 X = glm::vec3(baseModelMatrix[0]); // model X column
			glm::vec3 Y = glm::vec3(baseModelMatrix[1]); // model Y column
			glm::vec3 Z = glm::vec3(baseModelMatrix[2]); // model Z column

			glm::vec3 v = X * n.x + Y * n.y + Z * n.z;

			// Normalize with PS2-style guard
			float len2 = glm::dot(v, v);
			glm::vec3 dir = (len2 < 1e-4f) ? glm::vec3(0.0f) : (v / std::sqrt(len2));

			// Intensity = abs(dot(dir, camera X))
			glm::vec3 camX = glm::vec3(g_pcm->mat[0]);
			float intensity = std::abs(glm::dot(dir, camX));

			// Polynomial gain g0 + i*(g1 + i*(g2 + i*g3))
			const auto& c = glob.gleam->clqc; // has g0,g1,g2,g3
			float gain = c.g0 + intensity * (c.g1 + intensity * (c.g2 + intensity * c.g3));

			// Limit and apply
			gain = GLimitLm(&g_lmZeroOne, gain);
			alpha *= gain;
		}

		// Optional glbi fade/unfade (if you implement it)
		// target = 0.5 before tUnfade, else 1.0
		float target = (g_clock.tReal < globi.tUnfade) ? 0.5f : 1.0f;

		if (globi.uAlpha != target)
			globi.uAlpha = GSmooth(globi.uAlpha, target, g_clock.dt, &g_smpAlphaFade, nullptr);

		alpha *= globi.uAlpha;

		// Global multiplier (your equivalent of g_droSnap.uAlpha / g_uAlpha)
		alpha *= g_uAlpha;

		if (alpha <= 0.0f)
			continue;

		rpl.ro.uAlpha = alpha;

		rpl.ro.uFog = glob.uFog;

		if ((glob.grfglob & 4U) == 0)
			rpl.ro.darken = g_psw->rDarken;
		else
			rpl.ro.darken = 1.0;

		if (glob.pdmat != nullptr)
			rpl.ro.model = baseModelMatrix * *glob.pdmat;
		else
			rpl.ro.model = baseModelMatrix;

		if (glob.pwrbg != nullptr && glob.pwrbg->pwr != nullptr)
		{
			rpl.ro.warpType = glob.pwrbg->warpType;
			rpl.ro.warpCmat = glob.pwrbg->pwr->cmat;

			const size_t count = glob.pwrbg->cmat;

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
			}

		}
		else
			rpl.ro.warpType = WARP_NONE;

		if (glob.rtck != RTCK_None)
			AdjustAloRtckMat(palo, pcm, glob.rtck, (glm::vec3*)&posCenterWorld, rpl.ro.model);

		rpl.rp = glob.rp;

		if (rpl.ro.uAlpha < 1.0)
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

		switch (rpl.rp)
		{
			case RP_Background:
			rpl.z = -glm::length(pcm->pos - glm::vec3(rpl.ro.model * glm::vec4(glob.posCenter, 1.0f)));
			break;
			case RP_Cutout:
			case RP_CutoutAfterProjVolume:
			case RP_Translucent:
			rpl.z = glm::length(pcm->pos - glm::vec3(rpl.ro.model * glm::vec4(glob.posCenter, 1.0f)));
			break;
		}

		for (auto& subglob : glob.asubglob)
		{
			rpl.VAO  = subglob.VAO;
			rpl.cvtx = subglob.cvtx;
			
			rpl.pshd = subglob.pshd;

			if (subglob.pshd->shdk == SHDK_ThreeWay)
			{
				rpl.PFNBIND = BindThreeWay;
				rpl.ro.rko = RKO_ThreeWay;
				rpl.ro.fDynamic = glob.fDynamic;
				rpl.ro.posCenter = posCenterWorld;
				rpl.ro.sRadius = glob.sRadius;
				rpl.ro.unSelfIllum = subglob.unSelfIllum;
			}
			else
			{
				rpl.PFNBIND = BindOneWay;
				rpl.ro.rko = RKO_OneWay;
			}

			if (subglob.usesUvAnim && subglob.uvSai)
			{
				rpl.ro.uvOffsets.x = subglob.uvSai->tcx.du;
				rpl.ro.uvOffsets.y = subglob.uvSai->tcx.dv;
			}
			else
			{
				rpl.ro.uvOffsets.x = 0.0;
				rpl.ro.uvOffsets.y = 0.0;
			}
			
			if (rpl.ro.warpType != WARP_NONE)
			{
				rpl.ro.warpCvtx   = subglob.pwarp->vertexCount;
				rpl.ssboWarpState = subglob.pwarp->ssboState;
			}
			
			SubmitRpl(&rpl);
		}


		if (g_fRenderCelBorders > 0 && glob.csubcel > 0)
		{
			float celBase = baseAlphaCel * uAlpha;

			if (glob.sCelBorderMRD < glob.sMRD)
			{
				float cbDummy = 1.0f;
				const bool insideCB = FInsideCmMrd(pcm, glm::vec4(dpos, 0.0f), glob.sRadius, glob.sCelBorderMRD, cbDummy);

				if (!insideCB)
					celBase = 0.0f;
				else
					celBase = baseAlphaCel * uAlpha;
			}

			float celAlphaFinal = celBase * alpha;

			if ((palo->globset.grfglobset & 2) == 0)
				celAlphaFinal *= g_rgbaCel.a;
			else
				celAlphaFinal *= palo->globset.rgbaCel.a;

			if (celAlphaFinal > 0.0f)
			{
				rplCel.rocel.model = (glob.pdmat) ? (baseModelMatrix * *glob.pdmat) : baseModelMatrix;

				rplCel.rocel.celRgba = ((palo->globset.grfglobset & 2) == 0) ? g_rgbaCel : palo->globset.rgbaCel;
				rplCel.rocel.uAlphaCelBorder = celAlphaFinal;

				rplCel.rp = glob.rp;

				if (alpha != 1.0f)
				{
					if (rplCel.rp == RP_CelBorder || rplCel.rp == RP_CelBorderAfterProjVolume)
						rplCel.rp = RP_TranslucentCelBorder;
				}

				for (auto& sc : glob.asubcel)
				{
					rplCel.edgeSSBO  = sc.edgeSSBO;
					rplCel.edgeCount = sc.edgeCount;

					SubmitRplCel(&rplCel);
				}
			}
		}
	}
}

void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha)
{
	if (!palo || !pcm || !ppos0 || !ppos1) return;

    glm::vec3 p0 = *ppos0;
    glm::vec3 p1 = *ppos1;

    glm::vec3 dir   = p1 - p0;
    glm::vec3 toCam = p0 - pcm->pos;

    glm::vec3 axis1 = glm::cross(toCam, dir);
    float axis1Len = glm::length(axis1);

    // Original effectively requires axis1Len > 0.01 to proceed/render
    if (axis1Len <= 0.01f)
        return;

    axis1 /= axis1Len;

    glm::vec3 axis0 = glm::cross(axis1, dir);
    float axis0Len = glm::length(axis0);
    if (axis0Len < 0.0001f)
        return;
    axis0 /= axis0Len;

    // scale axes like original
    axis1 *= rWidth;
    glm::vec3 axis2 = dir * 0.01f;   // IMPORTANT: not dirUnit

    glm::mat3 rot;
    rot[0] = axis0;
    rot[1] = axis1;
    rot[2] = axis2;

    glm::mat4 model;
    LoadMatrixFromPosRot(p0, rot, model);

    RO ro{};
    ro.model = model;
    ro.uAlpha = uAlpha;
    ro.uAlphaCelBorder = uAlpha;

    palo->pvtalo->pfnRenderAloGlobset(palo, pcm, &ro);
}

void DeleteModel(ALO* palo)
{
	for (int i = 0; i < palo->globset.aglob.size(); i++)
	{
		GLOB &glob = palo->globset.aglob[i];

		for (int a = 0; a < glob.asubglob.size(); a++)
		{
			SUBGLOB &subglob = glob.asubglob[a];

			glDeleteVertexArrays(1, &subglob.VAO);
			glDeleteBuffers(1, &subglob.VBO);
			glDeleteBuffers(1, &subglob.EBO);

			// ---- DELETE WARP SSBO ----
			if (subglob.pwarp && subglob.pwarp->ssboState != 0)
			{
				glDeleteBuffers(1, &subglob.pwarp->ssboState);
				subglob.pwarp->ssboState = 0;
				subglob.pwarp->ssboStateBytes = 0;
				subglob.pwarp->state.clear(); // CPU mirror (optional)
			}
		}

		for (int b = 0; b < glob.asubcel.size(); b++)
		{
			glDeleteBuffers(1, &glob.asubcel[b].edgeSSBO);
			glob.asubcel[b].edgeSSBO = 0;
		}
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