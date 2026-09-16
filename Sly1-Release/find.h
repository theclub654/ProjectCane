#pragma once
#include "lo.h"

// Returns first parent list
DL*  PdlFromSwOid(SW* psw, OID oid);
void MatchSwObject(ALO* ploMatch, GRFFSO grffsoMask, int fIncludeRemoved, int fProxyMatch, LO* ploContext, int cploMax, int* pcploMatch, LO** aplo, int* pcpaloBest);
int  CploFindSwObjects(SW* psw, GRFFSO grffso, OID oid, LO* ploContext, int cploMax, LO** aplo);
LO*  PloFindSwObject(SW* psw, GRFFSO grffso, OID oid, LO* ploContext);
LO*  PloFindSwNearest(SW* psw, OID oid, LO* ploContext);
LO*  PloFindSwChild(SW* psw, OID oid, ALO* paloAncestor);
int  FIsCidDerivedFrom(CID cid, CID cidAncestor);
int  CploFindSwObjectsByClass(SW* psw, GRFFSO grffso, CID cid, ALO* ploContext, int cploMax, LO** aplo);
LO*  PloFindSwObjectByClass(SW* psw, GRFFSO grffso, CID cid, LO* ploContext);
ALO* PaloFindLoCommonParent(LO* plo, LO* ploOther);
