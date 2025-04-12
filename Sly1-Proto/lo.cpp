#include "lo.h"

LO* NewLo()
{
	return new LO{};
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

		// Adds object to hierarchy
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
	plo->pvtlo->pfnOnLoRemove(plo);
	SendLoMessage(plo, MSGID_removed, plo);
}

void SnipLo(LO* plo)
{

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
	// Returns object to be cloned
	LO* localObject = PloNew(plo->pvtbasic->cid, psw, paloParent, plo->oid, -1);
	// Clones the base object and it's children
	localObject->pvtlo->pfnCloneLoHierarchy(localObject, plo);
	// Returns cloned object
	return localObject;
}

void SendLoMessage(LO *plo, MSGID msgid, void *pv)
{

}

void LoadLoFromBrx(LO* plo, CBinaryInputStream* pbis)
{
	LoadOptionsFromBrx(plo, pbis);
}

void RemoveLo(LO* plo)
{
	// Loading objects parent child list
	DL* objectChildList = &plo->paloParent->dlChild;

	// If object doesnt have a parent load up the static world dlChild
	if (plo->paloParent == nullptr)
		objectChildList = &plo->psw->dlChild;

	// Returns if parent LO or SW has a child object or not
	bool isFound = FFindDlEntry(objectChildList, plo);

	if (isFound != 0)
	{
		// Returning if object is in world or not
		isFound = FIsLoInWorld(plo);

		if (isFound == 0)
			RemoveDlEntry(objectChildList, plo);

		else
		{
			RemoveDlEntry(objectChildList, plo);
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
	if (!plo) return 1;

	ALO* parent = plo->paloParent;

	while (true) {
		ALO* palo = parent;
		DL* pdl = palo ? &palo->dlChild : &plo->psw->dlChild;

		if (!FFindDlEntry(pdl, plo))
			return 0;

		if (!palo)
			break;

		parent = palo->paloParent;
		plo = static_cast<LO*>(palo);
	}

	return 1;
}

void PostLoLoad(LO* plo)
{

}

void SetLoParent(LO *plo, ALO *paloParent)
{
	plo->pvtlo->pfnRemoveLo(plo);
	plo->paloParent = paloParent;
	plo->pvtlo->pfnAddLo(plo);
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
	delete plo;
}
