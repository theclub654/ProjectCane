#include "unk14.h"

void* NewUnk14()
{
	return new UNK14{};
}

void CloneUnk14(UNK14* punk14, UNK14* punk14Base)
{
	RemoveDlEntry(PdlFromSwOid(punk14->psw, (OID)punk14->oid), punk14);
	AppendDlEntry(PdlFromSwOid(punk14Base->psw, (OID)punk14Base->oid), punk14);

	punk14->oid = punk14Base->oid;
	punk14->pchzName = punk14Base->pchzName;
	punk14->ppxr = punk14Base->ppxr;
}

void DeleteUnk14(UNK14* punk14)
{
	delete punk14;
}
