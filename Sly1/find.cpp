#include "find.h"

DL* PdlFromSwOid(SW *psw, OID oid)
{
	return &psw->objectParents[oid];
}