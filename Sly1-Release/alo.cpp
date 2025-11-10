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

void RemoveAloHierarchy(ALO* palo)
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

void AdjustAloRtckMat(ALO *palo, CM *pcm, RTCK rtck, glm::vec3 *pposCenter, glm::mat4 &pmat)
{
	// 1) dpos = -camera X
	glm::vec3 camX = glm::vec3(pcm->mat[1]);
	glm::vec3 dpos = camX;
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
	parentPalo.m_ppv = (void**)((uintptr_t)plo + parentPalo.m_ibDle);

	while (plo != nullptr)
	{
		// Clones ALO object
		PloCloneLo(plo, palo->psw, palo);
		// Loads next ALO object to clone
		plo = (LO*)*parentPalo.m_ppv;
		// Loads next ALO object to clone after this one
		parentPalo.m_ppv = (void**)((uintptr_t)plo + parentPalo.m_ibDle);
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
		for (int a = 0; a < palo->globset.aglob[i].asubglob.size(); a++)
			SetRpCount(palo->globset.aglob[i].rp, palo->globset.aglob[i].asubglob[a].pshd->grfshd);

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
	if (palo->palox == nullptr)
	{
	UpdateTrans:
		if (palo->paloParent == nullptr)
		{
			palo->xf.posWorld = palo->xf.pos;
			palo->xf.matWorld = palo->xf.mat;
		}

		else
		{
			glm::vec3 parentPos = palo->paloParent->xf.posWorld;
			glm::mat3 parentRot = palo->paloParent->xf.matWorld;

			// Transform local position by parent rotation and add parent position
			palo->xf.posWorld = parentRot * palo->xf.pos + parentPos;

			// Calculate world rotation by combining parent and local rotations (row-major)
			glm::mat3 localRot = palo->xf.mat;
			palo->xf.matWorld = parentRot * localRot;
		}
	}

	else
	{
		if ((palo->palox->grfalox & 0xCU) == 0)
		{
			palo->paloParent = palo->paloParent;
			goto UpdateTrans;
		}

		if (palo->paloParent == nullptr)
		{
			palo->xf.posWorld = palo->xf.pos;
		}

		else
		{
			// Load parent world rotation (row-major 3x3 matrix)
			glm::mat3 parentMatWorld = palo->paloParent->xf.matWorld;

			// Load parent world position
			glm::vec3 parentPosWorld = palo->paloParent->xf.posWorld;

			// Load local position of palo (child)
			glm::vec3 localPos = palo->xf.pos;

			// Calculate world position: matWorld * pos + posWorld
			glm::vec3 worldPos = parentMatWorld * localPos + parentPosWorld;

			// Store the result in palo's posWorld
			palo->xf.posWorld = worldPos;
		}

		if (palo->paloParent != nullptr)
		{
			// Load parent world matrix components (row-major)
			glm::mat3 parentMatWorld = palo->paloParent->xf.matWorld;

			// Load child local matrix components (row-major)
			glm::mat3 childMat = palo->xf.mat;

			// Process the first row of the child matrix
			glm::vec3 row1 = childMat[0];  // This corresponds to the 1st column of the matrix in SIMD
			glm::vec3 result1 = parentMatWorld * row1;  // Matrix-vector multiplication
			palo->xf.posWorld = result1;  // Store the result

			// Process the second row of the child matrix
			glm::vec3 row2 = childMat[1];  // This corresponds to the 2nd column of the matrix in SIMD
			glm::vec3 result2 = parentMatWorld * row2;
			palo->xf.posWorld += result2;  // Accumulate the result

			// Process the third row of the child matrix
			glm::vec3 row3 = childMat[2];  // This corresponds to the 3rd column of the matrix in SIMD
			glm::vec3 result3 = parentMatWorld * row3;
			palo->xf.posWorld += result3;  // Accumulate the result

			// Update the matWorld field of palo (store the results in matWorld as in the original code)
			palo->xf.matWorld[0] = result1;
			palo->xf.matWorld[1] = result2;
			palo->xf.matWorld[2] = result3;
			goto UpdateTrans;
		}

		palo->xf.matWorld = palo->xf.mat;
	}

	ALO* object = palo->dlChild.paloFirst;

	if (object == nullptr)
	{

	}

	else
	{
		while (true)
		{
			if (object->pvtalo->pfnUpdateAloXfWorldHierarchy == nullptr)
				object = object->dleChild.paloNext;

			else
			{
				object->pvtalo->pfnUpdateAloXfWorldHierarchy(object);
				object = object->dleChild.paloNext;
			}

			if (object == nullptr)
				break;
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
	}
}

void TranslateAloToPos(ALO* palo, glm::vec3& ppos)
{
	palo->xf.pos = ppos;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);
}

void ConvertAloPos(ALO* paloFrom, ALO* paloTo, glm::vec3& pposFrom, glm::vec3& pposTo)
{
	glm::vec3 worldPos;

	// Transform local to world space if `paloFrom` is specified and different from `paloTo`
	if (paloFrom != paloTo)
	{
		if (paloFrom != nullptr)
		{
			worldPos = paloFrom->xf.matWorld * (pposFrom)+paloFrom->xf.posWorld;
			pposFrom = worldPos;
		}

		// Convert world position to local position relative to `paloTo`
		if (paloTo != nullptr)
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

void ConvertAloVec(ALO* paloFrom, ALO* paloTo, glm::vec3* pvecFrom, glm::vec3* pvecTo)
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

void RotateAloToMat(ALO* palo, glm::mat3& pmat)
{
	palo->xf.mat = pmat;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);
}

void ConvertAloMat(ALO* paloFrom, ALO* paloTo, glm::mat3& pmatFrom, glm::mat3& pmatTo)
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
		void* nextParentObject = *plo.m_ppv;

		plo.m_ppv = (void**)((uintptr_t)nextParentObject + plo.m_ibDle);

		// If parent doesnt have a child break
		if (nextParentObject == nullptr) break;
	}

	s_pdliFirst = plo.m_pdliNext;
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
	GRFALOX grfalox = pbis->U32Read();

	if (grfalox != 0)
	{
		ALOX alox;
		palo->palox = std::make_shared <ALOX>(alox);

		palo->palox->grfalox = grfalox;

		int unk_1;

		if (grfalox & 1)
		{
			pbis->ReadMatrix();
		}

		if (grfalox & 2)
		{
			pbis->ReadMatrix();
		}

		if (((grfalox & 0xc) != 0) && (unk_1 = pbis->S16Read() != -1))
		{

		}

		if ((grfalox & 0x10) != 0)
			unk_1 = pbis->S16Read();

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
}

void BindAloAlox(ALO* palo)
{

}

void SnipAloObjects(ALO *palo, int csnip, SNIP *asnip)
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

void RenderAloSelf(ALO *palo, CM *pcm, RO *pro)
{
	palo->pvtalo->pfnRenderAloGlobset(palo, pcm, pro);
}

void DupAloRo(ALO *palo, RO *proOrig, RO *proDup)
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
	RO proDup{};
	float uAlpha{};

	// Duplicate rendering object from original
	DupAloRo(palo, pro, &proDup);

	glm::mat4 baseModelMatrix = proDup.model;

	float baseAlpha = proDup.uAlpha;
	float baseAlphaCel = proDup.uAlphaCelBorder;

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

		if (!SphereInFrustum(pcm->frustum, posCenterWorld, glob.sRadius))
			continue;

		if (!FInsideCmMrd(pcm, glm::vec4(pcm->pos, 1.0) - posCenterWorld, glob.sRadius, glob.sMRD, uAlpha))
			continue;

		for (auto& subglob : glob.asubglob)
		{
			subglob.rpl.ro.uAlpha = uAlpha;
			subglob.rpl.ro.uAlpha *= baseAlpha;

			if (glob.gleam.size() != 0)
			{
				glm::vec3 n = glob.gleam[0].normal;
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
				const auto& c = glob.gleam[0].clqc; // has g0,g1,g2,g3
				float gain = c.g0 + intensity * (c.g1 + intensity * (c.g2 + intensity * c.g3));

				// Limit and apply
				gain = GLimitLm(&g_lmZeroOne, gain);
				subglob.rpl.ro.uAlpha *= gain;
			}

			if (subglob.rpl.ro.uAlpha == 0.0)
				continue;

			subglob.rpl.ro.posCenter = posCenterWorld;

			if ((glob.grfglob & 4U) == 0)
				subglob.rpl.ro.darken = g_psw->rDarken;
			else
				subglob.rpl.ro.darken = 1.0;

			subglob.rpl.rp = glob.rp;
			subglob.rpl.ro.uAlpha = subglob.rpl.ro.uAlpha * g_uAlpha;

			if (subglob.rpl.ro.uAlpha != 1.0)
			{
				switch (subglob.rpl.rp)
				{
					case RP_Opaque:
					case RP_Cutout:
					case RP_OpaqueAfterProjVolume:
					case RP_CutoutAfterProjVolume:
					subglob.rpl.rp = RP_Translucent;
					break;
				}
			}

			if (glob.pdmat != nullptr)
				subglob.rpl.ro.model = baseModelMatrix * *glob.pdmat;
			else
				subglob.rpl.ro.model = baseModelMatrix;

			switch (subglob.rpl.rp)
			{
				case RP_Background:
				subglob.rpl.z = -glm::length2(pcm->pos - glm::vec3(subglob.rpl.ro.model * glm::vec4(glob.posCenter, 1.0f)));
				break;
				case RP_Cutout:
				case RP_CutoutAfterProjVolume:
				case RP_Translucent:
				subglob.rpl.z = glm::length2(pcm->pos - glm::vec3(subglob.rpl.ro.model * glm::vec4(subglob.posCenter, 1.0f)));
				break;
			}

			if (glob.rtck != RTCK_None)
				AdjustAloRtckMat(palo, pcm, glob.rtck, (glm::vec3*)&posCenterWorld, subglob.rpl.ro.model);
			
			SubmitRpl(&subglob.rpl);
		}

		if (glob.csubcel > 0)
		{
			if (g_fRenderCelBorders > 0)
			{
				const float alphaMRD = uAlpha;
				const float mainBase = baseAlpha * alphaMRD;
				float       mainAlpha = mainBase;

				//mainAlpha *= gleamGain;
				mainAlpha *= g_uAlpha;

				float celBase = 0.0f;

				if (glob.sCelBorderMRD < glob.sMRD) {
					float dummy = 1.0f;
					const bool insideCB = FInsideCmMrd(pcm, glm::vec4(pcm->pos, 1.0) - posCenterWorld, glob.sRadius, glob.sCelBorderMRD, dummy);
					celBase = insideCB ? (baseAlphaCel * alphaMRD) : 0.0f;
				}
				else {
					celBase = baseAlphaCel * alphaMRD;
				}

				float celAlphaFinal = celBase * mainAlpha;
				celAlphaFinal *= g_rgbaCel.a;

				if (celAlphaFinal > 0.0)
				{
					for (int a = 0; a < glob.asubcel.size(); ++a)
					{
						glob.asubcel[a].rplCel.rp = glob.rp;

						if (mainAlpha != 1.0f)
						{
							if (glob.asubcel[a].rplCel.rp == RP_CelBorder || glob.asubcel[a].rplCel.rp == RP_CelBorderAfterProjVolume)
								glob.asubcel[a].rplCel.rp = RP_TranslucentCelBorder;
						}

						if (glob.pdmat != nullptr)
							glob.asubcel[a].rplCel.ro.model = baseModelMatrix * *glob.pdmat;
						else
							glob.asubcel[a].rplCel.ro.model = baseModelMatrix;

						glob.asubcel[a].rplCel.ro.uAlpha = celAlphaFinal;

						SubmitRpl(&glob.asubcel[a].rplCel);
					}
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
	ro.uAlphaCelBorder = uAlpha;
	palo->pvtalo->pfnRenderAloGlobset(palo, pcm, &ro);
}

void DeleteModel(ALO* palo)
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

void DeleteAlo(ALO* palo)
{
	delete palo;
}

std::vector <ALO*> allSWAloObjs;