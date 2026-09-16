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

	// Appending an entry that is already in this intrusive list makes the
	// previous tail point to itself when it is also the current tail.  Several
	// post-load paths can be revisited, so preserve the existing membership
	// instead of corrupting the chain.
	if (node->pvNext != nullptr || node->pvPrev != nullptr || pdl->pvFirst == pv || pdl->pvLast == pv)
		return;

	node->pvNext = nullptr;

	if (pdl->pvLast == nullptr)
	{
		node->pvPrev = nullptr;
		pdl->pvFirst = pv;
	}
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
	entry0->pvPrev = nullptr;

	// Storing data in DL list if DL list is empty
	if (pdl->pvFirst == nullptr)
	{
		entry0->pvNext = nullptr;
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

void InsertDlEntryBefore(DL* pdl, void* pvNext, void* pv)
{
	if (pvNext == nullptr)
		AppendDlEntry(pdl, pv);
	else if (pvNext == pdl->pvFirst)
		PrependDlEntry(pdl, pv);
	else
	{
		DLE* entry = PdleFromDlEntry(pdl, pv);
		DLE* next  = PdleFromDlEntry(pdl, pvNext);

		void* pvPrev = next->pvPrev;
		DLE* prev    = PdleFromDlEntry(pdl, pvPrev);

		entry->pvPrev = pvPrev;
		entry->pvNext = pvNext;

		next->pvPrev = pv;
		prev->pvNext = pv;
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

void MergeDl(DL* dst, DL* src)
{
	if (src->pvFirst == nullptr)
		return;

	// If destination is empty, just transfer the whole list.
	if (dst->pvFirst == nullptr)
	{
		dst->pvFirst = src->pvFirst;
		dst->pvLast  = src->pvLast;
		dst->ibDle   = src->ibDle;

		ClearDl(src);
		return;
	}

	// Both lists have entries: link dst tail to src head.
	DLE* dstLast  = PdleFromDlEntry(dst, dst->pvLast);
	DLE* srcFirst = PdleFromDlEntry(src, src->pvFirst);

	dstLast->pvNext  = src->pvFirst;
	srcFirst->pvPrev = dst->pvLast;

	dst->pvLast = src->pvLast;

	ClearDl(src);
}

int CPvDl(DL* pdl)
{
	int count = 0;
	void* current = pdl->pvFirst;

	while (current != nullptr)
	{
		current = *(void**)((char*)current + pdl->ibDle);
		++count;
	}

	return count;
}

DLI* s_pdliFirst = nullptr;
