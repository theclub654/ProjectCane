#pragma once
#include "lo.h"

extern std::vector<LO*> allWorldObjs;

// Initializes Options
void StartupBrx();
// Returns a new static world object
LO* PloNew(CID cid, SW *psw, ALO *paloParent, OID oid, int isplice);
// Loads all the static world objects from the binary file
void LoadSwObjectsFromBrx(SW *psw, ALO *paloParent, CBinaryInputStream *pbis);
// Returns first parent list
DL* PdlFromSwOid(SW* psw, OID oid);
// THIS FUNCTION IS TEMPORARY UNTIL I GET AROUND TO RE IM ONLY LOOPING THROUGH THE BINARY DATA
void LoadOptionFromBrx(void* pvObject, CBinaryInputStream *pbis);
// GOTTA CHANGE AND ADD A PARAMETER ON THIS LATER THIS FUNCTION NEEDS TO BE LOOKED AT LATER ON JUST LOOPING THROUGH THE BINARY DATA
void LoadOptionsFromBrx(void* pvObject, CBinaryInputStream *pbis);
static char** g_mpoidachz;