#include "brx.h"

void StartupBrx()
{
	BuildEopids();
}

LO* PloNew(CID cid, SW* psw, ALO* paloParent, OID oid, int isplice)
{
	VTLO *pvtlo = (VTLO*)g_mpcidpvt[cid];

	SW *localObject = (SW*)new int[pvtlo->cb]();

	localObject->pvtlo = pvtlo;

	localObject->oid = oid;

	if (cid == CID_SW)
	{
		InitSwDlHash(localObject);
		localObject->paloParent = paloParent;
		psw = localObject;
	}

	else
		localObject->paloParent = paloParent;

	localObject->psw = psw;

	// Appending object to parent list
	if (localObject->pvtlo->pfnInitLo != 0)
		AppendDlEntry(PdlFromSwOid(localObject->psw, localObject->oid), localObject);
	
	// GOTTA REMOVE THIS
	if (localObject->pvtlo->pfnInitLo != 0)
		localObject->pvtlo->pfnInitLo(localObject);

	return (LO*)localObject;
}

void LoadSwObjectsFromBrx(SW *psw, ALO *paloParent, CBinaryInputStream *pbis)
{
	// Number of SW objects
	uint16_t numObjects = pbis->U16Read();

	for (int i = 0; i < numObjects; i++)
	{
		// Objects class ID
		CID cid = (CID)pbis->S16Read();
		// Objects ID
		OID oid = (OID)pbis->S16Read();
		// Objects splice event index
		int isplice = pbis->S16Read();
		//std::cout << i << "\n";
		LO *plo = PloNew(cid, psw, paloParent, oid, isplice);
		plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);
	}
}

DL* PdlFromSwOid(SW *psw, OID oid)
{
	return psw->firstObjectParents + (oid * 0x95675 & 0x1ff);
}

void LoadOptionFromBrx(void* pvObject, CBinaryInputStream* pbis)
{
	// THIS FUNCTION WILL BE CHANGED LATER ON WHEN I GET AROUND TO RE
	uint16_t emitMeshFlag = 0;
	int16_t unk_id;

	while (true)
	{
	label0:
		unk_id = pbis->S16Read();

		if (unk_id == 0x2)
			goto label0;

		if (unk_id < 0)
			break;
		//std::cout << std::hex << pbis->file.tellg() <<"\n";
		uint32_t eopid = g_aeopid[(unk_id * 8)];
		uint32_t grfeopid = g_aeopid[(unk_id * 8) + 1];
		uint32_t crefReq = g_aeopid[(unk_id * 8) + 5];

		if ((grfeopid & 0x400) != 0)
		{
			for (int i = 0; i < crefReq; i++)
			{
				uint16_t unk_0 = pbis->S16Read();

				switch (unk_0)
				{
				case 0x0:
					pbis->U8Read();
					break;
				case 0x1:
					pbis->F32Read();
					break;
				case 0x2:
					pbis->F32Read();
					pbis->F32Read();
					pbis->F32Read();
					break;
				default:
					if (((int)unk_0 & 0x7fffffffU) >> 0xc == 1)
						pbis->S16Read();
					break;
				case 0x5:
					pbis->S32Read();
					break;
				case 0x6:
					pbis->S16Read();
					break;
				case 0x7:
					pbis->S16Read();
					break;
				case 0xE:
					pbis->S16Read();
					break;
				case 0xF:
					pbis->S16Read();
					break;
				case 0x10:
					pbis->S16Read();
					break;
				case 0x11:
					pbis->S16Read();
					break;
				case 0x14:
					pbis->S16Read();
					break;
				case 0x9:
					pbis->F32Read();
					pbis->F32Read();
					pbis->F32Read();
					break;
				case 0xA:
					pbis->F32Read();
					pbis->F32Read();
					break;
				}
			}

			goto label0;
		}

		if ((grfeopid & 0x80) != 0)
		{
			switch (eopid)
			{
			case 0x0:
				pbis->U8Read();
				break;
			case 0x1:
				pbis->F32Read();
				break;
			case 0x2:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x9:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0xB:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x3:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0xC:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x4:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x5:
				pbis->S32Read();
				break;
			default:
				emitMeshFlag = pbis->S16Read();
				if (emitMeshFlag == 0x4 && unk_id == 0x156)
					loadEmitMesh = true;
				else if (emitMeshFlag == 0x4 && unk_id == 0x18C)
					loadEmitMesh = true;
				break;
			case 0xA:
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0xD:
				pbis->U32Read();
				break;
			}
		}
		else
		{
			switch (eopid)
			{
			case 0x0:
				pbis->U8Read();
				break;
			case 0x1:
				pbis->F32Read();
				break;
			case 0x2:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x9:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;

			case 0xB:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x4:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0x5:
				pbis->S32Read();
				break;
			case 0x6:
				pbis->S16Read();
				break;
			case 0x7:
				pbis->S16Read();
				break;
			case 0xE:
				pbis->S16Read();
				break;
			case 0xF:
				pbis->S16Read();
				break;
			case 0x10:
				pbis->S16Read();
				break;
			case 0x11:
				pbis->S16Read();
				break;
			default:
				emitMeshFlag = pbis->S16Read();
				if (emitMeshFlag == 0x4 && unk_id == 0x1A5)
					loadEmitMesh = true;
				else if (emitMeshFlag == 0x4 && unk_id == 0x16E)
					loadEmitMesh = true;
				break;
			case 0xA:
				pbis->F32Read();
				pbis->F32Read();
				break;
			case 0xD:
				pbis->U32Read();
				break;
			}
		}

	}
}

void LoadOptionsFromBrx(void* pvObject, CBinaryInputStream* pbis)
{
	while (true)
	{
		// Reading eopid from binary file
		int16_t eopid = pbis->S16Read();

		if (eopid < 0) break;

		LoadOptionFromBrx(pvObject, pbis);
	}
}