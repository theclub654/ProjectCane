#include "unk13.h"

void* NewUnk13()
{
	return new UNK13;
}

void InitUnk13(UNK13* punk13)
{
	InitLo(punk13);
}

void CloneUnk13(UNK13* punk13, UNK13* punk13Base)
{
	RemoveDlEntry(PdlFromSwOid(punk13->psw, (OID)punk13->oid), punk13);
	AppendDlEntry(PdlFromSwOid(punk13Base->psw, (OID)punk13Base->oid), punk13);

	punk13->oid = punk13Base->oid;
	punk13->pchzName = punk13Base->pchzName;
	punk13->ppxr = punk13Base->ppxr;
}

void DeleteUnk13(UNK13* punk13)
{
	delete punk13;
}
