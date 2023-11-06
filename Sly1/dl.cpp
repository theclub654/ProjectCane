#include "dl.h"

void InitDl(DL *pdl, int ibDle)
{
	// Setting base offset to data entry linked list
	pdl->ibDle = ibDle;
}

void ClearDl(DL *pdl)
{
	// Nulling DL
	pdl->pvFirst = (void*)0x0;
	pdl->pvLast = (void*)0x0;
}

void ClearDle(DLE *pdle)
{
	// Nulling entry list
	pdle->pvNext = (void*)0x0;
	pdle->pvPrev = (void*)0x0;
}

DLE* PdleFromDlEntry(DL *pdl, void *pv)
{
	// Returning the offset to entry
	return (DLE*)((uintptr_t)pv + pdl->ibDle);
}

void AppendDlEntry(DL *pdl, void *pv)
{
	// Loading entry from DL
	DLE *entry0 = PdleFromDlEntry(pdl, pv);

	// Checking to see if list is empty
	if (pdl->pvLast == nullptr)
		// If list is empty than it makes data the head node
		pdl->pvFirst = pv;

	else
	{
		// Loading last data's entry from list
		DLE *entry1 = PdleFromDlEntry(pdl, pdl->pvLast);
		entry0 = (DLE*)pdl;
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
	if (pdl->pvFirst == 0)
	{
		pdl->pvFirst = pv;
		pdl->pvLast = pv;
	}

	else
	{
		// Loading first entry linked list from list 
		DLE* entry1 = PdleFromDlEntry(pdl, pdl->pvFirst);
		entry0 = (DLE*)pdl;
		// Storing data in first DL entry node
		entry1->pvPrev = pv;
		// Storing data in first DL list since where tryna insert in front
		pdl->pvFirst = pv;
	}
}

void RemoveDlEntry(DL *pdl, void *pv)
{
	void *pv_1 = nullptr;
	DLE* entry0;
	DLE *entry1;
	DLI *entry2;

	// Loading entry thats being removed
	entry0 = PdleFromDlEntry(pdl, pv);

	if (s_pdliFirst == nullptr)
		pv = entry0->pvPrev;

	else
	{
		entry1 = (DLE*)s_pdliFirst->m_ppv;
		entry2 = s_pdliFirst;

		while (true)
		{
			if (entry1 == entry0)
			{
				if (entry0->pvPrev == 0x0)
					entry2->m_ppv = (void**)pdl;
				else
				{
					entry1 = PdleFromDlEntry(pdl, entry0->pvPrev);
					entry2->m_ppv = (void**)entry1;
				}
				entry2 = entry2->m_pdliNext;
			}

			else
				entry2 = entry2->m_pdliNext;

			if (entry2 == 0x0) break;

			entry1 = (DLE*)entry2->m_ppv;
		}

		pv_1 = entry0->pvPrev;
	}

	if (pv_1 == (void*)0x0)
		pdl = (DL*)entry0;

	else
	{
		entry1 = PdleFromDlEntry(pdl, pv_1);
		entry1 = entry0;
	}

	if (entry0->pvNext == nullptr)
		pdl = (DL*)entry0;

	else
	{
		entry1 = PdleFromDlEntry(pdl, entry0->pvNext);
		entry1 = entry0;
	}

	entry0->pvPrev = 0x0;
	entry0->pvNext = 0x0;
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
	if (pdl->pvFirst == (void*)0x0)
		return 1;
	else
		return 0;
}