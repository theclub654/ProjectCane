#pragma once
#include "spaprops.h"
#include "dl.h"
#include "bis.h"
#include "shd.h"
#include "splicemap.h"
#include "util.h"

class SW;
class ALO;

void* PloNew(CID cid, SW* psw, ALO* paloParent, OID oid, int isplice);
void LoadSwObjectsFromBrx(SW* psw, ALO* paloParent, CBinaryInputStream* pbis);
void LoadOptionFromBrx(void* pvObject, CBinaryInputStream* pbis);

class LO
{
	public:
		// Class ID
		CID cid;
		// Object ID
		OID oid;
		DLE dleOid;
		// Parent SW Object
		class SW *psw;
		// Objects Parent
		class ALO *paloParent;
		DLE dleChild;
		LO *ploCidNext;
		// Objects name
		char *pchzName;
		uint64_t dtickPerf;
};

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

// Initializes Local Object
void InitLo(LO *parentLo);
void SetLoDefaults(LO* parentLo);
// Adds LO to parent LO if LO doesnt have a parent it makes LO a parent
void AddLo(LO* plo); // GOTTA COME BACK
void AddLoHierarchy(LO* plo);
void RemoveLoHierarchy(LO* plo);
void SendLoMessage(LO *plo, MSGID msgid, void *pv); // GOTTA COME BACK
void LoadLoFromBrx(LO* plo, CBinaryInputStream* pbis);
// Remove LO 
void RemoveLo(LO* plo); // GOTTA COME BACK
// Returns whether LO is in world or not
int FIsLoInWorld(LO* plo);

#include "sw.h"
#include "alo.h"