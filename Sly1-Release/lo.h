#pragma once
#include "basic.h"
#include "dl.h"
#include "bis.h"
#include "util.h"
#include "clock.h"
#include <bitset>
class CFrame;

enum MSGID
{
    MSGID_Nil = -1,
    MSGID_callback = 0,
    MSGID_added = 1,
    MSGID_removed = 2,
    MSGID_translated = 3,
    MSGID_rotated = 4,
    MSGID_asega_limit = 5,
    MSGID_asega_retracted = 6,
    MSGID_asega_wrapped = 7,
    MSGID_sfx_start = 8,
    MSGID_sfx_end = 9,
    MSGID_water_entered = 10,
    MSGID_water_left = 11,
    MSGID_button_trigger = 12,
    MSGID_button_untrigger = 13,
    MSGID_break_break = 14,
    MSGID_break_piece = 15,
    MSGID_rip_removed = 16,
    MSGID_color_change = 17,
    MSGID_user_message = 18,
    MSGID_label = 19,
    MSGID_sma_transition = 20,
    MSGID_waypoint = 21,
    MSGID_waypoint_transition = 22,
    MSGID_damaged = 23,
    MSGID_param_read = 24,
    MSGID_param_write = 25,
    MSGID_Max = 26
};

typedef void* (*PFNMQ)(LO*, MSGID, void*);

enum SYMEVID 
{
    SYMEVID_Nil = -1,
    SYMEVID_OnPostLoad = 0,
    SYMEVID_OnUpdate = 1,
    SYMEVID_OnTrigger = 2,
    SYMEVID_OnUntrigger = 3,
    SYMEVID_OnAdd = 4,
    SYMEVID_OnRemove = 5,
    SYMEVID_OnTouch = 6,
    SYMEVID_OnUntouch = 7,
    SYMEVID_OnDeath = 8,
    SYMEVID_OnApply = 9,
    SYMEVID_OnFrame = 10,
    SYMEVID_OnRetract = 11,
    SYMEVID_OnContentsChange = 12,
    SYMEVID_OnFire = 13,
    SYMEVID_OnEnterState = 14,
    SYMEVID_OnEnterStepGuardState = 15,
    SYMEVID_OnDialogLabel = 16,
    SYMEVID_OnApplySm = 17,
    SYMEVID_OnRetractSm = 18,
    SYMEVID_OnBasketDamage = 19,
    SYMEVID_OnBreakDamage = 20,
    SYMEVID_OnRetractDialog = 21,
    SYMEVID_OnRestore = 22,
    SYMEVID_OnScriptApply = 23,
    SYMEVID_OnScriptLabel = 24,
    SYMEVID_OnScriptRetract = 25,
    SYMEVID_Max = 26
};

struct SOP
{
    LO* plo;
    int cpar;
    struct PAR* apar;
    struct SOP* psopNext;
};

struct DLR 
{
    OID oidChild;
    std::shared_ptr <DLR> pdlrNext;
};

// Proxy Source List
struct PSL 
{
    // Number of cloned LO's
    int cploCloneFree;
    // Container of the cloned LO's
    std::vector <LO*> aploClone;
};

struct XA
{
    class SO* psoSource;
    class SO* psoTarget;
    XA* pxaNextSource;
    std::shared_ptr<XA> pxaNextTarget;
};

// Proxy Root
// Stores all info about the proxy source object.
struct PXR 
{
    // Ptr to PRX object
    struct LO* plo;
    // OID for proxy root
    OID oidProxyRoot;
    // List that keeps track of PXR objects
    DLE dleProxyRoot;
    char* pchzProxyRoot;
};

struct MQ 
{
    PFNMQ pfnmq;
    void* pvContext;
    MSGID msgid;
    std::shared_ptr <CPair> pvCallbackData;
    std::shared_ptr <MQ> pmqNext;
};

// Merge
// Used to merge ALO object's
struct MRG
{
    DLE dle;
    // The ALO object thats gonna be merged
    std::vector <ALO*> apalo;
};

// Local Object
class LO : public BASIC
{
	public:
	// Object ID
	short oid;
	DLE dleOid;
	// Parent SW Object
	class SW* psw;
	// Objects Parent
	class ALO *paloParent;
	DLE dleChild;
    // Pointer to next LO
	class LO* ploCidNext;
    std::shared_ptr<MQ> pmqFirst;
	// Objects name
	char *pchzName;
    std::shared_ptr <CFrame> pframe;
    std::shared_ptr <PXR> ppxr;
};

// Creates a new local object
LO* NewLo();
// Initializes Local Object
void InitLo(LO* parentLo);
void SetLoDefaults(LO* plo);
// Adds LO to parent LO if LO doesnt have a parent it makes LO a parent
void AddLo(LO* plo); 
void AddLoHierarchy(LO* plo);
void RemoveLoHierarchy(LO* plo);
void SnipLo(LO* plo);
int  FFindLoParent(LO* plo, ALO* paloParent);
void CloneLoHierarchy(LO* plo, LO* ploBase);
void CloneLo(LO* plo, LO* ploBase);
LO*  PloCloneLo(LO* plo, SW* psw, ALO* paloParent);
void SendLoMessage(LO* plo, MSGID msgid, void* pv); 
void LoadLoFromBrx(LO* plo, CBinaryInputStream* pbis);
int  FMatchesLoName(LO* plo, OID oid);
void RemoveLo(LO* plo);
void DeferLoRemove(LO* plo);
void SetLoSuckHideLimits(LO* plo, LM* plmUSuck); // GOTTA COME BACK TO THIS TO UNCOMMENT
void OnLoAdd(LO* plo);
void OnLoRemove(LO* plo);
void*AccessLoSw(LO *plo);
void*GetLoSw(LO* plo);
// Returns whether LO is in world or not
int  FIsLoInWorld(LO* plo);
void GetLoInWorld(LO* plo, int* pfInWorld);
void*AccessLoPaloParent(LO* plo);
void*GetLoPaloParent(LO* plo);
void*AccessLoOid(LO* plo);
OID  GetLoOid(LO* plo);
OID  OidProxyLo(LO* plo);
OID  OidProxyLoPreferred(LO* plo);
void GetLoOidProxy(LO* plo, OID* poid);
void HandleLoSpliceEvent(LO* plo, SYMID symidEvent, int cargs, void** ppvargs);
int  FFilterSpliceEvent(ALO* plo, SYMEVID symevid, int cargs, void** ppvargs);
void PostSpliceEventCallback(LO* plo, SYMID symidEvent, void* pvarg);
void EnsureLoSidebagBool(LO* plo, OPTID optid, int f);
void EnsureLoSidebagInt(LO* plo, OPTID optid, int n);
void EnsureLoSidebagFloat(LO* plo, OPTID optid, float g);
void EnsureLoSidebagClq(LO* plo, OPTID optid, CLQ* pclq);
void EnsureLoSidebagLm(LO* plo, OPTID optid, LM* plm);
void EnsureLoSidebagOid(LO* plo, OPTID optid, OID oid);
void EnsureLoSidebagVector(LO* plo, OPTID optid, glm::vec3* pvec);
void PostLoLoad(LO* plo);
void SetLoParent(LO* plo, ALO* paloParent);
void SubscribeLoObject(LO* plo, LO* ploTarget);
void SubscribeSwPpmqStruct(SW* psw, std::shared_ptr<MQ>* ppmqFirst, PFNMQ pfnmq, void* pvContext);
void UnsubscribeLoObject(LO* plo, LO* ploTarget);
void UnsubscribeSwPpmqStruct(SW* psw, std::shared_ptr<MQ>* ppmqFirst, PFNMQ pfnmq, void* pvContext);
void SubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext);
void UnsubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext);
int  GetLoSize();
void DeleteLo(LO* plo);

extern std::vector <LO*> allWorldObjs;
extern OTYP s_aotypEvtParm[44];
extern int  s_aiEvtParm[27];
#if !defined(ALO_BUILDING) && !defined(CPLCY_BUILDING)
#include "alo.h"
#include "sw.h"
#endif
