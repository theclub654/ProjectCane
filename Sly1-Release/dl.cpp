#include "dl.h"

void InitDl(DL *pdl, int ibDle)
{
	// Setting base offset to data entry
	pdl->ibDle = ibDle;
}

void ClearDl(DL *pdl)
{
	// Nulling DL
	pdl->pvFirst = nullptr;
	pdl->pvLast  = nullptr;
}

void ClearDle(DLE *pdle)
{
	// Nulling entry list
	pdle->pvNext = nullptr;
	pdle->pvPrev = nullptr;
}

DLE* PdleFromDlEntry(DL *pdl, void *pv)
{
	// Returning the offset to entry list
	return (DLE*)((uintptr_t)pv + pdl->ibDle);
}

void AppendDlEntry(DL *pdl, void *pv)
{
	// Loading entry from data list 
	DLE *entry0 = PdleFromDlEntry(pdl, pv);

	// Checking to see if list is empty
	if (pdl->pvLast == nullptr)
		// If list is empty than it makes data the head node
		pdl->pvFirst = pv;

	else
	{
		// Loading last entry from list
		DLE *entry1 = PdleFromDlEntry(pdl, pdl->pvLast);
		entry0->pvPrev = (DLE*)pdl->pvLast;
		// Storing the new next data entry in entry list
		entry1->pvNext = pv;
	}

	// Storing data at the end of list
	pdl->pvLast = pv;
}

void PrependDlEntry(DL* pdl, void* pv)
{
	// Loading entry from DL
	DLE *entry0 = PdleFromDlEntry(pdl, pv);

	// Storing data in DL list if DL list is empty
	if (pdl->pvFirst == nullptr)
	{
		pdl->pvFirst = pv;
		pdl->pvLast  = pv;
	}

	else
	{
		// Loading first entry linked list from list 
		DLE* entry1 = PdleFromDlEntry(pdl, pdl->pvFirst);
		entry0->pvNext = (DLE*)pdl->pvFirst;
		// Storing data in first DL entry node
		entry1->pvPrev = pv;
		// Storing data in first DL list since where tryna insert in front
		pdl->pvFirst = pv;
	}
}

void RemoveDlEntry(DL *pdl, void *pv)
{
	if (!pdl || !pv) return;

	DLE* dle = PdleFromDlEntry(pdl, pv);
	if (!dle) return;

	void* prev = dle->pvPrev;
	void* next = dle->pvNext;

	// Update previous element's next pointer
	if (!prev) {
		// Removing first element
		pdl->pvFirst = next;
	}
	else {
		DLE* prevDle = PdleFromDlEntry(pdl, prev);
		if (prevDle) {
			prevDle->pvNext = next;
		}
	}

	// Update next element's previous pointer
	if (!next) {
		// Removing last element
		pdl->pvLast = prev;
	}
	else {
		DLE* nextDle = PdleFromDlEntry(pdl, next);
		if (nextDle) {
			nextDle->pvPrev = prev;
		}
	}

	// Clear links in removed entry
	dle->pvPrev = nullptr;
	dle->pvNext = nullptr;
}

int FFindDlEntry(DL *pdl, void *pv)
{
	// Loading entry from list
	DLE *entry = PdleFromDlEntry(pdl, pv);

	// Checks to see if list has a entry
	if (entry->pvNext != 0 || pdl->pvLast == pv)
		return 1;
	else
		return 0;
}

int FIsDlEmpty(DL* pdl)
{
	// Seeing if DL list is empty
	if (pdl->pvFirst == nullptr)
		return 1;
	else
		return 0;
}