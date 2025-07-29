#include "proxy.h"

PROXY* NewProxy()
{
	return new PROXY{};
}

void InitSwProxyDl(SW* psw)
{
	InitDl(&psw->dlProxy, offsetof(PROXY, dleProxy));
}

void InitProxy(PROXY *pproxy)
{
	InitAlo(pproxy);
	AppendDlEntry(&pproxy->psw->dlProxy, pproxy);
}

int GetProxySize()
{
	return sizeof(PROXY);
}

void LoadProxyFromBrx(PROXY* pproxy, CBinaryInputStream* pbis)
{
	// Proxy source objects to keep track of
	std::vector <LO*> proxyObjs;

	InitDl(&pproxy->dlProxyRoot, offsetof(PXR, dleProxyRoot));

	// Reading transformation for proxy object
	pproxy->xf.mat = pbis->ReadMatrix();
	pproxy->xf.pos = pbis->ReadVector();

	// Reading number of proxy objects
	byte numProxyObjs = pbis->U8Read();

	pproxy->pvtalo->pfnUpdateAloXfWorld(pproxy);

	for (int i = 0; i < numProxyObjs; i++)
	{
		LO* object = nullptr;
		// Loads class ID
		CID cid = (CID)pbis->S16Read();

		if (cid == CID_Nil)
		{
			// Loads proxy source index from file
			uint16_t ipsl = pbis->S16Read();
			// Returns proxy source object
			object = PloGetSwProxySource(pproxy->psw, ipsl);
			// Stores the proxy source object to keep track of.
			proxyObjs.push_back(object);
		}

		else
		{
			// Loads object ID from file
			OID oid = (OID)pbis->S16Read();
			// Loads splice index from file
			int16_t isplice = pbis->S16Read();
			// Returns new initialized proxy source object
			object = PloNew(cid, pproxy->psw, nullptr, oid, isplice);
			// Stores the proxy source object to keep track of.
			proxyObjs.push_back(object);
			// Loads proxy source object from file
			object->pvtlo->pfnLoadLoFromBrx(object, pbis);
			// Loads number of LO clones to make
			uint16_t cploClone = pbis->S16Read();
			// Adds proxy source to proxy source list
			AddSwProxySource(pproxy->psw, object, cploClone);
		}

		PXR pxr{};
		pxr.plo = object;
		pxr.oidProxyRoot = pproxy->oid;
		pxr.pchzProxyRoot = pproxy->pchzName;
		std::shared_ptr <PXR> ppxr = std::make_shared<PXR>(pxr);
		object->ppxr = ppxr;
		//std::cout << &pproxy->dlProxyRoot <<"\n";
		AppendDlEntry(&pproxy->dlProxyRoot, &pxr);
	}

	byte numObjs = pbis->U8Read();

	for (int i = 0; i < numObjs; i++)
	{
		// Loading oid needed to find
		OID oidFind = (OID)pbis->S16Read();
	
		LO* pvObject = nullptr;

		for (int a = 0; a < proxyObjs.size(); a++)
		{
			if (proxyObjs[a]->oid != oidFind && (proxyObjs[a]->pvtlo->grfcid & 1U) != 0)
				pvObject = PloFindSwObject(pproxy->psw, 0x101, oidFind, pvObject);

			if (pvObject != nullptr) break;

			else
				pvObject = proxyObjs[a];
		}

		// Loads splice index
		short isplice = pbis->S16Read();

		pbis->U32Read();

		if (pbis->S8Read() == 2)
			pbis->F32Read();

		// GOTTA COME BACK TO THIS
		LoadOptionsFromBrx(pvObject, pbis);
	}

	for (int i = 0; i < proxyObjs.size(); i++)
	{
		if (proxyObjs[i]->pvtalo->pfnApplyAloProxy != nullptr)
			proxyObjs[i]->pvtalo->pfnApplyAloProxy((ALO*)proxyObjs[i], pproxy);

		proxyObjs[i]->pvtlo->pfnSetLoParent(proxyObjs[i], pproxy->paloParent);
	}

	if (numProxyObjs == 1)
	{
		LoadSwObjectsFromBrx(pproxy->psw, pproxy, pbis);

		if (FIsDlEmpty(&pproxy->dlChild) == false)
		{
			DLI proxyDLI{};

			proxyDLI.m_ibDle = pproxy->dlChild.ibDle;
			proxyDLI.m_pdliNext = s_pdliFirst;
			LO* object = pproxy->dlChild.ploFirst;
			proxyDLI.m_ppv = (void**)((uintptr_t)object + proxyDLI.m_ibDle);
			s_pdliFirst = &proxyDLI;
			proxyDLI.m_pdl = &pproxy->dlChild;

			while(object != nullptr)
			{
				object->pvtlo->pfnSetLoParent(object, (ALO*)proxyObjs[0]);
				object = (LO*)*proxyDLI.m_ppv;
				proxyDLI.m_ppv = (void**)((uintptr_t)object + proxyDLI.m_ibDle);
			}

			s_pdliFirst = proxyDLI.m_pdliNext;
		}
	}

	pproxy->pvtlo->pfnRemoveLo(pproxy);
	pproxy->pvtlo->pfnAddLo(pproxy);
}

void CloneProxy(PROXY* pproxy, PROXY* pproxyBase)
{
	// Backup the original DLE and DL before cloning
	DLE savedDleProxy = pproxy->dleProxy;
	DL savedDlProxyRoot = pproxy->dlProxyRoot;

	// Clone base ALO part of PROXY
	CloneAlo(pproxy, pproxyBase);

	// Restore DLE and DL that CloneAlo may have overwritten
	pproxy->dleProxy = savedDleProxy;
	pproxy->dlProxyRoot = savedDlProxyRoot;

	// Clear the cloned object's DL list
	ClearDl(&pproxy->dlProxyRoot);

	// If the original proxy list has entries, replicate them
	if (savedDlProxyRoot.pvFirst != nullptr) {
		DL current = savedDlProxyRoot;
		ALO* aloParent = pproxy->paloParent;

		while (current.pvFirst != nullptr) {
			LO* matchingLO = nullptr;

			// Try to find an existing child LO whose ppxr matches the original entry
			for (LO* lo = aloParent->dlChild.ploFirst; lo != nullptr; lo = lo->dleChild.ploNext) {
				if (lo->ppxr.get() == current.pvFirst) {
					matchingLO = lo;
					break;
				}
			}

			// Allocate new PXR smart pointer
			std::shared_ptr<PXR> pxr = std::make_shared<PXR>();

			pxr->oidProxyRoot = pproxy->oid;
			pxr->pchzProxyRoot = pproxy->pchzName;

			if (matchingLO != nullptr) {
				pxr->plo = matchingLO;
				matchingLO->ppxr = pxr;
			}
			else {
				pxr->plo = current.ploFirst; // fallback to raw LO* from old list
			}

			AppendDlEntry(&pproxy->dlProxyRoot, &pxr);

			// Move to next DLE entry
			current.pvFirst = current.ploFirst->dleChild.ploNext;
		}
	}
}

void ApplyAloProxy(ALO* palo, PROXY* pproxyApply)
{
	if (!palo || !pproxyApply) return;

	// Convert position from proxy to world space
	glm::vec3 posWorld;
	ConvertAloPos(pproxyApply, nullptr, palo->xf.pos, posWorld);
	palo->pvtalo->pfnTranslateAloToPos(palo, posWorld);

	// Convert rotation matrix from proxy to world space
	glm::mat3 matWorld;
	ConvertAloMat(pproxyApply, nullptr, palo->xf.mat, matWorld);
	palo->pvtalo->pfnRotateAloToMat(palo, matWorld);

	// Calculate parent movement
	glm::vec3 vParent, wParent;
	CalculateAloMovement(pproxyApply->paloParent, nullptr, &posWorld, &vParent, &wParent, nullptr, nullptr);

	// Convert local velocity and angular velocity to world space
	glm::vec3 vPalo, wPalo;
	ConvertAloVec(pproxyApply, nullptr, &palo->xf.v, &vPalo);
	ConvertAloVec(pproxyApply, nullptr, &palo->xf.w, &wPalo);

	// Final world velocity = parent velocity + local velocity (transformed)
	glm::vec3 v = vParent + vPalo;
	glm::vec3 w = wParent + wPalo;

	palo->pvtalo->pfnSetAloVelocityVec(palo, &v);
	palo->pvtalo->pfnSetAloAngularVelocityVec(palo, &w);

	// Store original position
	palo->posOrig = palo->xf.pos;

	// Store original matrix (flattened copy)
	palo->matOrig = palo->xf.mat;

}

void ApplySoProxy(SO* pso, PROXY* pproxyApply)
{
	if (!pso || !pproxyApply) return;

	// Apply transform and velocity from proxy via base ALO logic
	ApplyAloProxy(pso, pproxyApply);

	// Load rotation matrix from proxy
	const glm::mat3& proxyMat = pproxyApply->xf.mat;

	// Transform normal force and torque vectors into proxy's orientation
	glm::vec3 normalForce  = proxyMat * pso->constrForce.normal;
	glm::vec3 normalTorque = proxyMat * pso->constrTorque.normal;

	// Apply transformed constraint directions
	SetSoConstraints(pso, pso->constrForce.ct, &normalForce, pso->constrTorque.ct, &normalTorque);
}

void DeleteProxy(PROXY *pproxy)
{
	delete pproxy;
}

bool loadEmitMesh = 0;