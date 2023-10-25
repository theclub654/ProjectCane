#pragma once
#include "lo.h"
#include "glob.h"

struct XF
{
	glm::mat3 mat;
	glm::vec3 pos;
	glm::mat3 matWorld;
	glm::vec3 posWorld;
	glm::vec3 v;
	glm::vec3 w;
	glm::vec3 dv;
	glm::vec3 dw;
};

class ALO : public LO
{
public:
	DL dlChild;
	DLE dleBusy;
	DLE dleMRD;
	ALO *paloRoot;
	ALO *paloFreezeRoot;
	DLE dleFreeze;
	DL dlFreeze;
	int cpmrg;
	struct MRG *apmrg[4];
	float sMRD;
	float sCelBorderMRD;
	int grfzon;
	float dsMRDSnap;
	XF xf;
	glm::vec3 posOrig;
	glm::mat3 matOrig;
	glm::vec3 eulOrig;
	DL dlAct;
	GLOBSET globset;
	float sFastShadowRadius;
	float sFastShadowDepth;
	int fRealClock;
	float dtUpdatePause;
	float sRadiusRenderSelf;
	float sRadiusRenderAll;
};

// Merge
// Used to merge ALO object's
struct MRG
{
	DLE dle;
	// Number of ALO objects to be merged
	int cpalo;
	// The ALO object thats gonna be merged
	ALO **apalo;
};

// Initialize ALO object
void InitAlo(ALO* palo); // NOT FINISHED
void AddAloHierarchy(ALO *palo); // NOT FINISHED
void RemoveAloHierarchy(ALO *palo); // NOT FINISHED
void OnAloAdd(ALO* palo); // NOT FINISHED
void OnAloRemove(ALO* palo);
void CloneAloHierarchy(ALO* palo);
void CloneAlo(ALO* palo);
// Loads ALO object from binary file
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
void LoadAloAloxFromBrx(CBinaryInputStream* pbis);
void UpdateAlo(ALO *palo, float dt);