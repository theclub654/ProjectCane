#include "proxy.h"

void* NewProxy()
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
	ProxyCount++;
	//std::cout << "ProxyCount: " << ProxyCount<<"\n";

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

		PXR proxyRoot{};

		proxyRoot.plo = object;
		proxyRoot.oidProxyRoot = pproxy->oid;
		proxyRoot.pchzProxyRoot = pproxy->pchzName;
		object->ppxr = &proxyRoot;
		AppendDlEntry(&pproxy->dlProxyRoot, &proxyRoot);
	}

	byte unk0 = pbis->U8Read();

	for (int i = 0; i < unk0; i++)
	{
		// Loading oid needed to find
		OID oidFind = (OID)pbis->S16Read();
	
		LO* pvObject = nullptr;

		/*for (int a = 0; a < proxyObjs.size(); a++)
		{
			if (proxyObjs[a]->oid != oidFind && (proxyObjs[a]->pvtlo->grfcid & 1U) != 0)
				pvObject = PloFindSwObject(pproxy->psw, 0x101, oidFind, pvObject);

			if (pvObject != nullptr) break;

			else
				pvObject = proxyObjs[a];
		}*/

		// Loads splice index
		short isplice = pbis->S16Read();

		pbis->U32Read();

		if (pbis->S8Read() == 2)
			pbis->F32Read();

		// GOTTA COME BACK TO THIS
		LoadOptionsFromBrx(proxyObjs[0], pbis);
	}

	for (int i = 0; i < proxyObjs.size(); i++)
	{
		proxyObjs[i]->pvtalo->pfnApplyAloProxy((ALO*)proxyObjs[i], pproxy);
		proxyObjs[i]->pvtlo->pfnSetLoParent(proxyObjs[i], pproxy->paloParent);
	}

	if (numProxyObjs == 1)
	{
		LoadSwObjectsFromBrx(pproxy->psw, pproxy, pbis);
		byte isEmpty = FIsDlEmpty(&pproxy->dlChild);

		if (isEmpty == false)
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
	LO lo = *pproxy;
	*pproxy = *pproxyBase;
	memcpy(pproxy, &lo, sizeof(LO));

	CloneLo(pproxy, pproxyBase);

	ClearDl(&pproxy->dlChild);
	ClearDl(&pproxy->dlProxyRoot);
}

void DeleteProxy(LO* plo)
{
	delete(PROXY*)plo;
}