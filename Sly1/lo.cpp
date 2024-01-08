#include "lo.h"

void* NewLo()
{
	return new LO;
}

void InitLo(LO* plo)
{
	plo->pvtlo->pfnSetLoDefaults(plo);
	plo->pvtlo->pfnAddLo(plo);
}

void SetLoDefaults(LO* parentLo)
{

}

void AddLo(LO *plo)
{
	// Loading objects parent child list
	DL *objectChildList = &plo->paloParent->dlChild;

	// If object doesnt have a parent load up the static world dlChild
	if (plo->paloParent == nullptr)
		objectChildList = &plo->psw->dlChild;

	// Returns if parent LO or SW has a child object or not
	bool isFound = FFindDlEntry(objectChildList, plo);

	if (isFound == 0)
	{
		// Storing object child in parent LO
		AppendDlEntry(objectChildList, plo);
		// Returns if LO is in world or not
		isFound = FIsLoInWorld(plo);

		if (isFound != 0)
			plo->pvtlo->pfnAddLoHierarchy(plo);
	}
}

void AddLoHierarchy(LO *plo)
{
	SendLoMessage(plo, MSGID_added, plo);
}

void RemoveLoHierarchy(LO* plo)
{
	SendLoMessage(plo, MSGID_removed, plo);
}

void CloneLoHierarchy(LO* plo, LO* ploBase)
{
	plo->pvtlo->pfnCloneLo(plo, ploBase);
}

void CloneLo(LO* plo, LO* ploBase)
{
	RemoveDlEntry(PdlFromSwOid(plo->psw, plo->oid), plo);
	AppendDlEntry(PdlFromSwOid(ploBase->psw, ploBase->oid), plo);

	plo->oid = ploBase->oid;
	plo->pchzName = ploBase->pchzName;
	plo->ppxr = ploBase->ppxr;
}

LO* PloCloneLo(LO* plo, SW* psw, ALO* paloParent)
{
	LO* localObject = PloNew(plo->pvtbasic->cid, psw, paloParent, plo->oid, -1);
	localObject->pvtlo->pfnCloneLoHierarchy(localObject, plo);
	return localObject;
}

void SendLoMessage(LO *plo, MSGID msgid, void *pv)
{

}

void LoadLoFromBrx(LO* plo, CBinaryInputStream* pbis)
{
	LoadOptionFromBrx(plo, pbis);
}

void RemoveLo(LO* plo)
{
	// Loading objects parent
	ALO* objectParent = plo->paloParent;
	// Loading objects parent child list
	DL* objectChild = &plo->paloParent->dlChild;

	// If object doesnt have a parent load up the static world dlChild
	if (objectParent == nullptr)
		objectChild = &plo->psw->dlChild;

	// Returns if parent LO or SW has a child object or not
	bool isFound = FFindDlEntry(objectChild, plo);

	if (isFound != 0)
	{
		// Returning if object is in world or not
		isFound = FIsLoInWorld(plo);

		if (isFound == 0)
			RemoveDlEntry(objectChild, plo);

		else
		{
			RemoveDlEntry(objectChild, plo);
			plo->pvtlo->pfnRemoveLoHierarchy(plo);
		}
	}
}

void OnLoAdd(LO* plo)
{

}

void OnLoRemove(LO* plo)
{

}

int FIsLoInWorld(LO *plo)
{
	if (plo != nullptr) 
	{
		ALO *loPaloParent = plo->paloParent;

		while (true) 
		{
			ALO *palo = loPaloParent;

			DL *pdl = &palo->dlChild;

			if (palo == (ALO*)0x0) 
				pdl = &plo->psw->dlChild;

			int isFound = FFindDlEntry(pdl, plo);

			if (isFound == 0)
				return 0;
			
			if (palo == (ALO*)0x0) break;

			loPaloParent = palo->paloParent;

			plo = (LO*)palo;
		}
	}
	return 1;
}

void PostLoLoad(LO* plo)
{

}

void SetLoParent(LO *plo, ALO *paloParent)
{

}

void SubscribeLoObject(LO* plo, LO* ploTarget)
{
}

void UnsubscribeLoObject(LO* plo, LO* ploTarget)
{
}

void SubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext)
{
}

void UnsubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext)
{
}

int GetLoSize()
{
	return sizeof(LO);
}

void DeleteLo(LO* plo)
{
	delete (LO*)plo;
}
