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
	//numProxy++;
	//std::cout << numProxy << "\n";

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

		AppendDlEntry(&pproxy->dlProxyRoot, &pxr);
	}

	byte numObjs = pbis->U8Read();

	for (int i = 0; i < numObjs; i++)
	{
		// Loading oid needed to find
		OID oidFind = (OID)pbis->S16Read();
	
		ALO* pvObject = nullptr;

		for (int a = 0; a < proxyObjs.size(); a++)
		{
			if (proxyObjs[a]->oid != oidFind && (proxyObjs[a]->pvtlo->grfcid & 1U) != 0)
				pvObject = (ALO*)PloFindSwObject(pproxy->psw, 0x101, oidFind, pvObject);

			if (pvObject != nullptr) 
				break;
			else
				pvObject = (ALO*)proxyObjs[a];
		}

		// Loads splice index
		short isplice = pbis->S16Read();

		int grfzon = pbis->U32Read();

		if ((pvObject->pvtlo->grfcid & 1U) != 0)
			pvObject->grfzon = grfzon;

		if (pbis->S8Read() == 2)
		{
			*(unsigned long*)&pvObject->bitfield = *(unsigned long*)&pvObject->bitfield & 0xffffffffcfffffff | 0x20000000;

			float sMRD = pbis->F32Read();

			pvObject->sMRD = sMRD;

			if (sMRD == 3.402823e+38)
				pvObject->sMRD = 1e+10;
		}

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

void PostProxyLoad(PROXY* pproxy)
{
	PostAloLoad(pproxy);
	pproxy->pvtlo->pfnRemoveLo(pproxy);
}

void DeleteProxy(PROXY *pproxy)
{
	delete pproxy;
}