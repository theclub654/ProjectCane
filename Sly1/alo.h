#pragma once
#include "lo.h"
#include "bis.h"
#include "vec.h"

class LO;

class ALO
{
	public:
		// Class ID
		CID cid;
		// Object ID
		OID oid;
		DLE dleOid;
		// Parent SW Object
		class SW* psw;
		// Objects Parent
		class ALO* paloParent;
		DLE dleChild;
		LO *ploCidNext;
		// Objects name
		char *pchzName;
		uint64_t dtickPerf;
		// Object child
		DL dlChild;
		DLE dleBusy;
		DLE dleMRD;
		ALO *paloRoot;
		ALO *paloFreezeRoot;
		DLE dleFreeze;
		DL dlFreeze;
		int cpmrg;
		//MRG *apmrg;
		float sMRD;
		float sCelBorderMRD;
		int grfzon;
		float dsMRDSnap;
		
		ALO(CID cid, SW* psw, ALO* paloParent, OID oid);
};

// Initialize ALO object
void InitAlo(ALO* palo); // NOT FINISHED
void RemoveAloHierarchy(ALO* palo); // NOT FINISHED
// Loads ALO object from binary file
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);