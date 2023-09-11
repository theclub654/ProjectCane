#pragma once
#include "vis.h"
#include "so.h"
#include "sw.h"
#include "keyhole.h"
#include "emitter.h"
#include "proxy.h"
#include "coin.h"
#include "pnt.h"
#include "jt.h"
#include "stepguard.h"
#include "mb.h"
#include "rog.h"
#include "puffer.h"
#include "mgv.h"
#include "break.h"
#include "button.h"
#include "squish.h"
#include "pzo.h"
#include "sensor.h"
#include "cnvo.h"
#include "hide.h"
#include "missile.h"
#include "fly.h"
#include "rat.h"
#include "dart.h"
#include "jlo.h"
#include "chkpnt.h"
#include "flash.h"
#include "tn.h"
#include "dialog.h"
#include "mouthgame.h"
#include "jack.h"
#include "shape.h"
#include "hnd.h"
#include "aseg.h"
#include "sm.h"
#include "path.h"
#include "rchm.h"

// Initializes Options
void StartupBrx();
// Returns a new static world object
void* PloNew(CID cid, SW *psw, ALO *paloParent, OID oid, int isplice);
// Loads all the static world objects from the binary file
void LoadSwObjectsFromBrx(SW *psw, ALO *paloParent, CBinaryInputStream *pbis);
// Returns first parent list
DL* PdlFromSwOid(SW* psw, OID oid);
// THIS FUNCTION IS TEMPORARY UNTIL I GET AROUND TO RE IT RE IM ONLY LOOPING THROUGH THE BINARY DATA
void LoadOptionFromBrx(void* pvObject, CBinaryInputStream *pbis);
// GOTTA CHANGE AND ADD A PARAMETER ON THIS LATER THIS FUNCTION NEEDS TO BE LOOKED AT LATER ON JUST LOOPING THROUGH THE BINARY DATA
void LoadOptionsFromBrx(void* pvObject, CBinaryInputStream *pbis);

static char** g_mpoidachz;