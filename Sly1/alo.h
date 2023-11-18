#pragma once
#include "lo.h"
#include "freeze.h"
#include "glob.h"
#include "sqtr.h"

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
struct POSEC
{
	OID oid;
	std::vector <float> agPoses;
};
struct RSMG
{
	OID oidRoot;
	OID oidSM;
	OID oidTriggerGoal;
	OID oidUntriggerGoal;
};
enum ACK
{
	ACK_Nil = -1,
	ACK_None = 0,
	ACK_Spring = 1,
	ACK_Velocity = 2,
	ACK_Smooth = 3,
	ACK_Spline = 4,
	ACK_Drive = 5,
	ACK_SmoothForce = 6,
	ACK_SmoothLock = 7,
	ACK_SpringLock = 8,
	ACK_SmoothNoLock = 9,
	ACK_Max = 10
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
	char frz[64];
	XF xf;
	glm::vec3 posOrig;
	glm::mat3 matOrig;
	glm::vec3 eulOrig;
	DL dlAct;
	struct ACT* pactPos;
	struct ACT* pactRot;
	struct ACT* pactScale;
	struct ACT** apactPose;
	struct ACT* pactRestore;
	struct ACTLA* pactla;
	struct ACTBANK* pactbank;
	struct IKH* pikh;
	struct CLQ* pclqPosSpring;
	struct CLQ* pclqPosDamping;
	struct CLQ* pclqRotSpring;
	struct CLQ* pclqRotDamping;
	struct SMPA* psmpaPos;
	struct SMPA* psmpaRot;
	struct ALOX* palox;
	int cframeStatic;
	GLOBSET globset;
	struct SHADOW *pshadow;
	struct THROB* pthrob;
	float sFastShadowRadius;
	float sFastShadowDepth;
	int fRealClock;
	struct FADER* pfader;
	float dtUpdatePause;
	struct ASEGD* pasegd;
	float sRadiusRenderSelf;
	float sRadiusRenderAll;
	struct SFX* psfx;
	char ficg[5];
	int cposec;
	std::vector <POSEC> aposec;
	struct ACTREF* pactrefCombo;
	struct DLR* pdlrFirst;
	ACK ackRot;
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
// Loads bone data from binary file
void LoadAloAloxFromBrx(CBinaryInputStream* pbis);
void UpdateAlo(ALO *palo, float dt);
void RenderAloAll(ALO* palo, CM* pcm, RO* proDup);
void RenderAloSelf(ALO* palo, CM* pcm, RO* pro);
void RenderAloGlobset(ALO* palo, CM* pcm, RO* pro);
void RenderAloLine(ALO* palo, CM* pcm, glm::vec3* ppos0, glm::vec3* ppos1, float rWidth, float uAlpha);
void RenderAloAsBone(ALO* palo, CM* pcm, RO* pro);
void DrawLo(ALO *palo);
// Deletes all asset data from VRAM
void FreeGLBuffers(SW* psw);
// Deletes Model from VRAM
void DeleteModel(ALO *palo);
void DeleteAlo(LO *palo);