#pragma once
#include "basic.h"
#include "dl.h"
#include "bis.h"
#include "glshaders.h"
#include "splicemap.h"
#include "util.h"
#include "clock.h"
#include <bitset>

typedef unsigned int SYMID;

LO* PloNew(CID cid, SW* psw, ALO* paloParent, OID oid, int isplice);
DL* PdlFromSwOid(SW* psw, OID oid);
void LoadSwObjectsFromBrx(SW* psw, ALO* paloParent, CBinaryInputStream* pbis);
void LoadOptionsFromBrx(void* pvObject, CBinaryInputStream* pbis);

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
struct SOP
{
    LO* plo;
    int cpar;
    struct PAR *apar;
    struct SOP *psopNext;
};

// Proxy Source List
struct PSL 
{
    // Number of cloned LO's
    int cploCloneFree;
    // Container of the cloned LO's
    std::vector <LO*> aploClone;
};

// Proxy Root
// Stores all info about the proxy source object.
struct PXR 
{
    // Ptr to PRX object
    struct LO *plo;
    // OID for proxy root
    OID oidProxyRoot;
    // List that keeps track of PXR objects
    DLE dleProxyRoot;
    char* pchzProxyRoot;
};

// Merge
// Used to merge ALO object's
struct MRG
{
    DLE dle;
    // Number of ALO objects to be merged
    int cpalo;
    // The ALO object thats gonna be merged
    struct ALO **apalo;
};

// Local Object
class LO : public BASIC
{
	public:
		// Object ID
		OID oid;
		DLE dleOid;
		// Parent SW Object
		class SW *psw;
		// Objects Parent
		class ALO *paloParent;
		DLE dleChild;
        // Pointer to next LO
		class LO *ploCidNext;
        struct MQ* pmqFirst;
		// Objects name
		char *pchzName;
        struct CFrame* pframe;
        std::shared_ptr <PXR> ppxr;
		uint64_t dtickPerf;
};

// Creates a new local object
LO* NewLo();
// Initializes Local Object
void InitLo(LO *parentLo);
void SetLoDefaults(LO* parentLo);
// Adds LO to parent LO if LO doesnt have a parent it makes LO a parent
void AddLo(LO* plo); // GOTTA COME BACK
void AddLoHierarchy(LO* plo);
void RemoveLoHierarchy(LO* plo);
void SnipLo(LO* plo);
void CloneLoHierarchy(LO* plo, LO* ploBase);
void CloneLo(LO* plo, LO* ploBase);
LO*  PloCloneLo(LO* plo, SW* psw, ALO* paloParent);
void SendLoMessage(LO *plo, MSGID msgid, void *pv); // GOTTA COME BACK
void LoadLoFromBrx(LO *plo, CBinaryInputStream* pbis);
void RemoveLo(LO *plo); // GOTTA COME BACK
void OnLoAdd(LO *plo);
void OnLoRemove(LO *plo);
// Returns whether LO is in world or not
int  FIsLoInWorld(LO *plo);
void GetLoInWorld(LO* plo, int* pfInWorld);
OID  GetLoOid(LO *plo);
OID  OidProxyLo(LO *plo);
void GetLoOidProxy(LO *plo, OID *poid);
void PostLoLoad(LO *plo); // GOTTA COMEB BACK
void SetLoParent(LO *plo, ALO *paloParent);
void SubscribeLoObject(LO* plo, LO* ploTarget);
void UnsubscribeLoObject(LO* plo, LO* ploTarget);
void SubscribeLoStruct(LO* plo, void* pfnmq, void* pvContext);
void UnsubscribeLoStruct(LO* plo, void *pfnmq, void* pvContext);
int  GetLoSize();
void DeleteLo(LO* plo);

#include "sw.h"
#include "alo.h"