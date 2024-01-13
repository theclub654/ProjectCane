#include "proxy.h"

void* NewProxy()
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
	std::vector <LO*> proxyObjs;

	InitDl(&pproxy->dlProxyRoot, 0x16);

	pproxy->xf.mat = pbis->ReadMatrix();
	pproxy->xf.pos = pbis->ReadVector();

	byte numProxyObjs = pbis->U8Read();

	pproxy->pvtalo->pfnUpdateAloXfWorld(pproxy);

	for (int i = 0; i < numProxyObjs; i++)
	{
		LO* object{};
		// Loads class ID
		CID cid = (CID)pbis->S16Read();

		if (cid == CID_Nil)
		{
			// Loads proxy source index from file
			uint16_t ipsl = pbis->S16Read();
			// Returns proxy source object based of proxy source index
			object = PloGetSwProxySource(pproxy->psw, ipsl);
		}

		else
		{
			// Loads object ID from file
			OID oid = (OID)pbis->S16Read();
			// Loads splice index from file
			int16_t isplice = pbis->S16Read();
			// Returns new initialized proxy object
			object = PloNew(cid, pproxy->psw, nullptr, oid, isplice);
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
		pbis->S16Read();
		pbis->S16Read();
		pbis->U32Read();
		int8_t unk1 = pbis->S8Read();

		if (unk1 == 2)
			pbis->F32Read();

		LoadOptionFromBrx(0, pbis);
	}

	for (int i = 0; i < proxyObjs.size(); i++)
		proxyObjs[i]->pvtalo->pfnApplyAloProxy((ALO*)proxyObjs[i], pproxy);

	if (numProxyObjs == 1)
		LoadSwObjectsFromBrx(pproxy->psw, pproxy, pbis);

	proxyObjs.clear();
	proxyObjs.shrink_to_fit();
}

void CloneProxy(PROXY* pproxy, PROXY* pproxyBase)
{
	LO lo = *pproxy;
	*pproxy = *pproxyBase;
	memcpy(pproxy, &lo, sizeof(LO));

	CloneLo(pproxy, pproxyBase);

	ClearDl(&pproxy->dlChild);
}

void DeleteProxy(LO* plo)
{
	delete(PROXY*)plo;
}