#include "proxy.h"

void* NewProxy()
{
	return new PROXY;
}

void InitProxy(PROXY *pproxy)
{
	InitAlo(pproxy);
	AppendDlEntry(&pproxy->psw->dlProxy, pproxy);
}

void LoadProxyFromBrx(PROXY *pproxy, CBinaryInputStream* pbis)
{
	pbis->ReadMatrix();
	pbis->ReadVector();

	byte numProxyObjs = pbis->U8Read();

	for (int i = 0; i < numProxyObjs; i++)
	{
		CID cid = (CID)pbis->S16Read();

		if (cid == CID_Nil)
			pbis->S16Read();

		else
		{
			OID oid = (OID)pbis->S16Read();
			int16_t isplice = pbis->S16Read();
			LO *object = PloNew(cid, pproxy->psw, 0, oid, isplice);
			object->pvtlo->pfnLoadLoFromBrx(object, pbis);
			pbis->S16Read();
		}
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

	if (numProxyObjs == 1)
		LoadSwObjectsFromBrx(pproxy->psw, pproxy, pbis);
}

void DeleteProxy(LO* plo)
{
	delete(PROXY*)plo;
}
