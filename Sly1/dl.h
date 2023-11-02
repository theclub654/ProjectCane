#pragma once
#include <iostream>
// Stores ptr to first entry in list and ptr to last entry in list
struct DL
{
	union
	{
		// Stores the first ptr to data
		void *pvFirst;
		class LO *ploFirst;
		class ALO *paloFirst;
		class SO *psoFirst;
	};

	union
	{
		// Stores the last ptr to the last data in list
		void *pvLast;
		class LO *ploLast;
		class ALO *paloLast;
		class SO *psoLast;
	};

	// Base offset to DL entry (DLE)
	uint64_t ibDle;
};

// DLE stores the next and previous ptr to data
// Linked list data structure
// Entry is just another word for linked list
struct DLE
{
	union
	{
		// Stores ptr to next data
		void* pvNext;
		class LO* ploNext;
		class ALO *paloNext;
		class SO *psoNext;
	};

	union
	{
		// Stores ptr to previous data
		void *pvPrev;
		class LO *ploPrev;
		class ALO *paloPrev;
		class SO *psoPrev;
	};
};


// Used for loading parent objects
struct DLI
{
	// Used for parent DL list
	DL *m_pdl;
	// Parent pointer value
	void **m_ppv;
	// Used for base offset to entry for parent object
	uint64_t m_ibDle;
	// Pointer to next parent object
	DLI *m_pdliNext;
};

// Initializing list
void InitDl(DL *pdl, int ibDle);
// Clearing DL list
void ClearDl(DL *pdl);
// Clearing entry linked list
void ClearDle(DLE *pdle);
// Returns entry from DL
DLE* PdleFromDlEntry(DL *pdl, void *pv);
// Appending data to end of DL
void AppendDlEntry(DL *pdl, void *pv);
// Inserts data at the front of DL list
void PrependDlEntry(DL *pdl, void *pv);
// Remove Entry from DL list
void RemoveDlEntry(DL *pdl, void *pv);
// Returns whether entry was found in DL list or not
int FFindDlEntry(DL *pdl, void *pv);
// Returns whether DL list is empty or not
int FIsDlEmpty(DL *pdl);


// Global DLI pointer to a parent object
static DLI *s_pdliFirst = nullptr;