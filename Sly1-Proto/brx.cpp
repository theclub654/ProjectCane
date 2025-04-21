#include "brx.h"

std::vector <EOPID> g_aeopid;

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
	
	// Insert into linked list of LO by CID
	LO** cidHead = &localObject->psw->aploCidHead[cid];
	localObject->ploCidNext = *cidHead;
	*cidHead = static_cast<LO*>(localObject);

	// Initializing local object
	localObject->pvtlo->pfnInitLo(localObject);
	
	// Storing pointer to object in global vector
	allWorldObjs.push_back(localObject);

	// Returining newly made objects
	return (LO*)localObject;
}

int IploFromStockOid(int oid)
{
	int stockOid = oid - 0xC;

	if (stockOid > 0x1C)
		return -1;
	else
		return stockOid;
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
		
		int stockOidIndex = IploFromStockOid(oid);

		if (stockOidIndex > -1)
		{
			psw->aploStock[stockOidIndex] = plo;
			SnipLo(plo);
		}
	}
}

DL* PdlFromSwOid(SW *psw, OID oid)
{
	return psw->adlHash + (oid * 0x95675 & 0x1ff);
}

void LoadOptionsFromBrx(void* pvObject, CBinaryInputStream* pbis)
{
	while (true)
	{
		// Reading eopid from binary file
		int16_t eopid = pbis->S16Read();

		if (eopid < 0) break;

		LoadOptionFromBrx(pvObject, g_aeopid[eopid], eopid ,pbis);
	}
}

void LoadOptionFromBrx(void* pvObject, EOPID eopid, int eopidID, CBinaryInputStream* pbis)
{
	void* objectDataPtr = nullptr;

	if ((eopid.grfeopid & 0x400) != 0)
	{
		for (int i = 0; i < eopid.optdat.ibSet; i++)
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

	if ((eopid.grfeopid & 0x80) != 0)
	{
		if ((eopid.grfeopid & 0x1000) == 0)
		{
			if (eopid.optdat.pfnget != nullptr)
				objectDataPtr = eopid.optdat.pfnget(pvObject);
		}

		else
		{
			if (eopid.optdat.pfnensure != nullptr)
				objectDataPtr = eopid.optdat.pfnensure(pvObject, 1);
		}

		switch (eopid.otyp)
		{
			case OTYP_Bool:
			{
				float optionTypeBool = pbis->U8Read();

				if (eopid.optdat.pfnget != nullptr)
					memcpy(objectDataPtr, &optionTypeBool, sizeof(byte));
				return;
			}

			case OTYP_Float:
			{
				float optionTypefloat = pbis->F32Read();

				if (eopid.optdat.pfnget != nullptr)
					memcpy(objectDataPtr, &optionTypefloat, sizeof(float));
				return;
			}
			

			case OTYP_Matrix:
			{
				glm::vec3 optionDataMat3 = pbis->ReadVector();

				if (eopid.optdat.pfnget != nullptr)
					memcpy(objectDataPtr, &optionDataMat3, sizeof(glm::vec3));
				return;
			}

			case OTYP_Int:
			{
				int optionDataInt = pbis->S32Read();

				if (eopid.optdat.pfnget != nullptr)
					memcpy(objectDataPtr, &optionDataInt, sizeof(int));
				return;
			}

			case OTYP_Lm:
			{
				glm::vec2 optionTypeLm = pbis->ReadVector2();
				if (eopid.optdat.pfnget != nullptr)
					memcpy(objectDataPtr, &optionTypeLm, sizeof(glm::vec2));
				return;
			}

			case OTYP_Vector:
			case OTYP_Clq:
			case OTYP_Smp:
			{
				glm::vec3 optionDataVec = pbis->ReadVector();

				if (eopid.optdat.pfnget != nullptr)
					memcpy(objectDataPtr, &optionDataVec, sizeof(glm::vec3));
				return;
			}

			case OTYP_Vector4:
			case OTYP_Smpa:
			{
				glm::vec4 optionDataVec4 = pbis->ReadVector4();

				if (eopid.optdat.pfnget != nullptr)
					memcpy(objectDataPtr, &optionDataVec4, sizeof(glm::vec4));
				return;
			}

			case OTYP_Rgba:
			{
				int optionDataUint = pbis->U32Read();

				if (eopid.optdat.pfnget != nullptr)
					memcpy(objectDataPtr, &optionDataUint, sizeof(int));
				return;
			}

			default:
				if (pbis->S16Read() == 0x4 && eopid.otyp == OTYP_Emitok)
					loadEmitMesh = true;
			return;
		}
	}

	switch (eopid.otyp)
	{
		case OTYP_Bool:
		{
			byte optionDataBool = pbis->U8Read();
			if (eopid.optdat.pfnsetbyte != nullptr)
				eopid.optdat.pfnsetbyte(pvObject, optionDataBool);
			return;
		}

		case OTYP_Float:
		{
			float optionDataFloat = pbis->F32Read();
			if (eopid.optdat.pfnsetfloat != nullptr)
				eopid.optdat.pfnsetfloat(pvObject, optionDataFloat);
			return;
		}

		case OTYP_Matrix:
		{
			glm::vec3 optionDataMat = pbis->ReadVector();

			if (eopid.optdat.pfnsetvec3 != nullptr)
				eopid.optdat.pfnsetvec3(pvObject, optionDataMat);
			return;
		}

		case OTYP_Int:
		{
			int optionDataInt = pbis->S32Read();

			if (eopid.optdat.pfnset != nullptr)
				eopid.optdat.pfnset(pvObject, optionDataInt);
			return;
		}
		case OTYP_Lm:
		{
			glm::vec2 optionDataLM = pbis->ReadVector2();

			if (eopid.optdat.pfnsetvec2 != nullptr)
				eopid.optdat.pfnsetvec2(pvObject, optionDataLM);
			break;
		}

		case OTYP_Vector:
		case OTYP_Clq:
		case OTYP_Smp:
		{
			glm::vec3 optionDataVec3 = pbis->ReadVector();

			if (eopid.optdat.pfnsetvec3 != nullptr)
				eopid.optdat.pfnsetvec3(pvObject, optionDataVec3);
			break;
		}

		case OTYP_Vector4:
		case OTYP_Smpa:
		{
			glm::vec4 optionDataVec4 = pbis->ReadVector4();

			if (eopid.optdat.pfnsetvec4 != nullptr)
				eopid.optdat.pfnsetvec4(pvObject, optionDataVec4);
			break;
		}

		case OTYP_Rgba:
		{
			int optionDataRGBA = pbis->U32Read();

			if (eopid.optdat.pfnset != nullptr)
				eopid.optdat.pfnset(pvObject, optionDataRGBA);
			break;
		}
		case OTYP_Oid:
		case OTYP_Cid:
		case OTYP_Sfxid:
		case OTYP_Wid:
		case OTYP_Tbid:
		case OTYP_Msgid:
		{
			short optionDataShort = pbis->S16Read();

			if (eopid.optdat.pfnsetshort != nullptr)
				eopid.optdat.pfnsetshort(pvObject, optionDataShort);
			break;
		}

		default:
			int16_t optionData = pbis->S16Read();
			if (eopid.optdat.pfnset != nullptr)
				eopid.optdat.pfnset(pvObject, optionData);
		return;
	}
}