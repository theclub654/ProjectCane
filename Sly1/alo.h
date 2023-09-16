#pragma once
#include "lo.h"
#include "glob.h"

class SO;
void OnSoAdd(SO *pso);

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
	float sFastShadowRadius;
	float sFastShadowDepth;
	int fRealClock;
	float dtUpdatePause;
	float sRadiusRenderSelf;
	float sRadiusRenderAll;
};

// Initialize ALO object
void InitAlo(ALO* palo); // NOT FINISHED
void AddAloHierarchy(ALO *palo); // NOT FINISHED
void RemoveAloHierarchy(ALO *palo); // NOT FINISHED
// Loads ALO object from binary file
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
void LoadAloAloxFromBrx(CBinaryInputStream* pbis);