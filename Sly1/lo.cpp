#include "lo.h"

void InitLo(LO* parentLo)
{
	SetLoDefaults(parentLo);
	parentLo->pvtlo->pfnAddLo(parentLo);
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
}

void CloneLo(LO* plo, LO* ploBase)
{
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

int FIsLoInWorld(LO* plo)
{
	if (plo != 0)
	{
		// Loading objects parent
		ALO* palo = plo->paloParent;

		while (true)
		{
			// Loading objects child list
			DL* objectChild = &palo->dlChild;

			// Loading SW dlChild if parent LO doenst have a parent
			if (palo == nullptr)
				objectChild = &plo->psw->dlChild;

			// Checks to see if object has a child 
			bool isFound = FFindDlEntry(objectChild, plo);

			// If not found than return 0
			if (isFound == 0)
				return 0;

			// If object doesnt have a parent
			if (palo == 0x0)
				break;

			// Loading parent object if LO has one
			plo = (LO*)palo->paloParent;
		}
	}

	// If object is found return 1
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
