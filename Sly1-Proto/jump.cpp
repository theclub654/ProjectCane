#include "jump.h"

JMT* NewJmt()
{
	return new JMT{};
}

int GetJmtSize()
{
	return sizeof(JMT);
}

void OnJmtAdd(JMT* pjmt)
{
	OnLoAdd(pjmt);
}

void OnJmtRemove(JMT* pjmt)
{
	OnLoRemove(pjmt);
	//RemoveDlEntry(&g_dlJmt, pjmt);
}

void CloneJmt(JMT* pjmt, JMT* pjmtBase)
{
	ClonePnt(pjmt, pjmtBase);

	// Clone the DLE structure (assuming it's a simple structure, shallow copy should suffice)
	pjmt->dleJmt = pjmtBase->dleJmt;

	// Clone the float sRadius
	pjmt->sRadius = pjmtBase->sRadius;

	// Clone the OID and pointers (assuming these need shallow copying)
	pjmt->oidSetIgnoreJt = pjmtBase->oidSetIgnoreJt;
	pjmt->psoSetIgnoreJt = pjmtBase->psoSetIgnoreJt;
}

void DeleteJmt(JMT* pjmt)
{
	delete pjmt;
}
