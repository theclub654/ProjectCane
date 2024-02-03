#pragma once
#include "lo.h"

extern std::vector<LO*> allWorldObjs;

static int LoadOptionsFromBrxCount = 0;

// Initializes Options
void StartupBrx();
// Returns a new static world object
LO* PloNew(CID cid, SW *psw, ALO *paloParent, OID oid, int isplice);
// Loads all the static world objects from the binary file
void LoadSwObjectsFromBrx(SW *psw, ALO *paloParent, CBinaryInputStream *pbis);
// Returns first parent list
DL* PdlFromSwOid(SW* psw, OID oid);
// Loads all the options data for a LO
void LoadOptionsFromBrx(void* pvObject, CBinaryInputStream *pbis);
void LoadOptionFromBrx(void* pvObject, EOPID peopid, CBinaryInputStream* pbis);

static char** g_mpoidachz;