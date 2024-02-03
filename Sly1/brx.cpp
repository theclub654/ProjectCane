#include "brx.h"

std::vector<EOPID> g_aeopid;

void StartupBrx()
{
	BuildEopids();
}

LO* PloNew(CID cid, SW* psw, ALO* paloParent, OID oid, int isplice)
{
	// Loading class object vtable
	VTLO *pvtlo = (VTLO*)g_mpcidpvt[cid];

	// Returning a address for the newly made object
	LO* localObject = (LO*)pvtlo->pfnNewLo();

	// Storing vtable with object
	localObject->pvtlo = pvtlo;
	// Storing the object ID with object
	localObject->oid = oid;

	if (cid == CID_SW)
	{
		InitSwDlHash((SW*)localObject);
		localObject->paloParent = paloParent;
		psw = (SW*)localObject;
	}

	else
		localObject->paloParent = paloParent;

	// Storing pointer to static world object
	localObject->psw = psw;

	// Appending object to fist parent list
    AppendDlEntry(PdlFromSwOid(localObject->psw, localObject->oid) , localObject);
	
	// Initializing local object
	localObject->pvtlo->pfnInitLo(localObject);
	
	// Storing pointer to object in global vector
	allWorldObjs.push_back(localObject);

	// Returining newly made objects
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
		// Making new object and initializing the object
		LO *plo = PloNew(cid, psw, paloParent, oid, isplice);
		// Loading object from binary file
		plo->pvtlo->pfnLoadLoFromBrx(plo, pbis);
	}
}

DL* PdlFromSwOid(SW *psw, OID oid)
{
	return psw->adlHash + (oid * 0x95675 & 0x1ff);
}

void LoadOptionsFromBrx(void* pvObject, CBinaryInputStream* pbis)
{
	/*LoadOptionsFromBrxCount++;
	std::cout << "LoadOptionsCount: " << LoadOptionsFromBrxCount << "\n";*/

	while (true)
	{
		// Reading eopid from binary file
		int16_t eopid = pbis->S16Read();

		if (eopid < 0) break;

		LoadOptionFromBrx(pvObject, g_aeopid[eopid], pbis);
	}
}

void LoadOptionFromBrx(void* pvObject, EOPID peopid, CBinaryInputStream* pbis)
{
	void* objectDataPtr{};
	uint16_t emitMeshFlag = 0;

	if ((peopid.grfeopid & 0x400) != 0)
	{
		for (int i = 0; i < peopid.optdat.ibSet; i++)
		{
			OTYP optionType = (OTYP)pbis->S16Read();

			switch (optionType)
			{
				case OTYP_Bool:
					pbis->U8Read();
				break;

				case OTYP_Float:
					pbis->F32Read();
				break;

				case OTYP_Vector:
					pbis->ReadVector();
				break;

				case OTYP_Int:
					pbis->S32Read();
				break;

				case OTYP_Clq:
					pbis->F32Read();
					pbis->F32Read();
					pbis->F32Read();
				break;

				case OTYP_Lm:
					pbis->F32Read();
					pbis->F32Read();
				break;

				case OTYP_Oid:
				case OTYP_Cid:
				case OTYP_Sfxid:
				case OTYP_Wid:
				case OTYP_Tbid:
				case OTYP_Msgid:
				case OTYP_Optid:
					pbis->S16Read();
				break;

				default:
					if ((optionType & 0x7fffffffU) >> 0xc == 1)
						pbis->S16Read();
				break;
			}
		}

		return;
	}

	if ((peopid.grfeopid & 0x80) != 0)
	{
		if ((peopid.grfeopid & 0x1000) == 0)
		{

		}

		else
		{
			if (peopid.optdat.pfnensure != nullptr)
				objectDataPtr = peopid.optdat.pfnensure(pvObject, 1);
		}

		switch (peopid.otyp)
		{
			case OTYP_Bool:
				pbis->U8Read();
			return;

			case OTYP_Float:
				pbis->F32Read();
			return;

			case OTYP_Matrix:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
			return;

			case OTYP_Int:
				pbis->S32Read();
			return;

			case OTYP_Lm:
				pbis->F32Read();
				pbis->F32Read();
			return;

			case OTYP_Vector:
			case OTYP_Clq:
			case OTYP_Smp:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
			return;

			case OTYP_Vector4:
			case OTYP_Smpa:
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
				pbis->F32Read();
			return;

			case OTYP_Rgba:
				pbis->U32Read();
			return;

			default:
				emitMeshFlag = pbis->S16Read();
				if (emitMeshFlag == 0x4 && peopid.otyp == OTYP_Emitok)
					loadEmitMesh = true;
			return;
		}
	}

	switch (peopid.otyp)
	{
		case OTYP_Bool:
			pbis->U8Read();
		return;

		case OTYP_Float:
			pbis->F32Read();
		return;

		case OTYP_Matrix:
			pbis->F32Read();
			pbis->F32Read();
			pbis->F32Read();
		return;

		case OTYP_Int:
			pbis->S32Read();
		return;

		case OTYP_Lm:
			pbis->F32Read();
			pbis->F32Read();
		break;

		case OTYP_Vector:
		case OTYP_Clq:
		case OTYP_Smp:
			pbis->F32Read();
			pbis->F32Read();
			pbis->F32Read();
		break;

		case OTYP_Vector4:
		case OTYP_Smpa:
			pbis->F32Read();
			pbis->F32Read();
			pbis->F32Read();
			pbis->F32Read();
		break;

		case OTYP_Rgba:
			pbis->U32Read();
		break;

		case OTYP_Oid:
		case OTYP_Cid:
		case OTYP_Sfxid:
		case OTYP_Wid:
		case OTYP_Tbid:
		case OTYP_Msgid:
			pbis->S16Read();
		break;

		default:
			pbis->S16Read();
		return;
	}
}
