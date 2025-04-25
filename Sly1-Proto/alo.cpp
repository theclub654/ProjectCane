#include "alo.h"
extern std::vector <ALO*> allSWAloObjs;

DLI* s_pdliFirst = nullptr;

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

	/*if (palo->bitfield.fForceCameraFade) 
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

void BindAlo(ALO *palo)
{
	BindAloAlox(palo);
	UpdateAloOrig(palo);
	BindGlobset(&palo->globset, palo);

	LO *plo = palo->dlChild.ploFirst;

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

void TranslateAloToPos(ALO* palo, glm::vec3 &ppos)
{
	palo->xf.pos = ppos;

	palo->pvtalo->pfnUpdateAloXfWorld(palo);
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

	if (palo->sMRD == 3.402823e+38)
		palo->sMRD = 1e+10;
	
	LoadOptionsFromBrx(palo, pbis);
	LoadGlobsetFromBrx(&palo->globset, palo->pvtalo->cid ,palo, pbis);
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
	GRFALOX grfalox = pbis->U32Read();

	if (grfalox != 0)
	{
		ALOX alox;
		palo->palox = std::make_shared <ALOX>(alox);

		palo->palox->grfalox = grfalox;
		palo->palox->matPreRotation = glm::identity <glm::mat3>();
		palo->palox->matPostRotation = glm::identity <glm::mat3>();

		int unk_1;

		if (grfalox & 1)
		{
			pbis->ReadMatrix();
		}

		if (grfalox & 2)
		{
			palo->palox->matPostRotation = pbis->ReadMatrix();
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
			pbis->ReadVector();
			pbis->F32Read();

			if ((grfalox & 0x100) != 0)
			{
				pbis->S16Read();
				pbis->S16Read();
				pbis->S16Read();
				pbis->S16Read();
				pbis->ReadVector();
			}
		}

		if ((grfalox & 0x200) != 0)
		{
			pbis->S16Read();
			pbis->S16Read();
		}

		if ((grfalox & 0x400) != 0)
		{
			pbis->U8Read();
		}
	}
}

void BindAloAlox(ALO* palo)
{

}

void SnipAloObjects(ALO* palo, int csnip, SNIP* asnip)
{
	SW* psw = palo->psw;

	for (int i = 0; i < csnip; ++i) {
		const SNIP& snip = asnip[i];
		uint32_t grffso = (snip.grfsnip & 0x1) ? 0x105 : 0x101;

		if ((snip.grfsnip & 0x20) == 0)
			grffso &= ~0x100; // Remove 0x100 if bit 0x20 is not set

		LO* plo = PloFindSwObject(psw, grffso, snip.oid, palo);

		if (plo != nullptr) {
			if ((snip.grfsnip & 0x08) == 0) {
				// Store the pointer to the found object at a specific offset
				// NOTE: Replace this with proper field access if possible.
				/*reinterpret_cast<LO**>(
					reinterpret_cast<char*>(palo->apmrg) + snip.ib - 0x7C)[0] = plo;*/
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
	/*if (((palo->grfzon & pcm->grfzon) != pcm->grfzon))
		return;*/

	if (!SphereInFrustum(pcm->frustum, palo->xf.posWorld, palo->sRadiusRenderAll))
		return;

	float alpha{};
	if (!FInsideCmMrd(pcm, pcm->pos - palo->xf.posWorld, palo->sRadiusRenderAll, palo->sMRD, alpha))
		return;

	palo->pvtalo->pfnRenderAloSelf(palo, pcm, pro);
}

void RenderAloSelf(ALO* palo, CM* pcm, RO* pro)
{
	palo->pvtalo->pfnRenderAloGlobset(palo, pcm, pro);
}

void DupAloRo(ALO *palo, RO *proOrig, RO *proDup)
{
	if (proOrig == nullptr)
	{
		glm::vec3 vecScale = glm::vec3(1.0);
		LoadMatrixFromPosRotScale(palo->xf.posWorld, palo->xf.matWorld, vecScale, proDup->modelmatrix);
		proDup->uAlpha = 1.0;
	}
	else
	{
		if (proOrig != proDup)
			proDup->modelmatrix = proOrig->modelmatrix;
	}
}

void RenderAloGlobset(ALO *palo, CM *pcm, RO *pro)
{
	RPL rpl{};
	rpl.PFNDRAW = DrawGlob;

	// Duplicate rendering object from original
	DupAloRo(palo, pro, &rpl.ro);

	glm::mat4 baseModelMatrix = rpl.ro.modelmatrix;

	for (int i = 0; i < palo->globset.aglob.size(); ++i) 
	{
		auto& glob = palo->globset.aglob[i];
		glm::vec3 posCenterWorld = glm::vec3(baseModelMatrix * glm::vec4(glob.posCenter, 1.0f));

		/*if ((palo->globset.aglobi[i].grfzon & pcm->grfzon) != pcm->grfzon)
			continue;*/

		if (!SphereInFrustum(pcm->frustum, posCenterWorld, glob.sRadius))
			continue;

		if (!FInsideCmMrd(pcm, pcm->pos - posCenterWorld, glob.sRadius, glob.sMRD, rpl.ro.uAlpha))
			continue;

		for (auto& subglob : glob.asubglob) {
			rpl.ro.VAO = &subglob.VAO;

			// Handle cel border logic
			if (g_fRenderCelBorders && subglob.fCelBorder == 1) 
			{
				rpl.ro.celVAO = &subglob.celVAO;
				rpl.ro.celcvtx = subglob.celcvtx;
				rpl.ro.fCelBorder = 1;
			}
			else 
			{
				rpl.ro.celVAO = nullptr;
				rpl.ro.fCelBorder = 0;
			}

			// Common render setup
			rpl.ro.fDynamic = glob.fDynamic;
			rpl.ro.uFog = glob.uFog;
			rpl.z = glob.gZOrder;
			/*if (rpl.z == 3.402823e+38)
				rpl.z = glm::length(rpl.ro.uAlpha);*/
			rpl.posCenter = glob.posCenter;
			rpl.ro.grfglob = &glob.grfglob;
			rpl.ro.pshd = subglob.pshd;
			rpl.ro.unSelfIllum = &subglob.unSelfIllum;
			rpl.ro.cvtx = subglob.cvtx;

			rpl.rp = (rpl.ro.uAlpha < 1.0f) ? RP_Translucent : glob.rp;

			// Handle dynamic matrix override
			if (glob.pdmat != nullptr) {
				rpl.ro.modelmatrix = baseModelMatrix * *glob.pdmat;
			}
			else {
				rpl.ro.modelmatrix = baseModelMatrix;
			}
			
			 /*if (glob.rtck != RTCK_None)
				 AdjustAloRtckMat(palo, pcm, glob.rtck, &glob.posCenter, rpl.ro.modelmatrix);*/

			SubmitRpl(&rpl);
		}
	}
}

void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha)
{

}

void RenderAloAsBone(ALO* palo, CM* pcm, RO* pro)
{

}

void DrawGlob(RPL *prpl)
{
	glBindVertexArray(*prpl->ro.VAO);

	glUniformMatrix4fv(glGetUniformLocation(glGlobShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(prpl->ro.modelmatrix));
	glUniform1f(glGetUniformLocation(glGlobShader.ID, "uFog"), prpl->ro.uFog);

	glUniform1f(glGetUniformLocation(glGlobShader.ID, "uAlpha"), prpl->ro.uAlpha);

	if ((*prpl->ro.grfglob & 4U) == 0)
		glUniform1f(glGetUniformLocation(glGlobShader.ID, "rDarken"), g_psw->rDarken);
	else
		glUniform1f(glGetUniformLocation(glGlobShader.ID, "rDarken"), 1.0);

	if (prpl->ro.pshd->shdk == SHDK_ThreeWay)
	{
		glUniform1i(glGetUniformLocation(glGlobShader.ID, "rko"), 1);
		
		glUniform1f(glGetUniformLocation(glGlobShader.ID,  "usSelfIllum"), *prpl->ro.unSelfIllum);
		glUniform1i(glGetUniformLocation(glGlobShader.ID,  "fDynamic"), prpl->ro.fDynamic);
		glUniform3fv(glGetUniformLocation(glGlobShader.ID, "posCenter"), 1, glm::value_ptr(prpl->posCenter));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->glShadowMap);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->glDiffuseMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->glSaturateMap);
	}
	else
	{
		glUniform1i(glGetUniformLocation(glGlobShader.ID, "rko"), 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, prpl->ro.pshd->glDiffuseMap);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	switch (prpl->rp)
	{
		case RP_Background:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(false);
		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

		glDisable(GL_BLEND);
		glDepthMask(true);
		break;

		case RP_ProjVolume:
		glEnable(GL_BLEND);
		if ((prpl->ro.pshd->grfshd & 2) == 0)
			glBlendFunc(GL_NONE, GL_ONE);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDepthMask(false);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 128, 128);
		glStencilOp(GL_NONE, GL_REPLACE, GL_NONE);
		glColorMask(0, 0, 0, 0);
		glFrontFace(GL_CW);
		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

		glColorMask(1, 1, 1, 1);
		glStencilOp(GL_KEEP, GL_NONE, GL_KEEP);
		glFrontFace(GL_CCW);
		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

		glDepthMask(true);
		glDepthFunc(GL_GREATER);
		glStencilFunc(GL_EQUAL, 128, 128);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glFrontFace(GL_CW);
		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		glDepthFunc(GL_LESS);
		glFrontFace(GL_CCW);
		glDepthMask(true);
		break;

		case RP_Cutout:
		case RP_CutoutAfterProjVolume:
		case RP_Translucent:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
		glDepthMask(false);
		glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

		glDisable(GL_BLEND);
		glDepthMask(true);
		break;

	default:
		if (prpl->ro.fCelBorder == 1)
		{
			// === First Pass: Draw main object and write to stencil ===
			glEnable(GL_STENCIL_TEST);
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);
			glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);

			// === Second Pass: Draw outline where stencil != 1 ===
			glBindVertexArray(*prpl->ro.celVAO);
			glUniform1i(glGetUniformLocation(glGlobShader.ID, "rko"), 2);
			glDepthMask(GL_FALSE);
			glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
			glStencilMask(0x00);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDrawElements(GL_TRIANGLES, prpl->ro.celcvtx, GL_UNSIGNED_SHORT, 0);

			// === Restore State ===
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
			glStencilMask(0xFF);
			glStencilFunc(GL_ALWAYS, 0, 0xFF);
			glDepthFunc(GL_LESS);
		}
		else
			glDrawElements(GL_TRIANGLES, prpl->ro.cvtx, GL_UNSIGNED_SHORT, 0);
		break;
	}

	glActiveTexture(GL_TEXTURE0);
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

			glDeleteVertexArrays(1, &palo->globset.aglob[i].asubglob[a].celVAO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].celVBO);
			glDeleteBuffers(1, &palo->globset.aglob[i].asubglob[a].celEBO);
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
