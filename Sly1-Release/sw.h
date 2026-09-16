#pragma once
#include <deque>
#include <memory>
#include <unordered_map>
#include "find.h"
#include "difficulty.h"
#include "cm.h"
#include "ui.h"
#include "sound.h"
#include "expl.h"
#include "blip.h"

constexpr int AXIS_COUNT = 3;
constexpr int MAX_ROOT_OBJECTS = 256;
constexpr int BOUNDS_PER_OBJECT = 2;
constexpr int BOUNDS_PER_AXIS = MAX_ROOT_OBJECTS * BOUNDS_PER_OBJECT;
constexpr int MAX_OXA = 256;
constexpr int AAOXF_COUNT = MAX_ROOT_OBJECTS * MAX_ROOT_OBJECTS;
// Number of entries in the world's shared RIP slot heap.  All RIP subtypes
// compete for these slots, matching SW::slotheapRip in the original engine.
constexpr size_t RIP_SLOT_COUNT = 384;
class RipPool;

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

enum OBRF
{
	OBRF_Min = 0,
	OBRF_Max = 1
};

struct OBR 
{
	OBRF obrf;
	class SO* pso;
	float *ps;
};

struct OX 
{
	struct XP* pxp;
	struct OX* poxNext;
	struct OX* poxOther;
	class  SO* psoOther;
};

struct SPLC
{
	std::shared_ptr <CPair>  ppairCompile;
	std::shared_ptr <CFrame> pframe;
	std::string achzObjectPath;
	std::string achzFile;
	std::string achzSpliceSource;
};

struct OXA 
{
	struct OX* pox;
	class  SO* pso;
	struct OXA* poxaNext;
	struct OXA* poxaPrev;
};

struct STSO
{
	class SO* psoTouch;
	std::shared_ptr <STSO> pstsoNext;
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
	std::vector <OBR> aaobr;
	std::vector <OXF> aaoxf;
	std::vector <OXA> aoxa;
	OXA* poxaFree;
	OXA* poxaAlloc;
	int fAaoxValid;
	// OX records are created and destroyed in large bursts when breakable
	// objects release several overlapping physics pieces.  Keep their storage
	// stable and recycle records instead of doing a heap allocation per pair.
	std::deque<OX> aox;
	std::vector<OX*> apoxFree;
	std::vector <std::unique_ptr <XP>> axp;
	RipPool* pripPool = nullptr;
	std::unordered_map<RIP*, std::shared_ptr<MQ>> ripMqOwners;
	class RIPG* pripgFree;
	class RIPG* pripgDefault;
	std::vector <std::unique_ptr <BLIP>> blipOwners;
	std::vector <std::unique_ptr <BLIPSP>> blipspOwners;
	std::vector <std::shared_ptr <ASEGA>> asegaOwners;
	std::vector <std::shared_ptr <ACT>> actOwners;
	DL dlAsega;
	DL dlAsegaRealClock;
	DL dlAsegaPending;
	std::vector <std::shared_ptr <SMA>> smaOwners;
	std::vector <std::shared_ptr <EXPLSTE>> explsteOwners;
	std::vector <std::shared_ptr <STSO>> aptsoOwner;
	std::vector<std::unique_ptr<AMB>> ambOwners;
	std::vector<std::shared_ptr<EXC>> excOwners;
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
	DL dlThndFlash;
	class VAULT* pvault;
	LO* aploStock[31];
	RGBA rgbaSky;
	LSM lsmDefault;
	// Sound bank index for level
	int ibnk;
	MUSID musid;
	int cisi;
	ISI aisi[16];
	VISMAP* pvismap;
	std::shared_ptr <MQ> pmqCallbackFirst;
	std::shared_ptr <MQ> pmqCallbackLast;
	int cpaloRemerge;
	std::vector <ALO*> apaloRemerge;
	// Object's gravity
	glm::vec3 dvGravity;
	int csplcSplice;
	std::vector <SPLC> asplcSplice;
	SYMID symidScheduledCallbackList;
	int symidMax;
	std::vector <std::string> mpsymidachz;
	int coptidExtra;
	std::vector <std::string> mpoptidachzExtra;
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
	GRFVAULT grfvault;
	float tGoldLose;
	int fDprizeEnabled;
	int cLetterbox;
	class ALO* paloHud;
	const char* audioLocation;
	float dtCommentaryAudio;
	const char *pchzSpeakers;
	int fMusicEnabled;
	float uMusicVolume;
	float tGoldEnable;
	int fRubyMusicActive;

	// PC-side ownership for the retail pointer-backed commentary fields above.
	// Kept at the end so reconstructed gameplay-field ordering is unchanged.
	std::array<uint8_t, 32> commentaryAudioDescriptor{};
	std::string commentarySpeakers;
};

//GOTTA COME BACK TO THIS 
// Create SW object
SW* NewSw();
// Returns size of SW
int  GetSwSize();
// Initializing the base offset to data
void InitSwDlHash(SW* psw);
// Initializing SW object
void InitSw(SW* psw); // GOTTA COME BACK TO THIS
void SetSwGravity(SW* psw, float z);

const char* GetResponseText(SW* psw, RESPK respk);

bool FOverflowSwLo(SW* psw, LO* plo, bool fHiPri);
std::shared_ptr <XA> PxaAllocSw(SW* psw);
void FreeSwXaList(SW*, std::shared_ptr<XA>& pxaFirst);
void FreeSwMqList(SW* psw, std::shared_ptr<MQ>& pmqFirst);
void EnsureSwCallback(SW* psw, PFNMQ pfnmq, void* pvContext, MSGID msgid, std::shared_ptr<CPair> pvCallbackData);
void PostSwCallback(SW* psw, PFNMQ pfnmq, void* pvContext, MSGID msgid, std::shared_ptr<CPair> pvCallbackData);
void ClearSwCallbacks(SW* psw, int grfcmq, PFNMQ pfnmq, void* pvContext, MSGID msgid, std::shared_ptr<CPair> pvCallback);
void ProcessSwCallbacks(SW* psw);
void ProcessSwSpliceScheduledCallbacks(SW* psw, float dt);
void AddSwProxySource(SW* psw, LO* ploProxySource, int cploClone);
// Returns SW proxy source LO based of proxy source index
LO*  PloGetSwProxySource(SW* psw, int ipsl);
void IntersectSwBoundingBox(SW* psw, SO* psoIntersect, const glm::vec3* ppos1, const glm::vec3* ppos2, PFNFILTER pfnFilter, void* pvContext, std::vector<SO*>& apso);
void IntersectSwBoundingSphere(SW* psw, SO* psoIntersect, const glm::vec3* pposCenter, float sRadius, PFNFILTER pfnFilter, void* pvContext, std::vector<SO*>& apso);
void RemoveOxa(OXA* poxa, OXA** ppoxaFirst);
void InitSwAoxa(SW* psw);
void AddOxa(OXA* poxa, OXA** ppoxaFirst);
OXA* PoxaAllocSw(SW* psw, SO* pso);
void FreeSwPoxa(SW* psw, OXA* poxa);
void CreateSwDefaultLights(SW* psw);
void CreateSwPrizes(SW* psw);
// Loads all world data from binary file
void LoadSwFromBrx(SW* psw, CBinaryInputStream* pbis);
// GOTTA COME BACK TO THIS
int  FClipLineHomogeneous(glm::vec4* apos);
// GOTTA COME BACK TO THIS
void DrawLineWorld(glm::vec3* ppos1, glm::vec3* ppos2, RGBA* prgba, CM* pcm, int fDepthTest);
void DrawBezierWorld(float dt, glm::vec3* ppos0, glm::vec3* pv0, glm::vec3* ppos1, glm::vec3* pv1, RGBA* prgba, CM* pcm);
// Loads level filenames from file
void LoadNameTableFromBrx(CBinaryInputStream* pbis);
// Loads the world table from file
void LoadWorldTableFromBrx(CBinaryInputStream* pbis);
// Adds a SW proxy source to apsl 
void GetSwParams(SW* psw, SOP** ppsop);
void*GetSwMusicEnabled(SW* psw);
void SetSwMusicEnabled(SW* psw, int fMusicEnabled);
void*GetSwMusicVolume(SW* psw);
void SetSwMusicVolume(SW* psw, float uMusicVolume);
void*GetSwGoldEnable(SW* psw);
void SetSwGoldEnable(SW* psw, float tGoldEnable);
void*GetSwCommentaryAudioDelay(SW* psw);
void SetSwCommentaryAudioDelay(SW* psw, float dtCommentaryAudio);
void*GetSwExcitement(SW* psw);
int* PdifficultyEnsureSwCcoinRichMin(SW* psw, ENSK ensk);
int* PdifficultyEnsureSwCcoinRichMax(SW* psw, ENSK ensk);
int* PdifficultyEnsureSwCcoinPoorMin(SW* psw, ENSK ensk);
int* PdifficultyEnsureSwCcoinPoorMax(SW* psw, ENSK ensk);

void*GetSwDprizeEnabled(SW* psw);
void SetSwDprizeEnabled(SW* psw, int fDprizeEnabled);

void*GetSwIllum(SW* psw);
void*GetSwIllumShadow(SW* psw);
void SetSwIllum(SW* psw, float uMidtone);
void SetSwIllumShadow(SW* psw, float uShadow);
void*GetSwSkyRgba(SW *psw);
void*GetSwDarken(SW* psw);
void SetSwDarken(SW* psw, float rDarken);
void*GetSwDarkenSmooth(SW* psw);
void SetSwDarkenSmooth(SW* psw, float rDarkenSmooth);
void SetSwExcitement(SW* psw, float gexcMenu);
void SetSwMusicRegister(SW* psw, int ireg, byte bVal);
void RefreshSwMidiRegister(SW* psw, int ireg);
int  FLevelSwVisited(SW* psw, WID wid);
int  FLevelSwPrimary(SW* psw, WID wid);
int  FLevelSwSecondary(SW* psw, WID wid);
int  FLevelSwTertiary(SW* psw, WID wid);
bool FLevelKeyCollected(SW* psw, int levelID);
bool FSecondaryTaskAvailable(SW* psw, int levelID);
bool FCommentaryUnlockedForLevel(SW* psw, int levelId);
int GetSwLevelDataValue(SW* psw, uint32_t worldLevelKey, int key);
void SetSwLevelDataValue(SW* psw, uint32_t worldLevelKey, int key, int value);

bool FWorldVisited(void* context, int iWorld);
bool FWorldComplete(void* context, int iWorld);
int  RecalculatePercentCompletion();
void SpawnRandomCoins(SW* psw, int ccoinMin, int ccoinMax, glm::vec3* ppos);
void SetDifficultySuck(float uSuck);
void GetSwPlayerSuck(SW* psw, float* puSuck);
void SetSuckRaw(float uSuckRaw);
void GetuSuckRaw(SW* psw, float* puSuckRaw);
void IncrementSwHandsOff(SW* psw);
void DecrementSwHandsOff(SW* psw);
bool FSwHandsOff(SW* psw);
void IsSwVagPlaying(SW* psw, int* pfPlaying);
void CancelSwDialogPlaying(SW* psw);
void SetSmaPrompt(PRK prk, SMA* psma);
void AcquireSwLetterbox(SW* psw);
void ReleaseSwLetterbox(SW* psw);
EXC* SetExcitement(SW* psw, IEXC iexc);
void UnsetExcitementSw(EXC* pexc);
void SetSwGameplayHud(SW* psw, ALO* palo);
void UpdateSwHud(SW* psw);
void UnlockSwVault(SW* psw, uint32_t iVault);
void SetSwVaultLocked(SW* psw, uint32_t iVault);
void GetCharmCount(void* context, int* pcCharm);
void GetLifeCount(void* context, int* pcLife);
void DeleteSwCollision();
void FreeSwCallbackQueue(SW* psw);
void ResetSwRipPool(SW* psw);
void DeleteSwRipPool(SW* psw);
// Delete SW object
void DeleteSw(SW* psw);
// Deletes all world data from memory
void DeleteWorld(SW* psw);

// Global pointer to parent scene world object
extern SW* g_psw;
extern glm::vec4 rgbaSky;
