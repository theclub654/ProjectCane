#pragma once
#include "so.h"
#include "wipe.h"
#include "tn.h"

enum EXITS
{
	EXITS_Nil = -1,
	EXITS_Blocked = 0,
	EXITS_Unblocked = 1,
	EXITS_Disabled = 2,
	EXITS_Enabled = 3,
	EXITS_Totals = 4,
	EXITS_Exiting = 5,
	EXITS_Max = 6
};
enum EDK
{
	EDK_Nil = -1,
	EDK_DestinationKey = 0,
	EDK_CurrentKey = 1,
	EDK_Max = 2
};

class XFM : public LO
{
	public:
	glm::vec3 posLocal;
	glm::mat3 matLocal;
};

class WARP : public XFM
{
	public:
	glm::vec3 v;
	int fDefault;
	int fDefaultSpecial;
	float radCmInit;
	int cpaseg;
	std::vector <ASEG*> apaseg;
	int coidHide;
	std::vector <OID> aoidHide;
	int coidShowWhenDifficult;
	short aoidShowWhenDifficult[4];
	short oidAlias;
	int crsmg;
	RSMG arsmg[4];
	int fDefaultDifficult;
	int fUseRadCmInit;
};

class EXIT : public ALO
{
	public:
	LEVELINFO* plevel = nullptr;
	LEVELINFO* plevelAlt = nullptr;
	int fDefault;
	int fKeyed;
	int fFollowDefault;
	int fTotals;
	EXITS exits;
	float tExits;
	int ctsurf;
	std::vector <TSURF> atsurf;
	int ctbsp;
	std::vector <TBSP> atbsp;
	WID widWarp;
	short oidWarp;
	EDK edkAlt;
	WID widAlt;
	short oidAlt;
	int cpaseg;
	std::vector <ASEG*> apaseg;
	WIPEK wipek;
	float tWipe;
	float dtUnblock;
	float dtTriggerWipe;
	std::string world;
	std::string worldAlt;
	short oidWarpAlt;
	int fUseDefaultExit;
};

class CAMERA : public ALO
{
	public:
	short oidTarget;
	struct PNT* ppntTarget;
	class ALO* paloTarget;
	glm::vec3 posEye;
	glm::vec3 vecView;
	glm::vec3 vecUp;
	int fSetCplcy;
};

XFM* NewXfm();
void InitXfm(XFM* pxfm);
int  GetXfmSize();
void LoadXfmFromBrx(XFM* pxfm, CBinaryInputStream* pbis);
void GetXfmPos(XFM* pxfm, glm::vec3* ppos);
void GetXfmMat(XFM* pxfm, glm::mat3* pmat);
void*GetXfmPosLocal(XFM* pxfm);
void SetXfmPosLocal(XFM* pxfm, glm::vec3* pposlocal);
void*GetXfmMatLocal(XFM* pxfm);
void SetXfmMatLocal(XFM* pxfm, glm::mat3* pmatLocal);
void CloneXfm(XFM* pxfm, XFM* pxfmBase);
void SetXfmParent(XFM* pxfm, ALO* paloParent);
void ApplyXfmProxy(XFM* pxfm, PROXY* pproxyApply);
void ConvertXfmWorldToLocal(XFM* pxfm, glm::vec3* pposWorld, glm::vec3* pposLocal);
void DeleteXfm(XFM* pxfm);

WARP*NewWarp();
int  GetWarpSize();
void*GetWarpV(WARP* pwarp);
void SetWarpV(WARP* pwarp, glm::vec3 v);
void*GetWarpVx(WARP* pwarp);
void SetWarpVx(WARP* pwarp, float x);
void*GetWarpVy(WARP* pwarp);
void SetWarpVy(WARP* pwarp, float y);
void*GetWarpVz(WARP* pwarp);
void SetWarpVz(WARP* pwarp, float z);
void*GetWarpFDefault(WARP* pwarp);
void SetWarpFDefault(WARP* pwarp, int fDefault);
void*GetWarpFDefaultDifficult(WARP* pwarp);
void SetWarpFDefaultDifficult(WARP* pwarp, int fDefaultDifficult);
void*GetWarpOidAlias(WARP* pwarp);
void SetWarpOidAlias(WARP* pwarp, int oidAlias);
void*GetWarpRadCmInit(WARP* pwarp);
WARP*PwarpFromOid(OID oid, OID oidContext);
void LoadWarpFromBrx(WARP* pwarp, CBinaryInputStream* pbis);
void CloneWarp(WARP* pwarp, WARP* pwarpBase);
void PostWarpLoad(WARP* pwarp);
void TriggerWarp(WARP* pwarp);
void SetWarpRsmg(WARP* pwarp, int fOnTrigger, int oidRoot, int oidSM, int oidGoal);
void SetWarpCmInitAngle(WARP* pwarp, float radCmInit);
void TeleportSwPlayer(SW* psw, OID oidWarp, OID oidWarpContext);
void DeleteWarp(WARP* pwarp);

EXIT*NewExit();
int  GetExitSize();
void*GetExitOidWarp(EXIT* pexit);
void SetExitOidWarp(EXIT* pexit, int oidWarp);
void*GetExitEdkAlt(EXIT* pexit);
void SetExitEdkAlt(EXIT* pexit, EDK edkAlt);
void*GetExitOidWarpAlt(EXIT* pexit);
void SetExitOidWarpAlt(EXIT* pexit, int oidWarpAlt);
void*GetExitWipek(EXIT* pexit);
void SetExitWipek(EXIT* pexit, WIPEK wipek);
void*GetExitDtUnblock(EXIT* pexit);
void SetExitDtUnblock(EXIT* pexit, float dtUnblock);
void*GetExitFDefault(EXIT* pexit);
void SetExitFDefault(EXIT* pexit, int fDefault);
void*GetExitFKeyed(EXIT* pexit);
void SetExitFKeyed(EXIT* pexit, int fKeyed);
void*GetExitFFollowDefault(EXIT* pexit);
void SetExitFFollowDefault(EXIT* pexit, int fFollowDefault);
void*GetExitFUseDefaultExit(EXIT* pexit);
void SetExitFUseDefaultExit(EXIT* pexit, int fUseDefaultExit);
void*GetExitFTotals(EXIT* pexit);
void SetExitFTotals(EXIT* pexit, int fTotals);
void*GetExitDtTriggerWipe(EXIT* pexit);
void SetExitDtTriggerWipe(EXIT* pexit, float dtTriggerWipe);
void*GetExitExits(EXIT* pexit);
EXIT*PexitDefault();
void LoadExitFromBrx(EXIT *pexit, CBinaryInputStream* pbis);
void CloneExit(EXIT* pexit, EXIT* pexitBase);
void PostExitLoad(EXIT* pexit);
void SetExitExits(EXIT* pexit, EXITS exits);
// GOTTA COME BACK TO THIS 
void TriggerDefaultExit(int fInhibitAsegs, WIPEK wipek);
void TriggerExit(EXIT* pexit);
void WipeExit(EXIT* pexit);
void UpdateExit(EXIT* pexit, float dt);
void DeleteExit(EXIT* pexit);

CAMERA*NewCamera();
void InitCamera(CAMERA* pcamera);
void SetCameraOidTarget(CAMERA* pcamera, OID oidTarget);
void*GetCameraOidTarget(CAMERA* pcamera);
void SetCameraPosEye(CAMERA* pcamera, glm::vec3 posEye);
void*GetCameraPosEye(CAMERA* pcamera);
void SetCameraVecView(CAMERA* pcamera, glm::vec3 vecView);
void*GetCameraVecView(CAMERA* pcamera);
void SetCameraVecUp(CAMERA* pcamera, glm::vec3 vecUp);
void*GetCameraVecUp(CAMERA* pcamera);
int  GetCameraSize();
void CloneCamera(CAMERA* pcamera, CAMERA* pcameraBase);
void PostCameraLoad(CAMERA* pcamera);
void EnableCamera(CAMERA* pcamera);
void DisableCamera(CAMERA* pcamera);
void DeleteCamera(CAMERA *pcamera);