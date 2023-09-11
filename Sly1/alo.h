#pragma once
#include "lo.h"
#include "glob.h"

class ALO : public LO
{
public:
	DL dlChild;
	DLE dleBusy;
	DLE dleMRD;
	ALO* paloRoot;
	ALO* paloFreezeRoot;
	DLE dleFreeze;
	DL dlFreeze;
	int cpmrg;
	float sMRD;
	float sCelBorderMRD;
	int grfzon;
	float dsMRDSnap;
	glm::vec3 posOrig;
	glm::mat3 matOrig;
	glm::vec3 eulOrig;
	DL dlAct;

	ALO(CID cid, SW* psw, ALO* paloParent, OID oid);
};

// Initialize ALO object
void InitAlo(ALO* palo); // NOT FINISHED
void RemoveAloHierarchy(ALO* palo); // NOT FINISHED
// Loads ALO object from binary file
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
void LoadAloAloxFromBrx(CBinaryInputStream* pbis);