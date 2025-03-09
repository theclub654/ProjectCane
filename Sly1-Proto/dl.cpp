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
	// Holds ptr to previous entry
	void* prevEntry{};
	// Data entry to be removed
	DLE* entry0{};
	DLE* entry1{};
	DLI* entry2{};

	// Loading data's entry thats being removed
	entry0 = PdleFromDlEntry(pdl, pv);

	if (s_pdliFirst == (DLI*)nullptr)
		// Loads ptr to previous entry data list
		prevEntry = entry0->pvPrev;

	else
	{
		// Loads previous entry's data list
		entry1 = (DLE*)s_pdliFirst->m_ppv;
		entry2 = s_pdliFirst;

		while (true)
		{
			if (entry1 == entry0)
			{
				if (entry0->pvPrev == nullptr)
					entry2->m_ppv = (void**)(uintptr_t)pdl;
				else
				{
					entry1 = PdleFromDlEntry(pdl, entry0->pvPrev);
					entry2->m_ppv = (void**)(uintptr_t)entry1;
				}
				entry2 = entry2->m_pdliNext;
			}

			else
				entry2 = entry2->m_pdliNext;

			if (entry2 == nullptr) 
				break;

			entry1 = (DLE*)entry2->m_ppv;
		}

		prevEntry = entry0->pvPrev;
	}

	if (prevEntry == nullptr)
		pdl->pvFirst = (DL*)entry0->pvNext;

	else
	{
		entry1 = PdleFromDlEntry(pdl, prevEntry);
		entry1->pvNext = entry0->pvNext;
	}

	if (entry0->pvNext == nullptr)
		pdl->pvLast = (DL*)entry0->pvPrev;

	else
	{
		entry1 = PdleFromDlEntry(pdl, entry0->pvNext);
		entry1->pvPrev = entry0->pvPrev;
	}

	entry0->pvPrev = nullptr;
	entry0->pvNext = nullptr;
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