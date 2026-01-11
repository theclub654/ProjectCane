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
	DLE* node = PdleFromDlEntry(pdl, pv);

	if (pdl->pvLast == nullptr)
		pdl->pvFirst = pv;
	else
	{
		DLE* lastNode = PdleFromDlEntry(pdl, pdl->pvLast);
		node->pvPrev = pdl->pvLast;    // <-- store previous OBJECT pointer
		lastNode->pvNext = pv;
	}

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
	DLE* dle = PdleFromDlEntry(pdl, pv);
	if (!dle) return;

	if (s_pdliFirst != nullptr)
	{
		for (DLI* it = s_pdliFirst; it != nullptr; it = it->m_pdliNext)
		{
			// m_ppv can equal the DLE being removed.
			// If so, repoint it so iteration remains valid.
			if ((DLE*)it->m_ppv == dle)
			{
				if (dle->pvPrev == nullptr)
				{
					// Removing first element: iterator should point at DL head storage.
					it->m_ppv = (void**)pdl; // points at pdl->pvFirst
				}
				else
				{
					// Otherwise point at previous element's DLE.
					DLE* prevDle = PdleFromDlEntry(pdl, dle->pvPrev);
					it->m_ppv = (void**)prevDle;
				}
			}
		}
	}

	void* prev = dle->pvPrev;
	void* next = dle->pvNext;

	// --- unlink from prev / fix head ---
	if (prev == nullptr)
		pdl->pvFirst = next;
	else
		PdleFromDlEntry(pdl, prev)->pvNext = next;

	// --- unlink from next / fix tail ---
	if (next == nullptr)
		pdl->pvLast = prev;
	else
		PdleFromDlEntry(pdl, next)->pvPrev = prev;

	// clear removed links
	dle->pvPrev = nullptr;
	dle->pvNext = nullptr;
}

int FFindDlEntry(DL *pdl, void *pv)
{
	// Loading entry from list
	DLE *entry = PdleFromDlEntry(pdl, pv);

	// Checks to see if list has a entry
	if (entry->pvNext != nullptr || pdl->pvLast == pv)
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

DLI* s_pdliFirst = nullptr;