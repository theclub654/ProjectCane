#include "proxy.h"
#include "frame.h"

PROXY* NewProxy()
{
	return new PROXY{};
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
	std::vector<LO*> proxyObjs;
	InitDl(&pproxy->dlProxyRoot, offsetof(PXR, dleProxyRoot));

	pproxy->xf.mat = pbis->ReadMatrix();
	pproxy->xf.pos = pbis->ReadVector();

	byte numProxyObjs = pbis->U8Read();
	pproxy->pvtalo->pfnUpdateAloXfWorld(pproxy);

	for (int i = 0; i < numProxyObjs; i++)
	{
		LO* object = nullptr;

		CID cid = (CID)pbis->S16Read();
		
		if (cid == CID_Nil)
		{
			uint16_t ipsl = pbis->S16Read();
			object = PloGetSwProxySource(pproxy->psw, ipsl);
		}
		else
		{
			OID oid = (OID)pbis->S16Read();
			int16_t isplice = pbis->S16Read();

			object = PloNew(cid, pproxy->psw, nullptr, oid, isplice);
			object->pvtlo->pfnLoadLoFromBrx(object, pbis);

			uint16_t cploClone = pbis->S16Read();
			AddSwProxySource(pproxy->psw, object, cploClone);
		}

		auto pxr = std::make_shared<PXR>();

		pxr->plo = object;
		pxr->oidProxyRoot = (OID)pproxy->oid;
		pxr->pchzProxyRoot = pproxy->pchzName;

		object->ppxr = pxr;

		pproxy->apxrProxyRoot.push_back(pxr);
		AppendDlEntry(&pproxy->dlProxyRoot, pxr.get());

		proxyObjs.push_back(object);
	}

	byte numObjs = pbis->U8Read();

	for (int i = 0; i < numObjs; i++)
	{
		OID oidFind = (OID)pbis->S16Read();

		ALO* pvObject = nullptr;

		for (int a = 0; a < numProxyObjs; a++)
		{
			ALO* candidate = (ALO*)proxyObjs[a];

			if (candidate->oid != oidFind)
			{
				if ((candidate->pvtlo->grfcid & 1U) != 0)
					candidate = (ALO*)PloFindSwObject(pproxy->psw, 0x101, oidFind, candidate);
				else
					candidate = nullptr;
			}

			if (candidate != nullptr)
			{
				pvObject = candidate;
				break;
			}
		}

		const int16_t isplice = pbis->S16Read();

		if (isplice != -1)
		{
			std::shared_ptr<CFrame> frame = PframeFromIsplice(static_cast<int>(isplice), pproxy->psw);

			if (frame && pvObject->pframe)
				frame->AddParent(pvObject->pframe);

			pvObject->pframe = frame;
		}

		int grfzon = pbis->U32Read();

		if ((pvObject->pvtlo->grfcid & 1U) != 0)
			pvObject->grfzon = grfzon;

		if (pbis->S8Read() == 2)
		{
			pvObject->mrds = 2;

			float sMRD = pbis->F32Read();

			pvObject->sMRD = sMRD;

			if (sMRD == 3.402823e+38f)
				pvObject->sMRD = 1e+10f;
		}

		LoadOptionsFromBrx(pvObject, pbis);
	}

	for (int i = 0; i < numProxyObjs; i++)
	{
		ALO* object = (ALO*)proxyObjs[i];

		if (object->pvtalo->pfnApplyAloProxy != nullptr)
			object->pvtalo->pfnApplyAloProxy(object, pproxy);

		object->pvtlo->pfnSetLoParent(object, pproxy->paloParent);
	}

	if (numProxyObjs == 1)
	{
		LoadSwObjectsFromBrx(pproxy->psw, pproxy, pbis);

		if (FIsDlEmpty(&pproxy->dlChild) == false)
		{
			DLI proxyDLI{};

			proxyDLI.m_ibDle = pproxy->dlChild.ibDle;
			proxyDLI.m_pdliNext = s_pdliFirst;
			proxyDLI.m_pdl = &pproxy->dlChild;

			LO* object = pproxy->dlChild.ploFirst;
			proxyDLI.m_ppv = (void**)((uintptr_t)object + proxyDLI.m_ibDle);

			s_pdliFirst = &proxyDLI;

			while (object != nullptr)
			{
				object->pvtlo->pfnSetLoParent(object, (ALO*)proxyObjs[0]);

				object = (LO*)*proxyDLI.m_ppv;

				if (object != nullptr)
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
	DLE savedDleProxy = pproxy->dleProxy;

	CloneAlo(pproxy, pproxyBase);

	pproxy->dleProxy = savedDleProxy;
	pproxy->dlProxyRoot.ibDle = pproxyBase->dlProxyRoot.ibDle;
	ClearDl(&pproxy->dlProxyRoot);
	pproxy->apxrProxyRoot.clear();
}

static LO* PloFindCloneCounterpart(ALO* palo, ALO* paloBase, LO* ploBaseFind)
{
	if (reinterpret_cast<LO*>(paloBase) == ploBaseFind)
		return reinterpret_cast<LO*>(palo);

	LO* ploBaseChild = paloBase->dlChild.ploFirst;
	LO* ploChild = palo->dlChild.ploFirst;

	while (ploBaseChild != nullptr && ploChild != nullptr)
	{
		if (ploBaseChild == ploBaseFind)
			return ploChild;

		if ((ploBaseChild->pvtlo->grfcid & 1U) != 0 &&
			(ploChild->pvtlo->grfcid & 1U) != 0)
		{
			LO* ploFound = PloFindCloneCounterpart(
				static_cast<ALO*>(ploChild), static_cast<ALO*>(ploBaseChild), ploBaseFind);
			if (ploFound != nullptr)
				return ploFound;
		}

		ploBaseChild = ploBaseChild->dleChild.ploNext;
		ploChild = ploChild->dleChild.ploNext;
	}

	return nullptr;
}

static void RebuildProxyRootsInHierarchy(ALO* palo, ALO* paloBase,
	ALO* paloCloneRoot, ALO* paloBaseRoot)
{
	if ((palo->pvtlo->grfcid & 0x100U) != 0 &&
		(paloBase->pvtlo->grfcid & 0x100U) != 0)
	{
		PROXY* pproxy = static_cast<PROXY*>(palo);
		PROXY* pproxyBase = static_cast<PROXY*>(paloBase);
		ClearDl(&pproxy->dlProxyRoot);
		pproxy->apxrProxyRoot.clear();

		for (PXR* ppxrBase = pproxyBase->dlProxyRoot.ppxrFirst;
			ppxrBase != nullptr; ppxrBase = ppxrBase->dleProxyRoot.ppxrNext)
		{
			LO* ploClone = PloFindCloneCounterpart(paloCloneRoot, paloBaseRoot, ppxrBase->plo);

			if (ploClone == nullptr)
				continue;

			auto ppxr = std::make_shared<PXR>();
			ppxr->plo = ploClone;
			ppxr->oidProxyRoot = static_cast<OID>(pproxy->oid);
			ppxr->pchzProxyRoot = pproxy->pchzName;
			ploClone->ppxr = ppxr;
			pproxy->apxrProxyRoot.push_back(ppxr);
			AppendDlEntry(&pproxy->dlProxyRoot, ppxr.get());
		}
	}

	LO* ploBaseChild = paloBase->dlChild.ploFirst;
	LO* ploChild = palo->dlChild.ploFirst;
	while (ploBaseChild != nullptr && ploChild != nullptr)
	{
		if ((ploBaseChild->pvtlo->grfcid & 1U) != 0 &&
			(ploChild->pvtlo->grfcid & 1U) != 0)
			RebuildProxyRootsInHierarchy(static_cast<ALO*>(ploChild),
				static_cast<ALO*>(ploBaseChild), paloCloneRoot, paloBaseRoot);

		ploBaseChild = ploBaseChild->dleChild.ploNext;
		ploChild = ploChild->dleChild.ploNext;
	}
}

void RebuildClonedProxyRoots(ALO* palo, ALO* paloBase)
{
	if (palo != nullptr && paloBase != nullptr)
		RebuildProxyRootsInHierarchy(palo, paloBase, palo, paloBase);
}

void PostProxyLoad(PROXY* pproxy)
{
	PostAloLoad(pproxy);
	pproxy->pvtlo->pfnRemoveLo(pproxy);
}

void DeleteProxy(PROXY *pproxy)
{
	for (int i = 0; i < pproxy->apxrProxyRoot.size(); i++)
		pproxy->apxrProxyRoot[i].reset();

	//pproxy->apxrProxyRoot.clear();

	delete pproxy;
}
