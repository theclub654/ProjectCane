#pragma once
#include <iostream>
// Stores ptr to first entry in list and ptr to last entry in list
struct DL
{
	// Stores the first ptr to data
	void* pvFirst;
	// Stores the last ptr to the last data in list
	void* pvLast;
	// Base offset to DL entry (DLE)
	int ibDle;
};

// DLE stores the next and previous ptr to data
// Linked list data structure
// Entry is just another word for linked list
struct DLE
{
	// Stores ptr to next data
	void* pvNext;
	// Stores ptr to previous data
	void* pvPrev;
};


// Used for loading parent objects
struct DLI
{
	// Used for parent DL list
	DL *m_pdl;
	// Used for parent PV
	void **m_ppv;
	// Used for base offset to entry for parent object
	int m_ibDle;
	// Used to take you to next parent object
	DLI *m_pdliNext;
};

// Global DLI pointer to a parent object
static DLI *s_pdliFirst;

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