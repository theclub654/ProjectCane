#include "brx.h"
#include "rwm.h"
#include <cstdio>

void StartupBrx()
{
	BuildEopids();
}

LO* PloNew(CID cid, SW* psw, ALO* paloParent, OID oid, int isplice)
{
	// Loading class object vtable
	VTLO *pvtlo = (VTLO*)g_mpcidpvt[cid];
	
	// Returning a address for the newly made object
	LO *plo = (LO*)pvtlo->pfnNewLo();

	// Storing vtable with object
	plo->pvtlo = pvtlo;
	// Storing the object ID with object
	plo->oid = oid;

    
	if (cid == CID_SW)
	{
		InitSwDlHash((SW*)plo);
		psw = (SW*)plo;
	}

	plo->paloParent = paloParent;
	// Storing pointer to scene world object
	plo->psw = psw;

	plo->pframe = PframeFromIsplice(isplice, psw);

	// Appending object to fist parent list
    AppendDlEntry(PdlFromSwOid(plo->psw, (OID)plo->oid) , plo);

	LO** head = plo->psw->aploCidHead + cid;
	plo->ploCidNext = *head;
	*head = plo;
	
	// Initializing local object
	plo->pvtlo->pfnInitLo(plo);

	// Storing pointer to object in global vector
	allWorldObjs.push_back(plo);

	// Returining newly made objects
	return plo;
}

int IploFromStockOid(int oid)
{
	int stockOid = oid - 0xC;

	if (stockOid > 30)
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

void LoadOptionsFromBrx(LO* pvObject, CBinaryInputStream* pbis)
{
	while (true)
	{
		// Reading eopid from binary file
		int16_t eopid = pbis->S16Read();
        
		if (eopid < 0) break;

		LoadOptionFromBrx(pvObject, &g_aeopid[eopid], pbis);
	}
}

void LoadOptionFromBrx(void* pvStruct, EOPID* peopid, CBinaryInputStream* pbis)
{
	const GRFEOPID grfeopid = peopid->grfeopid;

	if ((grfeopid & 0x400) != 0)
	{
		CRef aref[32]{};
		const int carg = static_cast<int>(peopid->optdat.crefReq);

		for (int iarg = 0; iarg < carg; ++iarg)
		{
			const OTYP otyp = static_cast<OTYP>(pbis->S16Read());

			switch (otyp)
			{
				case OTYP_Bool:
					aref[iarg].SetBool(static_cast<int8_t>(pbis->U8Read()));
					break;

				case OTYP_Float:
					aref[iarg].SetF32(pbis->F32Read());
					break;

				case OTYP_Vector:
					aref[iarg].SetVector(pbis->ReadVector());
					break;

				case OTYP_Int:
					aref[iarg].SetS32(pbis->S32Read());
					break;

				case OTYP_Oid:
				case OTYP_Cid:
				case OTYP_Sfxid:
				case OTYP_Wid:
				case OTYP_Tbid:
				case OTYP_Msgid:
				case OTYP_Optid:
					aref[iarg].SetS32(pbis->S16Read());
					break;

				case OTYP_Clq:
				{
					CLQ clq{};
					clq.g0 = pbis->F32Read();
					clq.g1 = pbis->F32Read();
					clq.g2 = pbis->F32Read();
					aref[iarg].SetClq(clq);
					break;
				}

				case OTYP_Lm:
				{
					LM lm{};
					lm.gMin = pbis->F32Read();
					lm.gMax = pbis->F32Read();
					aref[iarg].SetLm(lm);
					break;
				}

				default:
				{
					if ((static_cast<uint32_t>(static_cast<int16_t>(otyp)) & 0x7fffffffU) >> 12 == 1)
						aref[iarg].SetS32(pbis->S16Read());

					break;
				}
			}
		}

		peopid->optdat.pvThunkFnUser(static_cast<BASIC*>(pvStruct), carg, aref);
		return;
	}

	if ((grfeopid & 0x80) != 0)
	{
		void* pvBase = pvStruct;

		if ((grfeopid & 0x1000) != 0)
			pvBase = peopid->optdat.pfnensure(pvStruct, 1);

		std::byte* pbField = static_cast<std::byte*>(pvBase) + peopid->optdat.ibSetUser;

		switch (peopid->otyp)
		{
			case OTYP_Bool:
			*reinterpret_cast<uint8_t*>(pbField) = pbis->U8Read();
			return;

			case OTYP_Float:
			*reinterpret_cast<float*>(pbField) = pbis->F32Read();
			return;

			case OTYP_Vector:
			*reinterpret_cast<glm::vec3*>(pbField) = pbis->ReadVector();
			return;

			case OTYP_Clq:
			case OTYP_Smp:
			reinterpret_cast<float*>(pbField)[0] = pbis->F32Read();
			reinterpret_cast<float*>(pbField)[1] = pbis->F32Read();
			reinterpret_cast<float*>(pbField)[2] = pbis->F32Read();
			return;

			case OTYP_Vector4:
			case OTYP_Smpa:
			reinterpret_cast<float*>(pbField)[0] = pbis->F32Read();
			reinterpret_cast<float*>(pbField)[1] = pbis->F32Read();
			reinterpret_cast<float*>(pbField)[2] = pbis->F32Read();
			reinterpret_cast<float*>(pbField)[3] = pbis->F32Read();
			return;

			case OTYP_Matrix:
			{
				glm::vec3 euler = pbis->ReadVector();
				LoadRotateMatrix(&euler, reinterpret_cast<glm::mat3*>(pbField));
				return;
			}

			case OTYP_Int:
			*reinterpret_cast<int32_t*>(pbField) = pbis->S32Read();
			return;

			case OTYP_Oid:
			case OTYP_Cid:
			case OTYP_Sfxid:
			case OTYP_Wid:
			case OTYP_Tbid:
			case OTYP_Msgid:
			case OTYP_Optid:
			*reinterpret_cast<int32_t*>(pbField) = static_cast<int32_t>(pbis->S16Read());
			return;

			case OTYP_Lm:
			reinterpret_cast<float*>(pbField)[0] = pbis->F32Read();
			reinterpret_cast<float*>(pbField)[1] = pbis->F32Read();
			return;

			case OTYP_Rgba:
			*reinterpret_cast<uint32_t*>(pbField) = pbis->U32Read();
			return;

			default:
			*reinterpret_cast<int32_t*>(pbField) = static_cast<int32_t>(pbis->S16Read());
			return;
		}
	}

	PFNRAW pfnset = nullptr;

	if ((grfeopid & 0x100) != 0)
	{
		pfnset = peopid->optdat.pfnsetUser;

		if ((grfeopid & 0x1000) != 0)
			pvStruct = peopid->optdat.pfnensure(pvStruct, 1);
	}
	else
	{
		std::byte* pvt = *reinterpret_cast<std::byte**>(pvStruct);
		pfnset = *reinterpret_cast<PFNRAW*>(pvt + peopid->optdat.ibSetUser);
	}

	switch (peopid->otyp)
	{
		case OTYP_Bool:
		{
			const uint8_t value = pbis->U8Read();
			reinterpret_cast<void(*)(void*, uint8_t)>(pfnset)(pvStruct, value);
			return;
		}

		case OTYP_Float:
		{
			const float value = pbis->F32Read();
			reinterpret_cast<void(*)(void*, float)>(pfnset)(pvStruct, value);
			return;
		}

		case OTYP_Vector:
		{
			glm::vec3 value = pbis->ReadVector();
			reinterpret_cast<void(*)(void*, glm::vec3*)>(pfnset)(pvStruct, &value);
			return;
		}

		case OTYP_Clq:
		{
			CLQ value{};
			value.g0 = pbis->F32Read();
			value.g1 = pbis->F32Read();
			value.g2 = pbis->F32Read();
			reinterpret_cast<void(*)(void*, CLQ*)>(pfnset)(pvStruct, &value);
			return;
		}

		case OTYP_Smp:
		{
			SMP value{};
			value.svFast = pbis->F32Read();
			value.svSlow = pbis->F32Read();
			value.dtFast = pbis->F32Read();
			reinterpret_cast<void(*)(void*, SMP*)>(pfnset)(pvStruct, &value);
			return;
		}

		case OTYP_Vector4:
		{
			glm::vec4 value = pbis->ReadVector4();
			reinterpret_cast<void(*)(void*, glm::vec4*)>(pfnset)(pvStruct, &value);
			return;
		}

		case OTYP_Smpa:
		{
			SMPA value{};
			value.svFast = pbis->F32Read();
			value.sdvMax = pbis->F32Read();
			value.svSlow = pbis->F32Read();
			value.dtFast = pbis->F32Read();
			reinterpret_cast<void(*)(void*, SMPA*)>(pfnset)(pvStruct, &value);
			return;
		}

		case OTYP_Matrix:
		{
			glm::vec3 euler = pbis->ReadVector();
			glm::mat3 matrix{};
			LoadRotateMatrix(&euler, &matrix);
			reinterpret_cast<void(*)(void*, glm::mat3*)>(pfnset)(pvStruct, &matrix);
			return;
		}

		case OTYP_Int:
		{
			const int32_t value = pbis->S32Read();
			reinterpret_cast<void(*)(void*, int32_t)>(pfnset)(pvStruct, value);
			return;
		}

		case OTYP_Lm:
		{
			LM value{};
			value.gMin = pbis->F32Read();
			value.gMax = pbis->F32Read();
			reinterpret_cast<void(*)(void*, LM*)>(pfnset)(pvStruct, &value);
			return;
		}

		case OTYP_Rgba:
		{
			const uint32_t value = pbis->U32Read();
			reinterpret_cast<void(*)(void*, uint32_t)>(pfnset)(pvStruct, value);
			return;
		}

		case OTYP_Oid:
		case OTYP_Cid:
		case OTYP_Sfxid:
		case OTYP_Wid:
		case OTYP_Tbid:
		case OTYP_Msgid:
		{
			const int16_t value = pbis->S16Read();
			reinterpret_cast<void(*)(void*, int16_t)>(pfnset)(pvStruct, value);
			return;
		}

		default:
		{
			const int16_t value = pbis->S16Read();
			reinterpret_cast<void(*)(void*, int16_t)>(pfnset)(pvStruct, value);
			return;
		}
	}
}

void LoadSkipOptionFromBrx(void* pvObject, EOPID* peopid, int eopidID, CBinaryInputStream* pbis)
{
	(void)pvObject;
	(void)eopidID;

	if ((peopid->grfeopid & 0x400) != 0)
	{
		const int carg = static_cast<int>(peopid->optdat.crefReq);

		for (int iarg = 0; iarg < carg; ++iarg)
		{
			const OTYP otyp = static_cast<OTYP>(pbis->S16Read());

			switch (otyp)
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

				case OTYP_Matrix:
				pbis->ReadVector();
				break;

				case OTYP_Lm:
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
				case OTYP_Optid:
				pbis->S16Read();
				break;

				default:
				if ((static_cast<uint32_t>(static_cast<int16_t>(otyp)) & 0x7fffffffU) >> 12 == 1)
					pbis->S16Read();

				break;
			}
		}

		return;
	}

	switch (peopid->otyp)
	{
		case OTYP_Bool:
		pbis->U8Read();
		break;

		case OTYP_Float:
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

		case OTYP_Matrix:
		pbis->ReadVector();
		break;

		case OTYP_Int:
		pbis->S32Read();
		break;

		case OTYP_Lm:
		pbis->F32Read();
		pbis->F32Read();
		break;

		case OTYP_Rgba:
		pbis->U32Read();
		break;

		default:
		pbis->S16Read();
		break;
	}
}
