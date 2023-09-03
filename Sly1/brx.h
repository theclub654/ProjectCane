#pragma once
#include "sw.h"
#include "alo.h"
#include "cm.h"
#include "bis.h"
#include "find.h"

// Temporary flag
static bool loadEmitMesh = 0;

// Initializes Options
void StartupBrx();
// Returns a new static world object
void* PloNew(CID cid, SW *psw, ALO *paloParent, OID oid, int isplice);
// Loads all the static world objects from the binary file
void LoadSwObjectsFromBrx(SW *psw, ALO *paloParent, CBinaryInputStream *pbis);
// THIS FUNCTION IS TEMPORARY UNTIL I GET AROUND TO RE IT RN IM ONLY LOOPING THROUGH THE BINARY CODE
void LoadOptionFromBrx(void* pvObject, CBinaryInputStream *pbis);
// GOTTA CHANGE AND ADD A PARAMETER ON THIS LATER THIS FUNCTION NEEDS TO BE LOOKED AT LATER ON JUST LOOPING THROUGH THE BINARY DATA
void LoadOptionsFromBrx(void* pvObject, CBinaryInputStream *pbis);