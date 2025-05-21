#pragma once
#include "lo.h"
#include "difficulty.h"
#include "cm.h"

struct LSM
{
	// Ambient strength
	float uShadow;
	// Diffuse strength
	float uMidtone;
};
struct ISI
{
	SFXID sfxid;
	LM lmRepeat;
	LM lmRepDist;
};
struct RSE
{
	REVERBK rvrbk;
	int depth;
};

// Scene World
class SW : public LO
{
public:
	// Number of SO in world
	int cpsoAll;
	int cpsoRoot;
	DL dlRoot;
	// DL to dleChild in LO
	DL dlChild;
	DL dlMRD;
	DL dlBusy;
	DL dlBusySo;
	DL dlMRDRealClock;
	// Used to store the first parent of a local object.
	DL adlHash[512];
	LO* aploCidHead[162];
	DL dlAsega;
	DL dlAsegaRealClock;
	DL dlAsegaPending;
	DL dlSma;
	DL dlAmb;
	DL dlExc;
	DL dlExplste;
	DL dlLight;
	DL dlShadow;
	DL dlFader;
	DL dlRealClockFader;
	DL dlShape;
	DL dlPathzone;
	DL dlProxy;
	DL dlFly;
	DL dlDprize;
	DL dlRat;
	DL dlRathole;
	DL dlDartFree;
	DL dlSpire;
	DL dlRail;
	DL dlLanding;
	DL dlBusyLasen;
	DL dlBlipg;
	DL dlBlipgFree;
	DL dlCrfod;
	struct VAULT* pvault;
	struct LO* aploStock[29];
	RGBA rgbaSky;
	LSM lsmDefault;
	// Sound bank index for level
	int ibnk;
	MUSID musid;
	int cisi;
	ISI aisi[16];
	VISMAP* pvismap;
	struct MQ* pmqCallbackFirst;
	struct MQ* pmqCallbackLast;
	int cpaloRemerge;
	struct ALO** apaloRemerge;
	// Object's gravity
	glm::vec3 dvGravity;
	int csplcSplice;
	struct SPLC* asplcSplice;
	SYMID symidScheduledCallbackList;
	int symidMax;
	char** mpsymidachz;
	int coptidExtra;
	char** mpoptidachzExtra;
	// Number of proxy source list
	int cpsl;
	// Proxy source list
	PSL apsl[128];
	// Number of clue bottles for each level
	int cclueAll;
	// Number of check points for level
	int cchkpntAll;
	float dtChallengePar;
	// How much coins a level has
	int cgoldAll;
	float gexcMenu;
	int cHandsOff;
	RSE arse[4];
	int irse;
	struct MBG* pmbg;
	float rDarken;
	float rDarkenSmooth;
};

// Create SW object
SW* NewSw();
// Initializing SW object
void InitSw(SW* psw); // GOTTA COME BACK TO THIS
void InitSwBusySoDl(SW* psw);
void InitSwRootDl(SW* psw);
void InitSwAsegaDl(SW* psw);
void InitSwAsegaRealClockDl(SW* psw);
void InitSwAsegaPending(SW* psw);
void InitSwSmaDl(SW* psw);
void InitSwLightDl(SW* psw);
void InitSwShadowDl(SW* psw);
void InitSwProxyDl(SW* psw);
void InitSwFlyDl(SW* psw);
void InitSwDprizeDl(SW* psw);
void InitSwRatDl(SW* psw);
void InitSwRatholeDl(SW* psw);
void InitSwDartFreeDl(SW* psw);
void InitSwSpireDl(SW* psw);
void InitSwRailDl(SW* psw);
void InitSwLandingDl(SW* psw);
void InitSwLasenDl(SW* psw);
void InitSwBlipgDl(SW* psw);
void InitSwBlipgFreeDl(SW* psw);
void InitSwFaderDl(SW* psw);
void InitSwRealClockFader(SW* psw);
void InitSwCrfodDl(SW* psw);
void InitSwShapeDl(SW* psw);
void InitSwPathzoneDl(SW* psw);
// Returns size of SW
int  GetSwSize();
// Initializing the base offset to data
void InitSwDlHash(SW* psw);
// Loads all world data from binary file
void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis);// NOT FINISHED
// Loads level filenames from file
void LoadNameTableFromBrx(CBinaryInputStream* pbis);
// Loads the world table from file
void LoadWorldTableFromBrx(CBinaryInputStream* pbis);
// Adds a SW proxy source to apsl 
void AddSwProxySource(SW* psw, LO* ploProxySource, int cploClone);
// Returns SW proxy source LO based of proxy source index
LO* PloGetSwProxySource(SW* psw, int ipsl);
void GetSwParams(SW* psw, SOP** ppsop);
void*GetSwIllum(SW* psw);
void*GetSwIllumShadow(SW* psw);
void SetSwIllum(SW* psw, float uMidtone);
void SetSwIllumShadow(SW* psw, float uShadow);
void*GetSwDarken(SW* psw);
void SetSwDarken(SW* psw, float rDarken);
void*GetSwDarkenSmooth(SW* psw);
void SetSwDarkenSmooth(SW* psw, float rDarkenSmooth);
void MatchSwObject(ALO* ploMatch, GRFFSO grffsoMask, int fIncludeRemoved, int fProxyMatch, LO* ploContext, int cploMax, int* pcploMatch, LO** aplo, int* pcpaloBest);
int  CploFindSwObjects(SW* psw, GRFFSO grffso, OID oid, LO* ploContext, int cploMax, LO** aplo);
// Finds a LO
LO* PloFindSwObject(SW* psw, GRFFSO grffso, OID oid, LO* ploContext);
LO* PloFindSwNearest(SW* psw, OID oid, LO* ploContext);
// Update all objects in SW
void UpdateSw(SW* psw, float dt); // Gotta come back to this
void UpdateSwObjects(SW* psw, float dt);
void DeleteSwCollision();
// Delete SW object
void DeleteSw(SW* psw);
// Deletes all world data from memory
void DeleteWorld(SW* psw);

// Global pointer to parent scene world object
inline SW* g_psw = nullptr;
