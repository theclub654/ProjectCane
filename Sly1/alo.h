#pragma once
#include "lo.h"
#include "freeze.h"
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
struct RO
{
	glm::mat4 mat;
	float uAlpha;
	float uAlphaCelBorder;
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
	int cframeStatic;
	GLOBSET globset;
	struct SHADOW *pshadow;
	float sFastShadowRadius;
	float sFastShadowDepth;
	int fRealClock;
	float dtUpdatePause;
	float sRadiusRenderSelf;
	float sRadiusRenderAll;
	int cposec;
};

// Initialize ALO object
void InitAlo(ALO* palo); // NOT FINISHED
// Adds alo parent and all the alo childs into the world
void AddAloHierarchy(ALO *palo); // NOT FINISHED
void RemoveAloHierarchy(ALO *palo); // NOT FINISHED
void OnAloAdd(ALO* palo); // NOT FINISHED
void OnAloRemove(ALO* palo);
void CloneAloHierarchy(ALO* palo);
void CloneAlo(ALO* palo);
void ResolveAlo(ALO *palo);
// Loads ALO object from binary file
void LoadAloFromBrx(ALO* palo, CBinaryInputStream* pbis);
void LoadAloAloxFromBrx(CBinaryInputStream* pbis);
void UpdateAlo(ALO *palo, float dt);
void RenderAloAll(ALO* palo, CM* pcm, RO* proDup);
void RenderAloSelf(ALO* palo, CM* pcm, RO* pro);
void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro);
void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha);
void RenderAloAsBone(ALO* palo, CM* pcm, RO* pro);