#pragma once
#include "spaprops.h"
#include "sw.h"
#include "alo.h"

class LO
{
	public:
		// Class ID
		CID cid;
		// Object ID
		OID oid;
		DLE dleOid;
		// Parent SW Object
		SW *psw;
		// Objects Parent
		ALO *paloParent;
		DLE dleChild;
		LO *ploCidNext;
		// Objects name
		char *pchzName;
		uint64_t dtickPerf;
};

// Initializes LO
void InitLo(LO *parentLo);
void SetLoDefaults(LO* parentLo);
// Adds LO to parent LO if LO doesnt have a parent it makes LO a parent
void AddLo(LO* plo); // NOT FINISHED
// Remove LO 
void RemoveLo(LO* plo); // NOT FINISHED
// Returns whether LO is in world or not
int FIsLoInWorld(LO* plo);